# include "SkyAppUiLayoutProfile.hpp"

namespace SkyAppUiLayout
{
	namespace
	{
		inline constexpr StringView UiLayoutProfilePath = U"settings/ui_layout_profile.toml";
		inline constexpr int32 UiLayoutProfileSchemaVersion = 1;

		const UiLayoutProfile DefaultUiLayoutProfile{};

		// Single entry combines TOML key, member pointer and clamp range.
		// Loading + clamping are fused in LoadAndClampSection so each metric
		// is described in exactly one place.
		template <class Metrics>
		struct MetricEntry
		{
			StringView key;
			int32 Metrics::* member = nullptr;
			int32 minValue = std::numeric_limits<int32>::min();
			int32 maxValue = std::numeric_limits<int32>::max();
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
		void LoadAndClampSection(const TOMLReader& toml,
			StringView section,
			Metrics& metrics,
			const std::array<MetricEntry<Metrics>, N>& entries)
		{
			for (const auto& entry : entries)
			{
				LoadTomlInt(toml, section, entry.key, metrics.*(entry.member));
				metrics.*(entry.member) = Clamp(metrics.*(entry.member), entry.minValue, entry.maxValue);
			}
		}

		void ProcessSharedLayoutMetrics(const TOMLReader& toml, SharedLayoutMetrics& metrics, const SharedLayoutMetrics& defaults)
		{
			static constexpr std::array<MetricEntry<SharedLayoutMetrics>, 5> entries{ {
				{ U"panelMargin",             &SharedLayoutMetrics::panelMargin,             0,   240 },
				{ U"panelGap",                &SharedLayoutMetrics::panelGap,                0,   240 },
				{ U"uiEditGridCellSize",      &SharedLayoutMetrics::uiEditGridCellSize,      1,   128 },
				{ U"uiEditGridMajorLineSpan", &SharedLayoutMetrics::uiEditGridMajorLineSpan, 1,    64 },
				{ U"accordionHeaderHeight",   &SharedLayoutMetrics::accordionHeaderHeight,  24,   120 },
			} };
			LoadAndClampSection(toml, U"shared", metrics, entries);

			if (metrics.uiEditGridMajorLineSpan <= 0)
			{
				metrics.uiEditGridMajorLineSpan = defaults.uiEditGridMajorLineSpan;
			}
		}

		void ProcessRightColumnLayoutMetrics(const TOMLReader& toml, RightColumnLayoutMetrics& metrics)
		{
			static constexpr std::array<MetricEntry<RightColumnLayoutMetrics>, 1> entries{ {
				{ U"width", &RightColumnLayoutMetrics::width, 120, 720 },
			} };
			LoadAndClampSection(toml, U"rightColumn", metrics, entries);
		}

		void ProcessResourcePanelLayoutMetrics(const TOMLReader& toml, ResourcePanelLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
			static constexpr std::array<MetricEntry<ResourcePanelLayoutMetrics>, 6> entries{ {
				{ U"minWidth",         &ResourcePanelLayoutMetrics::minWidth,         120, 960 },
				{ U"resizeHandleSize", &ResourcePanelLayoutMetrics::resizeHandleSize,   8,  64 },
				{ U"iconButtonSize",   &ResourcePanelLayoutMetrics::iconButtonSize,    16,  96 },
				{ U"iconButtonGap",    &ResourcePanelLayoutMetrics::iconButtonGap,      0,  48 },
				// collapsedHeight / expandedHeight have inter-dependent clamps applied below.
				{ U"collapsedHeight",  &ResourcePanelLayoutMetrics::collapsedHeight },
				{ U"expandedHeight",   &ResourcePanelLayoutMetrics::expandedHeight  },
			} };
			LoadAndClampSection(toml, U"resourcePanel", metrics, entries);
			metrics.collapsedHeight = Clamp(metrics.collapsedHeight, Max(shared.accordionHeaderHeight, 48), 720);
			metrics.expandedHeight = Clamp(metrics.expandedHeight, metrics.collapsedHeight, 1080);
		}

		void ProcessModelHeightLayoutMetrics(const TOMLReader& toml, ModelHeightLayoutMetrics& metrics)
		{
			static constexpr std::array<MetricEntry<ModelHeightLayoutMetrics>, 2> entries{ {
				{ U"panelWidth",  &ModelHeightLayoutMetrics::panelWidth,  240, 1280 },
				{ U"panelHeight", &ModelHeightLayoutMetrics::panelHeight, 160, 1080 },
			} };
			LoadAndClampSection(toml, U"modelHeight", metrics, entries);
		}

		void ProcessBottomControlLayoutMetrics(const TOMLReader& toml, BottomControlLayoutMetrics& metrics)
		{
			static constexpr std::array<MetricEntry<BottomControlLayoutMetrics>, 7> entries{ {
				{ U"yOffset",              &BottomControlLayoutMetrics::yOffset,               40, 360 },
				{ U"panelWidth",           &BottomControlLayoutMetrics::panelWidth,           240, 1600 },
				{ U"panelHeight",          &BottomControlLayoutMetrics::panelHeight,           32, 160 },
				{ U"checkBoxWidth",        &BottomControlLayoutMetrics::checkBoxWidth,         48, 240 },
				{ U"buttonGap",            &BottomControlLayoutMetrics::buttonGap,              0,  48 },
				{ U"editorIconButtonSize", &BottomControlLayoutMetrics::editorIconButtonSize,  12,  64 },
				{ U"editorIconButtonGap",  &BottomControlLayoutMetrics::editorIconButtonGap,    0,  32 },
			} };
			LoadAndClampSection(toml, U"bottomControl", metrics, entries);
		}

