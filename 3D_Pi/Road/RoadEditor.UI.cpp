# include "../stdafx.h"
# include "RoadEditor.hpp"
# include "RoadEditorPanel.hpp"

void RoadEditor::drawUI(){
        RoadEditorPanel::Draw(*this);
    }



    void RoadEditor::drawPanelUI(){
        refreshRoadMaterialTextureIfDirty();
        m_hoverTooltip.clear();
        syncCollapsedIconRegistry();

        const RectF panel = getPanelRect();

        if (m_uiCollapsed)
        {
          const RectF expandButton = getCollapsedIconRect();

            if (MouseR.down() && expandButton.mouseOver())
            {
                m_panelDragging = true;
                m_panelDragOffset = Cursor::PosF() - expandButton.pos;
            }
            if (not MouseR.pressed())
            {
                m_panelDragging = false;
            }
            if (m_panelDragging)
            {
                updateCollapsedIconDrag(expandButton);
            }

            expandButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            expandButton.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, expandButton);

         if (expandButton.leftClicked())
            {
              expandFromCollapsedIcon();
            }

            setTooltipIfHovered(expandButton.mouseOver(), U"Expand Road Editor");

            if (not m_hoverTooltip.isEmpty())
            {
                ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
            }
            return;
        }

        ui::Panel(panel);

        const RectF collapseButton{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
        if (MouseR.down() && collapseButton.mouseOver())
        {
            m_panelDragging = true;
            m_panelDragOffset = Cursor::PosF() - m_panelPos;
        }
        if (not MouseR.pressed())
        {
            m_panelDragging = false;
        }
        if (m_panelDragging)
        {
            const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
            m_panelPos.x = Clamp(desiredPos.x, 0.0, static_cast<double>(Scene::Width()) - panel.w);
            m_panelPos.y = Clamp(desiredPos.y, 0.0, static_cast<double>(Scene::Height()) - panel.h);
        }

        m_font(U"Road Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
        m_font(U"Build roads and tune material live").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

        collapseButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        collapseButton.drawFrame(2.0, Palette::Black);
        ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseButton);

       if (collapseButton.leftClicked())
        {
            const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                collapseButton, SizeF{ ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize });
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"RoadEditor", desiredCollapsedPos);
            m_uiCollapsed = true;
            syncCollapsedIconRegistry();
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



    void RoadEditor::resetMaterialSettings(){
        m_materialSettings = road::DefaultRoadMaterialSettings();
        m_materialDirty = true;
        m_statusMessage = U"Material reset to defaults";
    }



    void RoadEditor::refreshRoadMaterialTextureIfDirty(){
        if (not m_materialDirty)
        {
            return;
        }

        road::ClampRoadMaterialSettings(m_materialSettings);
        m_roadTexture = Texture{ road::CreateRoadMaterialTexture(m_textureSourcePath, m_materialSettings), TextureDesc::MippedSRGB };
        m_roadShoulderTexture = Texture{ road::CreateRoadShoulderBlendTexture(m_textureSourcePath, m_materialSettings), TextureDesc::MippedSRGB };
        m_materialDirty = false;
    }



    void RoadEditor::setTooltipIfHovered(const bool hovered, const StringView text){
        if (hovered)
        {
            m_hoverTooltip = text;
        }
    }



    bool RoadEditor::drawAdjustableMaterialSlider(const StringView label, double& value, double& maxValue, const double minValue,
        const double maxLimit, const double maxStep, const Vec2& pos, const StringView tooltip,
        const double labelWidth , const double sliderWidth , const bool rebuildMesh ){
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



    bool RoadEditor::drawMaterialSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth , const double sliderWidth ){
        if (ui::SliderH(label, value, min, max, pos, labelWidth, sliderWidth))
        {
            m_materialDirty = true;
            return true;
        }

        return false;
    }



    bool RoadEditor::drawMaterialMeshSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth , const double sliderWidth ){
        if (drawMaterialSlider(label, value, min, max, pos, labelWidth, sliderWidth))
        {
            rebuildAllMeshes();
            return true;
        }

        return false;
    }



    void RoadEditor::drawEditTab(const RectF& panel){
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



    void RoadEditor::drawMaterialTab(const RectF& panel){
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



    void RoadEditor::drawScatterTab(const RectF& panel){
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
