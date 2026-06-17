# pragma once

    template <class DrawBody>
    void System::drawCollapsibleSection(const Font& uiFont, Vec2& uiPos, const double contentWidth, StringView title,
        bool& collapsed, const DrawBody& drawBody, const double collapsedHeight, const bool enableHeaderToggle)
    {
        if (not collapsed)
        {
            const Vec2 sectionTop = uiPos;
            drawBody();
            if (enableHeaderToggle)
            {
                const RectF collapseRect{ sectionTop.x + contentWidth - 38, sectionTop.y + 6, 30, 26 };
                if (ui::Button(uiFont, U"-", collapseRect))
                {
                    collapsed = true;
                }
            }
            return;
        }

        const RectF sectionRect{ uiPos, contentWidth, collapsedHeight };
        ui::Section(sectionRect);
        uiFont(title).draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
        if (enableHeaderToggle)
        {
            const RectF collapseRect{ sectionRect.rightX() - 38, sectionRect.y + 6, 30, 26 };
            if (ui::Button(uiFont, U"+", collapseRect))
            {
                collapsed = false;
            }
        }
        uiPos.y += sectionRect.h + ui::layout::SectionGap;
    }

    inline void System::drawUI()
    {
        const Font& uiFont = ui::DefaultFont();
        syncCollapsedIconRegistry();

        if (m_panelCollapsed)
        {
            const RectF toggleRect = getCollapsedToggleRect();

            if (MouseR.down() && toggleRect.mouseOver())
            {
                m_panelDragging = true;
                m_panelDragOffset = Cursor::PosF() - toggleRect.pos;
            }
            if (not MouseR.pressed())
            {
                m_panelDragging = false;
            }
            if (m_panelDragging)
            {
                updateCollapsedIconDrag(toggleRect);
            }

            toggleRect.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            toggleRect.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, toggleRect);
            if (toggleRect.leftClicked())
            {
                expandFromCollapsedIcon();
            }
            return;
        }

        const double panelHeight = getExpandedPanelHeight();
        m_panelPos.x = Clamp(m_panelPos.x, 0.0, static_cast<double>(Scene::Width()) - ui::layout::PanelWidth);
        m_panelPos.y = Clamp(m_panelPos.y, 0.0, static_cast<double>(Scene::Height()) - panelHeight);

        const RectF panelRect = getExpandedPanelRect();
        const RectF collapseBtn{ panelRect.x + panelRect.w - 74, panelRect.y + 10, CollapsedToggleSize, CollapsedToggleSize };
        const RectF headerBar{ panelRect.x, panelRect.y, panelRect.w, ui::layout::HeaderHeight };
        if (MouseL.down() && headerBar.mouseOver())
        {
            m_panelDragging = true;
            m_panelDragOffset = Cursor::PosF() - m_panelPos;
        }
        if (MouseR.down() && collapseBtn.mouseOver())
        {
            m_panelDragging = true;
            m_panelDragOffset = Cursor::PosF() - m_panelPos;
        }
        if (not (MouseL.pressed() || MouseR.pressed()))
        {
            m_panelDragging = false;
        }
        if (m_panelDragging)
        {
            const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
            m_panelPos.x = Clamp(desiredPos.x, 0.0, static_cast<double>(Scene::Width()) - ui::layout::PanelWidth);
            m_panelPos.y = Clamp(desiredPos.y, 0.0, static_cast<double>(Scene::Height()) - panelHeight);
        }

        const RectF contentRect = getPanelContentRect(panelRect);
        const double contentHeight = getPanelContentHeight();
        const double maxScrollY = Max(0.0, contentHeight - contentRect.h);
        if (contentRect.mouseOver())
        {
            m_panelScrollY = Clamp(m_panelScrollY - Mouse::Wheel() * ui::layout::ScrollStep, 0.0, maxScrollY);
        }
        else
        {
            m_panelScrollY = Clamp(m_panelScrollY, 0.0, maxScrollY);
        }

        ui::Panel(panelRect);

        const RectF effectHelpRect{ panelRect.x + ui::layout::PanelPadding, panelRect.y + 8, 64, 64 };
        effectHelpRect.rounded(8).draw(effectHelpRect.mouseOver() ? ui::GetTheme().itemHovered : ui::GetTheme().item);
        effectHelpRect.rounded(8).drawFrame(1.0, ui::GetTheme().panelBorder);
        if (m_effectHelpIcon)
        {
            m_effectHelpIcon.resized(64, 64).draw(effectHelpRect.pos);
        }
        if (effectHelpRect.mouseOver())
        {
            ui::Tooltip(uiFont, U"エフェクトチェイン全体の説明", Cursor::PosF().movedBy(18, 20));
        }

        collapseBtn.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        collapseBtn.drawFrame(2.0, Palette::Black);
        ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseBtn);
        if (collapseBtn.leftClicked())
        {
            const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                collapseBtn, SizeF{ CollapsedToggleSize, CollapsedToggleSize });
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"EffectEditor", desiredCollapsedPos);
            m_panelCollapsed = true;
            syncCollapsedIconRegistry();
        }

        if (contentRect.mouseOver() && MouseM.pressed())
        {
            m_panelScrollY = Clamp(m_panelScrollY - Cursor::DeltaF().y * 1.6, 0.0, maxScrollY);
        }

        {
            const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
            const Rect previousScissor = Graphics2D::GetScissorRect();
            Graphics2D::SetScissorRect(contentRect.asRect());

            Vec2 uiPos = contentRect.pos.movedBy(0, -m_panelScrollY);
            drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"環境", m_environmentCollapsed,
                [&]() { m_environment.drawUI(uiFont, uiPos, contentRect.w); }, Environment::HeaderHeight, false);
            m_lighting.drawUI(uiFont, uiPos, contentRect.w, m_lightingCollapsed);
            m_effectChain.drawChainListUI(uiFont, uiPos, contentRect.w);
            drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"プリセット", m_effectPresetCollapsed,
                [&]() { m_effectChain.drawPresetUI(uiFont, uiPos, contentRect.w, m_panelScrollY); });
            drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"パラメータ", m_effectParamsCollapsed,
                [&]() { m_effectChain.drawParamsUI(uiFont, uiPos, contentRect.w); });
            drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"パフォーマンス", m_performanceCollapsed,
                [&]() { drawPerformanceUI(uiFont, uiPos, contentRect.w); }, PerformanceSectionHeight);

            Graphics2D::SetScissorRect(previousScissor);
        }

        if (0.0 < maxScrollY)
        {
            const RectF scrollTrack{
                panelRect.rightX() - ui::layout::PanelPadding - ui::layout::ScrollbarWidth,
                contentRect.y,
                ui::layout::ScrollbarWidth,
                contentRect.h
            };
            const double thumbHeight = Max(ui::layout::ScrollbarMinThumbHeight, scrollTrack.h * (contentRect.h / contentHeight));
            const double thumbTravel = Max(0.0, scrollTrack.h - thumbHeight);
            const double thumbY = scrollTrack.y + (thumbTravel * (m_panelScrollY / maxScrollY));
            const RectF thumbRect{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight };

            if (MouseL.down() && thumbRect.mouseOver())
            {
                m_isScrollThumbDragging = true;
                m_scrollThumbGrabOffsetY = (Cursor::PosF().y - thumbRect.y);
            }
            if (not MouseL.pressed())
            {
                m_isScrollThumbDragging = false;
            }
            if (m_isScrollThumbDragging)
            {
                const double thumbTop = Clamp(Cursor::PosF().y - m_scrollThumbGrabOffsetY, scrollTrack.y, scrollTrack.y + thumbTravel);
                const double t = (thumbTravel > 0.0) ? ((thumbTop - scrollTrack.y) / thumbTravel) : 0.0;
                m_panelScrollY = t * maxScrollY;
            }

            scrollTrack.rounded(4).draw(ColorF{ 0.82, 0.86, 0.91, 0.8 });
            thumbRect.rounded(4).draw(ui::GetTheme().accent);
        }
        else
        {
            m_isScrollThumbDragging = false;
        }
    }

    inline bool System::wantsMouseWheelCapture() const
    {
        if (m_panelCollapsed)
        {
            return getCollapsedToggleRect().mouseOver();
        }

        return getExpandedPanelRect().mouseOver();
    }

    inline double System::getPanelContentHeight() const
    {
        const double environmentSectionBodyHeight = m_environmentCollapsed ? Environment::HeaderHeight : m_environment.getUIBodyHeight();
        const double environmentSectionHeight = environmentSectionBodyHeight + ui::layout::SectionGap;
        const double lightingSectionBodyHeight = m_lightingCollapsed ? m_lighting.getHeaderHeight() : m_lighting.getUIBodyHeight();
        const double lightingSectionHeight = lightingSectionBodyHeight + ui::layout::SectionGap;
        const double chainListHeight = m_effectChain.measureChainListHeight() + ui::layout::SectionGap;
        const double presetHeight = m_effectPresetCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : m_effectChain.measurePresetHeight();
        const double paramsHeight = m_effectParamsCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : m_effectChain.measureParamsHeight();
        const double performanceHeight = m_performanceCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : (PerformanceSectionHeight + ui::layout::SectionGap);
        return environmentSectionHeight + lightingSectionHeight + chainListHeight + presetHeight + paramsHeight + performanceHeight;
    }

    inline void System::drawPerformanceUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
    {
        const RectF sectionRect{ uiPos, contentWidth, PerformanceSectionHeight };
        ui::Section(sectionRect);
        uiFont(U"パフォーマンス").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
        uiFont(U"FPS 上限 / VSync / 省電力").draw(uiPos.movedBy(8, 26), Palette::Gray);
        bool changed = false;

        const RectF vSyncButton{ sectionRect.x + 8, sectionRect.y + 58, (sectionRect.w - 24) * 0.5, ui::layout::AddButtonHeight };
        if (ui::Button(uiFont, m_performanceSettings.vSyncEnabled ? U"VSync: ON" : U"VSync: OFF", vSyncButton))
        {
            m_performanceSettings.vSyncEnabled = (not m_performanceSettings.vSyncEnabled);
            Graphics::SetVSyncEnabled(m_performanceSettings.vSyncEnabled);
            changed = true;
        }

        const RectF powerSaveButton{ vSyncButton.rightX() + 8, vSyncButton.y, (sectionRect.w - 24) * 0.5, ui::layout::AddButtonHeight };
        if (ui::Button(uiFont, m_performanceSettings.powerSavingMode ? U"省電力: ON" : U"省電力: OFF", powerSaveButton))
        {
            m_performanceSettings.powerSavingMode = (not m_performanceSettings.powerSavingMode);
            changed = true;
        }

        double maxFPS = static_cast<double>(m_performanceSettings.maxFPS);
        if (ui::SliderH(U"Max FPS: {:.0f}"_fmt(maxFPS), maxFPS, 15.0, 240.0, uiPos.movedBy(8, 108), 120.0, contentWidth - 136.0))
        {
            m_performanceSettings.maxFPS = Clamp(static_cast<int32>(std::round(maxFPS)), 15, 240);
            changed = true;
        }

        double idleFPS = static_cast<double>(m_performanceSettings.idleFPS);
        if (ui::SliderH(U"Idle FPS: {:.0f}"_fmt(idleFPS), idleFPS, 5.0, static_cast<double>(m_performanceSettings.maxFPS), uiPos.movedBy(8, 148), 120.0, contentWidth - 136.0))
        {
            m_performanceSettings.idleFPS = Clamp(static_cast<int32>(std::round(idleFPS)), 5, m_performanceSettings.maxFPS);
            changed = true;
        }

        double backgroundFPS = static_cast<double>(m_performanceSettings.backgroundFPS);
        if (ui::SliderH(U"Background FPS: {:.0f}"_fmt(backgroundFPS), backgroundFPS, 1.0, static_cast<double>(m_performanceSettings.idleFPS), uiPos.movedBy(8, 188), 120.0, contentWidth - 136.0))
        {
            m_performanceSettings.backgroundFPS = Clamp(static_cast<int32>(std::round(backgroundFPS)), 1, m_performanceSettings.idleFPS);
            changed = true;
        }

        m_performanceSettings.maxFPS = Clamp(m_performanceSettings.maxFPS, 15, 240);
        m_performanceSettings.idleFPS = Clamp(m_performanceSettings.idleFPS, 5, m_performanceSettings.maxFPS);
        m_performanceSettings.backgroundFPS = Clamp(m_performanceSettings.backgroundFPS, 1, m_performanceSettings.idleFPS);

        if (changed)
        {
            m_lastSavedSettings = collectSettings();
            SaveSettings(m_lastSavedSettings);
        }

        uiFont(U"Target FPS: {}"_fmt(getTargetFPS())).draw(uiPos.movedBy(8, 226), ui::GetTheme().textMuted);
        uiPos.y += sectionRect.h + ui::layout::SectionGap;
    }

    inline double System::getExpandedPanelHeight() const
    {
        const double desiredPanelHeight = 46 + getPanelContentHeight();
        const double maxPanelHeight = (Scene::Height() - ui::layout::PanelMargin * 2);
        return Clamp(desiredPanelHeight, ui::layout::MinPanelHeight, maxPanelHeight);
    }

    inline RectF System::getExpandedPanelRect() const
    {
        return RectF{ m_panelPos, ui::layout::PanelWidth, getExpandedPanelHeight() };
    }

    inline RectF System::getPanelContentRect(const RectF& panelRect) const
    {
        return RectF{
            panelRect.x + ui::layout::PanelPadding,
            panelRect.y + ui::layout::HeaderHeight,
            panelRect.w - ui::layout::PanelPadding * 2 - ui::layout::ScrollbarWidth - 8,
            panelRect.h - ui::layout::HeaderHeight - ui::layout::PanelPadding
        };
    }

 inline RectF System::getCollapsedToggleRect() const
    {
       const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"EffectEditor", m_panelPos);
        return RectF{ resolvedPos, CollapsedToggleSize, CollapsedToggleSize };
    }

    inline void System::syncCollapsedIconRegistry() const
    {
        ui::editor_icon::RegisterCollapsedIcon(U"EffectEditor", m_panelCollapsed ? Optional<RectF>{ getCollapsedToggleRect() } : none);
    }

    inline void System::updateCollapsedIconDrag(const RectF& dragRect)
    {
        const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
        m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"EffectEditor", desiredPos, dragRect.size);
        syncCollapsedIconRegistry();
    }

    inline void System::expandFromCollapsedIcon()
    {
        const RectF collapsedIcon = getCollapsedToggleRect();
        m_panelCollapsed = false;
        m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(collapsedIcon, SizeF{ ui::layout::PanelWidth, getExpandedPanelHeight() });
        syncCollapsedIconRegistry();
    }
