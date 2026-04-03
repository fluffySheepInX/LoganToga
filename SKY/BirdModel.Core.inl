BirdModel::BirdModel(FilePathView path, const double displayHeight)
{
	const BirdModelAsset imported = LoadBirdModelAsset(path, displayHeight);
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
		}
	}

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

			const Array<Vertex3D> subVertices(m_bindPoseVertices.begin() + rd.globalVertexStart,
				m_bindPoseVertices.begin() + rd.globalVertexStart + rd.vertexCount);
			rd.dynamicMesh = DynamicMesh{ MeshData{ subVertices, subMeshInfo.localIndices } };
		}

		if (m_subMeshes.isEmpty())
		{
			m_dynamicMesh = DynamicMesh{ MeshData{ m_deformedVertices, m_indices } };
		}
	}

	m_currentLocalTransforms.resize(m_nodes.size());
	m_currentWorldTransforms.resize(m_nodes.size());
	m_currentBoneTransforms.resize(m_bones.size());
	evaluateAnimationPose();
}

void BirdModel::update(const double deltaTime)
{
	m_animationTime += deltaTime;
	evaluateAnimationPose();
}

void BirdModel::draw(const Vec3& position, const double yaw, const ColorF& color) const
{
	if (not m_loaded)
	{
		return;
	}

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
			subMesh.dynamicMesh.draw(position, Quaternion::RotateY(yaw), meshColor);
		}
		return;
	}

	if (not m_deformedVertices.isEmpty() && not m_indices.isEmpty())
	{
		m_dynamicMesh.draw(position, Quaternion::RotateY(yaw), color);
		return;
	}

	m_mesh.draw(position, Quaternion::RotateY(yaw), color);
}

Optional<Vec3> BirdModel::groundContactPoint(const Vec3& position, const double yaw) const
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

	return (position + (Quaternion::RotateY(yaw) * localGroundPoint));
}

bool BirdModel::isLoaded() const noexcept
{
	return m_loaded;
}

bool BirdModel::hasAnimations() const noexcept
{
	return (not m_animations.isEmpty());
}

double BirdModel::animationTime() const noexcept
{
	return m_animationTime;
}

size_t BirdModel::currentClipIndex() const noexcept
{
	return m_currentClipIndex;
}

void BirdModel::setClipIndex(const size_t index)
{
	if (index < m_animations.size() && index != m_currentClipIndex)
	{
		m_currentClipIndex = index;
		m_animationTime = 0.0;
	}
}

const Array<BirdModelNode>& BirdModel::nodes() const noexcept
{
	return m_nodes;
}

const Array<BirdModelBone>& BirdModel::bones() const noexcept
{
	return m_bones;
}

const Array<BirdModelAnimationClip>& BirdModel::animations() const noexcept
{
	return m_animations;
}

const Array<Mat4x4>& BirdModel::currentLocalTransforms() const noexcept
{
	return m_currentLocalTransforms;
}

const Array<Mat4x4>& BirdModel::currentWorldTransforms() const noexcept
{
	return m_currentWorldTransforms;
}

const Array<Mat4x4>& BirdModel::currentBoneTransforms() const noexcept
{
	return m_currentBoneTransforms;
}
