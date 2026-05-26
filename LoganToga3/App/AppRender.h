#pragma once
# include <Siv3D.hpp>
# include "App/AppStateData.h"

namespace LT3
{
	inline void DrawAppRuntime(const AppRuntimeState& runtime, const AppDefinitionState& definitions, const AppUiState& ui, const Font& uiFont, const Font& titleFont)
	{
		DrawBattleWorld(runtime.world, definitions.defs, definitions.renderAssets, runtime.resourceFlags, ui.mapEditor, ui.clickDebug, ui.mapEditor.showDebugInfo, uiFont, titleFont);
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
		DrawAppRuntime(app.runtime, app.definitions, app.ui, app.uiFont, app.titleFont);
		DrawAppUi(app.definitions, app.ui, app.uiFont);
	}
}
