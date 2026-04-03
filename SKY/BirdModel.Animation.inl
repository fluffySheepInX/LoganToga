void BirdModel::evaluateAnimationPose()
{
	if (m_currentLocalTransforms.size() != m_nodes.size())
	{
		m_currentLocalTransforms.resize(m_nodes.size());
	}

	for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
	{
		m_currentLocalTransforms[nodeIndex] = m_nodes[nodeIndex].localTransform;
	}

	if (m_animations.isEmpty() || m_currentClipIndex >= m_animations.size())
	{
		applyProceduralWingFlap();
		refreshWorldTransforms();
		return;
	}

	const BirdModelAnimationClip& clip = m_animations[m_currentClipIndex];
	const double ticksPerSecond = (0.0 < clip.ticksPerSecond) ? clip.ticksPerSecond : 25.0;
	const double clipTime = (0.0 < clip.duration)
		? (Math::Fraction((m_animationTime * ticksPerSecond) / clip.duration) * clip.duration)
		: 0.0;
	HashTable<String, const BirdModelAnimationChannel*> channelsByNodeName;
	channelsByNodeName.reserve(clip.channels.size());

	for (const auto& channel : clip.channels)
	{
		channelsByNodeName.emplace(channel.nodeName, &channel);
	}

	for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
	{
		NodeTransformParts transform = DecomposeTransform(m_nodes[nodeIndex].localTransform);

		if (const auto channelIt = channelsByNodeName.find(m_nodes[nodeIndex].name); channelIt != channelsByNodeName.end())
		{
			const BirdModelAnimationChannel& channel = *channelIt->second;
			transform.translation = SampleVectorKey(channel.positionKeys, clipTime, transform.translation);
			transform.rotation = SampleQuaternionKey(channel.rotationKeys, clipTime, transform.rotation);
			transform.scale = SampleVectorKey(channel.scalingKeys, clipTime, transform.scale);
		}

		m_currentLocalTransforms[nodeIndex] = ComposeTransform(transform);
	}

	refreshWorldTransforms();
}

void BirdModel::applyProceduralWingFlap()
{
	constexpr double FlapFrequency = 3.0;
	const double flapPhase = (m_animationTime * Math::TwoPi * FlapFrequency);

	for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
	{
		const String& name = m_nodes[nodeIndex].name;
		double amplitudeDeg = 0.0;
		double sign = 0.0;
		double phaseOffset = 0.0;

		if (name == U"shoulder.L")      { amplitudeDeg = 40.0; sign = -1.0; phaseOffset = 0.0; }
		else if (name == U"arm.L")      { amplitudeDeg = 18.0; sign = -1.0; phaseOffset = 0.4; }
		else if (name == U"arm2.L")     { amplitudeDeg = 10.0; sign = -1.0; phaseOffset = 0.7; }
		else if (name == U"hand.L")     { amplitudeDeg =  6.0; sign = -1.0; phaseOffset = 1.0; }
		else if (name == U"shoulder.R") { amplitudeDeg = 40.0; sign =  1.0; phaseOffset = 0.0; }
		else if (name == U"arm.R")      { amplitudeDeg = 18.0; sign =  1.0; phaseOffset = 0.4; }
		else if (name == U"arm2.R")     { amplitudeDeg = 10.0; sign =  1.0; phaseOffset = 0.7; }
		else if (name == U"hand.R")     { amplitudeDeg =  6.0; sign =  1.0; phaseOffset = 1.0; }

		if (amplitudeDeg == 0.0)
		{
			continue;
		}

		const float radians = static_cast<float>(Math::ToRadians(amplitudeDeg) * Math::Sin(flapPhase + phaseOffset) * sign);
		NodeTransformParts parts = DecomposeTransform(m_currentLocalTransforms[nodeIndex]);
		parts.rotation = parts.rotation * Quaternion::RotateZ(radians);
		m_currentLocalTransforms[nodeIndex] = ComposeTransform(parts);
	}
}

