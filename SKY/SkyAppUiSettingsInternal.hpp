# pragma once
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"

namespace SkyAppSupport::UiSettingsDetail
{
	inline constexpr double EditorSliderRowHeight = 32.0;
	inline constexpr double EditorSliderTrackHeight = 8.0;
	inline constexpr double EditorSliderKnobWidth = 14.0;
	inline constexpr double EditorSliderKnobHeight = 22.0;
	inline constexpr double EditorCheckBoxSize = 22.0;
	inline constexpr double EditorCheckBoxRowHeight = 28.0;

	[[nodiscard]] Array<String> WrapTooltipText(StringView text, double maxWidth);
	[[nodiscard]] double GetSliderMin(double value, double defaultMin);
	[[nodiscard]] double GetSliderMax(double value, double defaultMax);
	void DrawSettingsPanelFrame(const Rect& panelRect, StringView title);
	void DrawCameraSettingsPanelFrame(const Rect& panelRect);
	void DrawSkySettingsPanelFrame(const Rect& panelRect, StringView title);
	void DrawTerrainVisualSettingsPanelFrame(const Rect& panelRect);
	void DrawHoverTooltip(const RectF& anchorRect, StringView title, StringView description);
	[[nodiscard]] bool DrawTerrainPageButton(const Rect& rect, StringView label, bool selected);
	[[nodiscard]] bool DrawEditorSlider(int32 sliderId,
		StringView label,
		double& value,
		double minValue,
		double maxValue,
		const Vec2& pos,
		double labelWidth,
		double trackWidth);
	void DrawEditorCheckBox(bool& checked,
		StringView label,
		const Vec2& pos,
		double width,
		bool enabled = true);
}
