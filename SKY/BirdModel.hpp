# pragma once
# include <Siv3D.hpp>
# include "BirdModelTypes.hpp"

class UnitModel
{
public:
    explicit UnitModel(FilePathView path, double displayHeight, UnitModelProceduralAnimationType proceduralAnimationType = UnitModelProceduralAnimationType::None);

	void update(double deltaTime);

    void draw(const Vec3& position, double yaw, const ColorF& color = Palette::White, double scale = 1.0) const;
	[[nodiscard]] Optional<Vec3> groundContactPoint(const Vec3& position, double yaw, double scale = 1.0) const;

	[[nodiscard]] bool isLoaded() const noexcept;
	[[nodiscard]] bool hasAnimations() const noexcept;

	[[nodiscard]] double animationTime() const noexcept;
	[[nodiscard]] size_t currentClipIndex() const noexcept;
	void setClipIndex(size_t index);
   [[nodiscard]] const Array<UnitModelNode>& nodes() const noexcept;
	[[nodiscard]] const Array<UnitModelBone>& bones() const noexcept;
	[[nodiscard]] const Array<UnitModelAnimationClip>& animations() const noexcept;
	[[nodiscard]] const Array<Mat4x4>& currentLocalTransforms() const noexcept;
	[[nodiscard]] const Array<Mat4x4>& currentWorldTransforms() const noexcept;
	[[nodiscard]] const Array<Mat4x4>& currentBoneTransforms() const noexcept;

private:
    struct VertexBoneInfluence
	{
		int32 boneIndex = -1;
		float weight = 0.0f;
	};

	struct VertexSkinningData
	{
		int32 nodeIndex = -1;
		float totalBoneWeight = 0.0f;
		Array<VertexBoneInfluence> boneInfluences;
	};

	   struct SubMeshRenderData
	   {
		   uint32 globalVertexStart = 0;
		   uint32 vertexCount = 0;
		   DynamicMesh dynamicMesh;
		   ColorF materialColor{ 1.0, 1.0, 1.0, 1.0 };
          Texture diffuseTexture;
	   };

	   void evaluateAnimationPose();
     void applyProceduralAnimation();
	   void applyBirdWingFlap();
	   void refreshWorldTransforms();
	   void refreshBoneTransforms();
	   void refreshSkinnedMesh();

	   Mesh m_mesh;
	   DynamicMesh m_dynamicMesh;
	   bool m_loaded = false;
      UnitModelProceduralAnimationType m_proceduralAnimationType = UnitModelProceduralAnimationType::None;
	   double m_animationTime = 0.0;
	   size_t m_currentClipIndex = 0;
       Array<UnitModelNode> m_nodes;
	   Array<UnitModelBone> m_bones;
	   Array<UnitModelAnimationClip> m_animations;
	   Array<Vertex3D> m_bindPoseVertices;
	   Array<Vertex3D> m_deformedVertices;
	   Array<TriangleIndex32> m_indices;
	   Array<VertexSkinningData> m_vertexSkinning;
	   Mat4x4 m_normalizationTransform = Mat4x4::Identity();
	   Array<SubMeshRenderData> m_subMeshes;
	   Array<Mat4x4> m_currentLocalTransforms;
	   Array<Mat4x4> m_currentWorldTransforms;
	   Array<Mat4x4> m_currentBoneTransforms;
   };

using BirdModel = UnitModel;
