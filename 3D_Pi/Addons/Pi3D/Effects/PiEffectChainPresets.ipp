namespace Pi3D
{
    inline FilePath PiEffectChain::resolveTomlPath()
    {
        const Array<FilePath> candidates = {
            U"Addons/Pi3D/Resources/toml/effect_presets.toml",
            U"../Addons/Pi3D/Resources/toml/effect_presets.toml",
            U"3D_Pi/Addons/Pi3D/Resources/toml/effect_presets.toml",
        };
        for (const auto& p : candidates)
        {
            if (FileSystem::Exists(p))
            {
                return p;
            }
        }
        return candidates.front();
    }

    inline Array<PiEffectChain::PresetEntry> PiEffectChain::defaultPresets() const
    {
        return {
            PresetEntry{ U"cinematic", U"Cinematic", U"Bloom と ACES を軸にした映画風のチェイン", pe::GetCinematicPresetChain(m_descriptors) },
            PresetEntry{ U"dusty", U"Dusty (古い洋ゲー風)", U"黄土色寄り、乾いた空気感、古い洋ゲー風の画作り", pe::GetDustyPresetChain(m_descriptors) },
        };
    }

    inline Array<PiEffectChain::PresetEntry> PiEffectChain::loadPresetsFromToml() const
    {
        Array<PresetEntry> results;
        const TOMLReader toml{ resolveTomlPath() };
        if (not toml)
        {
            return results;
        }

        try
        {
            for (const auto& t : toml[U"presets"].tableArrayView())
            {
                PresetEntry e;
                e.key = t[U"key"].getOpt<String>().value_or(U"custom");
                e.displayName = t[U"displayName"].getOpt<String>().value_or(U"Custom");
                e.description = t[U"description"].getOpt<String>().value_or(U"");

                Array<size_t> chain;
                for (const auto& effectNameNode : t[U"effects"].arrayView())
                {
                    if (const auto effectName = effectNameNode.getOpt<String>())
                    {
                        const size_t idx = findEffectIndex(*effectName);
                        if (idx != 0)
                        {
                            chain << idx;
                        }
                    }
                }
                if (chain.isEmpty())
                {
                    chain = { 0 };
                }
                e.chain = chain;
                results << e;
            }
        }
        catch (const std::exception&)
        {
        }

        return results;
    }

    inline double PiEffectChain::getPresetSectionBodyHeight() const
    {
        return 92.0 + (m_presets.size() + 1) * (ui::layout::AddButtonHeight + 4.0);
    }

    inline double PiEffectChain::getPresetHeight() const
    {
        return (isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight()) + ui::layout::SectionGap;
    }

    inline bool PiEffectChain::isPresetSectionCollapsed() const
    {
        return m_presetSectionCollapsed;
    }

    inline String PiEffectChain::getCurrentPresetDisplayName() const
    {
        for (const auto& p : m_presets)
        {
            if (m_chain == p.chain)
            {
                return p.displayName;
            }
        }
        if ((m_chain.size() == 1) && (m_chain[0] == 0))
        {
            return U"なし";
        }
        return U"Custom";
    }

    inline String PiEffectChain::getCurrentPresetDescription() const
    {
        for (const auto& p : m_presets)
        {
            if (m_chain == p.chain)
            {
                return p.description;
            }
        }
        if ((m_chain.size() == 1) && (m_chain[0] == 0))
        {
            return U"ポストエフェクトなし";
        }
        return U"手動編集されたチェイン";
    }
}
