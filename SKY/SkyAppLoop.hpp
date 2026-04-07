# pragma once
# include "BirdModel.hpp"
# include "MapData.hpp"
# include "MapEditor.hpp"
# include "SkyAppSupport.hpp"

namespace SkyAppFlow
{
	struct SkyAppResources
	{
		Mesh groundPlane;
		Texture groundTexture;
		Model blacksmithModel;
		Model millModel;
		Model treeModel;
		Model pineModel;
		BirdModel birdModel;
		BirdModel ashigaruModel;
		MSRenderTexture renderTexture;

		SkyAppResources();
	};

	struct SkyAppState
	{
		MainSupport::CameraSettings cameraSettings;
		MainSupport::ModelHeightSettings modelHeightSettings;
		MapData mapData = MakeDefaultMapData();
      MainSupport::AppCamera3D camera{ Graphics3D::GetRenderTargetSize(), 40_deg, MainSupport::DefaultCameraEye, MainSupport::DefaultCameraFocus };
		Sky sky;
		double skyTime = 0.5;
		bool showUI = true;
		MainSupport::AppMode appMode = MainSupport::AppMode::Play;
		MapEditorState mapEditor;
		bool showBlacksmithMenu = false;
		Array<MainSupport::SpawnedSapper> spawnedSappers;
		Array<MainSupport::SpawnedSapper> enemySappers;
		Array<size_t> selectedSapperIndices;
      Optional<size_t> selectedMillIndex;
		SkyAppSupport::TimedMessage blacksmithMenuMessage;
		SkyAppSupport::TimedMessage cameraSaveMessage;
		SkyAppSupport::TimedMessage modelHeightMessage;
		SkyAppSupport::TimedMessage restartMessage;
		SkyAppSupport::TimedMessage mapDataMessage;
		bool modelHeightEditMode = false;
		Optional<Vec2> selectionDragStart;
		double playerBaseHitPoints = MainSupport::BaseMaxHitPoints;
		double enemyBaseHitPoints = MainSupport::BaseMaxHitPoints;
        MainSupport::ResourceStock playerResources;
		MainSupport::ResourceStock enemyResources;
		Array<MainSupport::ResourceAreaState> resourceAreaStates;
        int32 playerTier = 1;
      Array<double> millLastAttackTimes;
      int32 startupCameraFreezeFrames = 2;
		double nextEnemyReinforcementAt = 0.0;
		size_t enemyReinforcementCount = 0;
		Optional<bool> playerWon;
	};

	struct SkyAppFrameState
	{
		SkyAppSupport::SkyAppPanels panels;
		bool isEditorMode = false;
		bool showSapperMenu = false;
      bool showMillStatusEditor = false;
		bool isHoveringUI = false;
		Vec3 birdRenderPosition = MainSupport::BirdDisplayPosition;
		Vec3 ashigaruRenderPosition = MainSupport::AshigaruDisplayPosition;
	};

	void InitializeSkyAppState(SkyAppState& state);
	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state);
}
