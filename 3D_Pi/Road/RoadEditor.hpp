# pragma once
# include <Siv3D.hpp>
# include "../UI/RectUI.hpp"
# include "RoadTypes.hpp"
# include "RoadMaterialBuilder.hpp"
# include "RoadMeshBuilder.hpp"
# include "RoadSerializer.hpp"
# include "RoadGeometry.hpp"
# include "RoadPlacementTypes.hpp"
# include "RoadScatterRules.hpp"
# include "RoadSceneSnapshot.hpp"
# include "RoadGhostViewState.hpp"
# include "RoadEditSession.hpp"
# include "RoadPresetSerializer.hpp"

class RoadEditor
{
public:
    explicit RoadEditor(const FilePath& texturePath, const FilePath& savePath = U"data/roads.toml",
        const FilePath& presetsDir = U"data/road_presets/")
        : m_textureSourcePath{ texturePath }
        , m_savePath{ savePath }
        , m_presetsDir{ presetsDir }
    {
        load();
        m_presets = road::LoadAllPresets(m_presetsDir);
    }

    [[nodiscard]] bool isEnabled() const noexcept
    {
        return m_enabled;
    }

    [[nodiscard]] bool wantsMouseCapture() const
    {
        return isCursorOnUI();
    }

    void update(const BasicCamera3D& camera)
    {
        if (KeyR.down())
        {
            m_enabled = (not m_enabled);
            m_statusMessage = (m_enabled ? U"Road Editor: ON" : U"Road Editor: OFF");
        }

        if (not m_enabled)
        {
            m_hoverPoint.reset();
            m_snapPoint.reset();
            m_hoverScatterItemIndex.reset();
            return;
        }

        m_hoverPoint = cursorToGround(camera);
        m_snapPoint = findSnapPoint(m_hoverPoint);

        if (KeyS.down())
        {
            save();
            m_statusMessage = U"Road data saved";
        }

        if (KeyL.down())
        {
            load();
        }

        if (KeyG.down())
        {
            toggleGhostVisible();
        }

        if (KeyBackspace.down() && m_session.canRestore())
        {
            restoreSession();
        }

        if (KeyEnter.down())
        {
            confirmEditingRoad();
        }

        if (MouseR.down() && (not isCursorOnUI()))
        {
            cancelEditingRoad();
        }

        if (KeyControl.pressed() && KeyZ.down())
        {
            undo();
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

    void draw3D() const
    {
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

    void drawUI()
    {
        refreshRoadMaterialTextureIfDirty();
        m_hoverTooltip.clear();

        const RectF panel = getPanelRect();
        ui::Panel(panel);

        if (m_uiCollapsed)
        {
            const RectF expandButton{ panel.x + 6, panel.y + 6, panel.w - 12, panel.h - 12 };
            if (ui::Button(m_font, U"▶", expandButton))
            {
                m_uiCollapsed = false;
            }

            setTooltipIfHovered(expandButton.mouseOver(), U"Expand Road Editor");

            if (not m_hoverTooltip.isEmpty())
            {
                ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
            }
            return;
        }

        m_font(U"Road Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
        m_font(U"Build roads and tune material live").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

        const RectF collapseButton{ panel.x + panel.w - 44, panel.y + 10, 28, 28 };
        if (ui::Button(m_font, U"◀", collapseButton))
        {
            m_uiCollapsed = true;
        }
        setTooltipIfHovered(collapseButton.mouseOver(), U"Collapse Road Editor");

        const String activeTabLabel = (m_activeTabIndex == 0 ? U"Edit" : (m_activeTabIndex == 1 ? U"Material" : U"Scatter"));
        const RectF statusChip{ panel.x + 280, panel.y + 14, 104, 24 };
        statusChip.rounded(12).draw(ColorF{ 0.86, 0.92, 1.0, 0.95 });
        statusChip.rounded(12).drawFrame(1.0, ui::GetTheme().panelBorder);
        m_font(U"Tab: {}"_fmt(activeTabLabel)).drawAt(statusChip.center(), ColorF{ 0.18, 0.26, 0.42 });

        const RectF tabRow{ panel.x + 14, panel.y + 72, panel.w - 28, 34 };
        const double gap = 8.0;
        const double tabWidth = (tabRow.w - gap * 2.0) / 3.0;
        const RectF editTab{ tabRow.x, tabRow.y, tabWidth, tabRow.h };
        const RectF materialTab{ tabRow.x + tabWidth + gap, tabRow.y, tabWidth, tabRow.h };
        const RectF scatterTab{ tabRow.x + (tabWidth + gap) * 2.0, tabRow.y, tabWidth, tabRow.h };

        if (ui::Button(m_font, U"Edit", editTab))
        {
            m_activeTabIndex = 0;
        }

        if (ui::Button(m_font, U"Material", materialTab))
        {
            m_activeTabIndex = 1;
        }

        if (ui::Button(m_font, U"Scatter", scatterTab))
        {
            m_activeTabIndex = 2;
        }

        const RectF activeFrame = (m_activeTabIndex == 0 ? editTab : (m_activeTabIndex == 1 ? materialTab : scatterTab));
        activeFrame.rounded(6).drawFrame(2.0, ui::GetTheme().accent);

        if (m_activeTabIndex == 0)
        {
            drawEditTab(panel);
        }
        else if (m_activeTabIndex == 1)
        {
            drawMaterialTab(panel);
        }
        else
        {
            drawScatterTab(panel);
        }

        if (not m_hoverTooltip.isEmpty())
        {
            ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
        }
    }

private:
    static constexpr double GuideYOffset = 0.05;
    static constexpr double PointSpacing = 0.5;
    static constexpr double MinSnapDistance = 0.3;
    static constexpr double MaxSnapDistance = 3.0;

    FilePath m_textureSourcePath;
    Texture m_roadTexture;
    Texture m_roadShoulderTexture;
    FilePath m_savePath;
    FilePath m_assetCatalogPath = U"Road/road_placement_assets.toml";
    FilePath m_presetsDir = U"data/road_presets/";
    Array<RoadSceneSnapshot> m_presets;
    String m_presetNameInput = U"Preset1";
    size_t m_selectedPresetIndex = 0;
    RoadGhostViewState m_ghost;
    RoadEditSession m_session;
    Font m_font{ 18 };
    bool m_enabled = false;
    bool m_uiCollapsed = false;
    String m_statusMessage = U"Ready";
    double m_snapDistance = 1.0;
    Array<RoadPath> m_roads;
    Array<Optional<Mesh>> m_roadMeshes;
    Array<Optional<Mesh>> m_roadShoulderMeshes;
    Array<Mesh> m_connectionPatchMeshes;
    Optional<RoadPath> m_editingRoad;
    Optional<Mesh> m_editingMesh;
    Optional<Vec3> m_hoverPoint;
    Optional<Vec3> m_snapPoint;
    RoadMaterialSettings m_materialSettings = road::DefaultRoadMaterialSettings();
    size_t m_activeTabIndex = 0;
    bool m_materialDirty = true;
    Array<road::IntersectionCluster> m_intersectionClusters;
    Array<road::PlacementAsset> m_placementAssets = road::DefaultPlacementAssets();
    HashTable<String, Model> m_assetModelCache;
    HashTable<String, Texture> m_assetTextureCache;
    road::PlacementSettings m_placementSettings;
    String m_scatterDebugSummary;
    Array<road::PlacedScatterItem> m_scatterItems;
    Optional<size_t> m_hoverScatterItemIndex;
    String m_hoverTooltip;

    [[nodiscard]] RectF getPanelRect() const
    {
        if (m_uiCollapsed)
        {
            return RectF{ (Scene::Width() - 68), 20, 48, 48 };
        }

        return RectF{ (Scene::Width() - 640), 20, 620, Min(800.0, Scene::Height() - 40.0) };
    }

    [[nodiscard]] bool isCursorOnUI() const
    {
        return getPanelRect().mouseOver();
    }

    void resetMaterialSettings()
    {
        m_materialSettings = road::DefaultRoadMaterialSettings();
        m_materialDirty = true;
        m_statusMessage = U"Material reset to defaults";
    }

    void refreshRoadMaterialTextureIfDirty()
    {
        if (not m_materialDirty)
        {
            return;
        }

        road::ClampRoadMaterialSettings(m_materialSettings);
        m_roadTexture = Texture{ road::CreateRoadMaterialTexture(m_textureSourcePath, m_materialSettings), TextureDesc::MippedSRGB };
        m_roadShoulderTexture = Texture{ road::CreateRoadShoulderBlendTexture(m_textureSourcePath, m_materialSettings), TextureDesc::MippedSRGB };
        m_materialDirty = false;
    }

    void setTooltipIfHovered(const bool hovered, const StringView text)
    {
        if (hovered)
        {
            m_hoverTooltip = text;
        }
    }

    bool drawAdjustableMaterialSlider(const StringView label, double& value, double& maxValue, const double minValue,
        const double maxLimit, const double maxStep, const Vec2& pos, const StringView tooltip,
        const double labelWidth = 136.0, const double sliderWidth = 126.0, const bool rebuildMesh = false)
    {
        maxValue = Clamp(maxValue, minValue, maxLimit);
        value = Clamp(value, minValue, maxValue);

        const auto result = ui::SliderHEx(label, value, minValue, maxValue, pos, labelWidth, sliderWidth);
        setTooltipIfHovered(result.hovered, tooltip);

        bool changed = result.changed;
        if (result.decreaseClicked)
        {
            maxValue = Max(minValue, maxValue - maxStep);
            value = Min(value, maxValue);
            changed = true;
        }

        if (result.increaseClicked)
        {
            maxValue = Min(maxLimit, maxValue + maxStep);
            changed = true;
        }

        if (changed)
        {
            m_materialDirty = true;
            if (rebuildMesh)
            {
                rebuildAllMeshes();
            }
        }

        return changed;
    }

    bool drawMaterialSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth = 136.0, const double sliderWidth = 126.0)
    {
        if (ui::SliderH(label, value, min, max, pos, labelWidth, sliderWidth))
        {
            m_materialDirty = true;
            return true;
        }

        return false;
    }

    bool drawMaterialMeshSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth = 136.0, const double sliderWidth = 126.0)
    {
        if (drawMaterialSlider(label, value, min, max, pos, labelWidth, sliderWidth))
        {
            rebuildAllMeshes();
            return true;
        }

        return false;
    }

    void drawEditTab(const RectF& panel)
    {
        const RectF workflowSection{ panel.x + 14, panel.y + 120, panel.w - 28, 214 };
        const RectF presetSection   { panel.x + 14, panel.y + 346, panel.w - 28, 254 };
        const RectF statusSection   { panel.x + 14, panel.y + 612, panel.w - 28, 120 };
        ui::Section(workflowSection);
        ui::Section(presetSection);
        ui::Section(statusSection);

        m_font(U"Workflow").draw(workflowSection.pos.movedBy(12, 8), ui::GetTheme().text);
        const String workflow = U"R : Toggle editor"
            U"\nLDrag : Draw road"
            U"\nEnter : Confirm road"
            U"\nRClick : Cancel current road"
            U"\nCtrl+Z : Undo point / road"
            U"\nG : Toggle ghost  |  Backspace : Restore"
            U"\nS : Save  |  L : Load";
        m_font(workflow).draw(workflowSection.pos.movedBy(12, 38), ui::GetTheme().textMuted);
        ui::SliderH(U"Snap Distance", m_snapDistance, MinSnapDistance, MaxSnapDistance, workflowSection.pos.movedBy(12, 170), 150, 200);
        if (ui::Button(m_font, U"Clear All", RectF{ workflowSection.x + workflowSection.w - 126, workflowSection.y + 166, 112, 34 }))
        {
            clearAllPlacedData();
        }

        m_font(U"Preset & Trace").draw(presetSection.pos.movedBy(12, 8), ui::GetTheme().text);

        const double px = presetSection.x + 12;
        double py = presetSection.y + 38;

        m_font(U"Name:").draw(px, py + 6, ui::GetTheme().textMuted);
        const RectF nameBox{ px + 52, py, presetSection.w - 64, 32 };
        nameBox.rounded(6).draw(ui::GetTheme().item);
        nameBox.rounded(6).drawFrame(1.0, ui::GetTheme().panelBorder);
        m_font(m_presetNameInput).draw(nameBox.pos.movedBy(8, 6), ui::GetTheme().text);
        if (nameBox.mouseOver())
        {
            TextInput::UpdateText(m_presetNameInput);
        }

        py += 42;
        if (ui::Button(m_font, U"Save As Preset", RectF{ px, py, 170, 34 }))
        {
            saveCurrentAsPreset(m_presetNameInput);
        }
        setTooltipIfHovered(RectF{ px, py, 170, 34 }.mouseOver(), U"Save current roads as a named preset.");

        py += 44;
        m_font(U"Presets").draw(px, py, ui::GetTheme().textMuted);
        py += 26;

        if (m_presets.isEmpty())
        {
            m_font(U"(none)").draw(px + 8, py, ui::GetTheme().textMuted);
            py += 28;
        }
        else
        {
            const double listH = 28.0;
            for (size_t i = 0; i < m_presets.size(); ++i)
            {
                const RectF row{ px, py, presetSection.w - 24, listH - 2 };
                const bool selected = (m_selectedPresetIndex == i);
                const ColorF fill = selected ? ColorF{ 0.86, 0.93, 1.0, 1.0 } : (row.mouseOver() ? ColorF{ 0.94, 0.97, 1.0 } : ui::GetTheme().item);
                row.rounded(5).draw(fill);
                row.rounded(5).drawFrame(1.0, ui::GetTheme().panelBorder);
                m_font(m_presets[i].name).draw(row.pos.movedBy(8, 4), ui::GetTheme().text);
                if (row.leftClicked())
                {
                    m_selectedPresetIndex = i;
                }
                py += listH;
            }
        }

        py += 4;
        const double btnW = (presetSection.w - 36) / 3.0;
        const RectF traceBtn   { px,               py, btnW, 34 };
        const RectF restoreBtn { px + btnW + 6,    py, btnW, 34 };
        const RectF commitBtn  { px + (btnW + 6)*2, py, btnW, 34 };

        if (ui::Button(m_font, U"Start Trace", traceBtn))
        {
            startTraceSession();
        }
        setTooltipIfHovered(traceBtn.mouseOver(), U"Save restore point, clear roads, show preset as ghost.");

        {
            const bool canRestore = m_session.canRestore();
            if (ui::Button(m_font, U"Restore", restoreBtn))
            {
                if (canRestore)
                {
                    restoreSession();
                }
            }
            setTooltipIfHovered(restoreBtn.mouseOver(), canRestore ? U"Revert to pre-trace state." : U"No restore point. Start Trace first.");
        }

        if (ui::Button(m_font, U"Commit", commitBtn))
        {
            commitSession();
        }
        setTooltipIfHovered(commitBtn.mouseOver(), U"Accept new roads and close the trace session.");

        m_font(U"Status").draw(statusSection.pos.movedBy(12, 8), ui::GetTheme().text);
        const String sessionTag = m_session.active ? U" [TRACE]" : U"";
        const String ghostTag   = m_ghost.visible  ? U" [GHOST]" : U"";
        const String status = U"Roads: {}"_fmt(m_roads.size()) + sessionTag + ghostTag
            + U"\nEditing points: {}"_fmt((m_editingRoad ? m_editingRoad->points.size() : 0))
            + U"\nPresets: {}"_fmt(m_presets.size())
            + U"\nStatus: " + m_statusMessage;
        m_font(status).draw(statusSection.pos.movedBy(12, 38), ui::GetTheme().text);
    }

    void drawMaterialTab(const RectF& panel)
    {
        const double contentX = panel.x + 14;
        const double contentY = panel.y + 116;
        const double gap = 12.0;
        const double columnWidth = (panel.w - 28 - gap) * 0.5;
        const double sliderLabelWidth = 136.0;
        const double sliderWidth = columnWidth - sliderLabelWidth - 28.0;
        const double valueMaxLimit = 3.0;
        const double zeroToOneStep = 0.1;
        const double brightnessStep = 0.2;
        const double warmthStep = 0.1;
        const double widthStep = 0.25;
        const double fadeStep = 0.05;
        const RectF lookSection{ contentX, contentY, columnWidth, 260 };
        const RectF storySection{ contentX + columnWidth + gap, contentY, columnWidth, 260 };
        const RectF shoulderSection{ contentX, contentY + 272, columnWidth, 252 };
        const RectF previewSection{ contentX + columnWidth + gap, contentY + 272, columnWidth, 120 };
        ui::Section(lookSection);
        ui::Section(storySection);
        ui::Section(shoulderSection);
        ui::Section(previewSection);

        m_font(U"Look").draw(lookSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"Broad art direction controls first.").draw(lookSection.pos.movedBy(12, 34), ui::GetTheme().textMuted);
        double y = lookSection.y + 62;
        static double baseBrightnessMax = 1.8;
        static double baseWarmthMax = 0.4;
        static double macroVariationMax = 1.0;
        static double detailVariationMax = 1.0;
        static double trackStrengthMax = 1.0;
        static double trackWidthMax = 0.18;
        static double edgeMudMax = 1.0;
        static double pebbleMax = 1.0;
        static double sootMax = 1.0;
        static double shoulderWidthMax = 4.0;
        static double shoulderOpacityMax = 1.0;
        static double shoulderBrightnessMax = 1.8;
        static double shoulderOuterFadeMax = 0.98;

        drawAdjustableMaterialSlider(U"Base Brightness", m_materialSettings.baseBrightness, baseBrightnessMax, 0.4, valueMaxLimit, brightnessStep,
            Vec2{ lookSection.x + 12, y }, U"Road base color brightness.", sliderLabelWidth, sliderWidth);
        y += 42;
        drawAdjustableMaterialSlider(U"Base Warmth", m_materialSettings.baseWarmth, baseWarmthMax, -0.4, 1.5, warmthStep,
            Vec2{ lookSection.x + 12, y }, U"Warm/cool shift for the road base.", sliderLabelWidth, sliderWidth);
        y += 42;
        drawAdjustableMaterialSlider(U"Macro Variation", m_materialSettings.macroVariation, macroVariationMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ lookSection.x + 12, y }, U"Large-scale color breakup to reduce flatness.", sliderLabelWidth, sliderWidth);
        y += 42;
        drawAdjustableMaterialSlider(U"Detail", m_materialSettings.detailVariation, detailVariationMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ lookSection.x + 12, y }, U"Fine texture contrast for close-up detail.", sliderLabelWidth, sliderWidth);

        m_font(U"Surface Story").draw(storySection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"Control wear patterns and edge behavior.").draw(storySection.pos.movedBy(12, 34), ui::GetTheme().textMuted);
        y = storySection.y + 62;
        drawAdjustableMaterialSlider(U"Track Strength", m_materialSettings.trackStrength, trackStrengthMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ storySection.x + 12, y }, U"How strongly wheel ruts darken the surface.", sliderLabelWidth, sliderWidth);
        y += 38;
        drawAdjustableMaterialSlider(U"Track Width", m_materialSettings.trackWidth, trackWidthMax, 0.02, 0.6, 0.02,
            Vec2{ storySection.x + 12, y }, U"Width of the paired wheel-track bands.", sliderLabelWidth, sliderWidth);
        y += 38;
        drawAdjustableMaterialSlider(U"Edge Mud", m_materialSettings.edgeMudStrength, edgeMudMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ storySection.x + 12, y }, U"Amount of dark mud collected near the edges.", sliderLabelWidth, sliderWidth);
        y += 38;
        drawAdjustableMaterialSlider(U"Pebbles", m_materialSettings.pebbleStrength, pebbleMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ storySection.x + 12, y }, U"Small bright stone breakup across the road.", sliderLabelWidth, sliderWidth);
        y += 38;
        drawAdjustableMaterialSlider(U"Soot", m_materialSettings.sootStrength, sootMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ storySection.x + 12, y }, U"Burnt dark speckling layered over the road.", sliderLabelWidth, sliderWidth);

        m_font(U"Shoulder").draw(shoulderSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"Tune the ground-to-road blend layer.").draw(shoulderSection.pos.movedBy(12, 34), ui::GetTheme().textMuted);
        const RectF shoulderColorFadeButton{ shoulderSection.x + 12, shoulderSection.y + 58, shoulderSection.w - 24, 32 };
        if (ui::Button(m_font, (m_materialSettings.shoulderUseColorFade ? U"Color + Alpha Fade: ON" : U"Color + Alpha Fade: OFF"), shoulderColorFadeButton))
        {
            m_materialSettings.shoulderUseColorFade = (not m_materialSettings.shoulderUseColorFade);
            m_materialDirty = true;
        }
        setTooltipIfHovered(shoulderColorFadeButton.mouseOver(), U"Enable color blending toward the ground as the shoulder fades out.");

        y = shoulderSection.y + 102;
        drawAdjustableMaterialSlider(U"Width", m_materialSettings.shoulderWidthExpand, shoulderWidthMax, 0.0, 8.0, widthStep,
            Vec2{ shoulderSection.x + 12, y }, U"Extra blend width outside the core road mesh.", sliderLabelWidth, sliderWidth, true);
        y += 40;
        drawAdjustableMaterialSlider(U"Opacity", m_materialSettings.shoulderOpacity, shoulderOpacityMax, 0.0, valueMaxLimit, zeroToOneStep,
            Vec2{ shoulderSection.x + 12, y }, U"Visibility of the shoulder blend layer.", sliderLabelWidth, sliderWidth);
        y += 40;
        drawAdjustableMaterialSlider(U"Brightness", m_materialSettings.shoulderBrightness, shoulderBrightnessMax, 0.4, valueMaxLimit, brightnessStep,
            Vec2{ shoulderSection.x + 12, y }, U"Brightness of the road-to-ground shoulder tint.", sliderLabelWidth, sliderWidth);
        y += 40;
        drawAdjustableMaterialSlider(U"Outer Fade", m_materialSettings.shoulderOuterFade, shoulderOuterFadeMax, 0.55, 1.5, fadeStep,
            Vec2{ shoulderSection.x + 12, y }, U"Where the shoulder starts fading into the ground.", sliderLabelWidth, sliderWidth);

        m_font(U"Preview").draw(previewSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_roadShoulderTexture.resized(90, 40).draw(previewSection.pos.movedBy(12, 44));
        m_roadTexture.resized(90, 40).draw(previewSection.pos.movedBy(112, 44));

        if (ui::Button(m_font, U"Reset", RectF{ previewSection.x + previewSection.w - 98, previewSection.y + 12, 86, 34 }))
        {
            resetMaterialSettings();
            rebuildAllMeshes();
        }
    }

    void drawScatterTab(const RectF& panel)
    {
        const RectF modeSection{ panel.x + 14, panel.y + 120, panel.w - 28, 170 };
        const RectF assetSection{ panel.x + 14, panel.y + 302, panel.w - 28, 176 };
        const RectF settingsSection{ panel.x + 14, panel.y + 490, panel.w - 28, 158 };
        ui::Section(modeSection);
        ui::Section(assetSection);
        ui::Section(settingsSection);

        const auto categoryLabels = road::PlacementCategoryLabels();
        const auto modeLabels = road::PlacementModeLabels();

        m_font(U"1) Context").draw(modeSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"Choose category and action mode first.").draw(modeSection.pos.movedBy(12, 34), ui::GetTheme().textMuted);
        ui::RadioList(m_font, m_placementSettings.activeCategoryIndex, categoryLabels,
            RectF{ modeSection.x + 12, modeSection.y + 58, 166, ui::RadioListHeight(categoryLabels.size(), 26) }, 26);
        ui::RadioList(m_font, m_placementSettings.activeModeIndex, modeLabels,
            RectF{ modeSection.x + 190, modeSection.y + 58, 166, ui::RadioListHeight(modeLabels.size(), 26) }, 26);

        const auto activeCategory = road::PlacementCategoryFromIndex(m_placementSettings.activeCategoryIndex);
        Array<size_t> filteredAssetIndices;
        Array<String> filteredAssetLabels;
        for (size_t i = 0; i < m_placementAssets.size(); ++i)
        {
            if (m_placementAssets[i].category == activeCategory)
            {
                filteredAssetIndices << i;
                filteredAssetLabels << m_placementAssets[i].displayName;
            }
        }

        size_t selectedAssetIndex = 0;
        bool foundCurrentAsset = false;
        for (size_t i = 0; i < filteredAssetIndices.size(); ++i)
        {
            if (m_placementAssets[filteredAssetIndices[i]].id == m_placementSettings.activeAssetId)
            {
                selectedAssetIndex = i;
                foundCurrentAsset = true;
                break;
            }
        }

        if ((not foundCurrentAsset) && (not filteredAssetIndices.isEmpty()))
        {
            m_placementSettings.activeAssetId = m_placementAssets[filteredAssetIndices.front()].id;
        }

        m_font(U"2) Asset").draw(assetSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"Select the current visual variant.").draw(assetSection.pos.movedBy(12, 34), ui::GetTheme().textMuted);

        if (not filteredAssetLabels.isEmpty())
        {
            if (ui::RadioList(m_font, selectedAssetIndex, filteredAssetLabels,
                RectF{ assetSection.x + 12, assetSection.y + 58, assetSection.w - 24, ui::RadioListHeight(filteredAssetLabels.size(), 28) }, 28))
            {
                m_placementSettings.activeAssetId = m_placementAssets[filteredAssetIndices[selectedAssetIndex]].id;
            }
        }
        else
        {
            m_font(U"No assets for this category").draw(assetSection.pos.movedBy(12, 68), Palette::Orange);
        }

        m_font(U"3) Rules & Live Preview").draw(settingsSection.pos.movedBy(12, 8), ui::GetTheme().text);
        double y = settingsSection.y + 36;
        ui::SliderH(U"Density", m_placementSettings.density, 0.0, 1.0, Vec2{ settingsSection.x + 12, y }, 150, 200);
        y += 34;
        ui::SliderH(U"Edge Bias", m_placementSettings.edgeBias, 0.0, 1.0, Vec2{ settingsSection.x + 12, y }, 150, 200);
        y += 34;
        ui::SliderH(U"Intersection", m_placementSettings.intersectionBoost, 0.0, 1.0, Vec2{ settingsSection.x + 12, y }, 150, 200);
        y += 34;
        ui::SliderH(U"Brush Radius", m_placementSettings.brushRadius, 0.5, 6.0, Vec2{ settingsSection.x + 12, y }, 150, 200);

        if (m_hoverPoint)
        {
            if (const auto context = evaluateBoundaryContext(*m_hoverPoint))
            {
                const auto profile = road::EvaluatePlacementDensityProfile(*context, m_placementSettings);
                m_scatterDebugSummary = road::BuildPlacementDensitySummary(*context, profile)
                    + U"\nItems: {}"_fmt(m_scatterItems.size());
            }
            else
            {
                m_scatterDebugSummary = U"No nearby road context\nItems: {}"_fmt(m_scatterItems.size());
            }
        }
        else
        {
            m_scatterDebugSummary = U"Move cursor over ground\nItems: {}"_fmt(m_scatterItems.size());
        }

        const String modeHint = [this]()
        {
            switch (road::PlacementModeFromIndex(m_placementSettings.activeModeIndex))
            {
            case road::PlacementMode::Single:
                return U"Single: LClick place";
            case road::PlacementMode::Erase:
                return U"Erase: LClick delete";
            case road::PlacementMode::Brush:
                return U"Brush: coming soon";
            case road::PlacementMode::Select:
                return U"Select: coming soon";
            default:
                return U"";
            }
        }();

        m_font(modeHint).draw(settingsSection.pos.movedBy(12, 136), ui::GetTheme().textMuted);
        m_font(m_scatterDebugSummary).draw(settingsSection.pos.movedBy(196, 38), ui::GetTheme().text);
    }

    [[nodiscard]] const road::PlacementAsset* findActivePlacementAsset() const
    {
        const road::PlacementCategory activeCategory = road::PlacementCategoryFromIndex(m_placementSettings.activeCategoryIndex);

        for (const auto& asset : m_placementAssets)
        {
            if ((asset.category == activeCategory) && (asset.id == m_placementSettings.activeAssetId))
            {
                return &asset;
            }
        }

        for (const auto& asset : m_placementAssets)
        {
            if (asset.category == activeCategory)
            {
                return &asset;
            }
        }

        return (m_placementAssets.isEmpty() ? nullptr : &m_placementAssets.front());
    }

    [[nodiscard]] Optional<size_t> findHoveredScatterItemIndex() const
    {
        if (not m_hoverPoint)
        {
            return none;
        }

        Optional<size_t> result;
        double nearestDistance = 0.65;

        for (size_t i = 0; i < m_scatterItems.size(); ++i)
        {
            const double distance = m_scatterItems[i].position.distanceFrom(*m_hoverPoint);
            if (distance <= nearestDistance)
            {
                nearestDistance = distance;
                result = i;
            }
        }

        return result;
    }

    void updateScatterInteraction()
    {
        m_hoverScatterItemIndex = findHoveredScatterItemIndex();

        if (isCursorOnUI() || (not m_hoverPoint))
        {
            return;
        }

        const auto mode = road::PlacementModeFromIndex(m_placementSettings.activeModeIndex);
        if (not MouseL.down())
        {
            return;
        }

        if (mode == road::PlacementMode::Erase)
        {
            if (m_hoverScatterItemIndex)
            {
                m_scatterItems.remove_at(*m_hoverScatterItemIndex);
                m_hoverScatterItemIndex.reset();
                m_statusMessage = U"Scatter item erased";
            }
            return;
        }

        if (mode != road::PlacementMode::Single)
        {
            m_statusMessage = U"Brush/Select not implemented yet";
            return;
        }

        if (const auto context = evaluateBoundaryContext(*m_hoverPoint))
        {
            const auto profile = road::EvaluatePlacementDensityProfile(*context, m_placementSettings);
            const road::PlacementCategory category = road::PlacementCategoryFromIndex(m_placementSettings.activeCategoryIndex);
            double acceptance = 0.0;
            switch (category)
            {
            case road::PlacementCategory::Grass:
                acceptance = profile.grassDensity;
                break;
            case road::PlacementCategory::Pebble:
                acceptance = profile.pebbleDensity;
                break;
            case road::PlacementCategory::DirtDecal:
                acceptance = profile.mudDensity;
                break;
            case road::PlacementCategory::Prop:
                acceptance = Max(profile.pebbleDensity * 0.5, profile.grassDensity * 0.35);
                break;
            }

            if (acceptance < 0.08)
            {
                m_statusMessage = U"Placement rejected: low boundary density";
                return;
            }
        }

        const road::PlacementAsset* asset = findActivePlacementAsset();
        if (not asset)
        {
            m_statusMessage = U"No placement asset";
            return;
        }

        const double scale = Random(asset->defaultScaleMin, asset->defaultScaleMax);
        const double yaw = (asset->randomYaw ? Random(Math::TwoPi) : 0.0);
        m_scatterItems << road::PlacedScatterItem{
            .assetId = asset->id,
            .category = asset->category,
            .position = *m_hoverPoint,
            .yawRadians = yaw,
            .scale = scale,
            .tint = ColorF{ 1.0 }
        };

        m_statusMessage = U"Scatter item placed";
    }

    void drawScatterItems3D() const
    {
        for (size_t i = 0; i < m_scatterItems.size(); ++i)
        {
            const auto& item = m_scatterItems[i];
            const bool hovered = (m_hoverScatterItemIndex && (*m_hoverScatterItemIndex == i));
            const double radius = 0.09 * item.scale;

            if (const auto modelIt = m_assetModelCache.find(item.assetId); modelIt != m_assetModelCache.end())
            {
                const Quaternion rot = Quaternion::RotateY(item.yawRadians);
                modelIt->second.draw(item.position, rot);
                continue;
            }

            if (const auto texIt = m_assetTextureCache.find(item.assetId); texIt != m_assetTextureCache.end())
            {
                Plane{ item.position + Vec3{ 0, 0.004, 0 }, radius * 3.0 }.draw(texIt->second, hovered ? ColorF{ 1.0, 1.0, 1.0, 0.9 } : ColorF{ 1.0, 1.0, 1.0, 0.7 });
                continue;
            }

            switch (item.category)
            {
            case road::PlacementCategory::Grass:
                Cylinder{ item.position + Vec3{ 0, radius * 0.9, 0 }, radius * 0.45, radius * 2.2 }.draw(hovered ? ColorF{ 0.42, 0.88, 0.45 } : ColorF{ 0.34, 0.72, 0.31 });
                break;
            case road::PlacementCategory::Pebble:
                Sphere{ item.position + Vec3{ 0, radius * 0.55, 0 }, radius * 0.75 }.draw(hovered ? ColorF{ 0.82, 0.78, 0.66 } : ColorF{ 0.66, 0.61, 0.52 });
                break;
            case road::PlacementCategory::DirtDecal:
                Cylinder{ item.position + Vec3{ 0, 0.004, 0 }, radius * 1.7, 0.008 }.draw(hovered ? ColorF{ 0.34, 0.23, 0.16, 0.80 } : ColorF{ 0.26, 0.20, 0.15, 0.56 });
                break;
            case road::PlacementCategory::Prop:
                Box{ item.position + Vec3{ 0, radius, 0 }, radius * 1.4 }.draw(hovered ? ColorF{ 0.72, 0.72, 0.76 } : ColorF{ 0.52, 0.52, 0.57 });
                break;
            }
        }
    }

    void drawScatterHoverGuide3D() const
    {
        if (not m_hoverPoint)
        {
            return;
        }

        const auto mode = road::PlacementModeFromIndex(m_placementSettings.activeModeIndex);
        const ColorF ringColor = (mode == road::PlacementMode::Erase ? ColorF{ 0.92, 0.34, 0.34, 0.75 } : ColorF{ 0.30, 0.80, 0.95, 0.75 });
        Cylinder{ *m_hoverPoint + Vec3{ 0, 0.004, 0 }, 0.35, 0.008 }.draw(ringColor);

        if (m_hoverScatterItemIndex)
        {
            const auto& item = m_scatterItems[*m_hoverScatterItemIndex];
            Sphere{ item.position + Vec3{ 0, 0.20, 0 }, 0.15 }.draw(ColorF{ 1.0, 0.92, 0.28, 0.85 });
        }
    }

    [[nodiscard]] Optional<Vec3> cursorToGround(const BasicCamera3D& camera) const
    {
        const Ray ray = camera.screenToRay(Cursor::PosF());
        if (const auto hit = ray.intersectsAt(InfinitePlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } }))
        {
            return Vec3{ hit->x, 0.0, hit->z };
        }

        return none;
    }

    [[nodiscard]] Vec3 getCurrentInputPoint() const
    {
        if (m_snapPoint)
        {
            return *m_snapPoint;
        }

        if (m_hoverPoint)
        {
            return *m_hoverPoint;
        }

        return Vec3{ 0, 0, 0 };
    }

    [[nodiscard]] Optional<Vec3> findSnapPoint(const Optional<Vec3>& point) const
    {
        if (not point)
        {
            return none;
        }

        Optional<Vec3> result;
        double nearestDistance = m_snapDistance;

        for (const auto& road : m_roads)
        {
            if (road.points.isEmpty())
            {
                continue;
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                const double distance = endpoint.distanceFrom(*point);
                if (distance <= nearestDistance)
                {
                    nearestDistance = distance;
                    result = endpoint;
                }
            }
        }

        if (const auto ghostSnap = m_ghost.findSnapPoint(*point, nearestDistance))
        {
            result = ghostSnap;
        }

        return result;
    }

    void appendPoint(const Vec3& point)
    {
        const Vec3 snappedPoint{ point.x, 0.0, point.z };

        if (not m_editingRoad)
        {
            m_editingRoad = RoadPath{};
            m_editingRoad->points << snappedPoint;
            rebuildEditingMesh();
            return;
        }

        if (m_editingRoad->points.isEmpty())
        {
            m_editingRoad->points << snappedPoint;
            rebuildEditingMesh();
            return;
        }

        if (m_editingRoad->points.back().distanceFrom(snappedPoint) < PointSpacing)
        {
            return;
        }

        m_editingRoad->points << snappedPoint;
        rebuildEditingMesh();
    }

    void confirmEditingRoad()
    {
        if ((not m_editingRoad) || (m_editingRoad->points.size() < 2))
        {
            m_editingRoad.reset();
            m_editingMesh.reset();
            m_statusMessage = U"Road confirmation skipped";
            return;
        }

        if (m_snapPoint)
        {
            const Vec3 snappedEnd{ m_snapPoint->x, 0.0, m_snapPoint->z };
            if (m_editingRoad->points.back().distanceFrom(snappedEnd) < PointSpacing)
            {
                m_editingRoad->points.back() = snappedEnd;
            }
            else
            {
                m_editingRoad->points << snappedEnd;
            }
        }

        m_roads << *m_editingRoad;
        m_editingRoad.reset();
        m_editingMesh.reset();
        rebuildAllMeshes();
        m_statusMessage = U"Road confirmed";
    }

    void cancelEditingRoad()
    {
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_statusMessage = U"Current road canceled";
    }

    void clearAllPlacedData()
    {
        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_intersectionClusters.clear();
        m_scatterItems.clear();
        m_hoverScatterItemIndex.reset();
        m_statusMessage = U"Placed road data cleared";
    }

    void saveCurrentAsPreset(const String& name)
    {
        RoadSceneSnapshot snapshot;
        snapshot.name     = name;
        snapshot.roads    = m_roads;
        snapshot.material = m_materialSettings;

        if (road::SavePreset(m_presetsDir, snapshot))
        {
            for (auto& p : m_presets)
            {
                if (p.name == name)
                {
                    p = snapshot;
                    m_statusMessage = U"Preset updated: " + name;
                    return;
                }
            }
            m_presets << snapshot;
            m_selectedPresetIndex = m_presets.size() - 1;
            m_statusMessage = U"Preset saved: " + name;
        }
        else
        {
            m_statusMessage = U"Preset save failed";
        }
    }

    void startTraceSession()
    {
        if (m_presets.isEmpty())
        {
            m_statusMessage = U"No preset to trace from";
            return;
        }

        const auto& preset = m_presets[m_selectedPresetIndex];

        RoadSceneSnapshot restorePoint;
        restorePoint.name     = U"__restore__";
        restorePoint.roads    = m_roads;
        restorePoint.material = m_materialSettings;
        m_session.begin(restorePoint);

        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_intersectionClusters.clear();

        m_ghost.buildFrom(preset.roads, preset.material);
        m_ghost.visible = true;

        rebuildAllMeshes();
        m_statusMessage = U"Trace session started: " + preset.name;
    }

    void restoreSession()
    {
        if (not m_session.canRestore())
        {
            return;
        }

        const auto& rp = *m_session.restorePoint;
        m_roads           = rp.roads;
        m_materialSettings = rp.material;
        m_materialDirty   = true;

        m_editingRoad.reset();
        m_editingMesh.reset();
        m_ghost.clear();
        m_session.end();

        rebuildAllMeshes();
        refreshRoadMaterialTextureIfDirty();
        m_statusMessage = U"Scene restored";
    }

    void commitSession()
    {
        m_ghost.clear();
        m_session.end();
        m_statusMessage = U"Trace session committed";
    }

    void toggleGhostVisible()
    {
        if (m_ghost.roads.isEmpty())
        {
            m_statusMessage = U"No ghost loaded";
            return;
        }

        m_ghost.visible = not m_ghost.visible;
        m_statusMessage = m_ghost.visible ? U"Ghost: ON" : U"Ghost: OFF";
    }

    void undo()
    {
        if (m_editingRoad && (not m_editingRoad->points.isEmpty()))
        {
            m_editingRoad->points.pop_back();
            if (m_editingRoad->points.size() < 2)
            {
                m_editingMesh.reset();
            }
            else
            {
                rebuildEditingMesh();
            }

            if (m_editingRoad->points.isEmpty())
            {
                m_editingRoad.reset();
            }

            m_statusMessage = U"Editing point removed";
            return;
        }

        if (not m_roads.isEmpty())
        {
            m_roads.pop_back();
            rebuildAllMeshes();
            m_statusMessage = U"Last road removed";
        }
    }

    void rebuildAllMeshes()
    {
        road::RebuildRoadMeshes(m_roads, m_materialSettings, m_roadMeshes, m_roadShoulderMeshes, m_connectionPatchMeshes);
        m_intersectionClusters = road::BuildIntersectionClusters(m_roads);
    }

    void rebuildEditingMesh()
    {
        if (m_editingRoad)
        {
            m_editingMesh = road::BuildRoadMesh(*m_editingRoad);
        }
        else
        {
            m_editingMesh.reset();
        }
    }

    [[nodiscard]] Optional<road::RoadBoundaryContext> evaluateBoundaryContext(const Vec3& point) const
    {
        return road::EvaluateRoadBoundaryContext(point, m_roads, m_intersectionClusters);
    }

    void drawPathGuide(const Optional<RoadPath>& road, const ColorF& lineColor, const ColorF& pointColor) const
    {
        if (not road)
        {
            return;
        }

        for (size_t i = 1; i < road->points.size(); ++i)
        {
            Line3D{ road->points[i - 1] + Vec3{ 0, GuideYOffset, 0 }, road->points[i] + Vec3{ 0, GuideYOffset, 0 } }.draw(lineColor);
        }

        for (const auto& point : road->points)
        {
            Sphere{ point + Vec3{ 0, 0.08, 0 }, 0.10 }.draw(pointColor);
        }
    }

    void drawGhostRoads3D() const
    {
        if (not m_ghost.visible)
        {
            return;
        }

        const ColorF ghostRoad{ 0.72, 0.82, 1.0, m_ghost.opacity };
        const ColorF ghostShoulder{ 0.72, 0.82, 1.0, m_ghost.opacity * 0.6 };

        const size_t count = Min(m_ghost.roadMeshes.size(), m_ghost.shoulderMeshes.size());
        for (size_t i = 0; i < count; ++i)
        {
            if (m_ghost.shoulderMeshes[i])
            {
                m_ghost.shoulderMeshes[i]->draw(m_roadShoulderTexture, ghostShoulder);
            }
            if (m_ghost.roadMeshes[i])
            {
                m_ghost.roadMeshes[i]->draw(m_roadTexture, ghostRoad);
            }
        }

        for (const auto& road : m_ghost.roads)
        {
            if (road.points.size() < 2)
            {
                continue;
            }

            for (size_t i = 1; i < road.points.size(); ++i)
            {
                Line3D{ road.points[i - 1] + Vec3{ 0, GuideYOffset * 2.0, 0 },
                        road.points[i]     + Vec3{ 0, GuideYOffset * 2.0, 0 } }
                    .draw(ColorF{ 0.60, 0.76, 1.0, m_ghost.opacity * 1.4 });
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                Sphere{ endpoint + Vec3{ 0, 0.10, 0 }, 0.14 }
                    .draw(ColorF{ 0.50, 0.70, 1.0, m_ghost.opacity * 1.8 });
            }
        }
    }

    void save() const
    {
        road::SaveRoadData(m_savePath, m_materialSettings, m_roads);
    }

    void load()
    {
        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_materialSettings = road::DefaultRoadMaterialSettings();

        road::LoadRoadData(m_savePath, m_materialSettings, m_roads, m_statusMessage);
        loadPlacementAssets();
        m_materialDirty = true;
        refreshRoadMaterialTextureIfDirty();
        rebuildAllMeshes();
    }

    void loadPlacementAssets()
    {
        road::LoadPlacementAssetsFromToml(m_assetCatalogPath, m_placementAssets);
        m_assetModelCache.clear();
        m_assetTextureCache.clear();

        for (const auto& asset : m_placementAssets)
        {
            if (asset.resourcePath.isEmpty())
            {
                continue;
            }

            if (asset.renderType == road::PlacementRenderType::Model)
            {
                if (const Model model{ asset.resourcePath })
                {
                    Model::RegisterDiffuseTextures(model, TextureDesc::MippedSRGB);
                    m_assetModelCache.emplace(asset.id, model);
                }
            }
            else
            {
                if (const Texture texture{ asset.resourcePath, TextureDesc::MippedSRGB })
                {
                    m_assetTextureCache.emplace(asset.id, texture);
                }
            }
        }
    }
};
