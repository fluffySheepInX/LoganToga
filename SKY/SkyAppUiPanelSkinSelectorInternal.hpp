# pragma once
# include "SkyAppUiPanelFrameInternal.hpp"
# include "MainUi.hpp"

namespace SkyAppSupport
{
    namespace UiInternal
    {
        struct PanelSkinSelectorState
        {
            bool isOpen = false;
            MainSupport::PanelSkinTarget selectedTarget = MainSupport::PanelSkinTarget::Default;
            FilePath originalPath;
            FilePath workingPath;
            bool initializedForOpen = false;
            String statusMessage;
            double statusMessageUntil = 0.0;
        };

        [[nodiscard]] inline StringView ToPanelSkinTargetLabel(const MainSupport::PanelSkinTarget target)
        {
            switch (target)
            {
            case MainSupport::PanelSkinTarget::UnitEditor:
                return U"Unit Editor";

            case MainSupport::PanelSkinTarget::ToolModal:
                return U"Tool Modal";

            case MainSupport::PanelSkinTarget::Settings:
                return U"Settings";

            case MainSupport::PanelSkinTarget::CameraSettings:
                return U"Camera Settings";

            case MainSupport::PanelSkinTarget::Hud:
                return U"HUD";

            case MainSupport::PanelSkinTarget::MapEditor:
                return U"Map Editor";

            case MainSupport::PanelSkinTarget::Default:
            default:
                return U"Default";
            }
        }

        [[nodiscard]] inline bool DrawPanelSkinTargetButton(const Rect& rect, const StringView label, const bool selected)
        {
            static const Font buttonFont{ 16, Typeface::Bold };
            const bool hovered = rect.mouseOver();
            const ColorF fillColor = selected
                ? (hovered ? ColorF{ 0.52, 0.75, 0.95 } : ColorF{ 0.43, 0.67, 0.90 })
                : (hovered ? ColorF{ 0.84 } : ColorF{ 0.74 });
            rect.draw(fillColor).drawFrame(1, 0, ColorF{ 0.3 });
            buttonFont(label).drawAt(rect.center(), selected ? EditorTextOnSelectedPrimaryColor() : EditorTextOnCardPrimaryColor());
            return hovered && MouseL.down();
        }

        [[nodiscard]] inline String ToPanelSkinReferenceLabel(const FilePathView path)
        {
            const FilePath selectedPath = String{ path };
            return selectedPath.isEmpty()
                ? U"(default) {}"_fmt(FileSystem::FileName(DefaultPanelNinePatchPath))
                : FileSystem::FileName(selectedPath);
        }

        [[nodiscard]] inline String ToPanelSkinDescription(const FilePathView path)
        {
            const FilePath selectedPath = String{ path };
            return selectedPath.isEmpty()
                ? U"built-in asset: {}"_fmt(DefaultPanelNinePatchPath)
                : U"selected: {}"_fmt(selectedPath);
        }

        inline void DrawPanelSkinSelector(PanelSkinSelectorState& state)
        {
            if (not state.isOpen)
            {
                return;
            }

            if (not state.initializedForOpen)
            {
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
                state.initializedForOpen = true;
            }

            MainSupport::SetPanelNinePatchPath(state.selectedTarget, state.workingPath);
            Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.36 });
            const RectF panel{ Arg::center = Scene::CenterF(), 720, 404 };
           if (const auto& ninePatch = GetConfiguredPanelNinePatch(MainSupport::PanelSkinTarget::ToolModal))
            {
                ninePatch->draw(panel);
            }
            else
            {
                panel.rounded(20).draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
            }
            panel.rounded(20).drawFrame(2, 0, ColorF{ 0.74, 0.84, 0.96, 0.86 });
            SimpleGUI::GetFont()(U"Panel Skin").draw(panel.pos.movedBy(20, 16), Palette::White);
            SimpleGUI::GetFont()(U"nine-patch panel asset をファイルダイアログで切り替えます").draw(panel.pos.movedBy(22, 44), ColorF{ 0.82, 0.89, 0.98, 0.92 });

