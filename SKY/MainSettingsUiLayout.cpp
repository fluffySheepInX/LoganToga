# include "MainSettingsInternal.hpp"
# include "SkyAppUiLayout.hpp"

namespace MainSupport
{
	UiLayoutSettings LoadUiLayoutSettings(const int32 sceneWidth, const int32 sceneHeight)
	{
		UiLayoutSettings settings{
			.miniMapPosition = SkyAppUiLayout::DefaultMiniMapPosition(sceneWidth),
			.resourcePanelPosition = SkyAppUiLayout::DefaultResourcePanelPosition(sceneWidth),
         .resourcePanelSize = SkyAppUiLayout::DefaultResourcePanelSize(),
			.modelHeightPosition = SkyAppUiLayout::DefaultModelHeightPosition(sceneWidth, sceneHeight),
            .terrainVisualSettingsPosition = SkyAppUiLayout::DefaultTerrainVisualSettingsPosition(sceneWidth, sceneHeight),
			.unitEditorPosition = SkyAppUiLayout::DefaultUnitEditorPosition(sceneWidth),
			.unitEditorListPosition = SkyAppUiLayout::DefaultUnitEditorListPosition(),
		};

		const TOMLReader toml{ UiLayoutSettingsPath };

		if (not toml)
		{
			return settings;
		}

		settings.miniMapPosition = SettingsDetail::ReadTomlPoint(toml, U"miniMap", settings.miniMapPosition);
		settings.resourcePanelPosition = SettingsDetail::ReadTomlPoint(toml, U"resourcePanel", settings.resourcePanelPosition);
       settings.resourcePanelSize = SettingsDetail::ReadTomlPoint(toml, U"resourcePanelSize", settings.resourcePanelSize);
		settings.modelHeightPosition = SettingsDetail::ReadTomlPoint(toml, U"modelHeight", settings.modelHeightPosition);
      settings.terrainVisualSettingsPosition = SettingsDetail::ReadTomlPoint(toml, U"terrainVisualSettings", settings.terrainVisualSettingsPosition);
		settings.unitEditorPosition = SettingsDetail::ReadTomlPoint(toml, U"unitEditor", settings.unitEditorPosition);
		settings.unitEditorListPosition = SettingsDetail::ReadTomlPoint(toml, U"unitEditorList", settings.unitEditorListPosition);

		const Rect miniMapRect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, settings.miniMapPosition, true);
         settings.resourcePanelSize = SkyAppUiLayout::ClampResourcePanelSize(settings.resourcePanelSize, settings.resourcePanelPosition, sceneWidth, sceneHeight, false);
        const Rect resourcePanelRect = SkyAppUiLayout::ResourcePanel(sceneWidth, sceneHeight, settings.resourcePanelPosition, settings.resourcePanelSize, false, true);
		const Rect modelHeightRect = SkyAppUiLayout::ModelHeight(sceneWidth, sceneHeight, settings.modelHeightPosition);
       const Rect terrainVisualSettingsRect = SkyAppUiLayout::TerrainVisualSettings(sceneWidth, sceneHeight, settings.terrainVisualSettingsPosition);
		const Rect unitEditorRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, settings.unitEditorPosition);
		const Rect unitEditorListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, settings.unitEditorListPosition);
		settings.miniMapPosition = Point{ miniMapRect.x, miniMapRect.y };
		settings.resourcePanelPosition = Point{ resourcePanelRect.x, resourcePanelRect.y };
       settings.resourcePanelSize = Point{ resourcePanelRect.w, resourcePanelRect.h };
		settings.modelHeightPosition = Point{ modelHeightRect.x, modelHeightRect.y };
      settings.terrainVisualSettingsPosition = Point{ terrainVisualSettingsRect.x, terrainVisualSettingsRect.y };
		settings.unitEditorPosition = Point{ unitEditorRect.x, unitEditorRect.y };
		settings.unitEditorListPosition = Point{ unitEditorListRect.x, unitEditorListRect.y };
		return settings;
	}

	bool SaveUiLayoutSettings(const UiLayoutSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ UiLayoutSettingsPath };

		if (not writer)
		{
			return false;
		}

		writer.writeln(U"miniMap = [{}, {}]"_fmt(settings.miniMapPosition.x, settings.miniMapPosition.y));
		writer.writeln(U"resourcePanel = [{}, {}]"_fmt(settings.resourcePanelPosition.x, settings.resourcePanelPosition.y));
      writer.writeln(U"resourcePanelSize = [{}, {}]"_fmt(settings.resourcePanelSize.x, settings.resourcePanelSize.y));
		writer.writeln(U"modelHeight = [{}, {}]"_fmt(settings.modelHeightPosition.x, settings.modelHeightPosition.y));
     writer.writeln(U"terrainVisualSettings = [{}, {}]"_fmt(settings.terrainVisualSettingsPosition.x, settings.terrainVisualSettingsPosition.y));
		writer.writeln(U"unitEditor = [{}, {}]"_fmt(settings.unitEditorPosition.x, settings.unitEditorPosition.y));
		writer.writeln(U"unitEditorList = [{}, {}]"_fmt(settings.unitEditorListPosition.x, settings.unitEditorListPosition.y));
		return true;
	}
}