		void ProcessMiniMapLayoutMetrics(const TOMLReader& toml, MiniMapLayoutMetrics& metrics, const SharedLayoutMetrics& shared, const MiniMapLayoutMetrics& defaults)
		{
			// Load first; values <= 0 fall back to defaults; final clamp uses shared header height as floor.
			static constexpr std::array<MetricEntry<MiniMapLayoutMetrics>, 4> loadOnly{ {
				{ U"minWidth",         &MiniMapLayoutMetrics::minWidth         },
				{ U"minHeight",        &MiniMapLayoutMetrics::minHeight        },
				{ U"resizeHandleSize", &MiniMapLayoutMetrics::resizeHandleSize },
				{ U"expandedHeight",   &MiniMapLayoutMetrics::expandedHeight   },
			} };
			for (const auto& entry : loadOnly)
			{
				LoadTomlInt(toml, U"miniMap", entry.key, metrics.*(entry.member));
			}

			if (metrics.minWidth         <= 0) { metrics.minWidth         = defaults.minWidth;         }
			if (metrics.minHeight        <= 0) { metrics.minHeight        = defaults.minHeight;        }
			if (metrics.resizeHandleSize <= 0) { metrics.resizeHandleSize = defaults.resizeHandleSize; }
			if (metrics.expandedHeight   <= 0) { metrics.expandedHeight   = defaults.expandedHeight;   }

			metrics.minWidth         = Clamp(metrics.minWidth,         120, 1280);
			metrics.minHeight        = Clamp(metrics.minHeight,        120, 1280);
			metrics.resizeHandleSize = Clamp(metrics.resizeHandleSize,   8,   64);
			metrics.expandedHeight   = Clamp(metrics.expandedHeight,     0, 1080);
			metrics.minHeight        = Max(metrics.minHeight,       shared.accordionHeaderHeight);
			metrics.expandedHeight   = Max(metrics.expandedHeight,  shared.accordionHeaderHeight);
		}

		void ProcessSkySettingsLayoutMetrics(const TOMLReader& toml, SkySettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
			static constexpr std::array<MetricEntry<SkySettingsLayoutMetrics>, 1> entries{ {
				{ U"expandedHeight", &SkySettingsLayoutMetrics::expandedHeight, 0, 1400 },
			} };
			LoadAndClampSection(toml, U"skySettings", metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void ProcessCameraSettingsLayoutMetrics(const TOMLReader& toml, CameraSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
			static constexpr std::array<MetricEntry<CameraSettingsLayoutMetrics>, 1> entries{ {
				{ U"expandedHeight", &CameraSettingsLayoutMetrics::expandedHeight, 0, 1400 },
			} };
			LoadAndClampSection(toml, U"cameraSettings", metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void ProcessTerrainVisualSettingsLayoutMetrics(const TOMLReader& toml, TerrainVisualSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
			static constexpr std::array<MetricEntry<TerrainVisualSettingsLayoutMetrics>, 2> entries{ {
				{ U"panelWidth",     &TerrainVisualSettingsLayoutMetrics::panelWidth,     180, 1280 },
				{ U"expandedHeight", &TerrainVisualSettingsLayoutMetrics::expandedHeight,   0, 1400 },
			} };
			LoadAndClampSection(toml, U"terrainVisualSettings", metrics, entries);
			metrics.expandedHeight = Max(metrics.expandedHeight, shared.accordionHeaderHeight);
		}

		void ProcessFogSettingsLayoutMetrics(const TOMLReader& toml, FogSettingsLayoutMetrics& metrics, const SharedLayoutMetrics& shared)
		{
			static constexpr std::array<MetricEntry<FogSettingsLayoutMetrics>, 2> entries{ {
				{ U"panelWidth",  &FogSettingsLayoutMetrics::panelWidth,  160, 1280 },
				{ U"panelHeight", &FogSettingsLayoutMetrics::panelHeight,   0, 1400 },
			} };
			LoadAndClampSection(toml, U"fogSettings", metrics, entries);
			metrics.panelHeight = Max(metrics.panelHeight, shared.accordionHeaderHeight);
		}

		void LoadAndSanitizeUiLayoutProfile(const TOMLReader& toml, UiLayoutProfile& profile)
		{
			const UiLayoutProfile defaults = GetDefaultUiLayoutProfile();
			ProcessSharedLayoutMetrics(toml, profile.shared, defaults.shared);
			ProcessRightColumnLayoutMetrics(toml, profile.rightColumn);
			ProcessResourcePanelLayoutMetrics(toml, profile.resourcePanel, profile.shared);
			ProcessModelHeightLayoutMetrics(toml, profile.modelHeight);
			ProcessBottomControlLayoutMetrics(toml, profile.bottomControl);
			ProcessMiniMapLayoutMetrics(toml, profile.miniMap, profile.shared, defaults.miniMap);
			ProcessSkySettingsLayoutMetrics(toml, profile.skySettings, profile.shared);
			ProcessCameraSettingsLayoutMetrics(toml, profile.cameraSettings, profile.shared);
			ProcessTerrainVisualSettingsLayoutMetrics(toml, profile.terrainVisualSettings, profile.shared);
			ProcessFogSettingsLayoutMetrics(toml, profile.fogSettings, profile.shared);
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

			LoadAndSanitizeUiLayoutProfile(toml, profile);
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
