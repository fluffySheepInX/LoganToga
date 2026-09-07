#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "../UI/BattleNotifications.h"
# include "../Systems/BattleInputSystem.h"
# include "../Systems/CameraInputSystem.h"
# include "../Systems/EditorInputSystem.h"
# include "../UI/MapEditorAmbientSound.h"
# include "App/AppStateData.h"

namespace LT3
{
	inline constexpr double ResourceFlagRaiseDurationSec = 1.2;

	// テキスト編集とコンテキストメニューだけを破棄して Escape を消費します。
	inline bool CancelEditorModalInput(MapEditorState& editor)
	{
		if (IsDescriptionEditorOpen(editor))
		{
			CloseDescriptionEditor(editor);
			return true;
		}
		if (editor.commandRenameTargetIndex)
		{
			editor.commandRenameTargetIndex = none;
			editor.commandRenameEditText.clear();
			editor.commandRenameIsDuplicate = false;
			return true;
		}
		if (editor.unitRenameTargetIndex)
		{
			editor.unitRenameTargetIndex = none;
			editor.unitRenameEditText.clear();
			editor.unitRenameIsDuplicate = false;
			return true;
		}
		if (editor.skillRenameTargetIndex || editor.skillNameEditTargetIndex || editor.skillValueEditingRow >= 0
			|| editor.skillNextTagEditing || editor.skillResourceCostEditingIndex >= 0)
		{
			editor.skillRenameTargetIndex = none;
			editor.skillRenameEditText.clear();
			editor.skillNameEditTargetIndex = none;
			editor.skillNameEditText.clear();
			editor.skillValueEditingRow = -1;
			editor.skillValueEditingText.clear();
			editor.skillNextTagEditing = false;
			editor.skillNextTagEditingText.clear();
			editor.skillResourceCostEditingIndex = -1;
			editor.skillResourceCostEditingText.clear();
			return true;
		}
		if (editor.unitParamEditingRow >= 0 || editor.uniqueEditorValueEditingRow >= 0
			|| editor.uniqueSpeechEditingIndex >= 0 || editor.resourceCaptureTimeEditingIndex >= 0)
		{
			editor.unitParamEditingRow = -1;
			editor.unitParamEditingText.clear();
			editor.uniqueEditorValueEditingRow = -1;
			editor.uniqueEditorValueEditingText.clear();
			editor.uniqueSpeechEditingIndex = -1;
			editor.uniqueSpeechEditingText.clear();
			editor.resourceCaptureTimeEditingIndex = -1;
			editor.resourceCaptureTimeEditingText.clear();
			return true;
		}
		if (editor.skillContextMenuTargetIndex || editor.skillUnitContextMenuTargetIndex || editor.unitContextMenuTargetIndex || editor.commandContextMenuTargetIndex
			|| editor.skillValueStepMenuRow || editor.skillResourceCostStepMenuIndex || editor.unitParamStepMenuRow
			|| editor.uniqueEditorValueStepMenuRow || editor.resourceCaptureTimeStepMenuIndex || editor.aiUnitWeightMenuRow)
		{
			editor.skillContextMenuTargetIndex = none;
			editor.skillUnitContextMenuTargetIndex = none;
			editor.unitContextMenuTargetIndex = none;
			editor.commandContextMenuTargetIndex = none;
			editor.skillValueStepMenuRow = none;
			editor.skillResourceCostStepMenuIndex = none;
			editor.unitParamStepMenuRow = none;
			editor.uniqueEditorValueStepMenuRow = none;
			editor.resourceCaptureTimeStepMenuIndex = none;
			editor.aiUnitWeightMenuRow = none;
			editor.aiUnitWeightMenuKind = AiEditorUnitWeightMenuKind::None;
			return true;
		}

		return false;
	}

	// 戦闘中の配置プレビュー、編隊指定、選択を一つだけ解除して Escape を消費します。
	inline bool CancelBattleInteraction(BattleWorld& world)
	{
		if (world.selection.actionPlacementActive)
		{
			ResetActionPlacementPreview(world);
			return true;
		}
		if (world.selection.formationPlacementActive)
		{
			world.selection.formationPlacementActive = false;
			world.selection.formationUnits.clear();
			return true;
		}
		if (world.selection.selected != InvalidUnitId || !world.selection.selectedUnits.isEmpty()
			|| world.selection.selectedSkill != InvalidSkillDefId)
		{
			ClearSelection(world);
			return true;
		}

		return false;
	}

