	void CollectMeshNodeIndices(const aiNode* node, const HashTable<String, int32>& nodeIndicesByName, HashTable<uint32, int32>& meshNodeIndices)
	{
		if (node == nullptr)
		{
			return;
		}

		const String nodeName = ToSivString(node->mName);
		const int32 nodeIndex = nodeIndicesByName.contains(nodeName) ? nodeIndicesByName.at(nodeName) : -1;

		for (uint32 meshListIndex = 0; meshListIndex < node->mNumMeshes; ++meshListIndex)
		{
			meshNodeIndices.emplace(node->mMeshes[meshListIndex], nodeIndex);
		}

		for (uint32 childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
		{
			CollectMeshNodeIndices(node->mChildren[childIndex], nodeIndicesByName, meshNodeIndices);
		}
	}

	void FlattenNodes(const aiNode* node, const int32 parentIndex,
		Array<BirdModelNode>& nodes,
		HashTable<String, int32>& nodeIndicesByName)
	{
		if (node == nullptr)
		{
			return;
		}

		const int32 nodeIndex = static_cast<int32>(nodes.size());
		BirdModelNode& importedNode = nodes.emplace_back();
		importedNode.name = ToSivString(node->mName);
		importedNode.parentIndex = parentIndex;
		importedNode.localTransform = ToMat4x4(node->mTransformation);
		nodeIndicesByName.emplace(importedNode.name, nodeIndex);

		for (uint32 childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
		{
			FlattenNodes(node->mChildren[childIndex], nodeIndex, nodes, nodeIndicesByName);
		}
	}

	[[nodiscard]] ImportedAnimatedData LoadAnimatedData(FilePathView path)
	{
		if (not FileSystem::Exists(path))
		{
			return{};
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Unicode::ToUTF8(FileSystem::FullPath(path)),
			(aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_GenSmoothNormals
			| aiProcess_SortByPType
			| aiProcess_ConvertToLeftHanded
			| aiProcess_FlipUVs));

		if (scene == nullptr)
		{
			return{};
		}

		ImportedAnimatedData result;
		HashTable<String, int32> nodeIndicesByName;

		if (scene->mRootNode != nullptr)
		{
			FlattenNodes(scene->mRootNode, -1, result.nodes, nodeIndicesByName);
		}

		HashTable<uint32, int32> meshNodeIndices;

		if (scene->mRootNode != nullptr)
		{
			CollectMeshNodeIndices(scene->mRootNode, nodeIndicesByName, meshNodeIndices);
		}

		size_t totalVertices = 0;
		size_t totalTriangles = 0;

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if (mesh == nullptr)
			{
				continue;
			}

			totalVertices += mesh->mNumVertices;
			totalTriangles += mesh->mNumFaces;
		}

		result.vertices.reserve(totalVertices);
		result.indices.reserve(totalTriangles);
		result.vertexNodeIndices.reserve(totalVertices);

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if ((mesh == nullptr) || (mesh->mNumVertices == 0))
			{
				continue;
			}

			const uint32 vertexOffset = static_cast<uint32>(result.vertices.size());
			const int32 nodeIndex = meshNodeIndices.contains(meshIndex) ? meshNodeIndices.at(meshIndex) : -1;

			BirdModelSubMeshInfo subMeshInfo;
			subMeshInfo.globalVertexStart = vertexOffset;
			subMeshInfo.vertexCount = mesh->mNumVertices;

			if ((mesh->mMaterialIndex < scene->mNumMaterials) && (scene->mMaterials != nullptr))
			{
				const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				aiColor4D baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };

				if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) != aiReturn_SUCCESS)
				{
					material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
				}

				subMeshInfo.materialColor = ColorF{ baseColor.r, baseColor.g, baseColor.b, baseColor.a };
			}

			for (uint32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
			{
				const aiVector3D& pos = mesh->mVertices[vertexIndex];
				const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D{ 0.0f, 1.0f, 0.0f };
				const aiVector3D tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D{};

				result.vertices << Vertex3D{
					.pos = Float3{ pos.x, pos.y, pos.z },
					.normal = Float3{ normal.x, normal.y, normal.z },
					.tex = Float2{ tex.x, tex.y },
				};
				result.vertexNodeIndices << nodeIndex;
			}

			for (uint32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
			{
				const aiFace& face = mesh->mFaces[faceIndex];

				if (face.mNumIndices != 3)
				{
					continue;
				}

				result.indices << TriangleIndex32{
					.i0 = (vertexOffset + face.mIndices[0]),
					.i1 = (vertexOffset + face.mIndices[1]),
					.i2 = (vertexOffset + face.mIndices[2]),
				};

				subMeshInfo.localIndices << TriangleIndex32{
					.i0 = face.mIndices[0],
					.i1 = face.mIndices[1],
					.i2 = face.mIndices[2],
				};
			}

			result.subMeshes << std::move(subMeshInfo);
		}

		HashTable<String, size_t> boneIndicesByName;
		uint32 vertexOffset = 0;

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if (mesh == nullptr)
			{
				continue;
			}

			for (uint32 boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
			{
				const aiBone* bone = mesh->mBones[boneIndex];

				if (bone == nullptr)
				{
					continue;
				}

				const String boneName = ToSivString(bone->mName);
				size_t importedBoneIndex = 0;

				if (const auto it = boneIndicesByName.find(boneName); it != boneIndicesByName.end())
				{
					importedBoneIndex = it->second;
				}
				else
				{
					importedBoneIndex = result.bones.size();
					BirdModelBone& importedBone = result.bones.emplace_back();
					importedBone.name = boneName;
					importedBone.offsetMatrix = ToMat4x4(bone->mOffsetMatrix);
					if (const auto nodeIt = nodeIndicesByName.find(boneName); nodeIt != nodeIndicesByName.end())
					{
						importedBone.nodeIndex = nodeIt->second;
					}
					boneIndicesByName.emplace(boneName, importedBoneIndex);
				}

				BirdModelBone& importedBone = result.bones[importedBoneIndex];
				importedBone.weights.reserve(importedBone.weights.size() + bone->mNumWeights);

				for (uint32 weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
				{
					const aiVertexWeight& weight = bone->mWeights[weightIndex];
					importedBone.weights << BirdModelBoneWeight{
						.vertexIndex = (vertexOffset + weight.mVertexId),
						.weight = weight.mWeight,
					};
				}
			}

			vertexOffset += mesh->mNumVertices;
		}

		for (uint32 animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex)
		{
			const aiAnimation* animation = scene->mAnimations[animationIndex];

			if (animation == nullptr)
			{
				continue;
			}

			BirdModelAnimationClip& importedAnimation = result.animations.emplace_back();
			importedAnimation.name = ToSivString(animation->mName);
			if (importedAnimation.name.isEmpty())
			{
				importedAnimation.name = U"Animation{}"_fmt(animationIndex);
			}
			importedAnimation.duration = animation->mDuration;
			importedAnimation.ticksPerSecond = animation->mTicksPerSecond;

			for (uint32 channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex)
			{
				const aiNodeAnim* channel = animation->mChannels[channelIndex];

				if (channel == nullptr)
				{
					continue;
				}

				BirdModelAnimationChannel& importedChannel = importedAnimation.channels.emplace_back();
				importedChannel.nodeName = ToSivString(channel->mNodeName);
				importedChannel.positionKeys.reserve(channel->mNumPositionKeys);
				importedChannel.rotationKeys.reserve(channel->mNumRotationKeys);
				importedChannel.scalingKeys.reserve(channel->mNumScalingKeys);

				for (uint32 keyIndex = 0; keyIndex < channel->mNumPositionKeys; ++keyIndex)
				{
					const aiVectorKey& key = channel->mPositionKeys[keyIndex];
					importedChannel.positionKeys << BirdModelAnimationKey<Float3>{
						.time = key.mTime,
						.value = ToFloat3(key.mValue),
					};
				}

				for (uint32 keyIndex = 0; keyIndex < channel->mNumRotationKeys; ++keyIndex)
				{
					const aiQuatKey& key = channel->mRotationKeys[keyIndex];
					importedChannel.rotationKeys << BirdModelAnimationKey<Quaternion>{
						.time = key.mTime,
						.value = ToQuaternion(key.mValue),
					};
				}

				for (uint32 keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
				{
					const aiVectorKey& key = channel->mScalingKeys[keyIndex];
					importedChannel.scalingKeys << BirdModelAnimationKey<Float3>{
						.time = key.mTime,
						.value = ToFloat3(key.mValue),
					};
				}
			}
		}

		return result;
	}
