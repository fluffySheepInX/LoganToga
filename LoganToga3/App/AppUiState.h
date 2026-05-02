#pragma once
# include <Siv3D.hpp>
# include "../UI/BattleRenderer.h"
# include "../UI/MapEditor.h"

namespace LT3
{
    struct AppUiState
    {
        MapEditorState mapEditor;
        ClickDebugState clickDebug;
    };

    inline void InitializeAppUiState(AppUiState& ui)
    {
        ui = AppUiState{};
        LoadMapEditorAssets(ui.mapEditor);
    }

    inline void UpdateClickDebugState(ClickDebugState& debugState, const MapEditorState& mapEditor)
    {
        debugState.currentScreen = Cursor::PosF();
        debugState.currentWorld = ToQuarterWorld(debugState.currentScreen);
        debugState.currentCell = PickMapEditorCell(mapEditor, debugState.currentScreen);

        if (MouseL.down())
        {
            debugState.lastLeftScreen = debugState.currentScreen;
            debugState.lastLeftWorld = debugState.currentWorld;
            debugState.lastLeftCell = debugState.currentCell;
        }
        if (MouseR.down())
        {
            debugState.lastRightScreen = debugState.currentScreen;
            debugState.lastRightWorld = debugState.currentWorld;
            debugState.lastRightCell = debugState.currentCell;
        }
    }

    inline void UpdateAppUiState(AppUiState& ui)
    {
        UpdateClickDebugState(ui.clickDebug, ui.mapEditor);
    }
}
