#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "../Systems/BattleInputSystem.h"
# include "../Systems/CameraInputSystem.h"
# include "../Systems/EditorInputSystem.h"
# include "App/AppStateData.h"

namespace LT3
{
	inline constexpr double ResourceFlagRaiseDurationSec = 1.2;

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
			if (ui.mapEditor.unitCatalogDirty || ui.mapEditor.buildLineIconsDirty || ui.mapEditor.skillDefsDirty)
			{
				definitions.defs = CreateDefaultDefinitions(definitions.unitCatalog);
				definitions.renderAssets = BuildBattleRenderAssets(definitions.unitCatalog, &definitions.defs);
				ui.mapEditor.unitCatalogDirty = false;
				ui.mapEditor.buildLineIconsDirty = false;
				ui.mapEditor.skillDefsDirty = false;
			}
			return;
		}
		if (ui.mapEditor.enabled)
		{
			return;
		}

		HandleBattleInput(runtime.world, definitions.defs, ui.mapEditor, screenMouse, worldMouse);
	}

	inline void UpdateAppRuntimeState(AppRuntimeState& runtime, AppDefinitionState& definitions, AppUiState& ui)
	{
		if (ui.debugNewGameRequest != DebugNewGameRequest::None)
		{
			const bool enemyAiStopped = (ui.debugNewGameRequest == DebugNewGameRequest::EnemyAiStopped);
			ResetBattleRuntimeState(runtime, definitions.defs, enemyAiStopped);
			SyncBattleWorldMapFromEditor(ui.mapEditor, runtime.world, definitions.defs);
			SyncResourceFlagRuntimeState(runtime);
			ui.debugNewGameRequest = DebugNewGameRequest::None;
		}

		ProcessInput(runtime, definitions, ui);
		if (!ui.mapEditor.enabled)
		{
			UpdateBattleWorld(runtime.world, definitions.defs, Scene::DeltaTime());
		}

		UpdateResourceFlagRuntimeState(runtime);
	}

	inline void UpdateApp(AppState& app)
	{
		UpdateAppUiState(app.ui);
		UpdateAppRuntimeState(app.runtime, app.definitions, app.ui);
	}
}
