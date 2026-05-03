# pragma once
# include <Siv3D.hpp>
# include "../UI/RectUI.hpp"

struct GroundLayer
{
    String id;
    String label;
    FilePath texturePath;
    int32 categoryIndex = 0; // 0=Grass 1=Dirt 2=Plaza 3=Brick 4=Stone 5=Decal
    Vec2 position{ 0.0, 0.0 }; // world X, Z
    Vec2 size{ 10.0, 10.0 };
    double rotation = 0.0; // radians
    double yOffset = 0.014;
    ColorF tint{ 1.0 };
    double tilingScale = 4.0;
    double edgeSoftness = 0.12;
    double edgeNoiseAmount = 0.04;
    double edgeNoiseFrequency = 3.0;
    uint64 edgeNoiseSeed = 42;
    bool visible = true;
};

class TextureEditor
{
public:
    explicit TextureEditor(const FilePath& savePath = U"data/ground_layers.toml")
        : m_savePath{ savePath }
        , m_overlayPlane{ MeshData::OneSidedPlane(1.0, { 1, 1 }) }
    {
        scanAvailableTextures();
        load();
        m_lastPlacementReason = U"idle";
    }

    [[nodiscard]] bool wantsMouseCapture() const
    {
        return (not m_uiCollapsed) && getPanelRect().mouseOver();
    }

