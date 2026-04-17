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

void UnitModel::clearClipIndex()
{
	const size_t clearedIndex = m_animations.size();
	if (m_currentClipIndex != clearedIndex)
	{
		m_currentClipIndex = clearedIndex;
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

String UnitModel::diagnosticReport() const
{
	String report;
	const auto appendLine = [&report](const String& line)
		{
			report += line;
			report += U'\n';
		};

	appendLine(U"[UnitModel Diagnostics]");
	appendLine(U"loaded: {}"_fmt(m_loaded ? U"true" : U"false"));
	appendLine(U"nodes: {} bones: {} animations: {} subMeshes: {}"_fmt(m_nodes.size(), m_bones.size(), m_animations.size(), m_subMeshes.size()));
	appendLine(U"vertices: {} indices: {} staticVertices: {}"_fmt(m_bindPoseVertices.size(), m_indices.size(), m_mesh.num_vertices(), m_bindPoseVertices.size()));

	size_t nodeBoundVertexCount = 0;
	size_t skinnedVertexCount = 0;
	size_t totalInfluenceCount = 0;
	size_t maxInfluenceCount = 0;
	size_t nonUnitWeightVertexCount = 0;
	double totalDeclaredBoneWeight = 0.0;

	for (const auto& skinningData : m_vertexSkinning)
	{
		if (0 <= skinningData.nodeIndex)
		{
			++nodeBoundVertexCount;
		}

		if (skinningData.boneInfluences.isEmpty())
		{
			continue;
		}

		++skinnedVertexCount;
		totalInfluenceCount += skinningData.boneInfluences.size();
		maxInfluenceCount = Max(maxInfluenceCount, skinningData.boneInfluences.size());
		totalDeclaredBoneWeight += skinningData.totalBoneWeight;

		if (0.05 < Abs((skinningData.totalBoneWeight - 1.0f)))
		{
			++nonUnitWeightVertexCount;
		}
	}

	appendLine(U"nodeBoundVertices: {} skinnedVertices: {} avgInfluences: {:.2f} maxInfluences: {} nonUnitWeightVertices: {}"_fmt(
		nodeBoundVertexCount,
		skinnedVertexCount,
		(skinnedVertexCount == 0) ? 0.0 : (static_cast<double>(totalInfluenceCount) / skinnedVertexCount),
		maxInfluenceCount,
		nonUnitWeightVertexCount));
	appendLine(U"avgDeclaredBoneWeight: {:.4f}"_fmt((skinnedVertexCount == 0) ? 0.0 : (totalDeclaredBoneWeight / skinnedVertexCount)));

	if (m_bindPoseVertices.isEmpty() || m_vertexSkinning.isEmpty() || m_bones.isEmpty())
	{
		appendLine(U"bindPoseError(currentOrder): n/a");
		appendLine(U"bindPoseError(swappedOrder): n/a");
		return report;
	}

	Array<Mat4x4> bindLocalTransforms(m_nodes.size(), Mat4x4::Identity());
	for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
	{
		bindLocalTransforms[nodeIndex] = m_nodes[nodeIndex].localTransform;
	}

	const Array<Mat4x4> bindWorldTransforms = BuildWorldTransforms(m_nodes, bindLocalTransforms);
	const auto appendBindPoseMetrics = [this, &appendLine, &bindWorldTransforms](const StringView label, const bool worldTimesOffset)
		{
			double totalLocalError = 0.0;
			double maxLocalError = 0.0;
			double totalNodeWorldError = 0.0;
			double maxNodeWorldError = 0.0;
			size_t evaluatedVertexCount = 0;

			for (size_t vertexIndex = 0; vertexIndex < Min(m_bindPoseVertices.size(), m_vertexSkinning.size()); ++vertexIndex)
			{
				const VertexSkinningData& skinningData = m_vertexSkinning[vertexIndex];

				if (skinningData.boneInfluences.isEmpty())
				{
					continue;
				}

				const Vertex3D& source = m_bindPoseVertices[vertexIndex];
				Float3 skinnedPosition{};
				float accumulatedWeight = 0.0f;

				for (const auto& influence : skinningData.boneInfluences)
				{
					if ((influence.boneIndex < 0) || (m_bones.size() <= static_cast<size_t>(influence.boneIndex)))
					{
						continue;
					}

					const UnitModelBone& bone = m_bones[influence.boneIndex];
					if ((bone.nodeIndex < 0) || (bindWorldTransforms.size() <= static_cast<size_t>(bone.nodeIndex)))
					{
						continue;
					}

					const Mat4x4 boneTransform = worldTimesOffset
						? (bindWorldTransforms[bone.nodeIndex] * bone.offsetMatrix)
						: (bone.offsetMatrix * bindWorldTransforms[bone.nodeIndex]);
					skinnedPosition += (boneTransform.transformPoint(source.pos) * influence.weight);
					accumulatedWeight += influence.weight;
				}

				if (accumulatedWeight <= 0.0f)
				{
					continue;
				}

				skinnedPosition *= (1.0f / accumulatedWeight);
				const double localError = (skinnedPosition - source.pos).length();
				Float3 nodeWorldPosition = source.pos;
				if ((0 <= skinningData.nodeIndex) && (bindWorldTransforms.size() > static_cast<size_t>(skinningData.nodeIndex)))
				{
					nodeWorldPosition = bindWorldTransforms[skinningData.nodeIndex].transformPoint(source.pos);
				}
				const double nodeWorldError = (skinnedPosition - nodeWorldPosition).length();

				totalLocalError += localError;
				totalNodeWorldError += nodeWorldError;
				maxLocalError = Max(maxLocalError, localError);
				maxNodeWorldError = Max(maxNodeWorldError, nodeWorldError);
				++evaluatedVertexCount;
			}

			appendLine(U"{}: vertices={} avgLocal={:.4f} maxLocal={:.4f} avgNodeWorld={:.4f} maxNodeWorld={:.4f}"_fmt(
				label,
				evaluatedVertexCount,
				(evaluatedVertexCount == 0) ? 0.0 : (totalLocalError / evaluatedVertexCount),
				maxLocalError,
				(evaluatedVertexCount == 0) ? 0.0 : (totalNodeWorldError / evaluatedVertexCount),
				maxNodeWorldError));
		};

	appendBindPoseMetrics(U"bindPoseError(currentOrder)", true);
	appendBindPoseMetrics(U"bindPoseError(swappedOrder)", false);

	if (m_animations.isEmpty())
	{
		appendLine(U"animation: none");
	}
	else if (m_currentClipIndex < m_animations.size())
	{
		const UnitModelAnimationClip& clip = m_animations[m_currentClipIndex];
		appendLine(U"animation: clip={} duration={:.4f} ticksPerSecond={:.4f} currentTime={:.4f}"_fmt(
			clip.name,
			clip.duration,
			clip.ticksPerSecond,
			m_animationTime));
	}

	return report;
}
