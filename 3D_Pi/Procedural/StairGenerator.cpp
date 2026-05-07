# include "../stdafx.h"
# include "StairGenerator.hpp"

namespace procedural
{
    StairGenerator::StairGenerator()
    {
        load();
    }

    bool StairGenerator::isEnabled() const noexcept
    {
        return true;
    }

    bool StairGenerator::isPanelOpen() const noexcept
    {
        return m_panelOpen;
    }

    bool StairGenerator::handleCommand(const app::EditorCommand command)
    {
        switch (command)
        {
        case app::EditorCommand::Save:
            save();
            return true;
        case app::EditorCommand::Load:
            load();
            return true;
        case app::EditorCommand::Cancel:
            cancelTargetSelection();
            return true;
        default:
            return false;
        }
    }

    void StairGenerator::save() const
    {
        SaveProceduralData(m_savePath, m_document);
    }

    void StairGenerator::load()
    {
        LoadProceduralData(m_savePath, m_document);
        m_selectedIndex.reset();

        uint32 nextSerial = 1;
        for (const auto& naturalObject : m_naturalObjects)
        {
            nextSerial = Max(nextSerial, naturalObject.serial + 1);
        }
        m_nextNatureSerial = nextSerial;
    }

    bool StairGenerator::wantsMouseCapture() const{
            syncCollapsedIconRegistry();
            return (not m_uiHidden) && getPanelRect().mouseOver();
        }



        void StairGenerator::setUIHidden(const bool hidden){
            m_uiHidden = hidden;
            if (m_uiHidden)
            {
                m_dragging = false;
            }
        }



        void StairGenerator::cancelTargetSelection(){
            m_waitingForPosition = false;
        }



        void StairGenerator::update(const BasicCamera3D& camera, const bool cursorBlockedByUI){
            if (m_uiHidden)
            {
                m_dragging = false;
                return;
            }

            syncCollapsedIconRegistry();

            if (not m_panelOpen)
            {
                const RectF collapsedIcon = getCollapsedIconRect();
                if (MouseR.down() && collapsedIcon.mouseOver())
                {
                    m_dragging = true;
                    m_ignoreCollapsedClickUntilRelease = false;
                    m_dragOffset = (Cursor::PosF() - collapsedIcon.pos);
                }

                if (m_dragging)
                {
                    if (MouseR.pressed())
                    {
                        updateCollapsedIconDrag(collapsedIcon);
                    }
                    else
                    {
                        m_dragging = false;
                        m_ignoreCollapsedClickUntilRelease = false;
                    }
                }

                return;
            }

            const RectF panelRect = getPanelRect();
            const RectF headerRect{ panelRect.x, panelRect.y, panelRect.w, HeaderHeight };
            const RectF toggleRect{ panelRect.x + panelRect.w - 38, panelRect.y + 7, 28, 28 };

            if (MouseL.down() && headerRect.mouseOver() && (not toggleRect.mouseOver()))
            {
                m_dragging = true;
                m_dragOffset = (Cursor::PosF() - m_panelPos);
            }

            if (MouseR.down() && toggleRect.mouseOver())
            {
                m_dragging = true;
                m_dragOffset = (Cursor::PosF() - m_panelPos);
            }

            if (m_dragging)
            {
                if (MouseL.pressed() || MouseR.pressed())
                {
                    m_panelPos = Cursor::PosF() - m_dragOffset;
                    m_panelPos.x = Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - panelRect.w));
                    m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panelRect.h));
                }
                else
                {
                    m_dragging = false;
                }
            }

            if (m_waitingForPosition && MouseL.down() && (not cursorBlockedByUI))
            {
                const InfinitePlane gp{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };
                const Ray ray = camera.screenToRay(Cursor::PosF());
                if (const auto distance = ray.intersects(gp))
                {
                    m_generatePosition = ray.point_at(*distance);
                    m_waitingForPosition = false;
                }
            }
        }
}
