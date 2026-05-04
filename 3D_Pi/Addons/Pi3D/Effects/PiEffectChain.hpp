# pragma once
# include <Siv3D.hpp>
# include "../PiSettings.hpp"
# include "../../../Effects/Effects.hpp"
# include "../../../UI/Layout.hpp"
# include "../../../UI/RectUI.hpp"

namespace Pi3D
{
	class PiEffectChain
	{
	public:
		static constexpr size_t MaxChainLength = 6;

     PiEffectChain();

		bool onLightingPresetChanged(StringView presetName, const size_t presetIndex);
		void apply(const Texture& renderTexture, const RenderTexture& chainA, const RenderTexture& chainB, const Texture& sceneDepthTexture) const;

		[[nodiscard]] double getControlSectionsHeight() const;
		[[nodiscard]] double getChainListHeight() const;
		[[nodiscard]] double getPresetHeight() const;
		[[nodiscard]] bool isPresetSectionCollapsed() const;
		[[nodiscard]] Pi3D::EffectChainSettings getSettings() const;
		void applySettings(const Pi3D::EffectChainSettings& settings);
		[[nodiscard]] double getParamsHeight() const;

		void drawUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY);
		void drawChainListUI(const Font& uiFont, Vec2& uiPos, const double contentWidth);
		void drawPresetUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY);
		void drawParamsUI(const Font& uiFont, Vec2& uiPos, const double contentWidth);

	private:
        static constexpr double CollapsedSectionHeight = 42.0;
		static constexpr double ChainSectionBaseHeight = 82.0;
		static constexpr double EffectSelectRowHeight = 30.0;

		struct PresetEntry
		{
			String key;
			String displayName;
			String description;
			Array<size_t> chain;
		};

     [[nodiscard]] static FilePath resolveTomlPath();
		[[nodiscard]] Array<PresetEntry> defaultPresets() const;
		[[nodiscard]] Array<PresetEntry> loadPresetsFromToml() const;
		[[nodiscard]] double getPresetSectionBodyHeight() const;
		[[nodiscard]] double getChainSectionHeight(const size_t index) const;
        [[nodiscard]] const pe::EffectDescriptor& getDescriptor(const size_t effectIndex) const;
		[[nodiscard]] int32 getParamRows(const pe::EffectDescriptor& descriptor) const;
		[[nodiscard]] double getParamBlockHeight(const pe::EffectDescriptor& descriptor) const;
		[[nodiscard]] String getEffectTooltip(const pe::EffectDescriptor& descriptor) const;
		[[nodiscard]] size_t findEffectIndex(StringView name) const;
		[[nodiscard]] String getCurrentPresetDisplayName() const;
		[[nodiscard]] String getCurrentPresetDescription() const;

        Array<pe::EffectDescriptor> m_descriptors;
		Array<pe::Effect> m_effects;
		Array<PresetEntry> m_presets;
		Array<size_t> m_chain = { 0 };
       Array<bool> m_chainEnabled = { true };
		bool m_presetSectionCollapsed = false;
		Optional<size_t> m_openEffectSelectIndex;
		size_t m_prevLightingPresetIndex = 0;
		bool m_hasPrevLightingPreset = false;
	};
}

# include "PiEffectChainRuntime.ipp"
# include "PiEffectChainPresets.ipp"
# include "PiEffectChainUI.ipp"
