# include "SkyAppUiLayoutProfile.hpp"

namespace SkyAppUiLayout
{
	namespace
	{
		inline constexpr StringView UiLayoutProfilePath = U"settings/ui_layout_profile.toml";
		inline constexpr int32 UiLayoutProfileSchemaVersion = 1;

		const UiLayoutProfile DefaultUiLayoutProfile{};

		template <class Metrics>
		struct IntMetricFieldEntry
		{
			StringView key;
			int32 Metrics::* member = nullptr;
		};

		template <class Metrics>
		struct IntMetricClampEntry
		{
			int32 Metrics::* member = nullptr;
			int32 minValue = 0;
			int32 maxValue = 0;
		};

		void LoadTomlInt(const TOMLReader& toml, StringView section, StringView key, int32& value)
		{
			try
			{
				value = static_cast<int32>(toml[String{ section }][String{ key }].get<int64>());
			}
			catch (const std::exception&)
			{
			}
		}

		template <class Metrics, size_t N>
		void LoadMetricSection(const TOMLReader& toml,
			StringView section,
			Metrics& metrics,
			const std::array<IntMetricFieldEntry<Metrics>, N>& entries)
		{
			for (const auto& entry : entries)
			{
				LoadTomlInt(toml, section, entry.key, metrics.*(entry.member));
			}
		}

		template <class Metrics, size_t N>
		void ClampMetricValues(Metrics& metrics, const std::array<IntMetricClampEntry<Metrics>, N>& entries)
		{
			for (const auto& entry : entries)
			{
				metrics.*(entry.member) = Clamp(metrics.*(entry.member), entry.minValue, entry.maxValue);
			}
		}

		void LoadSharedLayoutMetrics(const TOMLReader& toml, SharedLayoutMetrics& metrics)
		{
          static constexpr std::array<IntMetricFieldEntry<SharedLayoutMetrics>, 5> entries{{
				{ U"panelMargin", &SharedLayoutMetrics::panelMargin },
				{ U"panelGap", &SharedLayoutMetrics::panelGap },
				{ U"uiEditGridCellSize", &SharedLayoutMetrics::uiEditGridCellSize },
				{ U"uiEditGridMajorLineSpan", &SharedLayoutMetrics::uiEditGridMajorLineSpan },
				{ U"accordionHeaderHeight", &SharedLayoutMetrics::accordionHeaderHeight },
			}};
			LoadMetricSection(toml, U"shared", metrics, entries);
		}

		void LoadRightColumnLayoutMetrics(const TOMLReader& toml, RightColumnLayoutMetrics& metrics)
		{
         static constexpr std::array<IntMetricFieldEntry<RightColumnLayoutMetrics>, 1> entries{{
				{ U"width", &RightColumnLayoutMetrics::width },
			}};
			LoadMetricSection(toml, U"rightColumn", metrics, entries);
		}

		void LoadResourcePanelLayoutMetrics(const TOMLReader& toml, ResourcePanelLayoutMetrics& metrics)
		{
         static constexpr std::array<IntMetricFieldEntry<ResourcePanelLayoutMetrics>, 6> entries{{
				{ U"minWidth", &ResourcePanelLayoutMetrics::minWidth },
				{ U"resizeHandleSize", &ResourcePanelLayoutMetrics::resizeHandleSize },
				{ U"iconButtonSize", &ResourcePanelLayoutMetrics::iconButtonSize },
				{ U"iconButtonGap", &ResourcePanelLayoutMetrics::iconButtonGap },
				{ U"collapsedHeight", &ResourcePanelLayoutMetrics::collapsedHeight },
				{ U"expandedHeight", &ResourcePanelLayoutMetrics::expandedHeight },
			}};
			LoadMetricSection(toml, U"resourcePanel", metrics, entries);
		}

		void LoadModelHeightLayoutMetrics(const TOMLReader& toml, ModelHeightLayoutMetrics& metrics)
		{
           static constexpr std::array<IntMetricFieldEntry<ModelHeightLayoutMetrics>, 2> entries{{
				{ U"panelWidth", &ModelHeightLayoutMetrics::panelWidth },
				{ U"panelHeight", &ModelHeightLayoutMetrics::panelHeight },
			}};
			LoadMetricSection(toml, U"modelHeight", metrics, entries);
		}

