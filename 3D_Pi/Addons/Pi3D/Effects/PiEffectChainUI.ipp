namespace Pi3D
{
    inline double PiEffectChain::getControlSectionsHeight() const
    {
        const double chainControlHeight = ui::layout::AddButtonHeight + ui::layout::SectionGap;
        const double presetSectionBodyHeight = isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight();
        const double presetSectionHeight = presetSectionBodyHeight + ui::layout::SectionGap;
        double chainListHeight = 0.0;
        for (size_t i = 0; i < m_chain.size(); ++i)
        {
            chainListHeight += getChainSectionHeight(i) + ui::layout::SectionGap;
        }
        return chainListHeight + chainControlHeight + presetSectionHeight;
    }

    inline double PiEffectChain::getChainListHeight() const
    {
        double height = ui::layout::AddButtonHeight + ui::layout::SectionGap;
        for (size_t i = 0; i < m_chain.size(); ++i)
        {
            height += getChainSectionHeight(i) + ui::layout::SectionGap;
        }
        return height;
    }

    inline double PiEffectChain::getParamsHeight() const
    {
        double paramsHeight = 0.0;
        for (const size_t effectIndex : m_chain)
        {
            const auto& descriptor = getDescriptor(effectIndex);
            const double paramBlockHeight = getParamBlockHeight(descriptor);
            if (0.0 < paramBlockHeight)
            {
                paramsHeight += (28.0 + paramBlockHeight + ui::layout::SectionGap);
            }
        }
        return paramsHeight;
    }

    inline void PiEffectChain::drawUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY)
    {
        drawChainListUI(uiFont, uiPos, contentWidth);
        drawPresetUI(uiFont, uiPos, contentWidth, panelScrollY);
        drawParamsUI(uiFont, uiPos, contentWidth);
    }

    inline void PiEffectChain::drawChainListUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
    {
        for (size_t i = 0; i < m_chain.size(); ++i)
        {
            const auto& descriptor = getDescriptor(m_chain[i]);
            const double sectionHeight = getChainSectionHeight(i);
            const RectF sectionRect{ uiPos, contentWidth, sectionHeight };
            ui::Section(sectionRect);

            uiFont(U"[{}]"_fmt(i + 1)).draw(sectionRect.pos.movedBy(10, 8), ui::GetTheme().text);

            const RectF selectRect{ sectionRect.x + 8, sectionRect.y + 36, contentWidth - 144, ui::layout::ButtonSize };
            if (ui::Button(uiFont, U"{} ▾"_fmt(descriptor.name), selectRect))
            {
                if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == i))
                {
                    m_openEffectSelectIndex.reset();
                }
                else
                {
                    m_openEffectSelectIndex = i;
                }
            }
            if (selectRect.mouseOver())
            {
                ui::Tooltip(uiFont, getEffectTooltip(descriptor), Cursor::PosF().movedBy(18, 20));
            }

            const RectF enableRect{ sectionRect.rightX() - 88, sectionRect.y + 8, 80, ui::layout::ButtonSize };
            if (ui::Button(uiFont, m_chainEnabled[i] ? U"ON" : U"OFF", enableRect))
            {
                m_chainEnabled[i] = (not m_chainEnabled[i]);
            }
            if (m_chainEnabled[i])
            {
                enableRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
            }

            const RectF removeRect{ sectionRect.rightX() - 80, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
            if ((m_chain.size() > 1) && ui::Button(uiFont, U"×", removeRect))
            {
                m_chain.erase(m_chain.begin() + i);
                m_chainEnabled.erase(m_chainEnabled.begin() + i);
                m_openEffectSelectIndex.reset();
                break;
            }

            if (i > 0)
            {
                const RectF upRect{ sectionRect.rightX() - 160, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
                if (ui::Button(uiFont, U"↑", upRect))
                {
                    std::swap(m_chain[i], m_chain[i - 1]);
                    std::swap(m_chainEnabled[i], m_chainEnabled[i - 1]);
                }
            }

            if ((i + 1) < m_chain.size())
            {
                const RectF downRect{ sectionRect.rightX() - 120, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
                if (ui::Button(uiFont, U"↓", downRect))
                {
                    std::swap(m_chain[i], m_chain[i + 1]);
                    std::swap(m_chainEnabled[i], m_chainEnabled[i + 1]);
                }
            }

            if (m_chain.size() < MaxChainLength)
            {
                const RectF duplicateRect{ sectionRect.rightX() - 40, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
                if (ui::Button(uiFont, U"+", duplicateRect))
                {
                    m_chain.insert(m_chain.begin() + i + 1, m_chain[i]);
                    m_chainEnabled.insert(m_chainEnabled.begin() + i + 1, m_chainEnabled[i]);
                    m_openEffectSelectIndex.reset();
                    break;
                }
            }

            if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == i))
            {
                const RectF listRect{ sectionRect.x + 8, sectionRect.y + ChainSectionBaseHeight, contentWidth - 16, m_descriptors.size() * EffectSelectRowHeight };
                for (size_t effectIndex = 0; effectIndex < m_descriptors.size(); ++effectIndex)
                {
                    const auto& listedDescriptor = getDescriptor(effectIndex);
                    const RectF rowRect{ listRect.x, listRect.y + effectIndex * EffectSelectRowHeight, listRect.w, EffectSelectRowHeight - 2 };
                    const bool selected = (m_chain[i] == effectIndex);
                    const bool hovered = rowRect.mouseOver();
                    const ColorF fill = selected ? ColorF{ 0.88, 0.94, 1.0, 1.0 } : (hovered ? ui::GetTheme().itemHovered : ui::GetTheme().item);
                    rowRect.rounded(5).draw(fill);
                    rowRect.rounded(5).drawFrame(1.0, ui::GetTheme().panelBorder);
                    uiFont(listedDescriptor.name).draw(rowRect.x + 10, rowRect.y + 3, ui::GetTheme().text);
                    uiFont(listedDescriptor.category).draw(rowRect.x + 150, rowRect.y + 3, Palette::Gray);
                    if (listedDescriptor.requiresSceneDepth)
                    {
                        uiFont(U"D").draw(rowRect.rightX() - 22, rowRect.y + 3, Palette::Indianred);
                    }
                    if (hovered)
                    {
                        ui::Tooltip(uiFont, getEffectTooltip(listedDescriptor), Cursor::PosF().movedBy(18, 20));
                    }
                    if (rowRect.leftClicked())
                    {
                        m_chain[i] = effectIndex;
                        m_openEffectSelectIndex.reset();
                    }
                }
            }

            uiPos.y += sectionHeight + ui::layout::SectionGap;
        }

        const RectF addRect{ uiPos, ui::layout::AddButtonWidth, ui::layout::AddButtonHeight };
        if ((m_chain.size() < MaxChainLength) && ui::Button(uiFont, U"+ 段を追加", addRect))
        {
            m_chain.push_back(0);
            m_chainEnabled.push_back(true);
        }
        uiPos.y += ui::layout::AddButtonHeight + ui::layout::SectionGap;
    }

    inline void PiEffectChain::drawPresetUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY)
    {
        const double presetSectionBodyHeight = isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight();
        const RectF presetSectionRect{ uiPos, contentWidth, presetSectionBodyHeight };
        ui::Section(presetSectionRect);
        uiFont(U"プリセット").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
        if (m_presetSectionCollapsed)
        {
            uiPos.y += presetSectionRect.h + ui::layout::SectionGap;
            return;
        }
        uiFont(U"現在: {}"_fmt(getCurrentPresetDisplayName())).draw(uiPos.movedBy(8, 28), Palette::Dimgray);
        uiFont(getCurrentPresetDescription()).draw(uiPos.movedBy(8, 54), Palette::Gray);

        double presetY = 88.0;
        for (const auto& preset : m_presets)
        {
            const RectF presetRect{ uiPos.x + 8, uiPos.y + presetY, contentWidth - 16, ui::layout::AddButtonHeight };
            if (ui::Button(uiFont, preset.displayName, presetRect))
            {
                m_chain = preset.chain;
                m_chainEnabled.assign(m_chain.size(), true);
                panelScrollY = 0.0;
            }
            presetY += (ui::layout::AddButtonHeight + 4.0);
        }

        const RectF noneRect{ uiPos.x + 8, uiPos.y + presetY, contentWidth - 16, ui::layout::AddButtonHeight };
        if (ui::Button(uiFont, U"なしに戻す", noneRect))
        {
            m_chain = { 0 };
            m_chainEnabled = { true };
            panelScrollY = 0.0;
        }

        uiPos.y += presetSectionRect.h + ui::layout::SectionGap;
    }

    inline void PiEffectChain::drawParamsUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
    {
        for (size_t i = 0; i < m_chain.size(); ++i)
        {
            const pe::Effect& e = m_effects[m_chain[i]];
            const auto& descriptor = getDescriptor(m_chain[i]);
            if (e.drawUI)
            {
                const double paramBlockHeight = getParamBlockHeight(descriptor);
                const RectF paramSectionRect{ uiPos, contentWidth, 28 + paramBlockHeight };
                ui::Section(paramSectionRect);
              uiFont(U"[{}]"_fmt(i + 1)).draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
                uiFont(descriptor.name).draw(uiPos.movedBy(44, 0), ui::GetTheme().text);
                uiFont(descriptor.category).draw(paramSectionRect.rightX() - 220, paramSectionRect.y, Palette::Gray);
                if (descriptor.requiresSceneDepth)
                {
                    uiFont(U"[Depth]").draw(paramSectionRect.rightX() - 140, paramSectionRect.y, Palette::Indianred);
                }
                if (not m_chainEnabled[i])
                {
                    uiFont(U"(OFF)").draw(paramSectionRect.rightX() - 70, paramSectionRect.y, Palette::Dimgray);
                }
                if (e.reset)
                {
                    const RectF resetRect{ paramSectionRect.rightX() - 78, paramSectionRect.y + 4, 70, 24 };
                    if (ui::Button(uiFont, U"Reset", resetRect))
                    {
                        e.reset();
                    }
                }
                e.drawUI(uiPos.movedBy(8, 28));
                uiPos.y += paramSectionRect.h + ui::layout::SectionGap;
            }
        }
    }

    inline double PiEffectChain::getChainSectionHeight(const size_t index) const
    {
        if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == index))
        {
            return ChainSectionBaseHeight + m_descriptors.size() * EffectSelectRowHeight + 8.0;
        }
        return ChainSectionBaseHeight;
    }

    inline const pe::EffectDescriptor& PiEffectChain::getDescriptor(const size_t effectIndex) const
    {
        return m_descriptors[effectIndex];
    }

    inline int32 PiEffectChain::getParamRows(const pe::EffectDescriptor& descriptor) const
    {
       return descriptor.uiRowCount;
    }

    inline double PiEffectChain::getParamBlockHeight(const pe::EffectDescriptor& descriptor) const
    {
        const int32 rows = getParamRows(descriptor);
        if (rows <= 0)
        {
            return 0.0;
        }
        return (40.0 * rows);
    }

    inline String PiEffectChain::getEffectTooltip(const pe::EffectDescriptor& descriptor) const
    {
        String tooltip = U"カテゴリ: {}"_fmt(descriptor.category);
        if (not descriptor.tooltip.isEmpty())
        {
            tooltip += U"\n{}"_fmt(descriptor.tooltip);
        }
        if (descriptor.requiresSceneDepth)
        {
            tooltip += U"\n要件: シーン深度テクスチャが必要です。";
        }
        return tooltip;
    }
}
