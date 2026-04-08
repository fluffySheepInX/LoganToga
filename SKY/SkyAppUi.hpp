# pragma once
# include "BirdModel.hpp"
# include "MapData.hpp"
# include "SkyAppSupport.hpp"

namespace SkyAppSupport
{
  enum class EscMenuAction
	{
		None,
		Restart,
	};

    void DrawSkySettingsPanel(Sky& sky, bool& isExpanded, const SkyAppPanels& panels);
 void DrawCameraSettingsPanel(MainSupport::AppCamera3D& camera,
		MainSupport::CameraSettings& cameraSettings,
       bool& isExpanded,
		BirdModel& birdModel,
		BirdModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels);
	void DrawBlacksmithMenu(const SkyAppPanels& panels,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
        const MapData& mapData,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
     MainSupport::ResourceStock& playerResources,
      int32& playerTier,
		double sapperCost,
		TimedMessage& blacksmithMenuMessage);
  void DrawMiniMap(bool& isExpanded,
		const SkyAppPanels& panels,
		const MainSupport::AppCamera3D& camera,
		const MapData& mapData,
        const Array<MainSupport::SpawnedSapper>& spawnedSappers,
		const Array<MainSupport::SpawnedSapper>& enemySappers,
        const Array<MainSupport::ResourceAreaState>& resourceAreaStates,
		const Array<size_t>& selectedSapperIndices);
 [[nodiscard]] EscMenuAction DrawEscMenu(const Rect& panelRect);
	void DrawSapperMenu(const SkyAppPanels& panels,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
        const MapData& mapData,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
     MainSupport::ResourceStock& playerResources,
      int32& playerTier,
		double sapperCost,
		TimedMessage& sapperMenuMessage);
   void DrawMillStatusEditor(const SkyAppPanels& panels,
      MapData& mapData,
		size_t selectedMillIndex,
		FilePathView path,
		TimedMessage& mapDataMessage);
}