		void LoadBottomControlLayoutMetrics(const TOMLReader& toml, BottomControlLayoutMetrics& metrics)
		{
           static constexpr std::array<IntMetricFieldEntry<BottomControlLayoutMetrics>, 7> entries{{
				{ U"yOffset", &BottomControlLayoutMetrics::yOffset },
				{ U"panelWidth", &BottomControlLayoutMetrics::panelWidth },
				{ U"panelHeight", &BottomControlLayoutMetrics::panelHeight },
				{ U"checkBoxWidth", &BottomControlLayoutMetrics::checkBoxWidth },
				{ U"buttonGap", &BottomControlLayoutMetrics::buttonGap },
				{ U"editorIconButtonSize", &BottomControlLayoutMetrics::editorIconButtonSize },
				{ U"editorIconButtonGap", &BottomControlLayoutMetrics::editorIconButtonGap },
			}};
			LoadMetricSection(toml, U"bottomControl", metrics, entries);
		}

		void LoadMiniMapLayoutMetrics(const TOMLReader& toml, MiniMapLayoutMetrics& metrics)
		{
         static constexpr std::array<IntMetricFieldEntry<MiniMapLayoutMetrics>, 4> entries{{
				{ U"minWidth", &MiniMapLayoutMetrics::minWidth },
				{ U"minHeight", &MiniMapLayoutMetrics::minHeight },
				{ U"resizeHandleSize", &MiniMapLayoutMetrics::resizeHandleSize },
				{ U"expandedHeight", &MiniMapLayoutMetrics::expandedHeight },
			}};
			LoadMetricSection(toml, U"miniMap", metrics, entries);
		}

		void LoadSkySettingsLayoutMetrics(const TOMLReader& toml, SkySettingsLayoutMetrics& metrics)
		{
           static constexpr std::array<IntMetricFieldEntry<SkySettingsLayoutMetrics>, 1> entries{{
				{ U"expandedHeight", &SkySettingsLayoutMetrics::expandedHeight },
			}};
			LoadMetricSection(toml, U"skySettings", metrics, entries);
		}

		void LoadCameraSettingsLayoutMetrics(const TOMLReader& toml, CameraSettingsLayoutMetrics& metrics)
		{
            static constexpr std::array<IntMetricFieldEntry<CameraSettingsLayoutMetrics>, 1> entries{{
				{ U"expandedHeight", &CameraSettingsLayoutMetrics::expandedHeight },
			}};
			LoadMetricSection(toml, U"cameraSettings", metrics, entries);
		}

		void LoadTerrainVisualSettingsLayoutMetrics(const TOMLReader& toml, TerrainVisualSettingsLayoutMetrics& metrics)
		{
         static constexpr std::array<IntMetricFieldEntry<TerrainVisualSettingsLayoutMetrics>, 2> entries{{
				{ U"panelWidth", &TerrainVisualSettingsLayoutMetrics::panelWidth },
				{ U"expandedHeight", &TerrainVisualSettingsLayoutMetrics::expandedHeight },
			}};
			LoadMetricSection(toml, U"terrainVisualSettings", metrics, entries);
		}

		void LoadFogSettingsLayoutMetrics(const TOMLReader& toml, FogSettingsLayoutMetrics& metrics)
		{
           static constexpr std::array<IntMetricFieldEntry<FogSettingsLayoutMetrics>, 2> entries{{
				{ U"panelWidth", &FogSettingsLayoutMetrics::panelWidth },
				{ U"panelHeight", &FogSettingsLayoutMetrics::panelHeight },
			}};
			LoadMetricSection(toml, U"fogSettings", metrics, entries);
		}

		void SanitizeSharedLayoutMetrics(SharedLayoutMetrics& metrics, const SharedLayoutMetrics& defaults)
		{
           static constexpr std::array<IntMetricClampEntry<SharedLayoutMetrics>, 5> entries{{
				{ &SharedLayoutMetrics::panelMargin, 0, 240 },
				{ &SharedLayoutMetrics::panelGap, 0, 240 },
				{ &SharedLayoutMetrics::uiEditGridCellSize, 1, 128 },
				{ &SharedLayoutMetrics::uiEditGridMajorLineSpan, 1, 64 },
				{ &SharedLayoutMetrics::accordionHeaderHeight, 24, 120 },
			}};
			ClampMetricValues(metrics, entries);

			if (metrics.uiEditGridMajorLineSpan <= 0)
			{
				metrics.uiEditGridMajorLineSpan = defaults.uiEditGridMajorLineSpan;
			}
		}

