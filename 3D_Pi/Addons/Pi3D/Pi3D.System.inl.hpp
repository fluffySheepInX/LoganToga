# pragma once

    inline System::System()
        : m_sceneSize{ Scene::Size() }
        , m_renderTexture{ std::make_unique<MSRenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float, HasDepth::Yes) }
        , m_chainA{ std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float) }
        , m_chainB{ std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float) }
        , m_fogTexture{ std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float) }
        , m_sceneDepthTexture{ std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R32_Float, HasDepth::Yes) }
        , m_dofDepthVS{ HLSL{ PiShaderLoader::HLSL(U"dof_depth"), U"VS" } | GLSL{ PiShaderLoader::GLSLVertex(U"dof_depth"), m_dofDepthVSBindings } }
        , m_dofDepthPS{ HLSL{ PiShaderLoader::HLSL(U"dof_depth"), U"PS" } | GLSL{ PiShaderLoader::GLSLFragment(U"dof_depth"), m_dofDepthPSBindings } }
    {
        applySettings(LoadSettings());
    }

    inline void System::update()
    {
        applyFramePacing();
        resizeIfNeeded();
        updateActivityState();
        syncCollapsedIconRegistry();
        m_environment.update(Scene::DeltaTime());
        m_currentBackground = m_lighting.apply();
        if (m_effectChain.onLightingPresetChanged(m_lighting.getCurrentPresetName(), m_lighting.getPresetIndex()))
        {
            m_panelScrollY = 0.0;
        }
        const PersistentSettings currentSettings = collectSettings();
        if (currentSettings != m_lastSavedSettings)
        {
            SaveSettings(currentSettings);
            m_lastSavedSettings = currentSettings;
        }
    }

    template <class DrawScene>
    void System::begin3D(const DrawScene& drawScene)
    {
        const ScopedRenderTarget3D target{ m_renderTexture->clear(m_currentBackground) };
        drawScene();
        Graphics3D::Flush();
    }

    template <class DrawScene>
    void System::end3D(const DrawScene& drawSceneForDepth)
    {
        const bool needsFog = m_environment.needsSceneDepth();
        const bool hasActiveEffects = m_effectChain.hasActiveEffects();
        const bool needsSceneDepth = (needsFog || m_effectChain.needsSceneDepth());
        if (needsSceneDepth)
        {
            const ScopedRenderTarget3D target{ m_sceneDepthTexture->clear(ColorF{ 100000.0, 0.0, 0.0, 1.0 }) };
            const ScopedCustomShader3D shader{ m_dofDepthVS, m_dofDepthPS };
            drawSceneForDepth();
            Graphics3D::Flush();
        }
        m_renderTexture->resolve();

        const auto copyResolvedSceneToFogTexture = [&]()
        {
            const ScopedRenderTarget2D rt{ *m_fogTexture };
            const ScopedRenderStates2D blend{ BlendState::Opaque };
            m_fogTexture->clear(ColorF{ 0, 0, 0, 1 });
            m_renderTexture->draw();
            Graphics2D::Flush();
        };

        const auto renderFogToFogTexture = [&]()
        {
            const ScopedRenderTarget2D rt{ *m_fogTexture };
            const ScopedRenderStates2D blend{ BlendState::Opaque };
            m_fogTexture->clear(ColorF{ 0, 0, 0, 1 });
            m_environment.applyFog(*m_renderTexture, *m_sceneDepthTexture);
            Graphics2D::Flush();
        };

        if (needsFog)
        {
            renderFogToFogTexture();

            if (hasActiveEffects)
            {
                m_effectChain.apply(*m_fogTexture, *m_chainA, *m_chainB, *m_sceneDepthTexture);
            }
            else
            {
                m_fogTexture->draw();
            }
        }
        else
        {
            if (hasActiveEffects)
            {
                copyResolvedSceneToFogTexture();
                m_effectChain.apply(*m_fogTexture, *m_chainA, *m_chainB, *m_sceneDepthTexture);
            }
            else
            {
                m_renderTexture->draw();
            }
        }
    }

    inline Environment& System::environment()
    {
        return m_environment;
    }

    inline EffectChain& System::effects()
    {
        return m_effectChain;
    }

    inline Lighting& System::lighting()
    {
        return m_lighting;
    }

    inline void System::resizeIfNeeded()
    {
        const Size currentSceneSize = Scene::Size();
        if (currentSceneSize == m_sceneSize)
        {
            return;
        }

        m_sceneSize = currentSceneSize;
        m_renderTexture = std::make_unique<MSRenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float, HasDepth::Yes);
        m_chainA = std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float);
        m_chainB = std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float);
        m_fogTexture = std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R16G16B16A16_Float);
        m_sceneDepthTexture = std::make_unique<RenderTexture>(m_sceneSize, TextureFormat::R32_Float, HasDepth::Yes);
    }

    inline PersistentSettings System::collectSettings() const
    {
        PersistentSettings settings;
        settings.lighting = m_lighting.getSettings();
        settings.environment = m_environment.getSettings();
        settings.performance = m_performanceSettings;
        settings.effects = m_effectChain.getSettings();
        settings.panelCollapsed = m_panelCollapsed;
        settings.panelPosX = m_panelPos.x;
        settings.panelPosY = m_panelPos.y;
        return settings;
    }

    inline void System::applySettings(const PersistentSettings& settings)
    {
        m_lighting.applySettings(settings.lighting);
        m_environment.applySettings(settings.environment);
        m_performanceSettings = settings.performance;
        m_performanceSettings.maxFPS = Clamp(m_performanceSettings.maxFPS, 15, 240);
        m_performanceSettings.idleFPS = Clamp(m_performanceSettings.idleFPS, 5, 120);
        m_performanceSettings.backgroundFPS = Clamp(m_performanceSettings.backgroundFPS, 1, 60);
        if (m_performanceSettings.maxFPS < m_performanceSettings.idleFPS)
        {
            m_performanceSettings.idleFPS = m_performanceSettings.maxFPS;
        }
        if (m_performanceSettings.idleFPS < m_performanceSettings.backgroundFPS)
        {
            m_performanceSettings.backgroundFPS = m_performanceSettings.idleFPS;
        }
        Graphics::SetVSyncEnabled(m_performanceSettings.vSyncEnabled);
        m_effectChain.applySettings(settings.effects);
        m_panelCollapsed = settings.panelCollapsed;
        m_panelPos = Vec2{ settings.panelPosX, settings.panelPosY };
        if (m_panelCollapsed)
        {
            m_panelPos = ui::editor_icon::GetDockedStackPosition(4);
        }
        else if (((settings.panelPosX == 16.0) && (settings.panelPosY == 16.0))
            || ((settings.panelPosX == 16.0) && (settings.panelPosY == 96.0)))
        {
            m_panelPos = ui::editor_icon::GetDockedStackPosition(4);
        }
        m_lastSavedSettings = collectSettings();
    }

    inline void System::updateActivityState()
    {
        const bool hasMouseInteraction = MouseL.pressed() || MouseR.pressed() || MouseM.pressed()
            || (Mouse::Wheel() != 0.0) || (Cursor::DeltaF().lengthSq() > 0.0);
        if (hasMouseInteraction)
        {
            m_lastInteractionClock = std::chrono::steady_clock::now();
        }
    }

    inline int32 System::getTargetFPS() const
    {
        if (not m_performanceSettings.powerSavingMode)
        {
            return m_performanceSettings.maxFPS;
        }

        if ((Scene::Width() <= 1) || (Scene::Height() <= 1))
        {
            return m_performanceSettings.backgroundFPS;
        }

        const auto now = std::chrono::steady_clock::now();
        const double idleSeconds = std::chrono::duration<double>(now - m_lastInteractionClock).count();
        if (idleSeconds >= 0.5)
        {
            return m_performanceSettings.idleFPS;
        }

        return m_performanceSettings.maxFPS;
    }

    inline void System::applyFramePacing()
    {
        const int32 targetFPS = Max(1, getTargetFPS());
        const auto now = std::chrono::steady_clock::now();
        if (m_hasLastFrameStart)
        {
            const double elapsedSec = std::chrono::duration<double>(now - m_lastFrameStartClock).count();
            const double targetSec = (1.0 / static_cast<double>(targetFPS));
            if (elapsedSec < targetSec)
            {
                std::this_thread::sleep_for(std::chrono::duration<double>(targetSec - elapsedSec));
            }
        }
        m_lastFrameStartClock = std::chrono::steady_clock::now();
        m_hasLastFrameStart = true;
    }
