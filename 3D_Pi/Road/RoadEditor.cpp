# include "../stdafx.h"
# include "RoadEditor.hpp"
# include "RoadEditorRenderer.hpp"

RoadEditor::RoadEditor(const FilePath& texturePath, const FilePath& savePath ,
        const FilePath& presetsDir )
        : m_textureSourcePath{ texturePath }


        , m_savePath{ savePath }


        , m_presetsDir{ presetsDir }

{
        load();
        m_presets = road::LoadAllPresets(m_presetsDir);
        m_panelPos = ui::editor_icon::GetDockedStackPosition(0);
    }



bool RoadEditor::isEnabled() const noexcept{
        return m_enabled;
    }

bool RoadEditor::isPanelOpen() const noexcept
{
    return (not m_uiCollapsed);
}

bool RoadEditor::handleCommand(const app::EditorCommand command)
{
    if (command == app::EditorCommand::Toggle)
    {
        m_enabled = (not m_enabled);
        m_statusMessage = (m_enabled ? U"Road Editor: ON" : U"Road Editor: OFF");
        return true;
    }

    if (not m_enabled)
    {
        return false;
    }

    switch (command)
    {
    case app::EditorCommand::Save:
        save();
        m_statusMessage = U"Road data saved";
        return true;
    case app::EditorCommand::Load:
        load();
        return true;
    case app::EditorCommand::Undo:
        undo();
        return true;
    case app::EditorCommand::Confirm:
        confirmEditingRoad();
        return true;
    case app::EditorCommand::Cancel:
        cancelEditingRoad();
        return true;
    case app::EditorCommand::ToggleGhost:
        toggleGhostVisible();
        return true;
    default:
        return false;
    }
}

bool RoadEditor::wantsMouseCapture() const{
      syncCollapsedIconRegistry();
        return isCursorOnUI();
    }



    void RoadEditor::update(const BasicCamera3D& camera){
        syncCollapsedIconRegistry();

        if (not m_enabled)
        {
            m_hoverPoint.reset();
            m_snapPoint.reset();
            m_hoverScatterItemIndex.reset();
            return;
        }

        m_hoverPoint = cursorToGround(camera);
        m_snapPoint = findSnapPoint(m_hoverPoint);

        if (KeyBackspace.down() && m_session.canRestore())
        {
            restoreSession();
        }

        if (MouseR.down() && (not isCursorOnUI()))
        {
            cancelEditingRoad();
        }

        if (m_activeTabIndex == 2)
        {
            updateScatterInteraction();
            return;
        }

        if ((not isCursorOnUI()) && m_hoverPoint && MouseL.pressed())
        {
            appendPoint(getCurrentInputPoint());
        }
    }



    void RoadEditor::draw3D() const{
        RoadEditorRenderer::Draw(*this);
    }



    void RoadEditor::drawRenderer3D() const{
        const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };

        drawGhostRoads3D();

        const size_t roadMeshCount = Min(m_roadMeshes.size(), m_roadShoulderMeshes.size());
        for (size_t i = 0; i < roadMeshCount; ++i)
        {
            if (m_roadShoulderMeshes[i])
            {
                m_roadShoulderMeshes[i]->draw(m_roadShoulderTexture, ColorF{ 1.0, 0.92, 0.88, 0.88 });
            }

            if (m_roadMeshes[i])
            {
                m_roadMeshes[i]->draw(m_roadTexture, ColorF{ 1.0 });
            }
        }

        for (size_t i = roadMeshCount; i < m_roadShoulderMeshes.size(); ++i)
        {
            if (m_roadShoulderMeshes[i])
            {
                m_roadShoulderMeshes[i]->draw(m_roadShoulderTexture, ColorF{ 1.0, 0.92, 0.88, 0.88 });
            }
        }

        for (size_t i = roadMeshCount; i < m_roadMeshes.size(); ++i)
        {
            if (m_roadMeshes[i])
            {
                m_roadMeshes[i]->draw(m_roadTexture, ColorF{ 1.0 });
            }
        }

        for (const auto& patch : m_connectionPatchMeshes)
        {
            patch.draw(m_roadShoulderTexture, ColorF{ 1.0, 0.96, 0.92, 0.90 });
            patch.draw(m_roadTexture, ColorF{ 1.0, 0.96, 0.92 });
        }

        drawScatterItems3D();

        if (m_editingMesh)
        {
            m_editingMesh->draw(m_roadShoulderTexture, ColorF{ 1.0, 0.92, 0.82, 0.80 });
            m_editingMesh->draw(m_roadTexture, ColorF{ 1.0, 0.92, 0.82 });
        }

        if (not m_enabled)
        {
            return;
        }

        drawPathGuide(m_editingRoad, Palette::Yellow, Palette::Orange);

        if (m_hoverPoint)
        {
            Sphere{ *m_hoverPoint + Vec3{ 0, 0.08, 0 }, 0.12 }.draw(Palette::Skyblue);

            if (m_snapPoint)
            {
                Sphere{ *m_snapPoint + Vec3{ 0, 0.12, 0 }, 0.20 }.draw(Palette::Lime);
                Line3D{ *m_hoverPoint + Vec3{ 0, 0.08, 0 }, *m_snapPoint + Vec3{ 0, 0.12, 0 } }.draw(Palette::Lime);
            }

            if (m_editingRoad && (not m_editingRoad->points.isEmpty()) && m_activeTabIndex != 2)
            {
                Line3D{ m_editingRoad->points.back() + Vec3{ 0, 0.05, 0 }, getCurrentInputPoint() + Vec3{ 0, 0.05, 0 } }.draw(Palette::Skyblue);
            }
        }

        if (m_activeTabIndex == 2)
        {
            drawScatterHoverGuide3D();
        }
    }



RectF RoadEditor::getPanelRect() const{
        if (m_uiCollapsed)
        {
          return getCollapsedIconRect();
        }

        const double panelHeight = Min(800.0, Scene::Height() - 40.0);
        const Vec2 clampedPos{
            Clamp(m_panelPos.x, 0.0, static_cast<double>(Scene::Width()) - 620.0),
            Clamp(m_panelPos.y, 0.0, static_cast<double>(Scene::Height()) - panelHeight)
        };
        return RectF{ clampedPos, 620, panelHeight };
    }



RectF RoadEditor::getCollapsedIconRect() const{
        const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"RoadEditor", m_panelPos);
        return RectF{ resolvedPos, ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize };
    }



void RoadEditor::syncCollapsedIconRegistry() const{
        ui::editor_icon::RegisterCollapsedIcon(U"RoadEditor", m_uiCollapsed ? Optional<RectF>{ getCollapsedIconRect() } : none);
    }



void RoadEditor::updateCollapsedIconDrag(const RectF& dragRect){
        const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
        m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"RoadEditor", desiredPos, dragRect.size);
        syncCollapsedIconRegistry();
    }



void RoadEditor::expandFromCollapsedIcon(){
        const RectF collapsedIcon = getCollapsedIconRect();
        const double panelHeight = Min(800.0, Scene::Height() - 40.0);
        m_uiCollapsed = false;
        m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(collapsedIcon, SizeF{ 620.0, panelHeight });
        m_ignoreCollapsedClickUntilRelease = false;
        syncCollapsedIconRegistry();
    }



bool RoadEditor::isCursorOnUI() const{
        return getPanelRect().mouseOver();
    }

