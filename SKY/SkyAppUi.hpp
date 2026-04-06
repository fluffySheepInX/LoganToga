# pragma once
# include "BirdModel.hpp"
# include "MapData.hpp"
# include "SkyAppSupport.hpp"

namespace SkyAppSupport
{
	void DrawSkySettingsPanel(Sky& sky, const SkyAppPanels& panels);
	void DrawCameraSettingsPanel(DebugCamera3D& camera,
		MainSupport::CameraSettings& cameraSettings,
		BirdModel& birdModel,
		BirdModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels);
	void DrawBlacksmithMenu(const SkyAppPanels& panels,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
     MainSupport::ResourceStock& playerResources,
		double sapperCost,
		TimedMessage& blacksmithMenuMessage);
  void DrawMiniMap(const SkyAppPanels& panels,
		const DebugCamera3D& camera,
		const MapData& mapData,
        const Array<MainSupport::SpawnedSapper>& spawnedSappers,
		const Array<MainSupport::SpawnedSapper>& enemySappers,
        const Array<MainSupport::ResourceAreaState>& resourceAreaStates,
		const Array<size_t>& selectedSapperIndices);
	void DrawSapperMenu(const SkyAppPanels& panels,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
     MainSupport::ResourceStock& playerResources,
		double sapperCost,
		TimedMessage& sapperMenuMessage);
}
