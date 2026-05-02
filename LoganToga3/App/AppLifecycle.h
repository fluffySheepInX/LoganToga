#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "App/AppDefinitionState.h"
# include "App/AppRuntimeState.h"
# include "App/AppUiState.h"
# include "../Systems/BattleInputSystem.h"
# include "../Systems/CameraInputSystem.h"
# include "../Systems/EditorInputSystem.h"
# include "../UI/QuarterView.h"

namespace LT3
{
    struct AppState
    {
        Font titleFont{ FontMethod::MSDF, 38, Typeface::Bold };
        Font uiFont{ FontMethod::MSDF, 20, Typeface::Medium };
        AppDefinitionState definitions = CreateAppDefinitionState();
        AppRuntimeState runtime;
        AppUiState ui;
    };

    inline Vec2 ToWorldPos(const Vec2& screenPos)
    {
        return ToQuarterWorld(screenPos);
    }

    inline void InitializeGaussianAddon()
    {
        Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
        GaussianFSAddon::Condition({ 1600, 900 });
        GaussianFSAddon::SetLangSet({
            { U"Japan",     U"日本語" },
            { U"English",   U"English" },
            { U"Deutsch",   U"Deutsch" },
            { U"Test",      U"TestLang" },
            });
        GaussianFSAddon::SetLang(U"Japan");
        GaussianFSAddon::SetSceneSet({
            { U"1600*900", U"1600", U"900" },
            { U"1200*600", U"1200", U"600" },
            });
        GaussianFSAddon::SetScene(U"1600*900");
    }

    inline void InitializeApp(AppState& app)
    {
        Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
        InitializeAppRuntimeState(app.runtime, app.definitions);
        InitializeAppUiState(app.ui);
    }

    inline void ProcessInput(AppRuntimeState& runtime, const AppDefinitionState& definitions, AppUiState& ui)
    {
        if (GaussianFSAddon::IsModalActive())
        {
            return;
        }

        UpdateQuarterViewCamera(ui.mapEditor, runtime.world, definitions.defs);

        const Vec2 screenMouse = Cursor::PosF();
        const Vec2 worldMouse = ToWorldPos(screenMouse);
        if (HandleEditorInput(ui.mapEditor, runtime.world, screenMouse))
        {
            return;
        }
        if (ui.mapEditor.enabled)
        {
            return;
        }

        HandleBattleInput(runtime.world, definitions.defs, ui.mapEditor, worldMouse);
    }

    inline void UpdateAppRuntimeState(AppRuntimeState& runtime, const AppDefinitionState& definitions, AppUiState& ui)
    {
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

    inline void DrawAppRuntime(const AppRuntimeState& runtime, const AppDefinitionState& definitions, const AppUiState& ui, const Font& uiFont, const Font& titleFont)
    {
        DrawBattleWorld(runtime.world, definitions.defs, definitions.renderAssets, ui.mapEditor, ui.clickDebug, ui.mapEditor.showDebugInfo, uiFont, titleFont);
    }

    inline void DrawAppUi(const AppDefinitionState& definitions, AppUiState& ui, const Font& uiFont)
    {
        DrawMapEditorOverlay(ui.mapEditor, definitions.unitCatalog, Cursor::PosF(), uiFont);
    }

    inline void DrawApp(AppState& app)
    {
        DrawAppRuntime(app.runtime, app.definitions, app.ui, app.uiFont, app.titleFont);
        DrawAppUi(app.definitions, app.ui, app.uiFont);
    }

    inline bool ProcessGaussianAddonFrameEnd()
    {
        if (GaussianFSAddon::TriggerOrDisplayESC()) return false;
        if (GaussianFSAddon::TriggerOrDisplayLang()) return false;
        if (GaussianFSAddon::TriggerOrDisplaySceneSize()) return false;
        if (GaussianFSAddon::IsHide()) Window::Minimize();
        if (GaussianFSAddon::IsGameEnd()) return false;
        GaussianFSAddon::DragProcessWindow();
        return true;
    }
}
