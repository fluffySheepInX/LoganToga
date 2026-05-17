#pragma once
# include <Siv3D.hpp>
# include "../UI/BattleRenderer.h"
# include "../UI/MapEditor.h"
# include "../Data/BattleAssetPaths.h"

namespace LT3
{
    enum class DebugNewGameRequest
    {
        None,
        Normal,
        EnemyAiStopped,
    };

    struct AppUiState
    {
        MapEditorState mapEditor;
        ClickDebugState clickDebug;
        Texture debugClipboardCaptureIcon;
        FilePath debugClipboardCaptureIconPath;
        bool debugClipboardCaptureRequested = false;
        bool debugClipboardCaptureInFlight = false;
        DebugNewGameRequest debugNewGameRequest = DebugNewGameRequest::None;
    };

    inline RectF DebugClipboardCaptureButtonRect()
    {
        return RectF{ Scene::Width() - 76.0, 80.0, 64.0, 64.0 };
    }

    inline String DebugClipboardCaptureShortcutLabel()
    {
        return U"Ctrl+V";
    }

    inline RectF DebugNewGameButtonRect()
    {
        return RectF{ Scene::Width() - 244.0, 80.0, 156.0, 30.0 };
    }

    inline RectF DebugNewGameEnemyAiStopButtonRect()
    {
        return RectF{ Scene::Width() - 244.0, 116.0, 156.0, 30.0 };
    }

    inline void ShowDebugClipboardCaptureToast(const String& title, const String& message, const FilePath& imagePath)
    {
# if SIV3D_PLATFORM(WINDOWS)
        if (Platform::Windows::ToastNotification::IsAvailable())
        {
            ToastNotificationItem item;
            item.title = title;
            item.message = message;
            item.imagePath = imagePath;
            item.audio = false;
            Platform::Windows::ToastNotification::Show(item);
        }
# else
        (void)title;
        (void)message;
        (void)imagePath;
# endif
    }

    inline void NotifyDebugClipboardCaptureResult(const AppUiState& ui, bool success)
    {
        ShowDebugClipboardCaptureToast(
            success ? U"キャプチャをコピーしました" : U"キャプチャに失敗しました",
            success ? U"現在の画面をクリップボードにコピーしました。" : U"画面キャプチャ、またはクリップボードへのコピーに失敗しました。",
            ui.debugClipboardCaptureIconPath);
    }

