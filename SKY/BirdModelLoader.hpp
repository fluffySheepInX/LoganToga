# pragma once
# include <Siv3D.hpp>
# include "BirdModelTypes.hpp"

struct BirdModelAsset
{
	Mesh mesh;
	bool loaded = false;
   Array<Vertex3D> staticVertices;
	Array<TriangleIndex32> staticIndices;
	Array<Vertex3D> vertices;
	Array<TriangleIndex32> indices;
	Array<int32> vertexNodeIndices;
	Mat4x4 normalizationTransform = Mat4x4::Identity();
	Array<BirdModelNode> nodes;
	Array<BirdModelBone> bones;
	Array<BirdModelAnimationClip> animations;
	Array<BirdModelSubMeshInfo> subMeshes;
};

[[nodiscard]] BirdModelAsset LoadBirdModelAsset(FilePathView path, double displayHeight);
