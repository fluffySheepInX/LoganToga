#pragma once

#include "Remake2Common.h"

struct MenuButtonStyle
{
	double cornerRadius = 10.0;
	double hoverExpand = 2.0;
	double pressOffsetX = 1.0;
	double pressOffsetY = 2.0;
	double pressInsetX = 2.0;
	double pressInsetY = 3.0;
	double baseBorderThickness = 2.0;
	double hoverBorderThickness = 3.0;
	double selectedBorderThickness = 4.0;
	ColorF fillColor{ 0.18, 0.22, 0.29, 0.96 };
	ColorF hoverFillColor{ 0.24, 0.29, 0.38, 0.98 };
	ColorF pressedFillColor{ 0.16, 0.20, 0.27, 0.98 };
	ColorF selectedFillColor{ 0.26, 0.38, 0.64, 0.98 };
	ColorF selectedHoverFillColor{ 0.34, 0.48, 0.76, 0.98 };
	ColorF frameColor{ 0.42, 0.56, 0.78, 0.96 };
	ColorF hoverFrameColor{ 0.78, 0.88, 1.0, 0.98 };
	ColorF selectedFrameColor{ 0.90, 0.96, 1.0, 0.98 };
	ColorF textColor{ 1.0, 1.0, 1.0, 1.0 };
	bool drawAccent = true;
	double accentHeight = 4.0;
	double accentMargin = 12.0;
	ColorF accentColor{ 0.58, 0.74, 1.0, 0.95 };
	ColorF selectedAccentColor{ 0.90, 0.96, 1.0, 0.98 };
};

struct MenuButtonVisualState
{
	RectF drawRect;
	ColorF fillColor;
	ColorF frameColor;
	ColorF accentColor;
	double frameThickness = 2.0;
	bool hovered = false;
	bool pressed = false;
};

[[nodiscard]] inline bool IsMenuButtonClicked(const RectF& rect)
{
	return rect.mouseOver() && MouseL.down();
}

[[nodiscard]] inline MenuButtonVisualState GetMenuButtonVisualState(const RectF& rect, const bool selected = false, const MenuButtonStyle& style = {})
{
	const bool hovered = rect.mouseOver();
	const bool pressed = hovered && MouseL.pressed();

	RectF drawRect = rect;
	if (pressed)
	{
		drawRect = RectF{
			rect.x + style.pressOffsetX,
			rect.y + style.pressOffsetY,
			Max(8.0, rect.w - (style.pressOffsetX + style.pressInsetX)),
			Max(8.0, rect.h - (style.pressOffsetY + style.pressInsetY))
		};
	}
	else if (hovered)
	{
		drawRect = RectF{
			rect.x - style.hoverExpand,
			rect.y - style.hoverExpand,
			rect.w + (style.hoverExpand * 2.0),
			rect.h + (style.hoverExpand * 2.0)
		};
	}

	const ColorF fillColor = pressed
		? (selected ? style.selectedFillColor : style.pressedFillColor)
		: (selected
			? (hovered ? style.selectedHoverFillColor : style.selectedFillColor)
			: (hovered ? style.hoverFillColor : style.fillColor));
	const ColorF frameColor = selected
		? style.selectedFrameColor
		: (hovered ? style.hoverFrameColor : style.frameColor);
	const double frameThickness = selected
		? style.selectedBorderThickness
		: (hovered ? style.hoverBorderThickness : style.baseBorderThickness);

	return MenuButtonVisualState{
		.drawRect = drawRect,
		.fillColor = fillColor,
		.frameColor = frameColor,
		.accentColor = selected ? style.selectedAccentColor : style.accentColor,
		.frameThickness = frameThickness,
		.hovered = hovered,
		.pressed = pressed,
	};
}

inline void DrawMenuButton(const RectF& rect, const String& label, const Font& font, const bool selected = false, const MenuButtonStyle& style = {})
{
	const auto visual = GetMenuButtonVisualState(rect, selected, style);
	RoundRect{ visual.drawRect, style.cornerRadius }.draw(visual.fillColor);
	RoundRect{ visual.drawRect, style.cornerRadius }.drawFrame(visual.frameThickness, 0, visual.frameColor);
	font(label).drawAt(visual.drawRect.center().movedBy(0, -1), style.textColor);
}