    inline void InitializeAppUiState(AppUiState& ui)
    {
        ui = AppUiState{};
        LoadMapEditorAssets(ui.mapEditor);
        ui.debugClipboardCaptureIconPath = ResolveSystemImagePath(U"copy.png");
        if (FileSystem::Exists(ui.debugClipboardCaptureIconPath))
        {
            ui.debugClipboardCaptureIcon = Texture{ ui.debugClipboardCaptureIconPath };
        }
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

    inline void UpdateDebugClipboardCaptureState(AppUiState& ui)
    {
        if (!ui.debugClipboardCaptureInFlight)
        {
            return;
        }

        if (!ScreenCapture::HasNewFrame())
        {
            return;
        }

        Image captured;
        bool success = false;
        if (ScreenCapture::GetFrame(captured) && (not captured.isEmpty()))
        {
            Clipboard::SetImage(captured);

            Image clipboardImage;
            success = Clipboard::GetImage(clipboardImage)
                && (clipboardImage.size() == captured.size())
                && (not clipboardImage.isEmpty());
        }

        NotifyDebugClipboardCaptureResult(ui, success);
        ui.debugClipboardCaptureInFlight = false;
    }

    inline bool HandleDebugClipboardCaptureButton(AppUiState& ui)
    {
        if (!ui.mapEditor.showDebugInfo)
        {
            return false;
        }

        const RectF buttonRect = DebugClipboardCaptureButtonRect();
        if (buttonRect.mouseOver())
        {
            Cursor::RequestStyle(CursorStyle::Hand);
        }

        if (!buttonRect.leftClicked())
        {
            return false;
        }

        if (!ui.debugClipboardCaptureRequested && !ui.debugClipboardCaptureInFlight)
        {
            ui.debugClipboardCaptureRequested = true;
        }

        return true;
    }

    inline bool HandleDebugClipboardCaptureShortcut(AppUiState& ui)
    {
        if (!ui.mapEditor.showDebugInfo)
        {
            return false;
        }

        if (!(KeyControl + KeyV).down())
        {
            return false;
        }

        if (!ui.debugClipboardCaptureRequested && !ui.debugClipboardCaptureInFlight)
        {
            ui.debugClipboardCaptureRequested = true;
        }

        return true;
    }

    inline bool HandleDebugNewGameButtons(AppUiState& ui)
    {
        if (!ui.mapEditor.showDebugInfo)
        {
            return false;
        }

        const RectF normalRect = DebugNewGameButtonRect();
        const RectF enemyStopRect = DebugNewGameEnemyAiStopButtonRect();

        if (normalRect.mouseOver() || enemyStopRect.mouseOver())
        {
            Cursor::RequestStyle(CursorStyle::Hand);
        }

        if (normalRect.leftClicked())
        {
            ui.debugNewGameRequest = DebugNewGameRequest::Normal;
            return true;
        }

        if (enemyStopRect.leftClicked())
        {
            ui.debugNewGameRequest = DebugNewGameRequest::EnemyAiStopped;
            return true;
        }

        return false;
    }

    inline void DrawDebugClipboardCaptureButton(const AppUiState& ui, const Font& uiFont)
    {
        if (!ui.mapEditor.showDebugInfo)
        {
            return;
        }

        const RectF buttonRect = DebugClipboardCaptureButtonRect();
        const bool hovered = buttonRect.mouseOver();
        const bool busy = ui.debugClipboardCaptureRequested || ui.debugClipboardCaptureInFlight;

        ColorF backColor{ 0.05, 0.07, 0.10, 0.72 };
        if (hovered)
        {
            backColor = ColorF{ 0.12, 0.16, 0.22, 0.88 };
        }
        if (busy)
        {
            backColor = ColorF{ 0.16, 0.14, 0.08, 0.92 };
        }

        buttonRect.draw(backColor).drawFrame(2.0, hovered || busy ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.25 });

        if (ui.debugClipboardCaptureIcon)
        {
            ui.debugClipboardCaptureIcon.resized(48, 48).drawAt(buttonRect.center());
        }
        else
        {
            uiFont(U"Copy").drawAt(12, buttonRect.center(), Palette::White);
        }

        if (hovered)
        {
            const String tipText = U"Shortcut: {}"_fmt(DebugClipboardCaptureShortcutLabel());
            const RectF tipRect{ buttonRect.x - 178.0, buttonRect.y + 18.0, 166.0, 28.0 };
            tipRect.rounded(5.0).draw(ColorF{ 0.02, 0.03, 0.045, 0.92 }).drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.22 });
            uiFont(tipText).drawAt(11, tipRect.center(), Palette::Lightgray);
        }
    }

    inline void DrawDebugNewGameButtons(const AppUiState& ui, const Font& uiFont)
    {
        if (!ui.mapEditor.showDebugInfo)
        {
            return;
        }

        const RectF normalRect = DebugNewGameButtonRect();
        const RectF enemyStopRect = DebugNewGameEnemyAiStopButtonRect();
        const bool normalHover = normalRect.mouseOver();
        const bool enemyStopHover = enemyStopRect.mouseOver();

        normalRect.draw(ColorF{ 0.14, 0.18, 0.12, 0.92 }).drawFrame(2.0, normalHover ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1, 1, 1, 0.22 });
        enemyStopRect.draw(ColorF{ 0.22, 0.12, 0.12, 0.92 }).drawFrame(2.0, enemyStopHover ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1, 1, 1, 0.22 });

        uiFont(U"NewGame").drawAt(14, normalRect.center(), Palette::White);
        uiFont(U"NewGame (AI Stop)").drawAt(12, enemyStopRect.center(), Palette::White);
    }

    inline void SubmitDebugClipboardCaptureRequest(AppUiState& ui)
    {
        if (!ui.debugClipboardCaptureRequested)
        {
            return;
        }

        ScreenCapture::RequestCurrentFrame();
        ui.debugClipboardCaptureRequested = false;
        ui.debugClipboardCaptureInFlight = true;
    }

    inline void UpdateAppUiState(AppUiState& ui)
    {
        UpdateClickDebugState(ui.clickDebug, ui.mapEditor);
        UpdateDebugClipboardCaptureState(ui);
    }
}
