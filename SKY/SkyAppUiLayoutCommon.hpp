# pragma once
# include "SkyAppUiLayoutProfile.hpp"

namespace SkyAppUiLayout
{
	namespace Detail
	{
		[[nodiscard]] inline const UiLayoutProfile& LayoutProfile()
		{
			return GetUiLayoutProfile();
		}

		[[nodiscard]] inline int32 RightColumnX(const int32 sceneWidth)
		{
			const auto& layout = LayoutProfile();
			return Max(layout.shared.panelMargin, (sceneWidth - layout.rightColumn.width - layout.shared.panelMargin));
		}

		[[nodiscard]] inline Point ClampPanelPosition(const Point& position, const int32 panelWidth, const int32 panelHeight, const int32 sceneWidth, const int32 sceneHeight)
		{
			return Point{
				Clamp(position.x, 0, Max(0, (sceneWidth - panelWidth))),
				Clamp(position.y, 0, Max(0, (sceneHeight - panelHeight)))
			};
		}

		// Generic fixed-size draggable panel: clamps position to scene
		// bounds and returns Rect with the requested width/height.
		// Used by ModelHeight / TerrainVisualSettings / FogSettings /
		// UnitEditorList etc.
		[[nodiscard]] inline Rect MakePanelRect(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const int32 panelWidth, const int32 panelHeight)
		{
			const Point clamped = ClampPanelPosition(position, panelWidth, panelHeight, sceneWidth, sceneHeight);
			return Rect{ clamped.x, clamped.y, panelWidth, panelHeight };
		}

		// Generic helpers for "position relative to a panel's top-left".
		[[nodiscard]] inline Vec2 PanelOffsetVec(const Rect& panelRect, const int32 xOffset, const int32 yOffset)
		{
			return Vec2{ static_cast<double>(panelRect.x + xOffset), static_cast<double>(panelRect.y + yOffset) };
		}

		[[nodiscard]] inline Rect PanelOffsetRect(const Rect& panelRect, const int32 xOffset, const int32 yOffset, const int32 width, const int32 height)
		{
			return Rect{ (panelRect.x + xOffset), (panelRect.y + yOffset), width, height };
		}

		// Standard top-right drag-handle area (shared by terrain visual /
		// fog settings panels).
		[[nodiscard]] inline Rect PanelTopRightDragHandle(const Rect& panelRect, const int32 rightInset = 88, const int32 topInset = 8, const int32 width = 72, const int32 height = 24)
		{
			return Rect{ (panelRect.rightX() - rightInset), (panelRect.y + topInset), width, height };
		}
	}

	inline const int32 UiEditGridCellSize = GetDefaultUiLayoutProfile().shared.uiEditGridCellSize;
	inline const int32 UiEditGridMajorLineSpan = GetDefaultUiLayoutProfile().shared.uiEditGridMajorLineSpan;
	inline const int32 AccordionHeaderHeight = GetDefaultUiLayoutProfile().shared.accordionHeaderHeight;

	[[nodiscard]] inline int32 SnapToUiEditGrid(const int32 value)
	{
		return static_cast<int32>(Math::Round(static_cast<double>(value) / UiEditGridCellSize) * UiEditGridCellSize);
	}

	[[nodiscard]] inline int32 AccordionPanelHeight(const bool expanded, const int32 expandedHeight)
	{
		return expanded ? expandedHeight : Detail::LayoutProfile().shared.accordionHeaderHeight;
	}

	[[nodiscard]] inline Point DefaultMiniMapPosition(const int32 sceneWidth)
	{
		const auto& layout = Detail::LayoutProfile();
		return Point{ Detail::RightColumnX(sceneWidth), (layout.shared.panelMargin + layout.resourcePanel.expandedHeight + layout.shared.panelGap) };
	}

	[[nodiscard]] inline Point DefaultMiniMapSize()
	{
		const auto& layout = Detail::LayoutProfile();
		return Point{ layout.rightColumn.width, layout.miniMap.expandedHeight };
	}

	[[nodiscard]] inline Point DefaultResourcePanelPosition(const int32 sceneWidth)
	{
		const auto& layout = Detail::LayoutProfile();
		return Point{ Detail::RightColumnX(sceneWidth), layout.shared.panelMargin };
	}

	[[nodiscard]] inline Point DefaultResourcePanelSize()
	{
		const auto& layout = Detail::LayoutProfile();
		return Point{ layout.resourcePanel.minWidth, layout.resourcePanel.expandedHeight };
	}

	[[nodiscard]] inline Point DefaultUnitEditorPosition(const int32 sceneWidth)
	{
		return Point{ (sceneWidth - 340), 124 };
	}

	[[nodiscard]] inline Point DefaultUnitEditorListPosition()
	{
		return Point{ 20, 124 };
	}

	[[nodiscard]] inline Point DefaultModelHeightPosition(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		const auto& layout = Detail::LayoutProfile();
		return Point{ (20 + 480 + layout.shared.panelGap), (20 + layout.cameraSettings.expandedHeight + layout.shared.panelGap) };
	}

	[[nodiscard]] inline Point DefaultTerrainVisualSettingsPosition(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		const auto& layout = Detail::LayoutProfile();
		return Point{ (20 + 480 + layout.shared.panelGap), (20 + layout.cameraSettings.expandedHeight + 16) };
	}

	[[nodiscard]] inline Point DefaultFogSettingsPosition(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		const auto& layout = Detail::LayoutProfile();
		return Point{ (20 + 480 + layout.shared.panelGap + layout.terrainVisualSettings.panelWidth + layout.shared.panelGap), (20 + layout.cameraSettings.expandedHeight + 16) };
	}

	[[nodiscard]] inline Point SnapToUiEditGrid(const Point& position)
	{
		return Point{ SnapToUiEditGrid(position.x), SnapToUiEditGrid(position.y) };
	}
}
