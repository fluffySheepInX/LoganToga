# pragma once
# include <Siv3D.hpp>
# include "Environment/PiEnvironment.hpp"
# include "Effects/PiEffectChain.hpp"
# include "Lighting/PiLighting.hpp"
# include "PiSettings.hpp"
# include "Shader/PiShaderLoader.hpp"
# include "../../UI/Layout.hpp"
# include "../../UI/RectUI.hpp"

namespace Pi3D
{
	using Environment = PiEnvironment;
	using EffectChain = PiEffectChain;
	using Lighting = PiLighting;

	class System
	{
	public:
		System()
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

		void update()
		{
           resizeIfNeeded();
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
		void begin3D(const DrawScene& drawScene)
		{
            const ScopedRenderTarget3D target{ m_renderTexture->clear(m_currentBackground) };
			drawScene();
           Graphics3D::Flush();
		}

		template <class DrawScene>
		void end3D(const DrawScene& drawSceneForDepth)
		{
			{
              const ScopedRenderTarget3D target{ m_sceneDepthTexture->clear(ColorF{ 100000.0, 0.0, 0.0, 1.0 }) };
				const ScopedCustomShader3D shader{ m_dofDepthVS, m_dofDepthPS };
				drawSceneForDepth();
               Graphics3D::Flush();
			}
           m_renderTexture->resolve();
            {
				const ScopedRenderTarget2D rt{ *m_fogTexture };
				const ScopedRenderStates2D blend{ BlendState::Opaque };
				m_fogTexture->clear(ColorF{ 0, 0, 0, 1 });
				m_environment.applyFog(*m_renderTexture, *m_sceneDepthTexture);
			}
			Graphics2D::Flush();
		  m_effectChain.apply(*m_fogTexture, *m_chainA, *m_chainB, *m_sceneDepthTexture);
		}

		void drawUI()
		{
			const Font& uiFont = ui::DefaultFont();

			// --- 折りたたみ状態: 小さいトグルボタンのみ表示 ---
			if (m_panelCollapsed)
			{
             const RectF toggleRect = getCollapsedToggleRect();
				const RectF btnRect{ m_panelPos.movedBy(6, 6), CollapsedToggleSize - 12, CollapsedToggleSize - 12 };

				// ドラッグ移動
                if (MouseL.down() && toggleRect.mouseOver() && (not btnRect.mouseOver()))
				{
					m_panelDragging = true;
					m_panelDragOffset = Cursor::PosF() - m_panelPos;
				}
				if (not MouseL.pressed())
				{
					m_panelDragging = false;
				}
				if (m_panelDragging)
				{
                  const Vec2 desiredPos = (Cursor::PosF() - m_panelDragOffset);
					m_panelPos.x = Clamp(desiredPos.x, 0.0, static_cast<double>(Scene::Width()) - CollapsedToggleSize);
					m_panelPos.y = Clamp(desiredPos.y, 0.0, static_cast<double>(Scene::Height()) - CollapsedToggleSize);
				}

				ui::Panel(RectF{ m_panelPos, CollapsedToggleSize, CollapsedToggleSize });
				if (ui::Button(uiFont, U"▶", btnRect))
				{
					m_panelCollapsed = false;
				}
				return;
			}

			// --- 展開状態 ---
           const double panelHeight = getExpandedPanelHeight();

			// パネル位置をクランプ
           m_panelPos.x = Clamp(m_panelPos.x, 0.0, static_cast<double>(Scene::Width()) - ui::layout::PanelWidth);
			m_panelPos.y = Clamp(m_panelPos.y, 0.0, static_cast<double>(Scene::Height()) - panelHeight);

           const RectF panelRect = getExpandedPanelRect();

			// ヘッダーバーをドラッグで移動
			const RectF headerBar{ panelRect.x, panelRect.y, panelRect.w - 44, ui::layout::HeaderHeight };
			if (MouseL.down() && headerBar.mouseOver())
			{
				m_panelDragging = true;
				m_panelDragOffset = Cursor::PosF() - m_panelPos;
			}
			if (not MouseL.pressed())
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
			uiFont(U"効果チェイン (上から順に適用)").draw(
				panelRect.pos.movedBy(ui::layout::PanelPadding, 12), ui::GetTheme().text);

			// 折りたたみボタン
			const RectF collapseBtn{ panelRect.rightX() - 44, panelRect.y + 8, 30, 28 };
			if (ui::Button(uiFont, U"◀", collapseBtn))
			{
				m_panelCollapsed = true;
			}

			// 中ボタンドラッグで高速スクロール
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
					[&]() { m_environment.drawUI(uiFont, uiPos, contentRect.w); }, Environment::HeaderHeight);
				drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"ライティング", m_lightingCollapsed,
					[&]() { m_lighting.drawUI(uiFont, uiPos, contentRect.w); }, m_lighting.getHeaderHeight());
				drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"エフェクトチェイン", m_chainCollapsed,
					[&]() { m_effectChain.drawChainListUI(uiFont, uiPos, contentRect.w); });
				drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"プリセット", m_effectPresetCollapsed,
					[&]() { m_effectChain.drawPresetUI(uiFont, uiPos, contentRect.w, m_panelScrollY); });
				drawCollapsibleSection(uiFont, uiPos, contentRect.w, U"パラメータ", m_effectParamsCollapsed,
					[&]() { m_effectChain.drawParamsUI(uiFont, uiPos, contentRect.w); });

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

		Environment& environment() { return m_environment; }
		EffectChain& effects() { return m_effectChain; }
		Lighting& lighting() { return m_lighting; }

		[[nodiscard]] bool wantsMouseWheelCapture() const
		{
			if (m_panelCollapsed)
			{
				return getCollapsedToggleRect().mouseOver();
			}

			return getExpandedPanelRect().mouseOver();
		}

	private:
     [[nodiscard]] double getPanelContentHeight() const
		{
			const double environmentSectionBodyHeight = m_environmentCollapsed ? Environment::HeaderHeight : Environment::UIBodyHeight;
			const double environmentSectionHeight = environmentSectionBodyHeight + ui::layout::SectionGap;
			const double lightingSectionBodyHeight = m_lightingCollapsed ? m_lighting.getHeaderHeight() : m_lighting.getUIBodyHeight();
			const double lightingSectionHeight = lightingSectionBodyHeight + ui::layout::SectionGap;
			const double chainListHeight = m_chainCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : m_effectChain.getChainListHeight();
			const double presetHeight = m_effectPresetCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : m_effectChain.getPresetHeight();
			const double paramsHeight = m_effectParamsCollapsed ? (CollapsedSectionHeight + ui::layout::SectionGap) : m_effectChain.getParamsHeight();
			return environmentSectionHeight + lightingSectionHeight + chainListHeight + presetHeight + paramsHeight;
		}

		[[nodiscard]] double getExpandedPanelHeight() const
		{
			const double desiredPanelHeight = 46 + getPanelContentHeight();
			const double maxPanelHeight = (Scene::Height() - ui::layout::PanelMargin * 2);
			return Clamp(desiredPanelHeight, ui::layout::MinPanelHeight, maxPanelHeight);
		}

		[[nodiscard]] RectF getExpandedPanelRect() const
		{
			return RectF{ m_panelPos, ui::layout::PanelWidth, getExpandedPanelHeight() };
		}

		[[nodiscard]] RectF getPanelContentRect(const RectF& panelRect) const
		{
			return RectF{
				panelRect.x + ui::layout::PanelPadding,
				panelRect.y + ui::layout::HeaderHeight,
				panelRect.w - ui::layout::PanelPadding * 2 - ui::layout::ScrollbarWidth - 8,
				panelRect.h - ui::layout::HeaderHeight - ui::layout::PanelPadding
			};
		}

		[[nodiscard]] RectF getCollapsedToggleRect() const
		{
			return RectF{ m_panelPos, CollapsedToggleSize, CollapsedToggleSize };
		}

      void resizeIfNeeded()
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

		template <class DrawBody>
		void drawCollapsibleSection(const Font& uiFont, Vec2& uiPos, const double contentWidth, StringView title,
			bool& collapsed, const DrawBody& drawBody, const double collapsedHeight = CollapsedSectionHeight)
		{
			if (not collapsed)
			{
             const Vec2 sectionTop = uiPos;
				drawBody();
             const RectF collapseRect{ sectionTop.x + contentWidth - 38, sectionTop.y + 6, 30, 26 };
				if (ui::Button(uiFont, U"-", collapseRect))
				{
					collapsed = true;
				}
				return;
			}

			const RectF sectionRect{ uiPos, contentWidth, collapsedHeight };
			ui::Section(sectionRect);
			uiFont(title).draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
			const RectF collapseRect{ sectionRect.rightX() - 38, sectionRect.y + 6, 30, 26 };
			if (ui::Button(uiFont, U"+", collapseRect))
			{
				collapsed = false;
			}
			uiPos.y += sectionRect.h + ui::layout::SectionGap;
		}

		[[nodiscard]] PersistentSettings collectSettings() const
		{
			PersistentSettings settings;
			settings.lighting = m_lighting.getSettings();
			settings.environment = m_environment.getSettings();
			settings.effects = m_effectChain.getSettings();
			settings.panelCollapsed = m_panelCollapsed;
			settings.panelPosX = m_panelPos.x;
			settings.panelPosY = m_panelPos.y;
			return settings;
		}

		void applySettings(const PersistentSettings& settings)
		{
			m_lighting.applySettings(settings.lighting);
			m_environment.applySettings(settings.environment);
			m_effectChain.applySettings(settings.effects);
			m_panelCollapsed = settings.panelCollapsed;
			m_panelPos = Vec2{ settings.panelPosX, settings.panelPosY };
            if ((settings.panelPosX == 16.0) && (settings.panelPosY == 16.0))
			{
				m_panelPos.y = 96.0;
			}
			m_lastSavedSettings = collectSettings();
		}

		Environment m_environment;
		EffectChain m_effectChain;
		Lighting m_lighting;
		ColorF m_currentBackground{ 0.4, 0.6, 0.8, 1.0 };
       Size m_sceneSize;
		double m_panelScrollY = 0.0;
		bool m_isScrollThumbDragging = false;
		double m_scrollThumbGrabOffsetY = 0.0;
		const Array<ConstantBufferBinding> m_dofDepthVSBindings = {
			{ U"VSPerView", 1 },
			{ U"VSPerObject", 2 },
			{ U"VSPerMaterial", 3 },
		};
		const Array<ConstantBufferBinding> m_dofDepthPSBindings = {
			{ U"PSPerView", 1 },
			{ U"PSPerMaterial", 3 },
		};
      std::unique_ptr<MSRenderTexture> m_renderTexture;
		std::unique_ptr<RenderTexture> m_chainA;
		std::unique_ptr<RenderTexture> m_chainB;
     std::unique_ptr<RenderTexture> m_fogTexture;
		std::unique_ptr<RenderTexture> m_sceneDepthTexture;
	 static constexpr double CollapsedSectionHeight = 42.0;
		static constexpr double CollapsedToggleSize = 48.0;
		bool m_environmentCollapsed = false;
		bool m_lightingCollapsed = false;
		bool m_chainCollapsed = false;
		bool m_effectPresetCollapsed = false;
		bool m_effectParamsCollapsed = false;
		bool m_panelCollapsed = false;
		Vec2 m_panelPos{ ui::layout::PanelMargin, ui::layout::PanelMargin };
		bool m_panelDragging = false;
		Vec2 m_panelDragOffset{ 0, 0 };
		PersistentSettings m_lastSavedSettings;
		const VertexShader m_dofDepthVS;
		const PixelShader m_dofDepthPS;
	};

	inline std::unique_ptr<System>& InstanceStorage()
	{
		static std::unique_ptr<System> s_instance;
		return s_instance;
	}

	inline bool RegisterAddon()
	{
		if (not InstanceStorage())
		{
			InstanceStorage() = std::make_unique<System>();
		}
		return true;
	}

	inline System& Instance()
	{
		if (not InstanceStorage())
		{
			RegisterAddon();
		}
		return *InstanceStorage();
	}

	inline void Update()
	{
		Instance().update();
	}

	template <class DrawScene>
	inline void Begin3D(const DrawScene& drawScene)
	{
		Instance().begin3D(drawScene);
	}

	template <class DrawScene>
	inline void End3D(const DrawScene& drawSceneForDepth)
	{
		Instance().end3D(drawSceneForDepth);
	}

	inline void DrawUI()
	{
		Instance().drawUI();
	}

	inline bool WantsMouseWheelCapture()
	{
		return Instance().wantsMouseWheelCapture();
	}

	inline Environment& EnvironmentRef()
	{
		return Instance().environment();
	}

	inline EffectChain& Effects()
	{
		return Instance().effects();
	}

	inline Lighting& LightingRef()
	{
		return Instance().lighting();
	}
}
