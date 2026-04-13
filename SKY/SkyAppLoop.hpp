# pragma once
# include <array>
# include "BirdModel.hpp"
# include "MapData.hpp"
# include "MapEditor.hpp"
# include "SkyAppSupport.hpp"
# include "TerrainSurface.hpp"

namespace SkyAppFlow
{
  enum class AttackEffectType
	{
		Laser,
      Explosion,
	};

	struct AttackEffectInstance
	{
		AttackEffectType type = AttackEffectType::Laser;
		Vec3 startPosition{ 0, 0, 0 };
		Vec3 endPosition{ 0, 0, 0 };
		ColorF color{ 0.40, 0.92, 1.0, 1.0 };
		double startedAt = 0.0;
		double lifetime = 0.12;
		double thickness = 5.0;
      double radius = 0.0;
	};

	struct SkyAppResources
	{
		Mesh groundPlane;
		Texture groundTexture;
      Texture roadTexture;
		Model blacksmithModel;
		Model millModel;
		Model treeModel;
		Model pineModel;
        Model grassPatchModel;
      std::array<UnitModel, MainSupport::UnitRenderModelCount> unitRenderModels;
		MSRenderTexture renderTexture;

       [[nodiscard]] UnitModel& GetUnitRenderModel(const MainSupport::UnitRenderModel renderModel)
		{
			return unitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];
		}

     [[nodiscard]] const UnitModel& GetUnitRenderModel(const MainSupport::UnitRenderModel renderModel) const
		{
			return unitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];
		}

		[[nodiscard]] SkyAppSupport::UnitRenderModelRegistryView GetUnitRenderModelRegistry() const
		{
			SkyAppSupport::UnitRenderModelRegistryView registry;

			for (const MainSupport::UnitRenderModel renderModel : MainSupport::GetUnitRenderModels())
			{
				const size_t index = MainSupport::GetUnitRenderModelIndex(renderModel);
				registry.models[index] = &unitRenderModels[index];
			}

			return registry;
		}

		SkyAppResources();
	};

	enum class UiLayoutPanel
	{
		MiniMap,
		ResourcePanel,
       ModelHeight,
       TerrainVisualSettings,
      UnitEditor,
	};

	struct UiPanelDragState
	{
		UiLayoutPanel panel = UiLayoutPanel::MiniMap;
		Point grabOffset{ 0, 0 };
		Point startPosition{ 0, 0 };
     MainSupport::UiLayoutSettings layoutAtDragStart;
		bool moved = false;
	};

	struct SkyAppState
	{
		MainSupport::CameraSettings cameraSettings;
		MainSupport::ModelHeightSettings modelHeightSettings;
     FilePath currentMapPath = FilePath{ MainSupport::MapDataPath };
		MapData mapData = MakeDefaultMapData();
     TerrainSurfaceData terrainSurface;
		uint64 terrainSurfaceRevision = 0;
      MainSupport::AppCamera3D camera{ Graphics3D::GetRenderTargetSize(), 40_deg, MainSupport::DefaultCameraEye, MainSupport::DefaultCameraFocus };
		Sky sky;
		double skyTime = 0.5;
     bool showUI = false;
     bool uiEditMode = false;
      bool skySettingsExpanded = false;
		bool cameraSettingsExpanded = false;
      bool terrainVisualSettingsExpanded = false;
      bool miniMapExpanded = true;
        MainSupport::TerrainVisualSettings terrainVisualSettings;
     MainSupport::UiLayoutSettings uiLayoutSettings;
	  Optional<UiPanelDragState> uiPanelDrag;
      bool showEscMenu = false;
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
       SkyAppSupport::TimedMessage uiLayoutMessage;
       SkyAppSupport::TimedMessage unitEditorMessage;
		bool modelHeightEditMode = false;
      MainSupport::UnitRenderModel modelHeightTarget = MainSupport::UnitRenderModel::Bird;
      bool unitEditorMode = false;
       bool showResourceAdjustUi = false;
      bool requestTitleScene = false;
		Optional<Vec2> selectionDragStart;
		double playerBaseHitPoints = MainSupport::BaseMaxHitPoints;
		double enemyBaseHitPoints = MainSupport::BaseMaxHitPoints;
        MainSupport::ResourceStock playerResources;
		MainSupport::ResourceStock enemyResources;
       MainSupport::ResourceStock initialPlayerResources{ .budget = MainSupport::StartingResources };
		Array<MainSupport::ResourceAreaState> resourceAreaStates;
       MainSupport::UnitEditorSettings unitEditorSettings;
      MainSupport::UnitEditorSelection unitEditorSelection;
       MainSupport::UnitEditorPage unitEditorPage = MainSupport::UnitEditorPage::Basic;
        int32 playerTier = 1;
      Array<double> millLastAttackTimes;
      Array<AttackEffectInstance> attackEffects;
      int32 startupCameraFreezeFrames = 2;
		double nextEnemyReinforcementAt = 0.0;
		size_t enemyReinforcementCount = 0;
       MainSupport::EnemyBattlePlan enemyBattlePlan = MainSupport::EnemyBattlePlan::SecureResources;
      MainSupport::EnemyBattlePlanOverride enemyBattlePlanOverride = MainSupport::EnemyBattlePlanOverride::Auto;
		Optional<size_t> enemyTargetResourceAreaIndex;
		double nextEnemyAiDecisionAt = 0.0;
		double nextEnemyProductionAt = 0.0;
		Optional<bool> playerWon;
	};

	struct SkyAppFrameState
	{
		SkyAppSupport::SkyAppPanels panels;
		bool isEditorMode = false;
		bool showSapperMenu = false;
      bool showMillStatusEditor = false;
     bool showUnitEditor = false;
      bool showEscMenu = false;
		bool isHoveringUI = false;
     std::array<Vec3, MainSupport::UnitRenderModelCount> previewRenderPositions{};

		[[nodiscard]] const Vec3& GetPreviewRenderPosition(const MainSupport::UnitRenderModel renderModel) const
		{
			return previewRenderPositions[MainSupport::GetUnitRenderModelIndex(renderModel)];
		}
	};

	void InitializeSkyAppState(SkyAppState& state);
	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state);
}
