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

	struct MoveOrderIndicator
	{
		Vec3 position{ 0, 0, 0 };
		double startedAt = 0.0;
		double lifetime = 0.72;
	};

	struct SkyAppResources
	{
		Mesh groundPlane;
		Texture groundTexture;
      Texture roadTexture;
		Texture tireTrackStartTexture;
		Texture tireTrackMiddleTexture;
		Texture tireTrackEndTexture;
		Model blacksmithModel;
		Model millModel;
		Model treeModel;
		Model pineModel;
      Model grassPatchModel;
		Array<String> loadWarnings;
      Array<FilePath> modelHeightEditorPreviewPaths;
		Array<String> modelHeightEditorPreviewLabels;
      std::array<UnitModel, MainSupport::UnitRenderModelCount> idleUnitRenderModels;
		std::array<UnitModel, MainSupport::UnitRenderModelCount> moveUnitRenderModels;
		std::array<UnitModel, MainSupport::UnitRenderModelCount> attackUnitRenderModels;
       Array<UnitModel> modelHeightEditorIdleUnitRenderModels;
		Array<UnitModel> modelHeightEditorMoveUnitRenderModels;
		Array<UnitModel> modelHeightEditorAttackUnitRenderModels;
		MSRenderTexture renderTexture;

       [[nodiscard]] UnitModel& GetUnitRenderModel(const MainSupport::UnitRenderModel renderModel, const MainSupport::UnitModelAnimationRole role = MainSupport::UnitModelAnimationRole::Idle)
		{
          switch (role)
			{
			case MainSupport::UnitModelAnimationRole::Move:
				return moveUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];

			case MainSupport::UnitModelAnimationRole::Attack:
				return attackUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];

			case MainSupport::UnitModelAnimationRole::Idle:
			default:
				return idleUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];
			}
		}

       [[nodiscard]] UnitModel& GetModelHeightEditorPreviewModel(size_t index, const MainSupport::UnitModelAnimationRole role = MainSupport::UnitModelAnimationRole::Idle);
		[[nodiscard]] const UnitModel& GetModelHeightEditorPreviewModel(size_t index, const MainSupport::UnitModelAnimationRole role = MainSupport::UnitModelAnimationRole::Idle) const;
		[[nodiscard]] StringView GetModelHeightEditorPreviewLabel(size_t index) const;

     [[nodiscard]] const UnitModel& GetUnitRenderModel(const MainSupport::UnitRenderModel renderModel, const MainSupport::UnitModelAnimationRole role = MainSupport::UnitModelAnimationRole::Idle) const
		{
          switch (role)
			{
			case MainSupport::UnitModelAnimationRole::Move:
				return moveUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];

			case MainSupport::UnitModelAnimationRole::Attack:
				return attackUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];

			case MainSupport::UnitModelAnimationRole::Idle:
			default:
				return idleUnitRenderModels[MainSupport::GetUnitRenderModelIndex(renderModel)];
			}
		}

		[[nodiscard]] SkyAppSupport::UnitRenderModelRegistryView GetUnitRenderModelRegistry() const
		{
			SkyAppSupport::UnitRenderModelRegistryView registry;

			for (const MainSupport::UnitRenderModel renderModel : MainSupport::GetUnitRenderModels())
			{
				const size_t index = MainSupport::GetUnitRenderModelIndex(renderModel);
                registry.idleModels[index] = &idleUnitRenderModels[index];
				registry.moveModels[index] = &moveUnitRenderModels[index];
				registry.attackModels[index] = &attackUnitRenderModels[index];
               registry.modelPaths[index] = FilePath{ MainSupport::GetUnitRenderModelDefaultModelPath(renderModel) };
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
       FogSettings,
      UnitEditor,
	};

	struct UiPanelDragState
	{
		UiLayoutPanel panel = UiLayoutPanel::MiniMap;
       bool resizing = false;
		Point grabOffset{ 0, 0 };
		Point startPosition{ 0, 0 };
       Point startCursor{ 0, 0 };
		Point startSize{ 0, 0 };
		MainSupport::UiLayoutSettings layoutAtDragStart;
		 bool moved = false;
	 };

	 // --- World scene sub-state ----------------------------------------
	 // Groups all fields describing the loaded map / terrain / visual
	 // settings for the world being rendered. Centralizing these makes
	 // it clear which parts of state survive a map reload (and which
	 // should be re-derived).
	 struct WorldSceneState
	 {
		 FilePath currentMapPath = FilePath{ MainSupport::MapDataPath };
		 MapData mapData = MakeDefaultMapData();
		 TerrainSurfaceData terrainSurface;
		 uint64 terrainSurfaceRevision = 0;
		 MainSupport::TerrainVisualSettings terrainVisualSettings;
	 };

	 // --- Enemy AI sub-state -------------------------------------------
	 // Groups all fields that drive the enemy AI loop (battle plan,
	 // production / reinforcement timers, current resource-area target).
	 // Kept as a self-contained struct so the AI logic can be reasoned
	 // about without scanning the entire SkyAppState definition.
	 struct EnemyAiState
	 {
		 MainSupport::EnemyBattlePlan battlePlan = MainSupport::EnemyBattlePlan::SecureResources;
		 MainSupport::EnemyBattlePlanOverride battlePlanOverride = MainSupport::EnemyBattlePlanOverride::Auto;
		 Optional<size_t> targetResourceAreaIndex;
		 double nextDecisionAt = 0.0;
		 double nextProductionAt = 0.0;
		 double nextReinforcementAt = 0.0;
		 size_t reinforcementCount = 0;
	 };

	 // --- Environment / camera / sky sub-state -------------------------
	 // Groups camera, sky, fog-of-war and related session-level fields
	 // (current AppMode, startup-freeze counter, transient messages tied
	 // to camera / restart actions).
	 struct EnvironmentState
	 {
		 MainSupport::CameraSettings cameraSettings;
		 MainSupport::AppCamera3D camera{ Graphics3D::GetRenderTargetSize(), 40_deg, MainSupport::DefaultCameraEye, MainSupport::DefaultCameraFocus };
		 Sky sky;
		 double skyTime = 0.5;
		 SkyAppSupport::FogOfWarState fogOfWar;
		 SkyAppSupport::FogOfWarSettings fogOfWarSettings;
		 int32 startupCameraFreezeFrames = 2;
		 MainSupport::AppMode appMode = MainSupport::AppMode::Play;
	 };

	  // --- Editor / debug tools sub-state -------------------------------
	  // Groups all fields belonging to in-game editor / preview tools:
	  // model-height tweak mode, tire-track preview, unit-parameter
	  // editor, and the map editor sub-state. None of these affect a
	  // running battle when the editor toggles are off.
	  struct EditorToolsState
	  {
		  MainSupport::ModelHeightSettings modelHeightSettings;
		  bool modelHeightEditMode = false;
		  bool modelHeightTextureMode = false;
		  MainSupport::UnitRenderModel modelHeightTarget = MainSupport::UnitRenderModel::Bird;
		  size_t modelHeightPreviewModelIndex = 0;
		  MainSupport::UnitModelAnimationRole modelHeightPreviewAnimationRole = MainSupport::UnitModelAnimationRole::Idle;
		  MainSupport::TireTrackTextureSegment tireTrackTextureTarget = MainSupport::TireTrackTextureSegment::Start;
		  bool unitEditorMode = false;
		  MainSupport::UnitEditorSettings unitEditorSettings;
		  MainSupport::UnitEditorSelection unitEditorSelection;
		  MainSupport::UnitEditorPage unitEditorPage = MainSupport::UnitEditorPage::Basic;
		  MapEditorState mapEditor;
	  };

		// --- Battle session sub-state -------------------------------------
		// Groups everything that constitutes an in-progress match: spawned
		// units, base HP, per-team resources, mill cooldowns, transient
		// attack effects, selection state, and the win/loss outcome.
		// ResetMatch() effectively resets all of this.
		struct BattleState
		{
			Array<MainSupport::SpawnedSapper> spawnedSappers;
			Array<MainSupport::SpawnedSapper> enemySappers;
			Array<size_t> selectedSapperIndices;
			Optional<size_t> selectedMillIndex;
			Optional<Vec2> selectionDragStart;
			double playerBaseHitPoints = MainSupport::BaseMaxHitPoints;
			double enemyBaseHitPoints = MainSupport::BaseMaxHitPoints;
			MainSupport::ResourceStock playerResources;
			MainSupport::ResourceStock enemyResources;
			MainSupport::ResourceStock initialPlayerResources{ .budget = MainSupport::StartingResources };
			Array<MainSupport::ResourceAreaState> resourceAreaStates;
			int32 playerTier = 1;
			size_t battleCommandSelectedSlotIndex = 0;
			int32 battleCommandUnlockedSlotCount = 1;
			Array<double> millLastAttackTimes;
			Array<AttackEffectInstance> attackEffects;
			Optional<MoveOrderIndicator> moveOrderIndicator;
			Optional<bool> playerWon;
		};

			// --- HUD / UI sub-state -------------------------------------------
			// Groups all in-game HUD-level toggle flags, panel-expanded flags,
			// UI layout-edit state, and short-lived UI messages. None of these
			// affect simulation directly; they purely drive what's drawn and
			// which panels are interactive.
			struct HudUiState
			{
				bool showUI = false;
				bool uiEditMode = false;
				bool skySettingsExpanded = false;
				bool cameraSettingsExpanded = false;
				bool terrainVisualSettingsExpanded = false;
				bool fogSettingsExpanded = false;
				bool miniMapExpanded = true;
				MainSupport::UiLayoutSettings uiLayoutSettings;
				Optional<UiPanelDragState> uiPanelDrag;
				bool showEscMenu = false;
				bool showBlacksmithMenu = false;
				bool showResourceAdjustUi = false;
				bool requestTitleScene = false;
			};

				struct SkyAppState
			   {
				   WorldSceneState world;
				   EnvironmentState env;
				   EditorToolsState editor;
				   BattleState battle;
				   HudUiState hud;
				EnemyAiState enemyAi;
				SkyAppSupport::MessageBus messages;
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
		Array<Vec3> modelHeightPreviewRenderPositions;

		[[nodiscard]] const Vec3& GetPreviewRenderPosition(const MainSupport::UnitRenderModel renderModel) const
		{
			return previewRenderPositions[MainSupport::GetUnitRenderModelIndex(renderModel)];
		}

		[[nodiscard]] const Vec3& GetModelHeightPreviewRenderPosition(const size_t index) const
		{
			return modelHeightPreviewRenderPositions[index];
		}
	};

	void InitializeSkyAppState(SkyAppState& state);
	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state);
}