		void SanitizeRightColumnLayoutMetrics(RightColumnLayoutMetrics& metrics)
		{
         static constexpr std::array<IntMetricClampEntry<RightColumnLayoutMetrics>, 1> entries{{
				{ &RightColumnLayoutMetrics::width, 120, 720 },
			}};
			ClampMetricValues(metrics, entries);
		}

		void SanitizeResourcePanelLayoutMetrics(ResourcePanelLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
           static constexpr std::array<IntMetricClampEntry<ResourcePanelLayoutMetrics>, 4> entries{{
				{ &ResourcePanelLayoutMetrics::minWidth, 120, 960 },
				{ &ResourcePanelLayoutMetrics::resizeHandleSize, 8, 64 },
				{ &ResourcePanelLayoutMetrics::iconButtonSize, 16, 96 },
				{ &ResourcePanelLayoutMetrics::iconButtonGap, 0, 48 },
			}};
			ClampMetricValues(metrics, entries);
			metrics.collapsedHeight = Clamp(metrics.collapsedHeight, Max(shared.accordionHeaderHeight, 48), 720);
			metrics.expandedHeight = Clamp(metrics.expandedHeight, metrics.collapsedHeight, 1080);
		}

		void SanitizeModelHeightLayoutMetrics(ModelHeightLayoutMetrics& metrics)
		{
          static constexpr std::array<IntMetricClampEntry<ModelHeightLayoutMetrics>, 2> entries{{
				{ &ModelHeightLayoutMetrics::panelWidth, 240, 1280 },
				{ &ModelHeightLayoutMetrics::panelHeight, 160, 1080 },
			}};
			ClampMetricValues(metrics, entries);
		}

		void SanitizeBottomControlLayoutMetrics(BottomControlLayoutMetrics& metrics)
		{
          static constexpr std::array<IntMetricClampEntry<BottomControlLayoutMetrics>, 7> entries{{
				{ &BottomControlLayoutMetrics::yOffset, 40, 360 },
				{ &BottomControlLayoutMetrics::panelWidth, 240, 1600 },
				{ &BottomControlLayoutMetrics::panelHeight, 32, 160 },
				{ &BottomControlLayoutMetrics::checkBoxWidth, 48, 240 },
				{ &BottomControlLayoutMetrics::buttonGap, 0, 48 },
				{ &BottomControlLayoutMetrics::editorIconButtonSize, 12, 64 },
				{ &BottomControlLayoutMetrics::editorIconButtonGap, 0, 32 },
			}};
			ClampMetricValues(metrics, entries);
		}

