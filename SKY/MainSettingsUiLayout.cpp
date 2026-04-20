# include "MainSettingsInternal.hpp"
# include "SettingsRegistry.hpp"
# include "SettingsSchemas.hpp"
# include "SkyAppUiLayout.hpp"

namespace MainSupport
{
	UiLayoutSettings LoadUiLayoutSettings(const int32 sceneWidth, const int32 sceneHeight)
	{
		UiLayoutSettings settings{
			.miniMapPosition = SkyAppUiLayout::DefaultMiniMapPosition(sceneWidth),
          .miniMapSize = SkyAppUiLayout::DefaultMiniMapSize(),
			.resourcePanelPosition = SkyAppUiLayout::DefaultResourcePanelPosition(sceneWidth),
         .resourcePanelSize = SkyAppUiLayout::DefaultResourcePanelSize(),
			.modelHeightPosition = SkyAppUiLayout::DefaultModelHeightPosition(sceneWidth, sceneHeight),
            .terrainVisualSettingsPosition = SkyAppUiLayout::DefaultTerrainVisualSettingsPosition(sceneWidth, sceneHeight),
            .fogSettingsPosition = SkyAppUiLayout::DefaultFogSettingsPosition(sceneWidth, sceneHeight),
			.unitEditorPosition = SkyAppUiLayout::DefaultUnitEditorPosition(sceneWidth),
			.unitEditorListPosition = SkyAppUiLayout::DefaultUnitEditorListPosition(),
          .battleCommandIconSize = SkyAppUiLayout::ClampBattleCommandIconSize(128),
		};

		const TOMLReader toml{ UiLayoutSettingsPath };

		if (not toml)
		{
			return settings;
		}

		SettingsSchemas::VisitUiLayoutPositions(TomlSchema::LoadVisitor{ toml, U"" }, settings);
		settings.battleCommandIconSize = SkyAppUiLayout::ClampBattleCommandIconSize(toml[U"battleCommandIconSize"].getOr<int32>(settings.battleCommandIconSize));

      settings.miniMapSize = SkyAppUiLayout::ClampMiniMapSize(settings.miniMapSize, settings.miniMapPosition, sceneWidth, sceneHeight, true);
		const Rect miniMapRect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, settings.miniMapPosition, settings.miniMapSize, true);
         settings.resourcePanelSize = SkyAppUiLayout::ClampResourcePanelSize(settings.resourcePanelSize, settings.resourcePanelPosition, sceneWidth, sceneHeight, false);
        const Rect resourcePanelRect = SkyAppUiLayout::ResourcePanel(sceneWidth, sceneHeight, settings.resourcePanelPosition, settings.resourcePanelSize, false, true);
		const Rect modelHeightRect = SkyAppUiLayout::ModelHeight(sceneWidth, sceneHeight, settings.modelHeightPosition);
       const Rect terrainVisualSettingsRect = SkyAppUiLayout::TerrainVisualSettings(sceneWidth, sceneHeight, settings.terrainVisualSettingsPosition);
       const Rect fogSettingsRect = SkyAppUiLayout::FogSettings(sceneWidth, sceneHeight, settings.fogSettingsPosition);
		const Rect unitEditorRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, settings.unitEditorPosition);
		const Rect unitEditorListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, settings.unitEditorListPosition);
		settings.miniMapPosition = Point{ miniMapRect.x, miniMapRect.y };
		settings.resourcePanelPosition = Point{ resourcePanelRect.x, resourcePanelRect.y };
       settings.resourcePanelSize = Point{ resourcePanelRect.w, resourcePanelRect.h };
		settings.modelHeightPosition = Point{ modelHeightRect.x, modelHeightRect.y };
      settings.terrainVisualSettingsPosition = Point{ terrainVisualSettingsRect.x, terrainVisualSettingsRect.y };
      settings.fogSettingsPosition = Point{ fogSettingsRect.x, fogSettingsRect.y };
		settings.unitEditorPosition = Point{ unitEditorRect.x, unitEditorRect.y };
		settings.unitEditorListPosition = Point{ unitEditorListRect.x, unitEditorListRect.y };
        settings.miniMapSize = Point{ miniMapRect.w, miniMapRect.h };
		return settings;
	}

	bool SaveUiLayoutSettings(const UiLayoutSettings& settings)
	{
		return SaveSettingsFile(UiLayoutSettingsPath, [&](TextWriter& writer)
		{
			SettingsSchemas::VisitUiLayoutPositions(TomlSchema::SaveVisitor{ writer, U"" }, settings);
			writer.writeln(U"battleCommandIconSize = {}"_fmt(SkyAppUiLayout::ClampBattleCommandIconSize(settings.battleCommandIconSize)));
		});
	}
}
