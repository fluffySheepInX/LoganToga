UnitModel::UnitModel(FilePathView path, const double displayHeight, const UnitModelProceduralAnimationType proceduralAnimationType)
{
    const UnitModelAsset imported = LoadUnitModelAsset(path, displayHeight);
 m_proceduralAnimationType = proceduralAnimationType;
 const auto initializeImportedAsset = [this, &imported]()
	{
		m_mesh = imported.mesh;
		m_loaded = imported.loaded;
		m_bindPoseVertices = imported.vertices;
		m_deformedVertices = imported.vertices;
		m_indices = imported.indices;
		m_nodes = imported.nodes;
		m_bones = imported.bones;
		m_animations = imported.animations;
		m_normalizationTransform = imported.normalizationTransform;
		m_vertexSkinning.resize(m_bindPoseVertices.size());
	};
	const auto initializeVertexSkinning = [this, &imported]()
	{
		bool hasBoneInfluences = false;

		for (size_t vertexIndex = 0; vertexIndex < Min(m_vertexSkinning.size(), imported.vertexNodeIndices.size()); ++vertexIndex)
		{
			m_vertexSkinning[vertexIndex].nodeIndex = imported.vertexNodeIndices[vertexIndex];
		}

		for (size_t boneIndex = 0; boneIndex < m_bones.size(); ++boneIndex)
		{
			for (const auto& weight : m_bones[boneIndex].weights)
			{
				if (m_vertexSkinning.size() <= weight.vertexIndex)
				{
					continue;
				}

				VertexSkinningData& skinningData = m_vertexSkinning[weight.vertexIndex];
				skinningData.totalBoneWeight += weight.weight;
				skinningData.boneInfluences << VertexBoneInfluence{
					.boneIndex = static_cast<int32>(boneIndex),
					.weight = weight.weight,
				};
				hasBoneInfluences = true;
			}
		}

		return hasBoneInfluences;
	};
	const auto applyStaticMeshFallback = [this, &imported](const bool hasBoneInfluences)
	{
		if ((not hasBoneInfluences)
			&& m_animations.isEmpty()
			&& (not imported.staticVertices.isEmpty())
			&& (not imported.staticIndices.isEmpty()))
		{
			m_bindPoseVertices = imported.staticVertices;
			m_deformedVertices = imported.staticVertices;
			m_indices = imported.staticIndices;
			m_normalizationTransform = Mat4x4::Identity();

			for (auto& skinningData : m_vertexSkinning)
			{
				skinningData.nodeIndex = -1;
			}
		}
	};
	const auto initializeRenderMeshes = [this, &imported]()
	{
		if (not m_deformedVertices.isEmpty() && not m_indices.isEmpty())
		{
			for (const auto& subMeshInfo : imported.subMeshes)
			{
				if (subMeshInfo.vertexCount == 0 || subMeshInfo.localIndices.isEmpty())
				{
					continue;
				}

				SubMeshRenderData& rd = m_subMeshes.emplace_back();
				rd.globalVertexStart = subMeshInfo.globalVertexStart;
				rd.vertexCount = subMeshInfo.vertexCount;
				rd.materialColor = subMeshInfo.materialColor;
				rd.diffuseTexture = subMeshInfo.diffuseTexture;

				const Array<Vertex3D> subVertices(m_bindPoseVertices.begin() + rd.globalVertexStart,
					m_bindPoseVertices.begin() + rd.globalVertexStart + rd.vertexCount);
				rd.dynamicMesh = DynamicMesh{ MeshData{ subVertices, subMeshInfo.localIndices } };
			}

			if (m_subMeshes.isEmpty())
			{
				m_dynamicMesh = DynamicMesh{ MeshData{ m_deformedVertices, m_indices } };
			}
		}
	};
	const auto initializeTransformBuffers = [this]()
	{
		m_currentLocalTransforms.resize(m_nodes.size());
		m_currentWorldTransforms.resize(m_nodes.size());
		m_currentBoneTransforms.resize(m_bones.size());
	};

	initializeImportedAsset();
	const bool hasBoneInfluences = initializeVertexSkinning();
	applyStaticMeshFallback(hasBoneInfluences);
	initializeRenderMeshes();
	initializeTransformBuffers();
	evaluateAnimationPose();
}