    [[nodiscard]] bool wantsMouseWheelCapture() const
    {
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

    void update(const BasicCamera3D& camera)
    {
        if (KeyT.down())
        {
            m_enabled = (not m_enabled);
            m_statusMessage = (m_enabled ? U"Texture Editor: ON" : U"Texture Editor: OFF");
        }

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

        if (KeyS.down())
        {
            save();
            m_statusMessage = U"Saved";
        }

        if (KeyL.down())
        {
            load();
        }

        if (KeyControl.pressed() && KeyZ.down())
        {
            undoLastLayer();
        }

        if (KeyControl.pressed() && KeyD.down())
        {
            duplicateSelectedLayer();
        }

        if (KeyDelete.down() && hasSelectedLayer())
        {
            removeLayer(m_selectedLayerIndex);
        }

    }

    void draw3D()
    {
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

    void drawUI()
    {
        m_hoverTooltip.clear();
        const RectF panel = getPanelRect();
        ui::Panel(panel);

        if (m_uiCollapsed)
        {
            const RectF btn{ panel.x + 6, panel.y + 6, panel.w - 12, panel.h - 12 };
            if (ui::Button(m_font, U"T", btn))
            {
                m_uiCollapsed = false;
            }
            setTooltipIfHovered(btn.mouseOver(), U"Expand Texture Editor (T to toggle)");
            if (not m_hoverTooltip.isEmpty())
            {
                ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
            }
            return;
        }

        m_font(U"Texture Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
        m_font(U"Ground layer painting").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

        const RectF stateChip{ panel.x + 268, panel.y + 14, 112, 24 };
        stateChip.rounded(12).draw(m_enabled ? ColorF{ 0.86, 0.94, 0.88, 0.96 } : ColorF{ 0.96, 0.90, 0.86, 0.96 });
        stateChip.rounded(12).drawFrame(1.0, ui::GetTheme().panelBorder);
        m_smallFont(U"Update: {}"_fmt(m_enabled ? U"ON" : U"OFF")).drawAt(stateChip.center(), ColorF{ 0.18, 0.26, 0.42 });
        setTooltipIfHovered(stateChip.mouseOver(), U"T toggles update-time editor behavior.");

        const RectF collapseBtn{ panel.x + panel.w - 44, panel.y + 10, 28, 28 };
        if (ui::Button(m_font, U"◀", collapseBtn))
        {
            m_uiCollapsed = true;
        }
        setTooltipIfHovered(collapseBtn.mouseOver(), U"Collapse Texture Editor");

        const RectF tabRow{ panel.x + 14, panel.y + 72, panel.w - 28, 34 };
        const double tg = 8.0;
        const double tw = (tabRow.w - tg * 2.0) / 3.0;
        const RectF layersTab{ tabRow.x, tabRow.y, tw, tabRow.h };
        const RectF propsTab{ tabRow.x + tw + tg, tabRow.y, tw, tabRow.h };
        const RectF edgeTab{ tabRow.x + (tw + tg) * 2.0, tabRow.y, tw, tabRow.h };

        if (ui::Button(m_font, U"Layers", layersTab))
        {
            m_activeTabIndex = 0;
        }
        if (ui::Button(m_font, U"Properties", propsTab))
        {
            m_activeTabIndex = 1;
        }
        if (ui::Button(m_font, U"Edge", edgeTab))
        {
            m_activeTabIndex = 2;
        }

        const RectF activeTabRect = (m_activeTabIndex == 0 ? layersTab : m_activeTabIndex == 1 ? propsTab : edgeTab);
        activeTabRect.rounded(6).drawFrame(2.0, ui::GetTheme().accent);

        if (m_activeTabIndex == 0)
        {
            drawLayersTab(panel);
        }
        else if (m_activeTabIndex == 1)
        {
            drawPropertiesTab(panel);
        }
        else
        {
            drawEdgeTab(panel);
        }

        if (not m_hoverTooltip.isEmpty())
        {
            ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
        }
    }

private:
    static constexpr int32 TexRes = 256;
    static constexpr double PanelWidth = 440.0;
    static constexpr double PanelHeight = 730.0;

    FilePath m_savePath;
    Font m_font{ 18 };
    Font m_smallFont{ 12 };
    bool m_enabled = false;
    bool m_uiCollapsed = false;
    String m_statusMessage = U"Ready";
    size_t m_activeTabIndex = 0;
    size_t m_selectedLayerIndex = 0;
    Array<GroundLayer> m_layers;
    Array<DynamicTexture> m_layerTextures;
    Array<bool> m_layerDirty;
    HashTable<FilePath, Image> m_sourceImageCache;
    Mesh m_overlayPlane;
    Array<FilePath> m_availableTextures;
    String m_hoverTooltip;
    bool m_autoYOffset = true;
    double m_autoYOffsetStep = 0.003;
    double m_baseYOffset = 0.002;
    bool m_placeAtClickRequested = false;
    double m_layerListScroll = 0.0;
    double m_textureListScroll = 0.0;
    Point m_lastCursorScreenPos{ 0, 0 };
    Optional<Vec2> m_lastCursorGroundPos;
    bool m_lastPlacementPanelHover = false;
    bool m_lastPlacementClickSeen = false;
    bool m_lastPlacementApplied = false;
    String m_lastPlacementReason;
    double m_placementDiagnosticsCopiedUntil = -1.0;

    [[nodiscard]] RectF getPanelRect() const
    {
        if (m_uiCollapsed)
        {
            return RectF{ 20, 192, 48, 48 };
        }
        return RectF{ 20.0, 20.0, PanelWidth, Min(PanelHeight, Scene::Height() - 40.0) };
    }

    [[nodiscard]] Optional<Vec2> cursorToGround(const BasicCamera3D& camera) const
    {
        const Ray ray = camera.screenToRay(Cursor::PosF());
        const InfinitePlane groundPlane3D{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

        if (const auto distance = ray.intersects(groundPlane3D))
        {
            const Vec3 hitPos = ray.point_at(*distance);
            return Vec2{ hitPos.x, hitPos.z };
        }

        return none;
    }

    [[nodiscard]] bool hasSelectedLayer() const
    {
        return (not m_layers.isEmpty()) && (m_selectedLayerIndex < m_layers.size());
    }

    [[nodiscard]] RectF getLayerListSectionRect() const
    {
        const RectF panel = getPanelRect();
        const double cx = panel.x + 14;
        const double cy = panel.y + 166;
        const double cw = panel.w - 28;
        constexpr double RowHeight = 40.0;
        const double maxListH = Min(static_cast<double>(Max<size_t>(m_layers.size(), 1)) * RowHeight + 36.0, 340.0);
        return RectF{ cx, cy, cw, maxListH };
    }

    [[nodiscard]] RectF getTextureListRect() const
    {
        const RectF panel = getPanelRect();
        const double cx = panel.x + 14;
        const double cy = panel.y + 660;
        const double cw = panel.w - 28;
        return RectF{ cx + 12, cy + 92, cw - 24, 132 };
    }

    [[nodiscard]] double getLayerListMaxScroll() const
    {
        constexpr double RowHeight = 40.0;
        const RectF listSection = getLayerListSectionRect();
        const double visibleListHeight = Max(0.0, listSection.h - 36.0);
        const double contentHeight = static_cast<double>(m_layers.size()) * RowHeight;
        return Max(0.0, contentHeight - visibleListHeight);
    }

    void setTooltipIfHovered(const bool hovered, const StringView text)
    {
        if (hovered)
        {
            m_hoverTooltip = text;
        }
    }

    void applyPlacement(const Vec2& groundPos, const StringView source)
    {
        if (not hasSelectedLayer())
        {
            m_lastPlacementReason = U"blocked: no selected layer";
            return;
        }

        m_layers[m_selectedLayerIndex].position = groundPos;
        markLayerDirty(m_selectedLayerIndex);
        m_placeAtClickRequested = false;
        m_lastPlacementApplied = true;
        m_lastPlacementReason = U"applied via {}"_fmt(source);
        m_statusMessage = U"Placed texture at ({:.2f}, {:.2f}) via {}"_fmt(groundPos.x, groundPos.y, source);
    }

    [[nodiscard]] String buildPlacementDiagnostics() const
    {
        const String selectedLayer = hasSelectedLayer()
            ? U"{} (# {})"_fmt(m_layers[m_selectedLayerIndex].label, m_selectedLayerIndex)
            : U"(none)";
        const String groundText = m_lastCursorGroundPos
            ? U"({:.3f}, {:.3f})"_fmt(m_lastCursorGroundPos->x, m_lastCursorGroundPos->y)
            : U"none";
        const String layerPosText = hasSelectedLayer()
            ? U"({:.3f}, {:.3f})"_fmt(m_layers[m_selectedLayerIndex].position.x, m_layers[m_selectedLayerIndex].position.y)
            : U"none";

        return U"[TextureEditor.PlaceAtClick]\n"
            U"updateEnabled = {}\n"_fmt(m_enabled ? U"true" : U"false")
            + U"requestArmed = {}\n"_fmt(m_placeAtClickRequested ? U"true" : U"false")
            + U"selectedLayer = {}\n"_fmt(selectedLayer)
            + U"cursorScreen = ({}, {})\n"_fmt(m_lastCursorScreenPos.x, m_lastCursorScreenPos.y)
            + U"cursorGround = {}\n"_fmt(groundText)
            + U"panelHover = {}\n"_fmt(m_lastPlacementPanelHover ? U"true" : U"false")
            + U"leftClickDown = {}\n"_fmt(m_lastPlacementClickSeen ? U"true" : U"false")
            + U"placementApplied = {}\n"_fmt(m_lastPlacementApplied ? U"true" : U"false")
            + U"layerPosition = {}\n"_fmt(layerPosText)
            + U"reason = {}\n"_fmt(m_lastPlacementReason)
            + U"status = {}"_fmt(m_statusMessage);
    }

    [[nodiscard]] static String categoryName(const int32 index)
    {
        switch (index)
        {
        case 0: return U"Grass";
        case 1: return U"Dirt";
        case 2: return U"Plaza";
        case 3: return U"Brick";
        case 4: return U"Stone";
        case 5: return U"Decal";
        default: return U"Other";
        }
    }

    void scanAvailableTextures()
    {
        m_availableTextures.clear();
        for (const auto& dirPath : { String{ U"example/texture/" }, String{ U"texture/" } })
        {
            if (not FileSystem::IsDirectory(dirPath))
            {
                continue;
            }
            for (const auto& path : FileSystem::DirectoryContents(dirPath, Recursive::No))
            {
                const String ext = FileSystem::Extension(path).lowercased();
                if (ext == U"jpg" || ext == U"png")
                {
                    m_availableTextures << path;
                }
            }
        }
    }

    [[nodiscard]] const Image& getOrLoadSourceImage(const FilePath& path)
    {
        if (const auto it = m_sourceImageCache.find(path); it != m_sourceImageCache.end())
        {
            return it->second;
        }
        m_sourceImageCache.emplace(path, Image{ path });
        return m_sourceImageCache[path];
    }

    [[nodiscard]] Image buildLayerImage(const GroundLayer& layer)
    {
        Image result{ TexRes, TexRes };
        const Image& src = getOrLoadSourceImage(layer.texturePath);
        const bool hasSrc = (not src.isEmpty());
        PerlinNoise noise{ layer.edgeNoiseSeed };
        const double softness = Max(0.001, layer.edgeSoftness);

        for (int32 py = 0; py < TexRes; ++py)
        {
            for (int32 px = 0; px < TexRes; ++px)
            {
                const double u = (px + 0.5) / TexRes;
                const double v = (py + 0.5) / TexRes;

                // edge distance: 0 at quad edge, 0.5 at center
                const double edgeDist = Min(Min(u, 1.0 - u), Min(v, 1.0 - v));

                // noise warp
                const double noiseVal = noise.noise1D(u * layer.edgeNoiseFrequency + v * layer.edgeNoiseFrequency * 1.31);
                const double warpedDist = edgeDist + noiseVal * layer.edgeNoiseAmount;

                // smoothstep alpha
                const double t = Clamp(warpedDist / softness, 0.0, 1.0);
                const double alpha = t * t * (3.0 - 2.0 * t);

                // sample source texture with tiling (nearest neighbor)
                ColorF srcColor{ 0.55, 0.55, 0.55 };
                if (hasSrc)
                {
                    double su = std::fmod(u * layer.tilingScale, 1.0);
                    double sv = std::fmod(v * layer.tilingScale, 1.0);
                    if (su < 0.0) su += 1.0;
                    if (sv < 0.0) sv += 1.0;
                    const int32 sx = static_cast<int32>(su * src.width()) % src.width();
                    const int32 sy = static_cast<int32>(sv * src.height()) % src.height();
                    srcColor = ColorF{ src[sy][sx] };
                }

                result[py][px] = Color{ ColorF{
                    srcColor.r * layer.tint.r,
                    srcColor.g * layer.tint.g,
                    srcColor.b * layer.tint.b,
                    alpha * layer.tint.a
                } };
            }
        }

        return result;
    }

    void ensureAllLayerTextures()
    {
        for (size_t i = 0; i < m_layers.size(); ++i)
        {
            if (not m_layerDirty[i])
            {
                continue;
            }

            const Image img = buildLayerImage(m_layers[i]);
            if (m_layerTextures[i])
            {
                m_layerTextures[i].fill(img);
            }
            else
            {
                m_layerTextures[i] = DynamicTexture{ img };
            }
            m_layerDirty[i] = false;
        }
    }

    void markLayerDirty(const size_t index)
    {
        if (index < m_layerDirty.size())
        {
            m_layerDirty[index] = true;
        }
    }

    void addLayer(GroundLayer layer)
    {
        if (layer.id.isEmpty())
        {
            layer.id = U"layer_{}"_fmt(m_layers.size());
        }
        if (layer.label.isEmpty())
        {
            layer.label = U"Layer {}"_fmt(m_layers.size());
        }
        m_layers << layer;
        m_layerTextures.emplace_back();
        m_layerDirty << true;
        m_selectedLayerIndex = m_layers.size() - 1;
        m_layerListScroll = getLayerListMaxScroll();
    }

    void removeLayer(const size_t index)
    {
        if (index >= m_layers.size())
        {
            return;
        }
        m_layers.remove_at(index);
        m_layerTextures.remove_at(index);
        m_layerDirty.remove_at(index);
        if ((not m_layers.isEmpty()) && m_selectedLayerIndex >= m_layers.size())
        {
            m_selectedLayerIndex = m_layers.size() - 1;
        }
        m_layerListScroll = Clamp(m_layerListScroll, 0.0, getLayerListMaxScroll());
        m_statusMessage = U"Layer deleted";
    }

    void clearAllLayers()
    {
        m_layers.clear();
        m_layerTextures.clear();
        m_layerDirty.clear();
        m_selectedLayerIndex = 0;
        m_placeAtClickRequested = false;
        m_layerListScroll = 0.0;
        m_statusMessage = U"All layers cleared";
    }

    void undoLastLayer()
    {
        if (not m_layers.isEmpty())
        {
            removeLayer(m_layers.size() - 1);
            m_statusMessage = U"Undo: layer removed";
        }
    }

    void duplicateSelectedLayer()
    {
        if (not hasSelectedLayer())
        {
            return;
        }
        GroundLayer copy = m_layers[m_selectedLayerIndex];
        copy.id = U"layer_{}"_fmt(m_layers.size());
        copy.label = copy.label + U" (copy)";
        copy.position.x += 2.0;
        addLayer(copy);
        m_statusMessage = U"Layer duplicated";
    }

    void moveLayerUp(const size_t index)
    {
        if (index == 0 || index >= m_layers.size())
        {
            return;
        }
        std::swap(m_layers[index], m_layers[index - 1]);
        std::swap(m_layerTextures[index], m_layerTextures[index - 1]);
        std::swap(m_layerDirty[index], m_layerDirty[index - 1]);
        m_selectedLayerIndex = index - 1;
    }

    void moveLayerDown(const size_t index)
    {
        if ((index + 1) >= m_layers.size())
        {
            return;
        }
        std::swap(m_layers[index], m_layers[index + 1]);
        std::swap(m_layerTextures[index], m_layerTextures[index + 1]);
        std::swap(m_layerDirty[index], m_layerDirty[index + 1]);
        m_selectedLayerIndex = index + 1;
    }

    void cycleTexture(GroundLayer& layer, const int32 direction)
    {
        if (m_availableTextures.isEmpty())
        {
            return;
        }

        size_t currentIndex = 0;
        bool found = false;
        for (size_t i = 0; i < m_availableTextures.size(); ++i)
        {
            if (m_availableTextures[i] == layer.texturePath)
            {
                currentIndex = i;
                found = true;
                break;
            }
        }

        const size_t n = m_availableTextures.size();
        currentIndex = found
            ? ((currentIndex + n + direction) % n)
            : 0;
        layer.texturePath = m_availableTextures[currentIndex];
    }

    // ---- UI Tabs ----

    void drawLayersTab(const RectF& panel)
    {
        const double cx = panel.x + 14;
        double cy = panel.y + 120;
        const double cw = panel.w - 28;

        // Action buttons row
        const double btnW = (cw - 32.0) / 5.0;
        const RectF addBtn{ cx, cy, btnW, 34 };
        const RectF dupBtn{ cx + btnW + 8, cy, btnW, 34 };
        const RectF delBtn{ cx + (btnW + 8) * 2, cy, btnW, 34 };
        const RectF clearBtn{ cx + (btnW + 8) * 3, cy, btnW, 34 };
        const RectF saveBtn{ cx + (btnW + 8) * 4, cy, btnW, 34 };

        if (ui::Button(m_font, U"+ Add", addBtn))
        {
            GroundLayer layer;
            if (not m_availableTextures.isEmpty())
            {
                layer.texturePath = m_availableTextures.front();
            }
            addLayer(layer);
            m_statusMessage = U"Layer added";
        }

        if (ui::Button(m_font, U"Dup", dupBtn))
        {
            duplicateSelectedLayer();
        }

        if (ui::Button(m_font, U"Delete", delBtn))
        {
            if (hasSelectedLayer())
            {
                removeLayer(m_selectedLayerIndex);
            }
        }

        if (ui::Button(m_font, U"Clear", clearBtn))
        {
            clearAllLayers();
        }

        if (ui::Button(m_font, U"Save", saveBtn))
        {
            save();
            m_statusMessage = U"Saved";
        }

        cy += 46;

        // Layer list
        constexpr double RowHeight = 40.0;
        const RectF listSection = getLayerListSectionRect();
        ui::Section(listSection);
        m_font(U"Layers ({} total)"_fmt(m_layers.size())).draw(listSection.pos.movedBy(12, 8), ui::GetTheme().text);

        const double visibleListHeight = Max(0.0, listSection.h - 36.0);
        const double contentHeight = static_cast<double>(m_layers.size()) * RowHeight;
        const double maxScroll = Max(0.0, contentHeight - visibleListHeight);
        if (listSection.mouseOver())
        {
            m_layerListScroll = Clamp(m_layerListScroll - Mouse::Wheel() * RowHeight, 0.0, maxScroll);
        }
        else
        {
            m_layerListScroll = Clamp(m_layerListScroll, 0.0, maxScroll);
        }

        const size_t startIndex = static_cast<size_t>(m_layerListScroll / RowHeight);
        const double rowOffset = Math::Fmod(m_layerListScroll, RowHeight);
        const size_t visibleCount = static_cast<size_t>(visibleListHeight / RowHeight) + 2;

        for (size_t i = startIndex; i < m_layers.size() && (i - startIndex) < visibleCount; ++i)
        {
            auto& layer = m_layers[i];
            const bool selected = (i == m_selectedLayerIndex);
            const double rowY = listSection.y + 34 + ((i - startIndex) * RowHeight) - rowOffset;

            const RectF row{ listSection.x + 8, rowY, listSection.w - 16, RowHeight - 4 };
            if ((row.y + row.h) < (listSection.y + 34.0) || row.y > (listSection.y + listSection.h - 2.0))
            {
                continue;
            }

            const ColorF rowFill = selected
                ? ColorF{ 0.88, 0.94, 1.0, 1.0 }
                : (row.mouseOver() ? ui::GetTheme().itemHovered : ui::GetTheme().item);
            row.rounded(6).draw(rowFill);
            row.rounded(6).drawFrame(selected ? 2.0 : 1.0, selected ? ui::GetTheme().accent : ui::GetTheme().panelBorder);

            // Visibility toggle
            const RectF visBtn{ row.x + 6, row.y + 9, 22, 22 };
            visBtn.rounded(4).draw(layer.visible ? ColorF{ 0.35, 0.82, 0.48 } : ColorF{ 0.72, 0.72, 0.72 });
            visBtn.rounded(4).drawFrame(1.0, ui::GetTheme().panelBorder);
            if (visBtn.leftClicked())
            {
                layer.visible = not layer.visible;
            }

            m_font(layer.label).draw(row.x + 36, row.y + 8, ui::GetTheme().text);
            m_font(categoryName(layer.categoryIndex)).draw(row.x + 210, row.y + 8, ui::GetTheme().textMuted);

            // Up / Down order buttons
            const RectF upBtn{ row.x + row.w - 54, row.y + 7, 22, 22 };
            const RectF dnBtn{ row.x + row.w - 28, row.y + 7, 22, 22 };
            if (ui::Button(m_font, U"↑", upBtn))
            {
                moveLayerUp(i);
                break;
            }
            if (ui::Button(m_font, U"↓", dnBtn))
            {
                moveLayerDown(i);
                break;
            }
            setTooltipIfHovered(upBtn.mouseOver() || dnBtn.mouseOver(),
                U"Reorder layer. Draw order = bottom(index 0) to top(index N).");

            if (row.leftClicked() && not visBtn.mouseOver() && not upBtn.mouseOver() && not dnBtn.mouseOver())
            {
                m_selectedLayerIndex = i;
            }
        }

        if (maxScroll > 0.0)
        {
            const RectF scrollTrack{ listSection.x + listSection.w - 10, listSection.y + 34, 6, listSection.h - 40 };
            scrollTrack.rounded(3).draw(ColorF{ 0.82, 0.86, 0.90, 1.0 });

            const double thumbHeight = Max(18.0, scrollTrack.h * (visibleListHeight / contentHeight));
            const double thumbY = scrollTrack.y + (scrollTrack.h - thumbHeight) * (m_layerListScroll / maxScroll);
            RectF{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight }.rounded(3).draw(ui::GetTheme().accent);
        }

        cy += listSection.h + 10;

        // Auto yOffset section
        const RectF autoSection{ cx, cy, cw, 116 };
        ui::Section(autoSection);
        m_font(U"Z-Fighting Prevention").draw(autoSection.pos.movedBy(12, 8), ui::GetTheme().text);

        const RectF autoToggle{ cx + 12, cy + 36, cw - 24, 30 };
        if (ui::Button(m_font, m_autoYOffset ? U"Auto Y-Offset: ON" : U"Auto Y-Offset: OFF", autoToggle))
        {
            m_autoYOffset = (not m_autoYOffset);
        }
        if (m_autoYOffset)
        {
            autoToggle.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
        }
        setTooltipIfHovered(autoToggle.mouseOver(),
            U"ON: each layer gets baseY + index * step, eliminating Z-fighting automatically.");

        if (m_autoYOffset)
        {
            ui::SliderH(U"Base Y", m_baseYOffset, 0.001, 0.02,
                Vec2{ cx + 12, cy + 74 }, 80.0, cw - 100.0);
        }

        // show current computed Y for selected layer
        if (hasSelectedLayer())
        {
            const double previewY = m_autoYOffset
                ? (m_baseYOffset + static_cast<double>(m_selectedLayerIndex) * m_autoYOffsetStep)
                : m_layers[m_selectedLayerIndex].yOffset;
            m_font(U"finalY[{}]: {:.4f}"_fmt(m_selectedLayerIndex, previewY)).draw(
                Vec2{ cx + 12, cy + 74 }, ui::GetTheme().textMuted);
        }

        cy += 124;

        // Status section
        const RectF statusSection{ cx, cy, cw, 54 };
        ui::Section(statusSection);
        m_font(U"Status: " + m_statusMessage).draw(statusSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"T:toggle  Ctrl+Z:undo  Ctrl+D:dup  Del:delete  ↑↓:order").draw(statusSection.pos.movedBy(12, 32), ui::GetTheme().textMuted);
    }

    void drawPropertiesTab(const RectF& panel)
    {
        if (not hasSelectedLayer())
        {
            m_font(U"No layer selected.\nAdd a layer in the Layers tab.").draw(
                panel.pos.movedBy(20, 130), ui::GetTheme().textMuted);
            return;
        }

        auto& layer = m_layers[m_selectedLayerIndex];
        bool dirty = false;

        const double cx = panel.x + 14;
        double cy = panel.y + 120;
        const double cw = panel.w - 28;
        const double lw = 116.0;
        const double sw = cw - lw - 20.0;

        // Position
        {
            const RectF sec{ cx, cy, cw, 184 };
            ui::Section(sec);
            m_font(U"Position").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            dirty |= ui::SliderH(U"X", layer.position.x, -80.0, 80.0, Vec2{ cx + 12, cy + 38 }, lw, sw);
            dirty |= ui::SliderH(U"Z", layer.position.y, -80.0, 80.0, Vec2{ cx + 12, cy + 72 }, lw, sw);
            const RectF placeBtn{ cx + 12, cy + 108, 216, 28 };
            const RectF probeBtn{ cx + 236, cy + 108, 70, 28 };
            const RectF copyBtn{ cx + 314, cy + 108, 70, 28 };
            if (ui::Button(m_font, m_placeAtClickRequested ? U"Click on ground to place..." : U"Place at clicked position", placeBtn))
            {
                m_placeAtClickRequested = (not m_placeAtClickRequested);
                if (m_placeAtClickRequested)
                {
                    m_statusMessage = U"Click on the ground to place the selected texture";
                }
                else
                {
                    m_statusMessage = U"Click placement canceled";
                }
            }
            if (ui::Button(m_font, U"Probe", probeBtn))
            {
                if (m_lastCursorGroundPos)
                {
                    applyPlacement(*m_lastCursorGroundPos, U"probe button");
                }
                else
                {
                    m_lastPlacementApplied = false;
                    m_lastPlacementReason = U"probe failed: no cached ground hit";
                    m_statusMessage = U"Probe failed: cursor ray did not hit the ground";
                }
            }
            if (ui::Button(m_font, (Scene::Time() < m_placementDiagnosticsCopiedUntil) ? U"Copied" : U"Copy", copyBtn))
            {
                Clipboard::SetText(buildPlacementDiagnostics());
                m_placementDiagnosticsCopiedUntil = (Scene::Time() + 1.5);
                m_statusMessage = U"Placement diagnostics copied";
            }
            if (m_placeAtClickRequested)
            {
                placeBtn.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
            }
            m_smallFont(U"armed={} update={} click={} panel={} hit={}"_fmt(
                m_placeAtClickRequested ? U"true" : U"false",
                m_enabled ? U"ON" : U"OFF",
                m_lastPlacementClickSeen ? U"true" : U"false",
                m_lastPlacementPanelHover ? U"true" : U"false",
                m_lastCursorGroundPos ? U"yes" : U"no")).draw(Vec2{ cx + 12, cy + 144 }, ui::GetTheme().textMuted);
            m_smallFont(U"reason: {}"_fmt(m_lastPlacementReason)).draw(Vec2{ cx + 12, cy + 160 }, ui::GetTheme().textMuted);
            cy += 192;
        }

        // Size
        {
            const RectF sec{ cx, cy, cw, 110 };
            ui::Section(sec);
            m_font(U"Size").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            dirty |= ui::SliderH(U"Width", layer.size.x, 0.5, 80.0, Vec2{ cx + 12, cy + 38 }, lw, sw);
            dirty |= ui::SliderH(U"Height", layer.size.y, 0.5, 80.0, Vec2{ cx + 12, cy + 72 }, lw, sw);
            cy += 118;
        }

        // Transform + Tiling
        {
            const RectF sec{ cx, cy, cw, 110 };
            ui::Section(sec);
            m_font(U"Transform").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            double rotDeg = Math::ToDegrees(layer.rotation);
            if (ui::SliderH(U"Rotation", rotDeg, -180.0, 180.0, Vec2{ cx + 12, cy + 38 }, lw, sw))
            {
                layer.rotation = Math::ToRadians(rotDeg);
                dirty = true;
            }
            dirty |= ui::SliderH(U"Tiling", layer.tilingScale, 0.5, 16.0, Vec2{ cx + 12, cy + 72 }, lw, sw);
            cy += 118;
        }

        // Tint
        {
            const RectF sec{ cx, cy, cw, 148 };
            ui::Section(sec);
            m_font(U"Tint").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            dirty |= ui::SliderH(U"R", layer.tint.r, 0.0, 1.5, Vec2{ cx + 12, cy + 38 }, lw, sw);
            dirty |= ui::SliderH(U"G", layer.tint.g, 0.0, 1.5, Vec2{ cx + 12, cy + 72 }, lw, sw);
            dirty |= ui::SliderH(U"B", layer.tint.b, 0.0, 1.5, Vec2{ cx + 12, cy + 106 }, lw, sw);
            cy += 156;
        }

        // Texture
        {
            const RectF sec{ cx, cy, cw, 236 };
            ui::Section(sec);
            m_font(U"Texture").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);

            const String texName = layer.texturePath.isEmpty()
                ? U"(none)"
                : FileSystem::FileName(layer.texturePath);
            m_font(texName).draw(sec.pos.movedBy(12, 36), ui::GetTheme().textMuted);

            const RectF prevBtn{ cx + 12, cy + 56, 70, 26 };
            const RectF nextBtn{ cx + 90, cy + 56, 70, 26 };
            const RectF loadBtn{ cx + 168, cy + 56, 70, 26 };
            if (ui::Button(m_font, U"◀ Prev", prevBtn))
            {
                cycleTexture(layer, -1);
                dirty = true;
            }
            if (ui::Button(m_font, U"Next ▶", nextBtn))
            {
                cycleTexture(layer, 1);
                dirty = true;
            }
            if (ui::Button(m_font, U"Load", loadBtn))
            {
                load();
                m_statusMessage = U"Loaded";
                return;
            }

            const RectF catBtn{ cx + cw - 86, cy + 56, 74, 26 };
            if (ui::Button(m_font, categoryName(layer.categoryIndex), catBtn))
            {
                layer.categoryIndex = (layer.categoryIndex + 1) % 6;
            }
            setTooltipIfHovered(catBtn.mouseOver(), U"Click to cycle category");

            const RectF listRect{ cx + 12, cy + 92, cw - 24, 132 };
            ui::Section(listRect);

            constexpr double TextureRowHeight = 28.0;
            const double visibleListHeight = Max(0.0, listRect.h - 8.0);
            const double contentHeight = static_cast<double>(m_availableTextures.size()) * TextureRowHeight;
            const double maxScroll = Max(0.0, contentHeight - visibleListHeight);

            if (listRect.mouseOver())
            {
                m_textureListScroll = Clamp(m_textureListScroll - Mouse::Wheel() * TextureRowHeight, 0.0, maxScroll);
            }
            else
            {
                m_textureListScroll = Clamp(m_textureListScroll, 0.0, maxScroll);
            }

            const size_t startIndex = static_cast<size_t>(m_textureListScroll / TextureRowHeight);
            const double rowOffset = Math::Fmod(m_textureListScroll, TextureRowHeight);
            const size_t visibleCount = static_cast<size_t>(visibleListHeight / TextureRowHeight) + 2;

            for (size_t i = startIndex; i < m_availableTextures.size() && (i - startIndex) < visibleCount; ++i)
            {
                const double rowY = listRect.y + 4.0 + ((i - startIndex) * TextureRowHeight) - rowOffset;
                const RectF row{ listRect.x + 4, rowY, listRect.w - 16, TextureRowHeight - 2.0 };

                if ((row.y + row.h) < (listRect.y + 2.0) || row.y > (listRect.y + listRect.h - 2.0))
                {
                    continue;
                }

                const bool selectedTexture = (m_availableTextures[i] == layer.texturePath);
                const ColorF fill = selectedTexture
                    ? ColorF{ 0.88, 0.94, 1.0, 1.0 }
                    : (row.mouseOver() ? ui::GetTheme().itemHovered : ui::GetTheme().item);
                row.rounded(5).draw(fill);
                row.rounded(5).drawFrame(selectedTexture ? 2.0 : 1.0, selectedTexture ? ui::GetTheme().accent : ui::GetTheme().panelBorder);

                m_font(FileSystem::BaseName(m_availableTextures[i])).draw(row.x + 8, row.y + 3, ui::GetTheme().text);

                if (row.leftClicked())
                {
                    layer.texturePath = m_availableTextures[i];
                    dirty = true;
                }
            }

            if (maxScroll > 0.0)
            {
                const RectF scrollTrack{ listRect.x + listRect.w - 10, listRect.y + 4, 6, listRect.h - 8 };
                scrollTrack.rounded(3).draw(ColorF{ 0.82, 0.86, 0.90, 1.0 });

                const double thumbHeight = Max(18.0, scrollTrack.h * (visibleListHeight / contentHeight));
                const double thumbY = scrollTrack.y + (scrollTrack.h - thumbHeight) * (m_textureListScroll / maxScroll);
                RectF{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight }.rounded(3).draw(ui::GetTheme().accent);
            }
        }

        if (dirty)
        {
            markLayerDirty(m_selectedLayerIndex);
        }
    }

    void drawEdgeTab(const RectF& panel)
    {
        if (not hasSelectedLayer())
        {
            m_font(U"No layer selected.").draw(panel.pos.movedBy(20, 130), ui::GetTheme().textMuted);
            return;
        }

        auto& layer = m_layers[m_selectedLayerIndex];
        bool dirty = false;

        const double cx = panel.x + 14;
        double cy = panel.y + 120;
        const double cw = panel.w - 28;
        const double lw = 140.0;
        const double sw = cw - lw - 20.0;

        // Edge Blend
        {
            const RectF sec{ cx, cy, cw, 186 };
            ui::Section(sec);
            m_font(U"Edge Blend").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            m_font(U"Fade applied toward the boundary.").draw(sec.pos.movedBy(12, 34), ui::GetTheme().textMuted);
            dirty |= ui::SliderH(U"Softness", layer.edgeSoftness, 0.01, 0.5, Vec2{ cx + 12, cy + 60 }, lw, sw);
            setTooltipIfHovered(RectF{ cx + 12, cy + 60, cw - 24, 34 }.mouseOver(), U"Fade region width (0=hard edge, 0.5=full fade from center)");
            dirty |= ui::SliderH(U"Alpha", layer.tint.a, 0.0, 1.0, Vec2{ cx + 12, cy + 98 }, lw, sw);
            dirty |= ui::SliderH(U"Y Offset", layer.yOffset, 0.001, 0.08, Vec2{ cx + 12, cy + 136 }, lw, sw);
            setTooltipIfHovered(RectF{ cx + 12, cy + 136, cw - 24, 34 }.mouseOver(), U"Height above ground. Increase slightly if z-fighting occurs.");
            cy += 194;
        }

        // Edge Noise
        {
            const RectF sec{ cx, cy, cw, 210 };
            ui::Section(sec);
            m_font(U"Edge Noise").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            m_font(U"Perlin noise warps the boundary.").draw(sec.pos.movedBy(12, 34), ui::GetTheme().textMuted);
            dirty |= ui::SliderH(U"Amount", layer.edgeNoiseAmount, 0.0, 0.30, Vec2{ cx + 12, cy + 60 }, lw, sw);
            setTooltipIfHovered(RectF{ cx + 12, cy + 60, cw - 24, 34 }.mouseOver(), U"Amount=0: clean rect. Higher values give more jagged edge.");
            dirty |= ui::SliderH(U"Frequency", layer.edgeNoiseFrequency, 0.5, 12.0, Vec2{ cx + 12, cy + 98 }, lw, sw);
            setTooltipIfHovered(RectF{ cx + 12, cy + 98, cw - 24, 34 }.mouseOver(), U"Noise frequency. Low=large blobs, High=tight detail.");

            m_font(U"Seed: {}"_fmt(layer.edgeNoiseSeed)).draw(Vec2{ cx + 12, cy + 148 }, ui::GetTheme().text);
            const RectF seedDecBtn{ cx + 164, cy + 144, 36, 28 };
            const RectF seedIncBtn{ cx + 208, cy + 144, 36, 28 };
            if (ui::Button(m_font, U"-", seedDecBtn))
            {
                if (layer.edgeNoiseSeed > 0)
                {
                    --layer.edgeNoiseSeed;
                }
                dirty = true;
            }
            if (ui::Button(m_font, U"+", seedIncBtn))
            {
                ++layer.edgeNoiseSeed;
                dirty = true;
            }
            setTooltipIfHovered(seedDecBtn.mouseOver() || seedIncBtn.mouseOver(), U"Noise seed. Each value gives a different boundary shape.");

            m_font(U"Recommended per category:").draw(Vec2{ cx + 12, cy + 180 }, ui::GetTheme().textMuted);
            m_font(U"Grass=mid  Plaza=low  Dirt=high  Decal=high").draw(Vec2{ cx + 12, cy + 198 }, ui::GetTheme().textMuted);
        }

        if (dirty)
        {
            markLayerDirty(m_selectedLayerIndex);
        }
    }

    // ---- Persistence ----

    void save() const
    {
        const FilePath dir = FileSystem::ParentPath(m_savePath);
        if (not dir.isEmpty())
        {
            FileSystem::CreateDirectories(dir);
        }

        TextWriter writer{ m_savePath };
        if (not writer)
        {
            return;
        }

        writer.writeln(U"[settings]");
        writer.writeln(U"autoYOffset = {}"_fmt(m_autoYOffset ? U"true" : U"false"));
        writer.writeln(U"autoYOffsetStep = {:.5f}"_fmt(m_autoYOffsetStep));
        writer.writeln(U"baseYOffset = {:.5f}"_fmt(m_baseYOffset));
        writer.writeln(U"");

        for (const auto& layer : m_layers)
        {
            writer.writeln(U"[[layers]]");
            writer.writeln(U"id = \"{}\""_fmt(layer.id));
            writer.writeln(U"label = \"{}\""_fmt(layer.label));
            writer.writeln(U"texturePath = \"{}\""_fmt(layer.texturePath));
            writer.writeln(U"categoryIndex = {}"_fmt(layer.categoryIndex));
            writer.writeln(U"positionX = {:.3f}"_fmt(layer.position.x));
            writer.writeln(U"positionZ = {:.3f}"_fmt(layer.position.y));
            writer.writeln(U"sizeW = {:.3f}"_fmt(layer.size.x));
            writer.writeln(U"sizeH = {:.3f}"_fmt(layer.size.y));
            writer.writeln(U"rotation = {:.5f}"_fmt(layer.rotation));
            writer.writeln(U"yOffset = {:.5f}"_fmt(layer.yOffset));
            writer.writeln(U"tintR = {:.4f}"_fmt(layer.tint.r));
            writer.writeln(U"tintG = {:.4f}"_fmt(layer.tint.g));
            writer.writeln(U"tintB = {:.4f}"_fmt(layer.tint.b));
            writer.writeln(U"tintA = {:.4f}"_fmt(layer.tint.a));
            writer.writeln(U"tilingScale = {:.3f}"_fmt(layer.tilingScale));
            writer.writeln(U"edgeSoftness = {:.5f}"_fmt(layer.edgeSoftness));
            writer.writeln(U"edgeNoiseAmount = {:.5f}"_fmt(layer.edgeNoiseAmount));
            writer.writeln(U"edgeNoiseFrequency = {:.4f}"_fmt(layer.edgeNoiseFrequency));
            writer.writeln(U"edgeNoiseSeed = {}"_fmt(layer.edgeNoiseSeed));
            writer.writeln(U"visible = {}"_fmt(layer.visible ? U"true" : U"false"));
            writer.writeln(U"");
        }
    }

    void load()
    {
        m_layers.clear();
        m_layerTextures.clear();
        m_layerDirty.clear();
        m_selectedLayerIndex = 0;
        m_layerListScroll = 0.0;
        m_textureListScroll = 0.0;

        if (not FileSystem::Exists(m_savePath))
        {
            loadDefaults();
            return;
        }

        const TOMLReader toml{ m_savePath };
        if (not toml)
        {
            loadDefaults();
            return;
        }

        const auto settings = toml[U"settings"];
        m_autoYOffset     = settings[U"autoYOffset"].getOr<bool>(true);
        m_autoYOffsetStep = settings[U"autoYOffsetStep"].getOr<double>(0.003);
        m_baseYOffset     = settings[U"baseYOffset"].getOr<double>(0.002);

        for (const auto& v : toml[U"layers"].tableArrayView())
        {
            GroundLayer layer;
            layer.id           = v[U"id"].getOr<String>(U"");
            layer.label        = v[U"label"].getOr<String>(U"Layer");
            layer.texturePath  = v[U"texturePath"].getOr<String>(U"");
            layer.categoryIndex = v[U"categoryIndex"].getOr<int32>(0);
            layer.position.x   = v[U"positionX"].getOr<double>(0.0);
            layer.position.y   = v[U"positionZ"].getOr<double>(0.0);
            layer.size.x       = v[U"sizeW"].getOr<double>(10.0);
            layer.size.y       = v[U"sizeH"].getOr<double>(10.0);
            layer.rotation     = v[U"rotation"].getOr<double>(0.0);
            layer.yOffset      = v[U"yOffset"].getOr<double>(0.014);
            layer.tint.r       = v[U"tintR"].getOr<double>(1.0);
            layer.tint.g       = v[U"tintG"].getOr<double>(1.0);
            layer.tint.b       = v[U"tintB"].getOr<double>(1.0);
            layer.tint.a       = v[U"tintA"].getOr<double>(1.0);
            layer.tilingScale       = v[U"tilingScale"].getOr<double>(4.0);
            layer.edgeSoftness      = v[U"edgeSoftness"].getOr<double>(0.12);
            layer.edgeNoiseAmount   = v[U"edgeNoiseAmount"].getOr<double>(0.04);
            layer.edgeNoiseFrequency = v[U"edgeNoiseFrequency"].getOr<double>(3.0);
            layer.edgeNoiseSeed     = static_cast<uint64>(v[U"edgeNoiseSeed"].getOr<int64>(42));
            layer.visible      = v[U"visible"].getOr<bool>(true);
            addLayer(layer);
        }

        m_statusMessage = U"Loaded {} layers"_fmt(m_layers.size());
    }

    void loadDefaults()
    {
        // Grass base
        {
            GroundLayer g;
            g.id = U"grass_base"; g.label = U"Grass Base";
            g.texturePath = U"example/texture/grass.jpg";
            g.categoryIndex = 0;
            g.position = Vec2{ 0.0, 0.0 }; g.size = Vec2{ 36.0, 30.0 };
            g.rotation = 0.0; g.yOffset = 0.004;
            g.tint = ColorF{ 0.60, 0.68, 0.56, 0.95 };
            g.tilingScale = 6.0;
            g.edgeSoftness = 0.14; g.edgeNoiseAmount = 0.05;
            g.edgeNoiseFrequency = 2.5; g.edgeNoiseSeed = 10;
            addLayer(g);
        }
        // Plaza center
        {
            GroundLayer p;
            p.id = U"plaza_center"; p.label = U"Plaza Center";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ -1.0, 0.4 }; p.size = Vec2{ 12.5, 10.5 };
            p.rotation = Math::ToRadians(-4.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.74, 0.66, 0.60, 0.96 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.04;
            p.edgeNoiseFrequency = 3.0; p.edgeNoiseSeed = 7;
            addLayer(p);
        }
        // Plaza east
        {
            GroundLayer p;
            p.id = U"plaza_east"; p.label = U"Plaza East";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ 5.5, 3.2 }; p.size = Vec2{ 8.0, 6.0 };
            p.rotation = Math::ToRadians(12.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.76, 0.68, 0.61, 0.92 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.04;
            p.edgeNoiseFrequency = 3.5; p.edgeNoiseSeed = 22;
            addLayer(p);
        }
        // Plaza west
        {
            GroundLayer p;
            p.id = U"plaza_west"; p.label = U"Plaza West";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ -8.3, 3.8 }; p.size = Vec2{ 7.5, 5.2 };
            p.rotation = Math::ToRadians(-10.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.72, 0.65, 0.59, 0.90 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.035;
            p.edgeNoiseFrequency = 3.0; p.edgeNoiseSeed = 35;
            addLayer(p);
        }
        m_statusMessage = U"Defaults loaded";
    }
};
