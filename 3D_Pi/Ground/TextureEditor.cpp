# include "../stdafx.h"
# include "TextureEditor.hpp"

TextureEditor::TextureEditor(const FilePath& savePath )
        : m_savePath{ savePath }
        , m_document{}
        , m_autoYOffset{ m_document.autoYOffset }
        , m_autoYOffsetStep{ m_document.autoYOffsetStep }
        , m_baseYOffset{ m_document.baseYOffset }
        , m_layers{ m_document.layers }
        , m_overlayPlane{ MeshData::OneSidedPlane(1.0, { 1, 1 }) }

{
        scanAvailableTextures();
        load();
        m_lastPlacementReason = U"idle";
        m_panelPos = ui::editor_icon::GetDockedStackPosition(1);
    }

bool TextureEditor::isEnabled() const noexcept
{
    return m_enabled;
}

bool TextureEditor::isPanelOpen() const noexcept
{
    return (not m_uiCollapsed);
}

bool TextureEditor::handleCommand(const app::EditorCommand command)
{
    if (command == app::EditorCommand::Toggle)
    {
        m_enabled = (not m_enabled);
        m_statusMessage = (m_enabled ? U"Texture Editor: ON" : U"Texture Editor: OFF");
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
        m_statusMessage = U"Saved";
        return true;
    case app::EditorCommand::Load:
        load();
        return true;
    case app::EditorCommand::Undo:
        undoLastLayer();
        return true;
    case app::EditorCommand::Duplicate:
        duplicateSelectedLayer();
        return true;
    case app::EditorCommand::DeleteSelection:
        if (hasSelectedLayer())
        {
            removeLayer(m_selectedLayerIndex);
            return true;
        }
        return false;
    default:
        return false;
    }
}



bool TextureEditor::wantsMouseCapture() const{
       syncCollapsedIconRegistry();
       return getPanelRect().mouseOver();
    }



bool TextureEditor::wantsMouseWheelCapture() const{
        if (not m_enabled || m_uiCollapsed)
        {
            return false;
        }

        if (m_activeTabIndex == 0)
        {
            return getLayerListSectionRect().mouseOver();
        }

        if (m_activeTabIndex == 1)
        {
            return getTextureListRect().mouseOver();
        }

        return false;
    }



    void TextureEditor::update(const BasicCamera3D& camera){
        syncCollapsedIconRegistry();

        m_lastCursorScreenPos = Cursor::Pos();
        m_lastCursorGroundPos = cursorToGround(camera);
        m_lastPlacementPanelHover = getPanelRect().mouseOver();
        m_lastPlacementClickSeen = MouseL.down();
        m_lastPlacementApplied = false;

        if (m_placeAtClickRequested)
        {
            if (not hasSelectedLayer())
            {
                m_placeAtClickRequested = false;
                m_lastPlacementReason = U"canceled: no selected layer";
                m_statusMessage = U"Placement canceled: no selected layer";
            }
            else if (m_lastPlacementClickSeen)
            {
                if (m_lastPlacementPanelHover)
                {
                    m_lastPlacementReason = U"blocked: cursor is over the editor panel";
                }
                else if (not m_lastCursorGroundPos)
                {
                    m_lastPlacementReason = U"blocked: cursor ray did not hit the ground plane";
                }
                else
                {
                    applyPlacement(*m_lastCursorGroundPos, U"scene click");
                }
            }
            else
            {
                m_lastPlacementReason = m_enabled
                    ? U"armed: waiting for the next left click on the ground"
                    : U"armed: waiting for the next left click (editor update is OFF)";
            }
        }
        else
        {
            m_lastPlacementReason = U"idle";
        }

        if (not m_enabled)
        {
            return;
        }

    }



    void TextureEditor::draw3D(){
        ensureAllLayerTextures();

        for (size_t i = 0; i < m_layers.size(); ++i)
        {
            const auto& layer = m_layers[i];
            if (not layer.visible || not m_layerTextures[i])
            {
                continue;
            }

            const double finalY = m_autoYOffset
                ? (m_baseYOffset + static_cast<double>(i) * m_autoYOffsetStep)
                : layer.yOffset;

            const ScopedRenderStates3D renderState{ BlendState::NonPremultiplied, RasterizerState::SolidCullNone };
            const Transformer3D transform{
                Mat4x4::Identity()
                    .scaled(Float3{ static_cast<float>(layer.size.x), 1.0f, static_cast<float>(layer.size.y) })
                    .rotated(Quaternion::RotateY(static_cast<float>(layer.rotation)))
                    .translated(Vec3{ layer.position.x, finalY, layer.position.y })
            };
            m_overlayPlane.draw(m_layerTextures[i]);
        }
    }



RectF TextureEditor::getPanelRect() const{
        if (m_uiCollapsed)
        {
          return getCollapsedIconRect();
        }
        const double panelHeight = Min(PanelHeight, Scene::Height() - 40.0);
        const Vec2 clampedPos{
            Clamp(m_panelPos.x, 0.0, static_cast<double>(Scene::Width()) - PanelWidth),
            Clamp(m_panelPos.y, 0.0, static_cast<double>(Scene::Height()) - panelHeight)
        };
        return RectF{ clampedPos, PanelWidth, panelHeight };
    }



RectF TextureEditor::getCollapsedIconRect() const{
        const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"TextureEditor", m_panelPos);
        return RectF{ resolvedPos, ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize };
    }



void TextureEditor::syncCollapsedIconRegistry() const{
        ui::editor_icon::RegisterCollapsedIcon(U"TextureEditor", m_uiCollapsed ? Optional<RectF>{ getCollapsedIconRect() } : none);
    }



void TextureEditor::updateCollapsedIconDrag(const RectF& dragRect){
        const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
        m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"TextureEditor", desiredPos, dragRect.size);
        syncCollapsedIconRegistry();
    }



void TextureEditor::expandFromCollapsedIcon(){
        const RectF collapsedIcon = getCollapsedIconRect();
        const double panelHeight = Min(PanelHeight, Scene::Height() - 40.0);
        m_uiCollapsed = false;
        m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(collapsedIcon, SizeF{ PanelWidth, panelHeight });
        m_ignoreCollapsedClickUntilRelease = false;
        syncCollapsedIconRegistry();
    }