void UnitModel::update(const double deltaTime)
{
	m_animationTime += deltaTime;
	evaluateAnimationPose();
}

void UnitModel::draw(const Vec3& position, const double yaw, const ColorF& color, const double scale) const
{
	if (not m_loaded)
	{
		return;
	}

	const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
	const Mat4x4 modelTransform = Mat4x4::Identity()
		.scaled(scale)
		.rotated(Quaternion::RotateY(yaw))
		.translated(position);

	if (not m_subMeshes.isEmpty())
	{
		for (const auto& subMesh : m_subMeshes)
		{
			const ColorF meshColor{
				color.r * subMesh.materialColor.r,
				color.g * subMesh.materialColor.g,
				color.b * subMesh.materialColor.b,
				color.a * subMesh.materialColor.a,
			};

			if (subMesh.diffuseTexture)
			{
                subMesh.dynamicMesh.draw(modelTransform, subMesh.diffuseTexture, meshColor);
			}
			else
			{
                subMesh.dynamicMesh.draw(modelTransform, meshColor);
			}
		}
		return;
	}

	if (not m_deformedVertices.isEmpty() && not m_indices.isEmpty())
	{
      m_dynamicMesh.draw(modelTransform, color);
		return;
	}

 m_mesh.draw(modelTransform, color);
}

Optional<Vec3> UnitModel::groundContactPoint(const Vec3& position, const double yaw, const double scale) const
{
	const Array<Vertex3D>* vertices = nullptr;

	if (not m_deformedVertices.isEmpty())
	{
		vertices = &m_deformedVertices;
	}
	else if (not m_bindPoseVertices.isEmpty())
	{
		vertices = &m_bindPoseVertices;
	}

	if (vertices == nullptr || vertices->isEmpty())
	{
		return none;
	}

	double minY = Math::Inf;

	for (const auto& vertex : *vertices)
	{
		minY = Min(minY, static_cast<double>(vertex.pos.y));
	}

	Vec3 localGroundPoint{ 0, minY, 0 };
	size_t groundVertexCount = 0;
	constexpr double GroundPointTolerance = 0.03;

	for (const auto& vertex : *vertices)
	{
		if (Abs(static_cast<double>(vertex.pos.y) - minY) <= GroundPointTolerance)
		{
			localGroundPoint += Vec3{ vertex.pos.x, 0.0, vertex.pos.z };
			++groundVertexCount;
		}
	}

	if (0 < groundVertexCount)
	{
		localGroundPoint.x /= static_cast<double>(groundVertexCount);
		localGroundPoint.z /= static_cast<double>(groundVertexCount);
	}

  return (position + (Quaternion::RotateY(yaw) * (localGroundPoint * scale)));
}

bool UnitModel::isLoaded() const noexcept
{
	return m_loaded;
}

bool UnitModel::hasAnimations() const noexcept
{
	return (not m_animations.isEmpty());
}

double UnitModel::animationTime() const noexcept
{
	return m_animationTime;
}

size_t UnitModel::currentClipIndex() const noexcept
{
	return m_currentClipIndex;
}

void UnitModel::setClipIndex(const size_t index)
{
	if (index < m_animations.size() && index != m_currentClipIndex)
	{
		m_currentClipIndex = index;
		m_animationTime = 0.0;
	}
}

const Array<UnitModelNode>& UnitModel::nodes() const noexcept
{
	return m_nodes;
}

const Array<UnitModelBone>& UnitModel::bones() const noexcept
{
	return m_bones;
}

const Array<UnitModelAnimationClip>& UnitModel::animations() const noexcept
{
	return m_animations;
}

const Array<Mat4x4>& UnitModel::currentLocalTransforms() const noexcept
{
	return m_currentLocalTransforms;
}

const Array<Mat4x4>& UnitModel::currentWorldTransforms() const noexcept
{
	return m_currentWorldTransforms;
}

const Array<Mat4x4>& UnitModel::currentBoneTransforms() const noexcept
{
	return m_currentBoneTransforms;
}
