#pragma once
# include <Siv3D.hpp>
# include "App/AppStateData.h"
# include "../UI/BattleNotifications.h"

namespace LT3
{
	inline void DrawAppRuntime(const AppRuntimeState& runtime, const AppDefinitionState& definitions, const AppUiState& ui, const LocalizationCatalog& localizedTexts, const LocalizationCatalog& defaultTexts, const Font& uiFont, const Font& titleFont)
	{
		if (runtime.world.definitionGeneration == runtime.battleDefinitionGeneration
			&& HasValidBattleWorldState(runtime.world, runtime.battleDefinitions))
		{
			DrawBattleWorld(runtime.world, runtime.battleDefinitions, runtime.battleRenderAssets, runtime.resourceFlags, ui.mapEditor, ui.clickDebug, ui.mapEditor.showDebugInfo, localizedTexts, defaultTexts, uiFont, titleFont);
		}
		DrawBattleNotifications(runtime.notifications, uiFont);
	}

	inline void DrawAppUi(const AppDefinitionState& definitions, AppUiState& ui, const Font& uiFont)
	{
		DrawMapEditorOverlay(ui.mapEditor, definitions.unitCatalog, definitions.defs, Cursor::PosF(), uiFont);
		DrawDebugNewGameButtons(ui, uiFont);
		DrawDebugClipboardCaptureButton(ui, uiFont);
		SubmitDebugClipboardCaptureRequest(ui);
	}

	inline void DrawApp(AppState& app)
	{
		const LocalizationCatalog emptyLocalization;
		DrawAppRuntime(app.runtime, app.definitions, app.ui, emptyLocalization, emptyLocalization, app.uiFont, app.titleFont);
		DrawAppUi(app.definitions, app.ui, app.uiFont);
	}
}
