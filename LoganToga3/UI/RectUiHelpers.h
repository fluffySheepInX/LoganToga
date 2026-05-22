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

	inline void DrawRectButton(const RectF& rect, StringView text, bool active, const Font& uiFont, const RectButtonStyle& style = RectButtonStyle{})
	{
		const ColorF backColor = active ? style.activeBack : style.normalBack;
		const ColorF frameColor = rect.mouseOver() ? style.hoverFrame : style.normalFrame;
		const ColorF textColor = active ? style.activeText : style.normalText;
		rect.draw(backColor).drawFrame(style.frameThickness, frameColor);
		uiFont(text).drawAt(style.fontSize, rect.center(), textColor);
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
