# pragma once
# include "MainContext.hpp"

namespace SkyAppUiLayout
{
	struct SharedLayoutMetrics
	{
		int32 panelMargin = 20;
		int32 panelGap = 20;
		int32 uiEditGridCellSize = 8;
		int32 uiEditGridMajorLineSpan = 4;
		int32 accordionHeaderHeight = 36;
	};

	struct RightColumnLayoutMetrics
	{
		int32 width = 220;
	};

	struct ResourcePanelLayoutMetrics
	{
		int32 minWidth = 220;
		int32 resizeHandleSize = 18;
		int32 iconButtonSize = 36;
		int32 iconButtonGap = 8;
		int32 collapsedHeight = 96;
		int32 expandedHeight = 248;
	};

	struct ModelHeightLayoutMetrics
	{
		int32 panelWidth = 460;
		int32 panelHeight = 420;
	};

	struct BottomControlLayoutMetrics
	{
		int32 yOffset = 100;
		int32 panelWidth = 804;
		int32 panelHeight = 52;
		int32 checkBoxWidth = 96;
		int32 buttonGap = 8;
		int32 editorIconButtonSize = 24;
		int32 editorIconButtonGap = 8;
	};

	struct MiniMapLayoutMetrics
	{
     int32 minWidth = 180;
		int32 minHeight = 180;
		int32 resizeHandleSize = 18;
		int32 expandedHeight = 220;
	};

	struct SkySettingsLayoutMetrics
	{
		int32 expandedHeight = 466;
	};

	struct CameraSettingsLayoutMetrics
	{
		int32 expandedHeight = 416;
	};

	struct TerrainVisualSettingsLayoutMetrics
	{
		int32 panelWidth = 420;
		int32 expandedHeight = 560;
	};

	struct FogSettingsLayoutMetrics
	{
		int32 panelWidth = 340;
		int32 panelHeight = 382;
	};

	struct UiLayoutProfile
	{
		SharedLayoutMetrics shared{};
		RightColumnLayoutMetrics rightColumn{};
		ResourcePanelLayoutMetrics resourcePanel{};
		ModelHeightLayoutMetrics modelHeight{};
		BottomControlLayoutMetrics bottomControl{};
		MiniMapLayoutMetrics miniMap{};
		SkySettingsLayoutMetrics skySettings{};
		CameraSettingsLayoutMetrics cameraSettings{};
		TerrainVisualSettingsLayoutMetrics terrainVisualSettings{};
		FogSettingsLayoutMetrics fogSettings{};
	};

	[[nodiscard]] const UiLayoutProfile& GetDefaultUiLayoutProfile();
	[[nodiscard]] const UiLayoutProfile& GetUiLayoutProfile();
}
