# pragma once
# include "MainContext.hpp"
# include "MapData.hpp"

namespace MainSupport
{
    struct PlacedModelRenderResources
	{
		const Model& millModel;
		const Model& treeModel;
		const Model& pineModel;
		const Model& grassPatchModel;
		const Texture& roadTexture;
		const Texture& tireTrackStartTexture;
		const Texture& tireTrackMiddleTexture;
		const Texture& tireTrackEndTexture;
	};

    void EnsureValidCameraSettings(CameraSettings& cameraSettings);
    void ThrowIfInvalidCameraPair(const Vec3& eye, const Vec3& focus, StringView context);
	void ThrowIfInvalidCameraState(const AppCamera3D& camera, StringView context);
	[[nodiscard]] Optional<Ray> TryScreenToRay(const AppCamera3D& camera, const Vec2& screenPosition);
	[[nodiscard]] Optional<Vec3> GetWheelZoomFocusPosition(const AppCamera3D& camera, const Vec3& playerBasePosition);
	[[nodiscard]] Vec3 GetSapperPopTargetPosition(const Vec3& rallyPoint, size_t sapperIndex);
    void DrawPlacedModel(const PlacedModel& placedModel, const ModelHeightSettings& modelHeightSettings, const PlacedModelRenderResources& renderResources);
   void DrawSelectionIndicator(const AppCamera3D& camera, const Vec3& position);
	void DrawGroundContactOverlay(const AppCamera3D& camera, const Vec3& position);
}
