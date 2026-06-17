# pragma once
# include <Siv3D.hpp>
# include <chrono>
# include <thread>
# include "Environment/PiEnvironment.hpp"
# include "Effects/PiEffectChain.hpp"
# include "Lighting/PiLighting.hpp"
# include "PiSettings.hpp"
# include "Shader/PiShaderLoader.hpp"
# include "../../UI/Layout.hpp"
# include "../../UI/RectUI.hpp"
# include "../../UI/EditorIconLayout.hpp"

namespace Pi3D
{
	using Environment = PiEnvironment;
	using EffectChain = PiEffectChain;
	using Lighting = PiLighting;

	class System
	{
	public:
		System();

      void update(const Optional<Vec3>& cameraEye = none, const Optional<Vec3>& cameraFocus = none);

		template <class DrawScene>
		void begin3D(const DrawScene& drawScene);

		template <class DrawScene>
		void end3D(const DrawScene& drawSceneForDepth);

		void drawUI();

		Environment& environment();
		EffectChain& effects();
		Lighting& lighting();

		[[nodiscard]] bool wantsMouseWheelCapture() const;

	private:
		static constexpr double PerformanceSectionHeight = 244.0;
		static constexpr double CollapsedSectionHeight = 42.0;
     static constexpr double CollapsedToggleSize = ui::editor_icon::CollapsedIconSize;

		[[nodiscard]] double getPanelContentHeight() const;
		void drawPerformanceUI(const Font& uiFont, Vec2& uiPos, const double contentWidth);
		[[nodiscard]] double getExpandedPanelHeight() const;
		[[nodiscard]] RectF getExpandedPanelRect() const;
		[[nodiscard]] RectF getPanelContentRect(const RectF& panelRect) const;
		[[nodiscard]] RectF getCollapsedToggleRect() const;
      void syncCollapsedIconRegistry() const;
		void updateCollapsedIconDrag(const RectF& dragRect);
		void expandFromCollapsedIcon();
		void resizeIfNeeded();

		template <class DrawBody>
		void drawCollapsibleSection(const Font& uiFont, Vec2& uiPos, const double contentWidth, StringView title,
			bool& collapsed, const DrawBody& drawBody, const double collapsedHeight = CollapsedSectionHeight, const bool enableHeaderToggle = true);

		[[nodiscard]] PersistentSettings collectSettings() const;
		void applySettings(const PersistentSettings& settings);
		void updateActivityState();
		[[nodiscard]] int32 getTargetFPS() const;
		void applyFramePacing();

		Environment m_environment;
		EffectChain m_effectChain;
		Lighting m_lighting;
		PerformanceSettings m_performanceSettings;
		ColorF m_currentBackground{ 0.4, 0.6, 0.8, 1.0 };
		Size m_sceneSize;
		std::chrono::steady_clock::time_point m_lastFrameStartClock = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point m_lastUpdateClock = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_lastInteractionClock = std::chrono::steady_clock::now();
		bool m_hasLastFrameStart = false;
        bool m_hasLastUpdateClock = false;
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
		std::unique_ptr<RenderTexture> m_underwaterTexture;
		std::unique_ptr<RenderTexture> m_sceneDepthTexture;
		bool m_environmentCollapsed = false;
		bool m_lightingCollapsed = false;
		bool m_chainCollapsed = false;
		bool m_effectPresetCollapsed = false;
		bool m_effectParamsCollapsed = false;
		bool m_performanceCollapsed = false;
		bool m_panelCollapsed = false;
        Vec2 m_panelPos{ ui::editor_icon::GetDockedStackPosition(4) };
		bool m_panelDragging = false;
		Vec2 m_panelDragOffset{ 0, 0 };
        Texture m_toggleIcon{ U"texture/effectEditor.png" };
        Texture m_effectHelpIcon{ U"texture/hatena.png" };
		PersistentSettings m_lastSavedSettings;
		const VertexShader m_dofDepthVS;
		const PixelShader m_dofDepthPS;
	};

# include "Pi3D.System.inl.hpp"
# include "Pi3D.UI.inl.hpp"

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

    inline void Update(const Optional<Vec3>& cameraEye = none, const Optional<Vec3>& cameraFocus = none)
	{
        Instance().update(cameraEye, cameraFocus);
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
