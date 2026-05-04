namespace Pi3D
{
    inline PiEffectChain::PiEffectChain()
        : m_descriptors{ pe::GetDefaultEffectDescriptors() }
        , m_effects{ pe::CreateDefaultEffects() }
        , m_presets{ loadPresetsFromToml() }
    {
        if (m_presets.isEmpty())
        {
            m_presets = defaultPresets();
        }
    }

    inline bool PiEffectChain::onLightingPresetChanged(StringView presetName, const size_t presetIndex)
    {
        if (not m_hasPrevLightingPreset)
        {
            m_prevLightingPresetIndex = presetIndex;
            m_hasPrevLightingPreset = true;
            return false;
        }

        if (presetIndex == m_prevLightingPresetIndex)
        {
            return false;
        }

        bool inserted = false;
        const size_t acesEffectIndex = findEffectIndex(U"Tonemap (ACES)");
        if ((presetName == U"マジックアワー")
            && (acesEffectIndex != 0)
            && (not m_chain.contains(acesEffectIndex)))
        {
            if ((m_chain.size() == 1) && (m_chain[0] == 0))
            {
                m_chain[0] = acesEffectIndex;
                inserted = true;
            }
            else if (m_chain.size() < MaxChainLength)
            {
                m_chain.push_back(acesEffectIndex);
                inserted = true;
            }
        }

        m_prevLightingPresetIndex = presetIndex;
        return inserted;
    }

    inline void PiEffectChain::apply(const Texture& renderTexture, const RenderTexture& chainA, const RenderTexture& chainB, const Texture& sceneDepthTexture) const
    {
        Graphics3D::Flush();
        pe::EffectContext context;
        context.sceneDepthTexture = sceneDepthTexture;
        context.sourceSize = renderTexture.size();
        context.time = Scene::Time();

        Array<size_t> activeChain;
        for (size_t i = 0; i < m_chain.size(); ++i)
        {
            if ((i < m_chainEnabled.size()) && m_chainEnabled[i])
            {
                activeChain << m_chain[i];
            }
        }
        if (activeChain.isEmpty())
        {
            renderTexture.draw();
            return;
        }

        const Texture* input = &renderTexture;
        const RenderTexture* targets[2] = { &chainA, &chainB };
        size_t targetIdx = 0;

        for (size_t i = 0; i < activeChain.size(); ++i)
        {
            const pe::Effect& e = m_effects[activeChain[i]];
            const bool isLast = (i + 1 == activeChain.size());

            if (isLast)
            {
                e.apply(*input, context);
            }
            else
            {
                const RenderTexture* dst = targets[targetIdx];
                {
                    const ScopedRenderTarget2D rt{ *dst };
                    const ScopedRenderStates2D blend{ BlendState::Opaque };
                    dst->clear(ColorF{ 0, 0, 0, 1 });
                    e.apply(*input, context);
                }
                Graphics2D::Flush();
                input = dst;
                targetIdx ^= 1;
            }
        }
    }

    inline Pi3D::EffectChainSettings PiEffectChain::getSettings() const
    {
        Pi3D::EffectChainSettings settings;
        settings.chainEffectNames.clear();
        settings.chainEnabled.clear();
        for (const size_t effectIndex : m_chain)
        {
            settings.chainEffectNames << String{ getDescriptor(effectIndex).name };
        }
        settings.chainEnabled = m_chainEnabled;
        if (settings.chainEffectNames.isEmpty())
        {
            settings.chainEffectNames = { U"なし" };
            settings.chainEnabled = { true };
        }
        return settings;
    }

    inline void PiEffectChain::applySettings(const Pi3D::EffectChainSettings& settings)
    {
        Array<size_t> resolvedChain;
        for (const auto& effectName : settings.chainEffectNames)
        {
            const size_t effectIndex = findEffectIndex(effectName);
            if ((effectIndex != 0) || (effectName == getDescriptor(0).name))
            {
                resolvedChain << effectIndex;
            }
        }
        if (resolvedChain.isEmpty())
        {
            resolvedChain = { 0 };
        }
        m_chain = resolvedChain;
        m_chainEnabled = settings.chainEnabled;
        if (m_chainEnabled.size() < m_chain.size())
        {
            m_chainEnabled.resize(m_chain.size(), true);
        }
        else if (m_chain.size() < m_chainEnabled.size())
        {
            m_chainEnabled.resize(m_chain.size());
        }
    }

    inline size_t PiEffectChain::findEffectIndex(StringView name) const
    {
        for (size_t i = 0; i < m_descriptors.size(); ++i)
        {
            if (m_descriptors[i].name == name)
            {
                return i;
            }
        }
        return 0;
    }
}
