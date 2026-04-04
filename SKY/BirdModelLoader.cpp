# include <Siv3D.hpp>
# include <assimp/Importer.hpp>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include "BirdModelLoader.hpp"

namespace
{
	[[nodiscard]] String ToSivString(const aiString& value)
	{
		return Unicode::FromUTF8(value.C_Str());
	}

	[[nodiscard]] Mat4x4 ToMat4x4(const aiMatrix4x4& value) noexcept
	{
		// Assimp: column-vector convention (M * v), translation at a4/b4/c4
		// Siv3D/DirectXMath: row-vector convention (v * M), translation at 4th row
		// → transpose required
		return Mat4x4{
			value.a1, value.b1, value.c1, value.d1,
			value.a2, value.b2, value.c2, value.d2,
			value.a3, value.b3, value.c3, value.d3,
			value.a4, value.b4, value.c4, value.d4,
		};
	}

	[[nodiscard]] Float3 ToFloat3(const aiVector3D& value) noexcept
	{
		return Float3{ value.x, value.y, value.z };
	}

	[[nodiscard]] Quaternion ToQuaternion(const aiQuaternion& value) noexcept
	{
		return Quaternion{ value.x, value.y, value.z, value.w };
	}

	struct ImportedStaticMesh
	{
		Mesh mesh;
		bool loaded = false;
       Array<Vertex3D> vertices;
		Array<TriangleIndex32> indices;
      Mat4x4 normalizationTransform = Mat4x4::Identity();
	};

	struct ImportedAnimatedData
	{
		Array<Vertex3D> vertices;
		Array<TriangleIndex32> indices;
		Array<int32> vertexNodeIndices;
		Array<BirdModelNode> nodes;
		Array<BirdModelBone> bones;
		Array<BirdModelAnimationClip> animations;
		Array<BirdModelSubMeshInfo> subMeshes;
	};

# include "BirdModelLoader.Static.inl"
# include "BirdModelLoader.Animated.inl"
}

# include "BirdModelLoader.Asset.inl"