void BirdModel::refreshWorldTransforms()
{
	if (m_currentWorldTransforms.size() != m_nodes.size())
	{
		m_currentWorldTransforms.resize(m_nodes.size());
	}

	for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
	{
		const int32 parentIndex = m_nodes[nodeIndex].parentIndex;

		if (parentIndex < 0)
		{
			m_currentWorldTransforms[nodeIndex] = m_currentLocalTransforms[nodeIndex];
		}
		else
		{
			m_currentWorldTransforms[nodeIndex] = (m_currentLocalTransforms[nodeIndex] * m_currentWorldTransforms[parentIndex]);
		}
	}

	refreshBoneTransforms();
}

void BirdModel::refreshBoneTransforms()
{
	if (m_currentBoneTransforms.size() != m_bones.size())
	{
		m_currentBoneTransforms.resize(m_bones.size());
	}

	for (size_t boneIndex = 0; boneIndex < m_bones.size(); ++boneIndex)
	{
		const BirdModelBone& bone = m_bones[boneIndex];

		if ((bone.nodeIndex < 0) || (m_currentWorldTransforms.size() <= static_cast<size_t>(bone.nodeIndex)))
		{
			m_currentBoneTransforms[boneIndex] = Mat4x4::Identity();
			continue;
		}

		m_currentBoneTransforms[boneIndex] = (bone.offsetMatrix * m_currentWorldTransforms[bone.nodeIndex]);
	}

	refreshSkinnedMesh();
}

void BirdModel::refreshSkinnedMesh()
{
	if (m_bindPoseVertices.isEmpty() || m_indices.isEmpty())
	{
		return;
	}

	if (m_deformedVertices.size() != m_bindPoseVertices.size())
	{
		m_deformedVertices = m_bindPoseVertices;
	}

	for (size_t vertexIndex = 0; vertexIndex < m_bindPoseVertices.size(); ++vertexIndex)
	{
		const Vertex3D& source = m_bindPoseVertices[vertexIndex];
		const VertexSkinningData& skinningData = m_vertexSkinning[vertexIndex];
		Float3 position{};
		Float3 normal{};
		float accumulatedWeight = 0.0f;

		for (const auto& influence : skinningData.boneInfluences)
		{
			if ((influence.boneIndex < 0) || (m_currentBoneTransforms.size() <= static_cast<size_t>(influence.boneIndex)))
			{
				continue;
			}

			const Mat4x4& boneTransform = m_currentBoneTransforms[influence.boneIndex];
			position += (boneTransform.transformPoint(source.pos) * influence.weight);
			normal += (TransformNormalByPointPair(boneTransform, source.pos, source.normal) * influence.weight);
			accumulatedWeight += influence.weight;
		}

		const float fallbackWeight = Clamp((1.0f - accumulatedWeight), 0.0f, 1.0f);

		if ((0.0f < fallbackWeight)
			&& (0 <= skinningData.nodeIndex)
			&& (m_currentWorldTransforms.size() > static_cast<size_t>(skinningData.nodeIndex)))
		{
			const Mat4x4& nodeTransform = m_currentWorldTransforms[skinningData.nodeIndex];
			position += (nodeTransform.transformPoint(source.pos) * fallbackWeight);
			normal += (TransformNormalByPointPair(nodeTransform, source.pos, source.normal) * fallbackWeight);
		}
		else if (accumulatedWeight == 0.0f)
		{
			position = source.pos;
			normal = source.normal;
		}

		Vertex3D& destination = m_deformedVertices[vertexIndex];
		destination = source;
		destination.pos = m_normalizationTransform.transformPoint(position);
		destination.normal = normal.isZero() ? source.normal : normal.normalized();
	}

	if (not m_subMeshes.isEmpty())
	{
		for (auto& subMesh : m_subMeshes)
		{
			const Array<Vertex3D> subVertices(m_deformedVertices.begin() + subMesh.globalVertexStart,
				m_deformedVertices.begin() + subMesh.globalVertexStart + subMesh.vertexCount);
			subMesh.dynamicMesh.fill(subVertices);
		}
	}
	else
	{
		m_dynamicMesh.fill(m_deformedVertices);
	}
}
