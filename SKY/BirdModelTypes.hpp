# pragma once
# include <Siv3D.hpp>

template <class Type>
struct BirdModelAnimationKey
{
	double time = 0.0;
	Type value{};
};

struct BirdModelNode
{
	String name;
	int32 parentIndex = -1;
	Mat4x4 localTransform = Mat4x4::Identity();
};

struct BirdModelBoneWeight
{
	uint32 vertexIndex = 0;
	float weight = 0.0f;
};

struct BirdModelBone
{
	String name;
	int32 nodeIndex = -1;
	Mat4x4 offsetMatrix = Mat4x4::Identity();
	Array<BirdModelBoneWeight> weights;
};

struct BirdModelAnimationChannel
{
	String nodeName;
  int32 nodeIndex = -1;
	Array<BirdModelAnimationKey<Float3>> positionKeys;
	Array<BirdModelAnimationKey<Quaternion>> rotationKeys;
	Array<BirdModelAnimationKey<Float3>> scalingKeys;
};

struct BirdModelAnimationClip
{
	String name;
	double duration = 0.0;
	double ticksPerSecond = 0.0;
	Array<BirdModelAnimationChannel> channels;
};

struct BirdModelSubMeshInfo
{
	uint32 globalVertexStart = 0;
	uint32 vertexCount = 0;
	Array<TriangleIndex32> localIndices;
	ColorF materialColor{ 1.0, 1.0, 1.0, 1.0 };
  Texture diffuseTexture;
};
