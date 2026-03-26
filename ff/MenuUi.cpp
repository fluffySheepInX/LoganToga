# include "MenuUi.h"

void DrawMenuButton(const RectF& rect, const Font& font, const String& label)
{
	const bool hovered = rect.mouseOver();
	const ColorF fillColor = hovered ? ColorF{ 0.34, 0.49, 0.80, 0.92 } : ColorF{ 0.20, 0.29, 0.54, 0.88 };
	const ColorF frameColor = hovered ? ColorF{ 0.95, 0.98, 1.0, 0.95 } : ColorF{ 0.80, 0.88, 1.0, 0.72 };

	rect.rounded(12).draw(fillColor);
	rect.rounded(12).drawFrame(2, frameColor);
	font(label).drawAt(24, rect.center(), Palette::White);
}
