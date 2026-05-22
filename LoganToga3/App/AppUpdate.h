#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "../Systems/BattleInputSystem.h"
# include "../Systems/CameraInputSystem.h"
# include "../Systems/EditorInputSystem.h"
# include "App/AppStateData.h"

namespace LT3
{
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
			ui.debugNewGameRequest = DebugNewGameRequest::None;
		}

		ProcessInput(runtime, definitions, ui);
		if (!ui.mapEditor.enabled)
		{
			UpdateBattleWorld(runtime.world, definitions.defs, Scene::DeltaTime());
		}
	}

	inline void UpdateApp(AppState& app)
	{
		UpdateAppUiState(app.ui);
		UpdateAppRuntimeState(app.runtime, app.definitions, app.ui);
	}
}
