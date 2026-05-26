#pragma once
# include <Siv3D.hpp>
# include "RectNumberStepperTypes.h"

namespace LT3
{
		enum class RectNumberStepperInputAction
		{
			None,
			StartValueEdit,
			CycleStep,
			OpenStepMenu,
			Decrement,
			Increment,
		};

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

		inline void DrawRectPanelFrame(const RectF& rect, const ColorF& backColor, const ColorF& frameColor, double frameThickness = 1.0)
		{
			rect.draw(backColor).drawFrame(frameThickness, frameColor);
		}

	inline void DrawRectPanelTitle(const RectF& panel, StringView title, const Font& uiFont,
		double offsetX = 24.0,
		double offsetY = 16.0,
		const ColorF& textColor = Palette::White)
	{
		uiFont(title).draw(panel.x + offsetX, panel.y + offsetY, textColor);
	}

	inline void DrawRectTabButton(const RectF& rect, StringView text, bool active, const Font& uiFont, int32 fontSize = 11)
	{
		DrawRectButton(rect, text, active, uiFont, RectButtonStyle{ .fontSize = fontSize });
	}

	inline void DrawRectListRow(const RectF& rect, bool selected,
		const ColorF& selectedBack = ColorF{ 0.16, 0.18, 0.13, 0.95 },
		const ColorF& normalBack = ColorF{ 0.08, 0.09, 0.11, 0.92 },
		const ColorF& selectedFrame = ColorF{ 1.0, 0.84, 0.0 },
		const ColorF& hoverFrame = ColorF{ 1.0, 0.84, 0.0 },
		const ColorF& normalFrame = ColorF{ 1, 1, 1, 0.14 },
		double frameThickness = 1.0)
	{
		const ColorF backColor = selected ? selectedBack : normalBack;
		const ColorF frameColor = selected ? selectedFrame : (rect.mouseOver() ? hoverFrame : normalFrame);
		rect.draw(backColor).drawFrame(frameThickness, frameColor);
	}

	inline void DrawRectVerticalScrollbar(const RectF& viewportRect, double contentHeight, double scrollValue,
		const ColorF& trackColor = ColorF{ 1, 1, 1, 0.08 },
		const ColorF& handleColor = ColorF{ 1.0, 0.84, 0.0, 0.70 },
		double width = 6.0,
		double xOffset = 4.0,
		double minHandleHeight = 32.0)
	{
		if (!(contentHeight > viewportRect.h))
		{
			return;
		}

		const double maxScroll = Max(0.0, contentHeight - viewportRect.h);
		if (maxScroll <= 0.0)
		{
			return;
		}

		const double clampedScroll = Clamp(scrollValue, 0.0, maxScroll);
		const double scrollRate = clampedScroll / maxScroll;
		const double handleHeight = Max(minHandleHeight, viewportRect.h * viewportRect.h / contentHeight);
		const double handleY = viewportRect.y + (viewportRect.h - handleHeight) * scrollRate;
		RectF{ viewportRect.x + viewportRect.w + xOffset, viewportRect.y, width, viewportRect.h }.draw(trackColor);
		RectF{ viewportRect.x + viewportRect.w + xOffset, handleY, width, handleHeight }.draw(handleColor);
	}

