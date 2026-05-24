#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	struct RectButtonStyle
	{
		ColorF normalBack{ 0.08, 0.09, 0.11, 0.92 };
		ColorF activeBack{ 0.12, 0.22, 0.18, 0.96 };
		ColorF normalFrame{ 1, 1, 1, 0.16 };
		ColorF hoverFrame{ 1.0, 0.84, 0.0 };
		ColorF normalText{ Palette::Lightgray };
		ColorF activeText{ Palette::White };
		double frameThickness = 2.0;
		int32 fontSize = 14;
	};

	struct RectNumberStepperRects
	{
		RectF minus;
		RectF value;
		RectF plus;
		RectF step;
	};

	struct RectNumberStepperStyle
	{
		RectButtonStyle buttonStyle{ .fontSize = 10 };
		ColorF valueBack{ 0.03, 0.035, 0.05, 0.92 };
		ColorF valueFrame{ 1, 1, 1, 0.18 };
		ColorF valueHoverFrame{ 1.0, 0.84, 0.0 };
		ColorF valueActiveFrame{ 0.25, 0.85, 1.0 };
		ColorF valueText{ Palette::Gold };
		ColorF disabledText{ 0.52, 0.56, 0.60 };
		int32 valueFontSize = 11;
	};

	inline void DrawRectButton(const RectF& rect, StringView text, bool active, const Font& uiFont, const RectButtonStyle& style = RectButtonStyle{})
	{
		const ColorF backColor = active ? style.activeBack : style.normalBack;
		const ColorF frameColor = rect.mouseOver() ? style.hoverFrame : style.normalFrame;
		const ColorF textColor = active ? style.activeText : style.normalText;
		rect.draw(backColor).drawFrame(style.frameThickness, frameColor);
		uiFont(text).drawAt(style.fontSize, rect.center(), textColor);
	}

	inline void DrawRectNumberStepper(const RectNumberStepperRects& rects, StringView valueText, StringView stepText, bool editingValue, bool activeStepMenu, bool enabled, const Font& uiFont, const RectNumberStepperStyle& style = RectNumberStepperStyle{})
	{
		RectButtonStyle buttonStyle = style.buttonStyle;
		if (!enabled)
		{
			buttonStyle.normalBack = ColorF{ 0.05, 0.06, 0.08, 0.70 };
			buttonStyle.activeBack = ColorF{ 0.05, 0.06, 0.08, 0.70 };
			buttonStyle.normalFrame = ColorF{ 1, 1, 1, 0.08 };
			buttonStyle.hoverFrame = ColorF{ 1, 1, 1, 0.08 };
			buttonStyle.normalText = style.disabledText;
			buttonStyle.activeText = style.disabledText;
		}

		DrawRectButton(rects.minus, U"-", false, uiFont, buttonStyle);
		DrawRectButton(rects.plus, U"+", false, uiFont, buttonStyle);
		DrawRectButton(rects.step, stepText, activeStepMenu, uiFont, buttonStyle);

		const ColorF frameColor = editingValue ? style.valueActiveFrame : (rects.value.mouseOver() ? style.valueHoverFrame : style.valueFrame);
		rects.value.draw(style.valueBack).drawFrame(editingValue ? 2.0 : 1.0, enabled ? frameColor : ColorF{ 1, 1, 1, 0.08 });
		uiFont(valueText).drawAt(style.valueFontSize, rects.value.center(), enabled ? style.valueText : style.disabledText);
	}

	inline bool HandleRectButtonClick(const RectF& rect)
	{
		return rect.leftClicked();
	}

	inline bool HandleToggleRectButton(const RectF& rect, bool& value)
	{
		if (!HandleRectButtonClick(rect))
		{
			return false;
		}

		value = !value;
		return true;
	}

	inline bool HandleIntTabButtons(int32& selectedIndex, int32 tabCount, const std::function<RectF(int32)>& rectAt)
	{
		for (int32 index = 0; index < tabCount; ++index)
		{
			if (rectAt(index).leftClicked())
			{
				selectedIndex = index;
				return true;
			}
		}

		return false;
	}

	inline Optional<int32> FindClickedRectIndex(int32 count, const std::function<RectF(int32)>& rectAt)
	{
		for (int32 index = 0; index < count; ++index)
		{
			if (rectAt(index).leftClicked())
			{
				return index;
			}
		}

		return none;
	}

	inline Optional<double> FindClickedDeltaButton(const Array<double>& deltas, const std::function<RectF(int32)>& rectAt)
	{
		for (int32 index = 0; index < static_cast<int32>(deltas.size()); ++index)
		{
			if (rectAt(index).leftClicked())
			{
				return deltas[index];
			}
		}

		return none;
	}
}
