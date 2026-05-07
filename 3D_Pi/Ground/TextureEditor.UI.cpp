# include "../stdafx.h"
# include "TextureEditor.hpp"

void TextureEditor::drawUI(){
        m_hoverTooltip.clear();
     syncCollapsedIconRegistry();
        const RectF panel = getPanelRect();

        if (m_uiCollapsed)
        {
           const RectF btn = getCollapsedIconRect();

            if (MouseR.down() && btn.mouseOver())
            {
                m_panelDragging = true;
                m_panelDragOffset = Cursor::PosF() - btn.pos;
            }
            if (not MouseR.pressed())
            {
                m_panelDragging = false;
            }
            if (m_panelDragging)
            {
                updateCollapsedIconDrag(btn);
            }

            btn.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            btn.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, btn);

          if (btn.leftClicked())
            {
              expandFromCollapsedIcon();
            }
            setTooltipIfHovered(btn.mouseOver(), U"Expand Texture Editor (T to toggle)");
            if (not m_hoverTooltip.isEmpty())
            {
                ui::Tooltip(m_font, m_hoverTooltip, Cursor::PosF().movedBy(18, 18));
            }
            return;
        }

        ui::Panel(panel);

        m_font(U"Texture Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
        m_font(U"Ground layer painting").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

        const RectF stateChip{ panel.x + 268, panel.y + 14, 112, 24 };
        stateChip.rounded(12).draw(m_enabled ? ColorF{ 0.86, 0.94, 0.88, 0.96 } : ColorF{ 0.96, 0.90, 0.86, 0.96 });
        stateChip.rounded(12).drawFrame(1.0, ui::GetTheme().panelBorder);
        m_smallFont(U"Update: {}"_fmt(m_enabled ? U"ON" : U"OFF")).drawAt(stateChip.center(), ColorF{ 0.18, 0.26, 0.42 });
        setTooltipIfHovered(stateChip.mouseOver(), U"T toggles update-time editor behavior.");

        const RectF collapseBtn{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
        if (MouseR.down() && collapseBtn.mouseOver())
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

        collapseBtn.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        collapseBtn.drawFrame(2.0, Palette::Black);
        ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseBtn);

      if (collapseBtn.leftClicked())
        {
            const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                collapseBtn, SizeF{ ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize });
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"TextureEditor", desiredCollapsedPos);
            m_uiCollapsed = true;
            syncCollapsedIconRegistry();
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



RectF TextureEditor::getLayerListSectionRect() const{
        const RectF panel = getPanelRect();
        const double cx = panel.x + 14;
        const double cy = panel.y + 166;
        const double cw = panel.w - 28;
        constexpr double RowHeight = 40.0;
        const double maxListH = Min(static_cast<double>(Max<size_t>(m_layers.size(), 1)) * RowHeight + 36.0, 340.0);
        return RectF{ cx, cy, cw, maxListH };
    }



RectF TextureEditor::getTextureListRect() const{
        const RectF panel = getPanelRect();
        const double cx = panel.x + 14;
        const double cy = panel.y + 660;
        const double cw = panel.w - 28;
        return RectF{ cx + 12, cy + 92, cw - 24, 132 };
    }



double TextureEditor::getLayerListMaxScroll() const{
        constexpr double RowHeight = 40.0;
        const RectF listSection = getLayerListSectionRect();
        const double visibleListHeight = Max(0.0, listSection.h - 36.0);
        const double contentHeight = static_cast<double>(m_layers.size()) * RowHeight;
        return Max(0.0, contentHeight - visibleListHeight);
    }



    void TextureEditor::setTooltipIfHovered(const bool hovered, const StringView text){
        if (hovered)
        {
            m_hoverTooltip = text;
        }
    }



    void TextureEditor::drawLayersTab(const RectF& panel){
        const double cx = panel.x + 14;
        double cy = panel.y + 120;
        const double cw = panel.w - 28;

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

            const RectF visBtn{ row.x + 6, row.y + 9, 22, 22 };
            visBtn.rounded(4).draw(layer.visible ? ColorF{ 0.35, 0.82, 0.48 } : ColorF{ 0.72, 0.72, 0.72 });
            visBtn.rounded(4).drawFrame(1.0, ui::GetTheme().panelBorder);
            if (visBtn.leftClicked())
            {
                layer.visible = not layer.visible;
            }

            m_font(layer.label).draw(row.x + 36, row.y + 8, ui::GetTheme().text);
            m_font(categoryName(layer.categoryIndex)).draw(row.x + 210, row.y + 8, ui::GetTheme().textMuted);

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

        if (hasSelectedLayer())
        {
            const double previewY = m_autoYOffset
                ? (m_baseYOffset + static_cast<double>(m_selectedLayerIndex) * m_autoYOffsetStep)
                : m_layers[m_selectedLayerIndex].yOffset;
            m_font(U"finalY[{}]: {:.4f}"_fmt(m_selectedLayerIndex, previewY)).draw(
                Vec2{ cx + 12, cy + 74 }, ui::GetTheme().textMuted);
        }

        cy += 124;

        const RectF statusSection{ cx, cy, cw, 54 };
        ui::Section(statusSection);
        m_font(U"Status: " + m_statusMessage).draw(statusSection.pos.movedBy(12, 8), ui::GetTheme().text);
        m_font(U"T:toggle  Ctrl+Z:undo  Ctrl+D:dup  Del:delete  ↑↓:order").draw(statusSection.pos.movedBy(12, 32), ui::GetTheme().textMuted);
    }



    void TextureEditor::drawPropertiesTab(const RectF& panel){
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

        {
            const RectF sec{ cx, cy, cw, 110 };
            ui::Section(sec);
            m_font(U"Size").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            dirty |= ui::SliderH(U"Width", layer.size.x, 0.5, 80.0, Vec2{ cx + 12, cy + 38 }, lw, sw);
            dirty |= ui::SliderH(U"Height", layer.size.y, 0.5, 80.0, Vec2{ cx + 12, cy + 72 }, lw, sw);
            cy += 118;
        }

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

        {
            const RectF sec{ cx, cy, cw, 148 };
            ui::Section(sec);
            m_font(U"Tint").draw(sec.pos.movedBy(12, 8), ui::GetTheme().text);
            dirty |= ui::SliderH(U"R", layer.tint.r, 0.0, 1.5, Vec2{ cx + 12, cy + 38 }, lw, sw);
            dirty |= ui::SliderH(U"G", layer.tint.g, 0.0, 1.5, Vec2{ cx + 12, cy + 72 }, lw, sw);
            dirty |= ui::SliderH(U"B", layer.tint.b, 0.0, 1.5, Vec2{ cx + 12, cy + 106 }, lw, sw);
            cy += 156;
        }

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



    void TextureEditor::drawEdgeTab(const RectF& panel){
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