	inline void DrawRectIconButton(const RectF& rect, StringView iconText, const Font& uiFont,
		int32 fontSize = 14,
		const ColorF& backColor = ColorF{ 0.08, 0.09, 0.11, 0.92 },
		double frameThickness = 2.0,
		const ColorF& textColor = Palette::White)
	{
		rect.draw(backColor).drawFrame(frameThickness, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(iconText).drawAt(fontSize, rect.center(), textColor);
	}

	inline void DrawRectPanelCloseButton(const RectF& rect, const Font& uiFont,
		int32 fontSize = 18,
		const ColorF& backColor = ColorF{ 0.12, 0.05, 0.05, 0.95 },
		double frameThickness = 1.0,
		const ColorF& textColor = Palette::White)
	{
		DrawRectIconButton(rect, U"×", uiFont, fontSize, backColor, frameThickness, textColor);
	}

	inline void DrawUiLayoutEditHandle(const RectF& dragHandle, const RectF& topAnchorToggle, bool dragging, bool topAnchored, const Font& uiFont,
		int32 fontSize = 11)
	{
		dragHandle.draw(dragging ? ColorF{ 1.0, 0.84, 0.0, 0.9 } : ColorF{ 1.0, 0.84, 0.0, 0.4 })
			.drawFrame(1, ColorF{ 1, 1, 1, 0.2 });
		uiFont(U"↕").drawAt(fontSize, dragHandle.center(), Palette::White);
		topAnchorToggle.draw(topAnchored ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, topAnchorToggle.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"↑").drawAt(fontSize, topAnchorToggle.center(), topAnchored ? Palette::White : Palette::Lightgray);
	}

	inline void DrawRectCheckRow(const RectF& rect, StringView label, bool checked, const Font& uiFont,
		int32 fontSize = 11,
		double checkOffsetX = 4.0,
		double checkOffsetY = 10.0,
		bool withStatePrefix = true)
	{
		const String text = withStatePrefix
			? (checked ? U"[ON] {}"_fmt(label) : U"[OFF] {}"_fmt(label))
			: String{ label };
		DrawRectTabButton(rect, text, checked, uiFont, fontSize);
		if (checked)
		{
			Line{ rect.x + checkOffsetX, rect.y + checkOffsetY, rect.x + checkOffsetX + 4.0, rect.y + checkOffsetY + 5.0 }.draw(2.0, Palette::White);
			Line{ rect.x + checkOffsetX + 4.0, rect.y + checkOffsetY + 5.0, rect.x + checkOffsetX + 12.0, rect.y + checkOffsetY - 5.0 }.draw(2.0, Palette::White);
		}
	}

	inline void DrawRectValueAdjustRow(const RectF& row, StringView label, StringView valueText,
		const RectF& decRect, const RectF& incRect, const Font& uiFont,
		int32 labelFontSize = 13,
		int32 buttonFontSize = 24,
		int32 valueFontSize = 26,
		double labelOffsetY = 18.0,
		const ColorF& rowBackColor = ColorF{ 0.05, 0.06, 0.08, 0.70 },
		const ColorF& rowFrameColor = ColorF{ 1, 1, 1, 0.08 },
		const ColorF& labelColor = Palette::Gold,
		const ColorF& valueColor = Palette::White)
	{
		row.draw(rowBackColor).drawFrame(1, rowFrameColor);
		uiFont(label).draw(labelFontSize, row.x, row.y - labelOffsetY, labelColor);
		DrawRectIconButton(decRect, U"-", uiFont, buttonFontSize, ColorF{ 0.08, 0.09, 0.11, 0.92 }, 2.0, Palette::White);
		DrawRectIconButton(incRect, U"+", uiFont, buttonFontSize, ColorF{ 0.08, 0.09, 0.11, 0.92 }, 2.0, Palette::White);
		uiFont(valueText).drawAt(valueFontSize, Vec2{ row.center().x, decRect.center().y }, valueColor);
	}

	inline void DrawRectNumberStepper(RectNumberStepperRects stepperRects, StringView valueText, StringView stepText, bool editingValue, bool activeStepMenu, bool enabled, const Font& uiFont, const RectNumberStepperStyle& style = RectNumberStepperStyle{})
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

		DrawRectButton(stepperRects.minus, U"-", false, uiFont, buttonStyle);
		DrawRectButton(stepperRects.plus, U"+", false, uiFont, buttonStyle);
		DrawRectButton(stepperRects.step, stepText, activeStepMenu, uiFont, buttonStyle);

		const ColorF frameColor = editingValue ? style.valueActiveFrame : (stepperRects.value.mouseOver() ? style.valueHoverFrame : style.valueFrame);
		stepperRects.value.draw(style.valueBack).drawFrame(editingValue ? 2.0 : 1.0, enabled ? frameColor : ColorF{ 1, 1, 1, 0.08 });
		uiFont(valueText).drawAt(style.valueFontSize, stepperRects.value.center(), enabled ? style.valueText : style.disabledText);
	}

	inline bool HandleRectButtonClick(const RectF& rect)
	{
		return rect.leftClicked();
	}

		inline RectNumberStepperInputAction DetectRectNumberStepperInput(RectNumberStepperRects rects)
		{
			if (rects.value.leftClicked())
			{
				return RectNumberStepperInputAction::StartValueEdit;
			}
			if (rects.step.leftClicked())
			{
				return RectNumberStepperInputAction::CycleStep;
			}
			if (rects.step.rightClicked() || rects.minus.rightClicked() || rects.plus.rightClicked())
			{
				return RectNumberStepperInputAction::OpenStepMenu;
			}
			if (rects.minus.leftClicked())
			{
				return RectNumberStepperInputAction::Decrement;
			}
			if (rects.plus.leftClicked())
			{
				return RectNumberStepperInputAction::Increment;
			}

			return RectNumberStepperInputAction::None;
		}

		inline double ApplyTemporaryStepModifier(double step)
		{
			double modified = step;
			if (KeyShift.pressed())
			{
				modified *= 10.0;
			}
			if (KeyControl.pressed())
			{
				modified *= 0.1;
			}
			return modified;
		}

		inline double NextCycledStepValue(const Array<double>& steps, double current)
		{
			if (steps.isEmpty())
			{
				return current;
			}

			int32 nextIndex = 0;
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				if (Math::Abs(steps[i] - current) < 0.0001)
				{
					nextIndex = (i + 1) % static_cast<int32>(steps.size());
					break;
				}
			}

			return steps[nextIndex];
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