     void SanitizeMiniMapLayoutMetrics(MiniMapLayoutMetrics& metrics, const SharedLayoutMetrics& shared, const MiniMapLayoutMetrics& defaults)
		{
          static constexpr std::array<IntMetricClampEntry<MiniMapLayoutMetrics>, 4> entries{{
				{ &MiniMapLayoutMetrics::minWidth, 120, 1280 },
				{ &MiniMapLayoutMetrics::minHeight, 120, 1280 },
				{ &MiniMapLayoutMetrics::resizeHandleSize, 8, 64 },
				{ &MiniMapLayoutMetrics::expandedHeight, 0, 1080 },
			}};

			if (metrics.minWidth <= 0)
			{
				metrics.minWidth = defaults.minWidth;
			}

			if (metrics.minHeight <= 0)
			{
				metrics.minHeight = defaults.minHeight;
			}

			if (metrics.resizeHandleSize <= 0)
			{
				metrics.resizeHandleSize = defaults.resizeHandleSize;
			}

			if (metrics.expandedHeight <= 0)
			{
				metrics.expandedHeight = defaults.expandedHeight;
			}

			ClampMetricValues(metrics, entries);
         metrics.minHeight = Max(metrics.minHeight, shared.accordionHeaderHeight);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void SanitizeSkySettingsLayoutMetrics(SkySettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
         static constexpr std::array<IntMetricClampEntry<SkySettingsLayoutMetrics>, 1> entries{{
				{ &SkySettingsLayoutMetrics::expandedHeight, 0, 1400 },
			}};
			ClampMetricValues(metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void SanitizeCameraSettingsLayoutMetrics(CameraSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
         static constexpr std::array<IntMetricClampEntry<CameraSettingsLayoutMetrics>, 1> entries{{
				{ &CameraSettingsLayoutMetrics::expandedHeight, 0, 1400 },
			}};
			ClampMetricValues(metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void SanitizeTerrainVisualSettingsLayoutMetrics(TerrainVisualSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
          static constexpr std::array<IntMetricClampEntry<TerrainVisualSettingsLayoutMetrics>, 2> entries{{
				{ &TerrainVisualSettingsLayoutMetrics::panelWidth, 180, 1280 },
				{ &TerrainVisualSettingsLayoutMetrics::expandedHeight, 0, 1400 },
			}};
			ClampMetricValues(metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void SanitizeFogSettingsLayoutMetrics(FogSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
          static constexpr std::array<IntMetricClampEntry<FogSettingsLayoutMetrics>, 2> entries{{
				{ &FogSettingsLayoutMetrics::panelWidth, 160, 1280 },
				{ &FogSettingsLayoutMetrics::panelHeight, 0, 1400 },
			}};
			ClampMetricValues(metrics, entries);
			metrics.panelHeight = Max(metrics.panelHeight, shared.accordionHeaderHeight);
		}

		void LoadUiLayoutProfileSections(const TOMLReader& toml, UiLayoutProfile& profile)
		{
			LoadSharedLayoutMetrics(toml, profile.shared);
			LoadRightColumnLayoutMetrics(toml, profile.rightColumn);
			LoadResourcePanelLayoutMetrics(toml, profile.resourcePanel);
			LoadModelHeightLayoutMetrics(toml, profile.modelHeight);
			LoadBottomControlLayoutMetrics(toml, profile.bottomControl);
			LoadMiniMapLayoutMetrics(toml, profile.miniMap);
			LoadSkySettingsLayoutMetrics(toml, profile.skySettings);
			LoadCameraSettingsLayoutMetrics(toml, profile.cameraSettings);
			LoadTerrainVisualSettingsLayoutMetrics(toml, profile.terrainVisualSettings);
			LoadFogSettingsLayoutMetrics(toml, profile.fogSettings);
		}

		void SanitizeUiLayoutProfile(UiLayoutProfile& profile)
		{
			const UiLayoutProfile defaults = GetDefaultUiLayoutProfile();
			SanitizeSharedLayoutMetrics(profile.shared, defaults.shared);
			SanitizeRightColumnLayoutMetrics(profile.rightColumn);
			SanitizeResourcePanelLayoutMetrics(profile.resourcePanel, profile.shared);
			SanitizeModelHeightLayoutMetrics(profile.modelHeight);
			SanitizeBottomControlLayoutMetrics(profile.bottomControl);
          SanitizeMiniMapLayoutMetrics(profile.miniMap, profile.shared, defaults.miniMap);
			SanitizeSkySettingsLayoutMetrics(profile.skySettings, profile.shared);
			SanitizeCameraSettingsLayoutMetrics(profile.cameraSettings, profile.shared);
			SanitizeTerrainVisualSettingsLayoutMetrics(profile.terrainVisualSettings, profile.shared);
			SanitizeFogSettingsLayoutMetrics(profile.fogSettings, profile.shared);
		}

		UiLayoutProfile LoadUiLayoutProfileFromDisk()
		{
			UiLayoutProfile profile = GetDefaultUiLayoutProfile();
			const TOMLReader toml{ UiLayoutProfilePath };

			if (not toml)
			{
				return profile;
			}

			try
			{
				if (toml[U"meta"][U"schemaVersion"].get<int32>() != UiLayoutProfileSchemaVersion)
				{
					return profile;
				}
			}
			catch (const std::exception&)
			{
				return profile;
			}

          LoadUiLayoutProfileSections(toml, profile);
			SanitizeUiLayoutProfile(profile);
			return profile;
		}
	}

	const UiLayoutProfile& GetDefaultUiLayoutProfile()
	{
		return DefaultUiLayoutProfile;
	}

	const UiLayoutProfile& GetUiLayoutProfile()
	{
		static const UiLayoutProfile profile = LoadUiLayoutProfileFromDisk();
		return profile;
	}
}
