# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "../UI/EditorIconLayout.hpp"
# include "../UI/RectUI.hpp"
# include "SceneAssets.hpp"

namespace app
{
    class SceneAssetsEditorAddon final : public IEditorAddon
    {
    public:
        explicit SceneAssetsEditorAddon(SceneAssets& sceneAssets)
            : m_sceneAssets{ sceneAssets } {}

        const EditorAddonDescriptor& descriptor() const noexcept override
        {
            static const EditorAddonDescriptor descriptor{
                U"SceneAssetsEditor",
                U"Scene Assets Editor",
                Optional<Input>{ KeyH },
                0,
                0,
                0,
                0
            };
            return descriptor;
        }

        void update(const EditorUpdateContext&) override
        {
            syncCollapsedIconRegistry();
            if (not m_enabled)
            {
                m_dragging = false;
            }
        }

        void draw3D(const EditorDraw3DContext&) override
        {
        }

        void drawUI(const EditorUIContext& context) override
        {
            if (context.uiHidden)
            {
                syncCollapsedIconRegistry();
                return;
            }

            syncCollapsedIconRegistry();
            const RectF panel = getEditorPanelRect();

            if (m_editorCollapsed)
            {
                const RectF button = getCollapsedIconRect();

                if (MouseR.down() && button.mouseOver())
                {
                    m_dragging = true;
                    m_dragOffset = Cursor::PosF() - button.pos;
                }
                if (not MouseR.pressed())
                {
                    m_dragging = false;
                }
                if (m_dragging)
                {
                    updateCollapsedIconDrag(button);
                }

                button.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
                button.drawFrame(2.0, Palette::Black);
                ui::editor_icon::DrawToggleIcon(m_toggleIcon, button);

                if (button.leftClicked())
                {
                    const double panelHeight = Min(610.0, Scene::Height() - 40.0);
                    m_editorCollapsed = false;
                    m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(button, SizeF{ EditorPanelWidth, panelHeight });
                    syncCollapsedIconRegistry();
                }
                if (not m_toggleIcon)
                {
                    m_font(U"H").drawAt(button.center(), ui::GetTheme().text);
                }
                return;
            }

            ui::Panel(panel);

            m_font(U"Shadow Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
            m_smallFont(U"H : toggle editor").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

            const RectF collapseButton{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
            const RectF dragHeader{ panel.x, panel.y, panel.w, 42 };
            if (MouseL.down() && dragHeader.mouseOver() && (not collapseButton.mouseOver()))
            {
                m_dragging = true;
                m_dragOffset = Cursor::PosF() - m_panelPos;
            }
            if (MouseR.down() && collapseButton.mouseOver())
            {
                m_dragging = true;
                m_dragOffset = Cursor::PosF() - m_panelPos;
            }
            if (not (MouseL.pressed() || MouseR.pressed()))
            {
                m_dragging = false;
            }
            if (m_dragging)
            {
                m_panelPos = Cursor::PosF() - m_dragOffset;
                m_panelPos.x = Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - EditorPanelWidth));
                m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panel.h));
            }
            collapseButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            collapseButton.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseButton);
            if (collapseButton.leftClicked())
            {
                const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                    collapseButton, SizeF{ CollapsedIconSize, CollapsedIconSize });
                m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", desiredCollapsedPos);
                m_editorCollapsed = true;
                syncCollapsedIconRegistry();
            }

            if (m_sceneAssets.editableModelCount() == 0)
            {
                m_smallFont(U"No scene models").draw(panel.pos.movedBy(16, 82), ui::GetTheme().textMuted);
                return;
            }

            m_selectedModelIndex = Min(m_selectedModelIndex, m_sceneAssets.editableModelCount() - 1);

            const Array<String> modelNames = m_sceneAssets.editableModelNames();

            const RectF listSection{ panel.x + 14, panel.y + 72, panel.w - 28, 188 };
            ui::Section(listSection);
            m_smallFont(U"Model").draw(listSection.pos.movedBy(12, 8), ui::GetTheme().textMuted);
            ui::RadioList(m_smallFont, m_selectedModelIndex, modelNames, RectF{ listSection.x + 12, listSection.y + 30, listSection.w - 24, 148 }, 30.0);

            auto selected = m_sceneAssets.getEditableModel(m_selectedModelIndex);
            if (not selected)
            {
                return;
            }

            const RectF propsSection{ panel.x + 14, panel.y + 268, panel.w - 28, 272 };
            ui::Section(propsSection);
            m_smallFont(U"Shadow Params").draw(propsSection.pos.movedBy(12, 8), ui::GetTheme().textMuted);

            const Vec2 base{ propsSection.x + 12, propsSection.y + 34 };
            const double labelWidth = 150.0;
            const double sliderWidth = propsSection.w - 38;

            bool changed = false;
            changed |= ui::SliderH(U"Size X", selected->shadowSize.x, 0.2, 8.0, base + Vec2{ 0, 0 }, labelWidth, sliderWidth - labelWidth);
            changed |= ui::SliderH(U"Size Z", selected->shadowSize.y, 0.2, 8.0, base + Vec2{ 0, 36 }, labelWidth, sliderWidth - labelWidth);
            changed |= ui::SliderH(U"Offset X", selected->shadowOffsetXZ.x, -1.5, 1.5, base + Vec2{ 0, 72 }, labelWidth, sliderWidth - labelWidth);
            changed |= ui::SliderH(U"Offset Z", selected->shadowOffsetXZ.y, -1.5, 1.5, base + Vec2{ 0, 108 }, labelWidth, sliderWidth - labelWidth);
            changed |= ui::SliderH(U"Opacity", selected->shadowOpacity, 0.0, 0.9, base + Vec2{ 0, 144 }, labelWidth, sliderWidth - labelWidth);

            const RectF projectedToggle{ propsSection.x + 12, propsSection.y + 214, propsSection.w - 24, 34 };
            if (ui::Button(m_smallFont, selected->useProjectedShadow ? U"Projected Shadow: ON" : U"Projected Shadow: OFF", projectedToggle))
            {
                selected->useProjectedShadow = (not selected->useProjectedShadow);
                changed = true;
            }

            if (changed && m_sceneAssets.applyEditableModel(m_selectedModelIndex, *selected))
            {
                m_status = U"Updated {} shadow params"_fmt(selected->id);
            }

            const RectF saveButton{ panel.x + 14, panel.y + panel.h - 82, panel.w - 28, 34 };
            if (ui::Button(m_smallFont, U"Save scene_assets.toml", saveButton))
            {
                m_status = m_sceneAssets.saveSettingsToConfig()
                    ? U"Saved shadow settings"
                    : U"Failed to save shadow settings";
            }

            m_smallFont(m_status).draw(panel.pos.movedBy(16, panel.h - 40), ui::GetTheme().textMuted);
        }

        bool wantsMouseCapture() const override
        {
            syncCollapsedIconRegistry();
            if (not m_enabled)
            {
                return false;
            }

            return getEditorPanelRect().mouseOver();
        }

        bool wantsMouseWheelCapture() const override
        {
            return false;
        }

        bool isEnabled() const override
        {
            return true;
        }

        bool isPanelOpen() const override
        {
            return (not m_editorCollapsed);
        }

        bool handleCommand(EditorCommand command) override
        {
            switch (command)
            {
            case EditorCommand::Toggle:
                m_enabled = (not m_enabled);
                m_dragging = false;
                m_status = (m_enabled ? U"Shadow Editor: ON" : U"Shadow Editor: OFF");
                syncCollapsedIconRegistry();
                return true;
            case EditorCommand::Save:
                if (not m_enabled)
                {
                    return false;
                }

                m_status = m_sceneAssets.saveSettingsToConfig()
                    ? U"Saved shadow settings"
                    : U"Failed to save shadow settings";
                return true;
            default:
                return false;
            }
        }

    private:
        static constexpr double EditorPanelWidth = 420.0;
        static constexpr double CollapsedIconSize = ui::editor_icon::CollapsedIconSize;

        [[nodiscard]] RectF getEditorPanelRect() const
        {
            if (m_editorCollapsed)
            {
                return getCollapsedIconRect();
            }

            const double panelHeight = Min(610.0, Scene::Height() - 40.0);
            const Vec2 clampedPos{
                Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - EditorPanelWidth)),
                Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panelHeight))
            };
            return RectF{ clampedPos, EditorPanelWidth, panelHeight };
        }

        [[nodiscard]] RectF getCollapsedIconRect() const
        {
            const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", m_panelPos);
            return RectF{ resolvedPos, CollapsedIconSize, CollapsedIconSize };
        }

        void syncCollapsedIconRegistry() const
        {
            ui::editor_icon::RegisterCollapsedIcon(
                U"ShadowEditor",
                (m_editorCollapsed ? Optional<RectF>{ getCollapsedIconRect() } : none));
        }

        void updateCollapsedIconDrag(const RectF& dragRect)
        {
            const Vec2 desiredPos = (Cursor::PosF() - m_dragOffset);
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", desiredPos, dragRect.size);
            syncCollapsedIconRegistry();
        }

        SceneAssets& m_sceneAssets;
        Font m_font{ 18 };
        Font m_smallFont{ 12 };
        bool m_enabled = false;
        bool m_editorCollapsed = true;
        bool m_dragging = false;
        Vec2 m_panelPos{ ui::editor_icon::GetDockedStackPosition(2) };
        Vec2 m_dragOffset{ 0, 0 };
        Texture m_toggleIcon{ U"texture/shadowEditor.png" };
        size_t m_selectedModelIndex = 0;
        String m_status = U"Shadow Editor: Ready";
    };
}
