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
        Title,
		Resize1280x720,
		Resize1600x900,
		Resize1920x1080,
	};

		enum class SapperMenuAction
		{
			None,
			UseExplosionSkill,
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
      const MainSupport::UnitEditorSettings& unitEditorSettings,
		TimedMessage& blacksmithMenuMessage);
  void DrawMiniMap(bool& isExpanded,
		const SkyAppPanels& panels,
     bool uiEditMode,
		const MainSupport::AppCamera3D& camera,
		const MapData& mapData,
        const Array<MainSupport::SpawnedSapper>& spawnedSappers,
		const Array<MainSupport::SpawnedSapper>& enemySappers,
        const Array<MainSupport::ResourceAreaState>& resourceAreaStates,
		const Array<size_t>& selectedSapperIndices);
 [[nodiscard]] EscMenuAction DrawEscMenu(const Rect& panelRect);
     [[nodiscard]] SapperMenuAction DrawSapperMenu(const SkyAppPanels& panels,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
     MainSupport::ResourceStock& playerResources,
        size_t selectedSapperIndex,
		TimedMessage& sapperMenuMessage);
    void DrawUnitEditor(const SkyAppPanels& panels,
        bool uiEditMode,
		MainSupport::UnitEditorSettings& unitEditorSettings,
		MainSupport::UnitEditorSection& activeSection,
		Array<MainSupport::SpawnedSapper>& spawnedSappers,
		Array<MainSupport::SpawnedSapper>& enemySappers,
		TimedMessage& unitEditorMessage);
   void DrawMillStatusEditor(const SkyAppPanels& panels,
      MapData& mapData,
		size_t selectedMillIndex,
		FilePathView path,
		TimedMessage& mapDataMessage);
}