            const Rect defaultTargetButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 84), 100, 28 };
            const Rect settingsTargetButton{ static_cast<int32>(panel.x + 130), static_cast<int32>(panel.y + 84), 150, 28 };
            const Rect cameraTargetButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 116), 120, 28 };
            const Rect hudTargetButton{ static_cast<int32>(panel.x + 150), static_cast<int32>(panel.y + 116), 130, 28 };
            const Rect mapEditorTargetButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 148), 120, 28 };
            const Rect unitEditorTargetButton{ static_cast<int32>(panel.x + 150), static_cast<int32>(panel.y + 148), 130, 28 };
            const Rect toolModalTargetButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 180), 260, 28 };
            const Rect assetLabelRect{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 214), 110, 28 };
            const Rect assetPathRect{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 244), 286, 70 };
            const Rect selectButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 324), 92, 30 };
            const Rect defaultButton{ static_cast<int32>(panel.x + 120), static_cast<int32>(panel.y + 324), 92, 30 };
            const Rect saveButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 360), 126, 30 };
            const Rect closeButton{ static_cast<int32>(panel.x + 154), static_cast<int32>(panel.y + 360), 88, 30 };
            const RectF previewRect{ panel.x + 330, panel.y + 84, 366, 268 };

            if (DrawPanelSkinTargetButton(defaultTargetButton, U"Default", state.selectedTarget == MainSupport::PanelSkinTarget::Default))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::Default;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(settingsTargetButton, U"Settings", state.selectedTarget == MainSupport::PanelSkinTarget::Settings))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::Settings;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(cameraTargetButton, U"Camera Settings", state.selectedTarget == MainSupport::PanelSkinTarget::CameraSettings))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::CameraSettings;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(hudTargetButton, U"HUD", state.selectedTarget == MainSupport::PanelSkinTarget::Hud))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::Hud;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(mapEditorTargetButton, U"Map Editor", state.selectedTarget == MainSupport::PanelSkinTarget::MapEditor))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::MapEditor;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(unitEditorTargetButton, U"Unit Editor", state.selectedTarget == MainSupport::PanelSkinTarget::UnitEditor))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::UnitEditor;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            if (DrawPanelSkinTargetButton(toolModalTargetButton, U"Tool Modal", state.selectedTarget == MainSupport::PanelSkinTarget::ToolModal))
            {
                state.selectedTarget = MainSupport::PanelSkinTarget::ToolModal;
                state.originalPath = MainSupport::GetConfiguredPanelNinePatchPath(state.selectedTarget);
                state.workingPath = state.originalPath;
            }

            SimpleGUI::GetFont()(U"Asset").draw(assetLabelRect.pos.movedBy(0, 4), ColorF{ 0.82, 0.89, 0.98, 0.92 });
            assetPathRect.rounded(12).draw(ColorF{ 0.12, 0.16, 0.22, 0.96 });
            assetPathRect.rounded(12).drawFrame(1.5, 0, ColorF{ 0.42, 0.52, 0.66, 0.88 });
            SimpleGUI::GetFont()(ToPanelSkinReferenceLabel(state.workingPath)).draw(assetPathRect.pos.movedBy(12, 12), Palette::White);
            const FilePath previewPath = state.workingPath.isEmpty()
                ? MainSupport::GetEffectivePanelNinePatchPath(state.selectedTarget)
                : state.workingPath;
            SimpleGUI::GetFont()(U"{} / {}"_fmt(ToPanelSkinTargetLabel(state.selectedTarget), ToPanelSkinDescription(state.workingPath))).draw(assetPathRect.pos.movedBy(12, 38), ColorF{ 0.82, 0.89, 0.98, 0.90 });
            DrawPanelNinePatchPreview(previewRect, previewPath, ToPanelSkinTargetLabel(state.selectedTarget));

            if (MainSupport::DrawTextButton(selectButton, U"Select..."))
            {
                if (const auto selectedPath = Dialog::OpenFile({ FileFilter::AllFiles() }))
                {
                    state.workingPath = *selectedPath;
                    MainSupport::SetPanelNinePatchPath(state.selectedTarget, state.workingPath);
                    state.statusMessage = U"{} skin selected"_fmt(ToPanelSkinTargetLabel(state.selectedTarget));
                    state.statusMessageUntil = (Scene::Time() + 2.5);
                }
            }

            if (MainSupport::DrawTextButton(defaultButton, U"Default"))
            {
                state.workingPath.clear();
                MainSupport::SetPanelNinePatchPath(state.selectedTarget, state.workingPath);
                state.statusMessage = U"{} uses default skin"_fmt(ToPanelSkinTargetLabel(state.selectedTarget));
                state.statusMessageUntil = (Scene::Time() + 2.5);
            }

            if (MainSupport::DrawTextButton(saveButton, U"Save TOML"))
            {
                state.statusMessage = MainSupport::SavePanelNinePatchPath(state.selectedTarget, state.workingPath)
                    ? U"Saved: {}"_fmt(MainSupport::PanelSkinSettingsPath)
                    : U"Save failed";
                state.statusMessageUntil = (Scene::Time() + 2.5);
                state.originalPath = state.workingPath;
            }

            if (MainSupport::DrawTextButton(closeButton, U"Close"))
            {
                MainSupport::SetPanelNinePatchPath(state.selectedTarget, state.originalPath);
                state.workingPath = state.originalPath;
                state.initializedForOpen = false;
                state.isOpen = false;
                return;
            }

            if (Scene::Time() < state.statusMessageUntil)
            {
                SimpleGUI::GetFont()(state.statusMessage).draw(panel.pos.movedBy(20, 372), ColorF{ 1.0, 0.94, 0.72, 0.96 });
            }

            if (KeyEscape.down())
            {
                MainSupport::SetPanelNinePatchPath(state.selectedTarget, state.originalPath);
                state.workingPath = state.originalPath;
                state.initializedForOpen = false;
                state.isOpen = false;
            }
        }

        [[nodiscard]] inline PanelSkinSelectorState& SharedPanelSkinSelectorState()
        {
            static PanelSkinSelectorState state;
            return state;
        }

        inline void OpenSharedPanelSkinSelector()
        {
            SharedPanelSkinSelectorState().isOpen = true;
        }

        inline void DrawSharedPanelSkinSelector()
        {
            DrawPanelSkinSelector(SharedPanelSkinSelectorState());
        }
    }
}
