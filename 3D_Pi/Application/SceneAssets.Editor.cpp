# include "../stdafx.h"
# include "SceneAssets.hpp"
# include "../UI/RectUI.hpp"

namespace app
{
    void SceneAssets::updateEditor()
    {
        syncCollapsedIconRegistry();
        if (KeyH.down())
        {
            m_editorEnabled = (not m_editorEnabled);
            m_editorStatus = (m_editorEnabled ? U"Shadow Editor: ON" : U"Shadow Editor: OFF");
        }
    }

    void SceneAssets::drawEditorUI()
    {
       syncCollapsedIconRegistry();
        const RectF panel = getEditorPanelRect();

        if (m_editorCollapsed)
        {
         const RectF button = getCollapsedIconRect();

            if (MouseL.down() && button.mouseOver())
            {
                m_editorDragging = true;
               m_ignoreCollapsedClickUntilRelease = false;
                m_editorDragOffset = Cursor::PosF() - button.pos;
            }
            if (not MouseL.pressed())
            {
                m_editorDragging = false;
            }
            if (m_editorDragging)
            {
                if (Cursor::PosF().distanceFrom(button.pos + m_editorDragOffset) > 3.0)
                {
                    m_ignoreCollapsedClickUntilRelease = true;
                }
                updateCollapsedIconDrag(button);
            }

            button.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            button.drawFrame(2.0, Palette::Black);
            if (m_toggleIcon)
            {
                const double iconScale = Min(button.w / m_toggleIcon.width(), button.h / m_toggleIcon.height());
                m_toggleIcon.scaled(iconScale).drawAt(button.center());
            }

           if ((not m_ignoreCollapsedClickUntilRelease) && button.leftClicked())
            {
                m_editorCollapsed = false;
             m_editorPanelPos = Vec2{ 20, 20 };
             m_ignoreCollapsedClickUntilRelease = false;
               syncCollapsedIconRegistry();
            }
          if (not MouseL.pressed())
            {
                m_ignoreCollapsedClickUntilRelease = false;
            }
         if (not m_toggleIcon)
            {
                m_editorFont(U"H").drawAt(button.center(), ui::GetTheme().text);
            }
            return;
        }

        ui::Panel(panel);

        m_editorFont(U"Shadow Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
        m_editorSmallFont(U"H : toggle update").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

        const RectF collapseButton{ panel.x + panel.w - 44, panel.y + 10, 28, 28 };
     const RectF dragHeader{ panel.x, panel.y, panel.w, 42 };
        if (MouseL.down() && dragHeader.mouseOver() && (not collapseButton.mouseOver()))
        {
            m_editorDragging = true;
         m_ignoreCollapsedClickUntilRelease = true;
            m_editorDragOffset = Cursor::PosF() - m_editorPanelPos;
        }
       if (MouseL.down() && collapseButton.mouseOver())
        {
            m_editorDragging = true;
            m_ignoreCollapsedClickUntilRelease = false;
            m_togglePressCursor = Cursor::PosF();
            m_editorDragOffset = Cursor::PosF() - m_editorPanelPos;
        }
        if (not MouseL.pressed())
        {
            m_editorDragging = false;
        }
        if (m_editorDragging)
        {
            m_editorPanelPos = Cursor::PosF() - m_editorDragOffset;
            m_editorPanelPos.x = Clamp(m_editorPanelPos.x, 0.0, Max(0.0, Scene::Width() - EditorPanelWidth));
            m_editorPanelPos.y = Clamp(m_editorPanelPos.y, 0.0, Max(0.0, Scene::Height() - panel.h));
          if (Cursor::PosF().distanceFrom(m_togglePressCursor) > 3.0)
            {
                m_ignoreCollapsedClickUntilRelease = true;
            }
        }
     if ((not m_ignoreCollapsedClickUntilRelease) && ui::Button(m_editorFont, U"◀", collapseButton))
        {
           m_editorPanelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", Vec2{ panel.x, panel.y });
            m_editorCollapsed = true;
           syncCollapsedIconRegistry();
        }
        if (not MouseL.pressed())
        {
            m_ignoreCollapsedClickUntilRelease = false;
        }

        if (m_models.isEmpty())
        {
            m_editorSmallFont(U"No scene models").draw(panel.pos.movedBy(16, 82), ui::GetTheme().textMuted);
            return;
        }

        m_selectedModelIndex = Min(m_selectedModelIndex, m_models.size() - 1);

        Array<String> modelNames;
        modelNames.reserve(m_models.size());
        for (const auto& model : m_models)
        {
            modelNames << model.settings.id;
        }

        const RectF listSection{ panel.x + 14, panel.y + 72, panel.w - 28, 188 };
        ui::Section(listSection);
        m_editorSmallFont(U"Model").draw(listSection.pos.movedBy(12, 8), ui::GetTheme().textMuted);
        ui::RadioList(m_editorSmallFont, m_selectedModelIndex, modelNames, RectF{ listSection.x + 12, listSection.y + 30, listSection.w - 24, 148 }, 30.0);

        auto& selected = m_models[m_selectedModelIndex];
        auto& selectedSettings = m_settings.models[m_selectedModelIndex];

        const RectF propsSection{ panel.x + 14, panel.y + 268, panel.w - 28, 272 };
        ui::Section(propsSection);
        m_editorSmallFont(U"Shadow Params").draw(propsSection.pos.movedBy(12, 8), ui::GetTheme().textMuted);

        const Vec2 base{ propsSection.x + 12, propsSection.y + 34 };
        const double labelWidth = 150.0;
        const double sliderWidth = propsSection.w - 38;

        bool changed = false;
        changed |= ui::SliderH(U"Size X", selected.shadowSize.x, 0.2, 8.0, base + Vec2{ 0, 0 }, labelWidth, sliderWidth - labelWidth);
        changed |= ui::SliderH(U"Size Z", selected.shadowSize.y, 0.2, 8.0, base + Vec2{ 0, 36 }, labelWidth, sliderWidth - labelWidth);
        changed |= ui::SliderH(U"Offset X", selected.shadowOffsetXZ.x, -1.5, 1.5, base + Vec2{ 0, 72 }, labelWidth, sliderWidth - labelWidth);
        changed |= ui::SliderH(U"Offset Z", selected.shadowOffsetXZ.y, -1.5, 1.5, base + Vec2{ 0, 108 }, labelWidth, sliderWidth - labelWidth);
        changed |= ui::SliderH(U"Opacity", selected.shadowOpacity, 0.0, 0.9, base + Vec2{ 0, 144 }, labelWidth, sliderWidth - labelWidth);

        const RectF projectedToggle{ propsSection.x + 12, propsSection.y + 214, propsSection.w - 24, 34 };
        if (ui::Button(m_editorSmallFont, selected.useProjectedShadow ? U"Projected Shadow: ON" : U"Projected Shadow: OFF", projectedToggle))
        {
            selected.useProjectedShadow = (not selected.useProjectedShadow);
            changed = true;
        }

        if (changed)
        {
            selected.shadowOpacity = Clamp(selected.shadowOpacity, 0.0, 0.9);
            selected.settings.shadowSizeXZ = selected.shadowSize;
            selected.settings.shadowOffsetXZ = selected.shadowOffsetXZ;
            selected.settings.shadowOpacity = selected.shadowOpacity;
            selected.settings.projectedShadow = selected.useProjectedShadow;

            selectedSettings.shadowSizeXZ = selected.shadowSize;
            selectedSettings.shadowOffsetXZ = selected.shadowOffsetXZ;
            selectedSettings.shadowOpacity = selected.shadowOpacity;
            selectedSettings.projectedShadow = selected.useProjectedShadow;
            m_editorStatus = U"Updated {} shadow params"_fmt(selected.settings.id);
        }

        const RectF saveButton{ panel.x + 14, panel.y + panel.h - 82, panel.w - 28, 34 };
        if (ui::Button(m_editorSmallFont, U"Save scene_assets.toml", saveButton))
        {
            saveSettingsToConfig();
            m_editorStatus = U"Saved shadow settings";
        }

        m_editorSmallFont(m_editorStatus).draw(panel.pos.movedBy(16, panel.h - 40), ui::GetTheme().textMuted);
    }

    bool SceneAssets::wantsMouseCapture() const
    {
        syncCollapsedIconRegistry();
        if (not m_editorEnabled)
        {
            return false;
        }

        return getEditorPanelRect().mouseOver();
    }

    RectF SceneAssets::getEditorPanelRect() const
    {
      if (m_editorCollapsed)
        {
            return getCollapsedIconRect();
        }

        const double panelHeight = Min(610.0, Scene::Height() - 40.0);
        const Vec2 clampedPos{
            Clamp(m_editorPanelPos.x, 0.0, Max(0.0, Scene::Width() - EditorPanelWidth)),
            Clamp(m_editorPanelPos.y, 0.0, Max(0.0, Scene::Height() - panelHeight))
        };
        return RectF{ clampedPos, EditorPanelWidth, panelHeight };
    }

    RectF SceneAssets::getCollapsedIconRect() const
    {
        const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", m_editorPanelPos);
        return RectF{ resolvedPos, CollapsedIconSize, CollapsedIconSize };
    }

    void SceneAssets::syncCollapsedIconRegistry() const
    {
        ui::editor_icon::RegisterCollapsedIcon(U"ShadowEditor", m_editorCollapsed ? Optional<RectF>{ getCollapsedIconRect() } : none);
    }

    void SceneAssets::updateCollapsedIconDrag(const RectF& dragRect)
    {
        const Vec2 desiredPos = (Cursor::PosF() - m_editorDragOffset);
        m_editorPanelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ShadowEditor", desiredPos, dragRect.size);
        syncCollapsedIconRegistry();
    }
}