	// エディターパネルまたは編集ツールを一つだけ閉じて Escape を消費します。
	inline bool CancelEditorTool(MapEditorState& editor)
	{
		if (editor.showSkillEditor)
		{
			editor.showSkillEditor = false;
			editor.showSkillSandboxPreview = false;
			return true;
		}
		if (editor.showCommandEditor)
		{
			editor.showCommandEditor = false;
			return true;
		}
		if (editor.showAiEditor)
		{
			editor.showAiEditor = false;
			return true;
		}
		if (editor.showBuildingEditor || editor.showUniqueEditor || editor.showUnitParameterEditor)
		{
			editor.showBuildingEditor = false;
			editor.showUniqueEditor = false;
			editor.showUnitParameterEditor = false;
			return true;
		}
		if (editor.showDecalEditor)
		{
			editor.showDecalEditor = false;
			editor.decalEditorAssetIndex = InvalidMapEditorAsset;
			return true;
		}
		if (editor.showPerlinNoisePanel)
		{
			editor.showPerlinNoisePanel = false;
			return true;
		}
		if (editor.showFogPanel)
		{
			editor.showFogPanel = false;
			return true;
		}
		if (editor.showStarToolMenu)
		{
			editor.showStarToolMenu = false;
			return true;
		}
		if (editor.zOrderMode)
		{
			editor.zOrderMode = false;
			editor.zOrderDragStartCell = none;
			editor.zOrderSelectionRect = none;
			return true;
		}
		if (editor.uiLayoutEditEnabled)
		{
			editor.uiLayoutEditEnabled = false;
			editor.uiLayoutDraggingSelectedInfo = false;
			editor.uiLayoutDraggingCommandPanel = false;
			editor.uiLayoutDraggingResourcePanel = false;
			editor.uiLayoutDraggingParamEditor = false;
			editor.uiLayoutDraggingBuildingEditor = false;
			editor.uiLayoutDraggingResourceNodeEditor = false;
			editor.uiLayoutDraggingDecalEditor = false;
			editor.uiLayoutDraggingPerlinNoisePanel = false;
			editor.uiLayoutDraggingZOrderPanel = false;
			return true;
		}

		return false;
	}

	// Escape を modal、editor、battle interaction の優先順で一度だけ消費します。
	inline bool HandleBattleEscapeCancel(AppRuntimeState& runtime, AppUiState& ui)
	{
		if (!KeyEscape.down())
		{
			return false;
		}
		if (GaussianFSAddon::IsModalActive())
		{
			return true;
		}
		if (CancelEditorModalInput(ui.mapEditor))
		{
			return true;
		}
		if (CancelEditorTool(ui.mapEditor))
		{
			return true;
		}
		return CancelBattleInteraction(runtime.world);
	}

	// 既存のエディタ dirty フラグを安全な定義再ロード種別へ分類する。
	inline DefinitionReloadKind ClassifyPendingDefinitionReload(const MapEditorState& editor)
	{
		if (editor.unitCatalogDirty || editor.skillDefsDirty)
		{
			return DefinitionReloadKind::Structural;
		}

		if (editor.buildLineIconsDirty)
		{
			return DefinitionReloadKind::AssetsOnly;
		}

		return DefinitionReloadKind::None;
	}

	inline void UpdateResourceFlagRuntimeState(AppRuntimeState& runtime)
	{
		SyncResourceFlagRuntimeState(runtime);

		const size_t nodeCount = runtime.resourceFlags.nodes.size();
		for (size_t i = 0; i < nodeCount; ++i)
		{
			auto& flagState = runtime.resourceFlags.nodes[i];
			const Faction owner = (i < runtime.world.resourceNodes.owner.size())
				? runtime.world.resourceNodes.owner[i]
				: Faction::Neutral;
			const double captureProgress = (i < runtime.world.resourceNodes.captureProgress.size())
				? runtime.world.resourceNodes.captureProgress[i]
				: 0.0;

			if (owner != flagState.lastOwner)
			{
				flagState.lastOwner = owner;
				flagState.displayFaction = owner;
				flagState.raiseTimerSec = 0.0;
				flagState.raising = (owner != Faction::Neutral && captureProgress >= 1.0);
				flagState.visible = (owner != Faction::Neutral && captureProgress >= 1.0 && !flagState.raising);
			}

			if (flagState.raising)
			{
				flagState.raiseTimerSec = Min(ResourceFlagRaiseDurationSec, flagState.raiseTimerSec + Scene::DeltaTime());
				if (flagState.raiseTimerSec >= ResourceFlagRaiseDurationSec)
				{
					flagState.raiseTimerSec = ResourceFlagRaiseDurationSec;
					flagState.raising = false;
					flagState.visible = true;
				}
			}
			else if (owner == Faction::Neutral || captureProgress < 1.0)
			{
				flagState.visible = false;
				flagState.displayFaction = owner;
				flagState.raiseTimerSec = 0.0;
			}
		}
	}

