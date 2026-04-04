	[[nodiscard]] Mat4x4 ComputeNormalizationTransform(const ImportedAnimatedData& animatedData, const double displayHeight)
	{
		if (animatedData.vertices.isEmpty())
		{
			return Mat4x4::Identity();
		}

		Array<Mat4x4> nodeWorldTransforms(animatedData.nodes.size(), Mat4x4::Identity());

		for (size_t nodeIndex = 0; nodeIndex < animatedData.nodes.size(); ++nodeIndex)
		{
			const BirdModelNode& node = animatedData.nodes[nodeIndex];

			if ((node.parentIndex < 0) || (animatedData.nodes.size() <= static_cast<size_t>(node.parentIndex)))
			{
				nodeWorldTransforms[nodeIndex] = node.localTransform;
			}
			else
			{
				nodeWorldTransforms[nodeIndex] = (node.localTransform * nodeWorldTransforms[node.parentIndex]);
			}
		}

		Float3 minPoint = animatedData.vertices.front().pos;
		Float3 maxPoint = animatedData.vertices.front().pos;

		for (size_t vertexIndex = 0; vertexIndex < animatedData.vertices.size(); ++vertexIndex)
		{
			Float3 worldPosition = animatedData.vertices[vertexIndex].pos;

			if ((vertexIndex < animatedData.vertexNodeIndices.size())
				&& (0 <= animatedData.vertexNodeIndices[vertexIndex])
				&& (animatedData.nodes.size() > static_cast<size_t>(animatedData.vertexNodeIndices[vertexIndex])))
			{
				worldPosition = nodeWorldTransforms[animatedData.vertexNodeIndices[vertexIndex]].transformPoint(worldPosition);
			}

			minPoint.x = Min(minPoint.x, worldPosition.x);
			minPoint.y = Min(minPoint.y, worldPosition.y);
			minPoint.z = Min(minPoint.z, worldPosition.z);
			maxPoint.x = Max(maxPoint.x, worldPosition.x);
			maxPoint.y = Max(maxPoint.y, worldPosition.y);
			maxPoint.z = Max(maxPoint.z, worldPosition.z);
		}

		const Float3 boundsCenter = ((minPoint + maxPoint) * 0.5f);
		const double boundsHeight = (maxPoint.y - minPoint.y);
		const double scale = (0.001 < boundsHeight)
			? (displayHeight / boundsHeight)
			: 1.0;

		return Mat4x4::Identity()
			.translated(-boundsCenter.x, -minPoint.y, -boundsCenter.z)
			.scaled(scale);
	}

	[[nodiscard]] ImportedStaticMesh LoadStaticGlbMesh(FilePathView path, const double displayHeight)
	{
		if (not FileSystem::Exists(path))
		{
			return{};
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Unicode::ToUTF8(FileSystem::FullPath(path)),
			(aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_PreTransformVertices
			| aiProcess_GenSmoothNormals
			| aiProcess_SortByPType
			| aiProcess_ConvertToLeftHanded
			| aiProcess_FlipUVs));

		if ((scene == nullptr) || (not scene->HasMeshes()))
		{
			return{};
		}

		Array<Vertex3D> vertices;
		Array<TriangleIndex32> indices;
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

		vertices.reserve(totalVertices);
		indices.reserve(totalTriangles);

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if ((mesh == nullptr) || (mesh->mNumVertices == 0))
			{
				continue;
			}

			const uint32 vertexOffset = static_cast<uint32>(vertices.size());

			for (uint32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
			{
				const aiVector3D& pos = mesh->mVertices[vertexIndex];
				const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D{ 0.0f, 1.0f, 0.0f };
				const aiVector3D tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D{};

				vertices << Vertex3D{
					.pos = Float3{ pos.x, pos.y, pos.z },
					.normal = Float3{ normal.x, normal.y, normal.z },
					.tex = Float2{ tex.x, tex.y },
				};
			}

			for (uint32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
			{
				const aiFace& face = mesh->mFaces[faceIndex];

				if (face.mNumIndices != 3)
				{
					continue;
				}

				indices << TriangleIndex32{
					.i0 = (vertexOffset + face.mIndices[0]),
					.i1 = (vertexOffset + face.mIndices[1]),
					.i2 = (vertexOffset + face.mIndices[2]),
				};
			}
		}

		if (vertices.isEmpty() || indices.isEmpty())
		{
			return{};
		}

       MeshData meshData{ vertices, indices };
		const Box bounds = meshData.computeBoundingBox();
		const double scale = (0.001 < bounds.h)
			? (displayHeight / bounds.h)
			: 1.0;
		const double bottomY = (bounds.center.y - (bounds.h * 0.5));
		const Mat4x4 normalizationTransform = Mat4x4::Identity()
			.translated(-bounds.center.x, -bottomY, -bounds.center.z)
			.scaled(scale);

		for (auto& vertex : vertices)
		{
			vertex.pos.x -= static_cast<float>(bounds.center.x);
			vertex.pos.y -= static_cast<float>(bottomY);
			vertex.pos.z -= static_cast<float>(bounds.center.z);
			vertex.pos *= static_cast<float>(scale);
		}

		meshData = MeshData{ vertices, indices };

		return{
			.mesh = Mesh{ meshData },
			.loaded = true,
           .vertices = std::move(vertices),
			.indices = std::move(indices),
			.normalizationTransform = normalizationTransform,
		};
	}