	inline void ProcessInput(AppRuntimeState& runtime, AppDefinitionState& definitions, AppUiState& ui)
	{
		if (GaussianFSAddon::IsModalActive())
		{
			return;
		}

		if (HandleDebugClipboardCaptureButton(ui))
		{
			return;
		}

		if (HandleDebugClipboardCaptureShortcut(ui))
		{
			return;
		}

		if (HandleDebugNewGameButtons(ui))
		{
			return;
		}

		if (HandleDebugEnemyMoveMarkersToggle(ui))
		{
			return;
		}

		UpdateQuarterViewCamera(ui.mapEditor, runtime.world, definitions.defs);

		const Vec2 screenMouse = Cursor::PosF();
		const Vec2 worldMouse = ToWorldPos(screenMouse);
		if (HandleEditorInput(ui.mapEditor, runtime.world, definitions.defs, definitions.unitCatalog, screenMouse))
		{
			const DefinitionReloadKind reloadKind = ClassifyPendingDefinitionReload(ui.mapEditor);
			if (reloadKind != DefinitionReloadKind::None)
			{
				definitions.defs = CreateDefaultDefinitions(definitions.unitCatalog);
				definitions.renderAssets = BuildBattleRenderAssets(definitions.unitCatalog, &definitions.defs);
				ui.mapEditor.unitCatalogDirty = false;
				ui.mapEditor.buildLineIconsDirty = false;
				ui.mapEditor.skillDefsDirty = false;
				ui.mapEditor.statusText = (reloadKind == DefinitionReloadKind::Structural)
					? U"Definition changes will apply to the next battle."
					: U"Definition assets will apply to the next battle.";
			}
			return;
		}
		if (ui.mapEditor.enabled)
		{
			return;
		}
		if (runtime.world.definitionGeneration != runtime.battleDefinitionGeneration
			|| !HasValidBattleDefinitionIds(runtime.world, runtime.battleDefinitions))
		{
			ui.mapEditor.statusText = U"Battle definition snapshot is invalid. Start a new battle.";
			return;
		}

		HandleBattleInput(runtime.world, runtime.battleDefinitions, ui.mapEditor, screenMouse, worldMouse);
	}

	inline void UpdateDecalAmbientSound(AppRuntimeState& runtime, const AppUiState& ui)
	{
		if (ui.mapEditor.enabled)
		{
			runtime.decalAmbientCooldownSec = 0.0;
			return;
		}

		runtime.decalAmbientCooldownSec = Max(0.0, runtime.decalAmbientCooldownSec - Scene::DeltaTime());
		if (runtime.decalAmbientCooldownSec > 0.0)
		{
			return;
		}

		const Array<DecalAmbientSoundCandidate> candidates = CollectDecalAmbientSoundCandidatesNearMouseOrCenter(ui.mapEditor, &runtime.activeMod);
		if (candidates.isEmpty())
		{
			runtime.decalAmbientCooldownSec = 0.8;
			return;
		}

		double totalWeight = 0.0;
		for (const auto& candidate : candidates)
		{
			totalWeight += Max(0.0, candidate.weight);
		}
		if (totalWeight <= 0.0)
		{
			runtime.decalAmbientCooldownSec = 1.0;
			return;
		}

		double roll = Random(0.0, totalWeight);
		const DecalAmbientSoundCandidate* selected = nullptr;
		for (const auto& candidate : candidates)
		{
			roll -= Max(0.0, candidate.weight);
			if (roll <= 0.0)
			{
				selected = &candidate;
				break;
			}
		}
		if (!selected)
		{
			selected = &candidates.back();
		}

		if (Audio* audio = runtime.audioAssets.findOrLoad(selected->path))
		{
			audio->playOneShot(Clamp(selected->volume, 0.0, 1.0));
		}

		runtime.decalAmbientCooldownSec = Random(1.2, 2.6);
	}

	inline void UpdateAppRuntimeState(AppRuntimeState& runtime, AppDefinitionState& definitions, AppUiState& ui)
	{
		if (ui.debugNewGameRequest != DebugNewGameRequest::None)
		{
			const bool enemyAiStopped = (ui.debugNewGameRequest == DebugNewGameRequest::EnemyAiStopped);
			PromoteBattleDefinitions(runtime, definitions, ui.mapEditor.definitionRevision);
			ResetBattleRuntimeState(runtime, runtime.battleDefinitions, enemyAiStopped);
			SyncBattleWorldMapFromEditor(ui.mapEditor, runtime.world, runtime.battleDefinitions);
			SyncResourceFlagRuntimeState(runtime);
			ui.debugNewGameRequest = DebugNewGameRequest::None;
		}

		const BattleWorldStoreInvariantResult storeInvariant = ValidateBattleWorldStoreInvariants(runtime.world);
		if (!storeInvariant.valid)
		{
			ui.mapEditor.statusText = U"Battle runtime store is invalid: " + storeInvariant.store + U"." + storeInvariant.column
				+ (storeInvariant.expectedCondition.isEmpty() ? U"" : U" (expected " + storeInvariant.expectedCondition + U")");
			return;
		}

		ProcessInput(runtime, definitions, ui);
		if (!ui.mapEditor.enabled
			&& runtime.world.definitionGeneration == runtime.battleDefinitionGeneration
			&& HasValidBattleWorldState(runtime.world, runtime.battleDefinitions))
		{
			UpdateBattleWorld(runtime.world, runtime.battleDefinitions, Scene::DeltaTime(), &runtime.notifications);
		}

		UpdateResourceFlagRuntimeState(runtime);
		UpdateDecalAmbientSound(runtime, ui);
		UpdateBattleNotifications(runtime.notifications, Scene::DeltaTime());
	}

	inline void UpdateApp(AppState& app)
	{
		UpdateAppUiState(app.ui);
		UpdateAppRuntimeState(app.runtime, app.definitions, app.ui);
	}
}
