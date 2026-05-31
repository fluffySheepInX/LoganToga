#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.Layout.h"

namespace LT3
{
	/// <summary>
	/// AI Editor の unit priority 行用 stepper 矩形を返します。
	/// </summary>
	inline RectNumberStepperRects AiEditorUnitWeightStepperRects(double rightX, const RectF& row, double valueWidth, double stepWidth)
	{
		const double buttonWidth = 18.0;
		const double gap = 3.0;
		const double totalWidth = buttonWidth * 2.0 + valueWidth + stepWidth + gap * 3.0;
		const double left = rightX - totalWidth;
		return RectNumberStepperRects{
			.minus = RectF{ left, row.y + 4.0, buttonWidth, 26.0 },
			.value = RectF{ left + buttonWidth + gap, row.y + 4.0, valueWidth, 26.0 },
			.plus = RectF{ left + buttonWidth + gap + valueWidth + gap, row.y + 4.0, buttonWidth, 26.0 },
			.step = RectF{ left + buttonWidth + gap + valueWidth + gap + buttonWidth + gap, row.y + 4.0, stepWidth, 26.0 },
		};
	}

	/// <summary>
	/// AI Editor の unit priority 行で削除ボタンの矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightDeleteRect(const RectF& row)
	{
		const double width = 34.0;
		return RectF{ AiEditorActionRightX(row) - width, row.y + 4.0, width, 26.0 };
	}

	/// <summary>
	/// AI Editor の unit priority 行で desired count stepper の矩形を返します。
	/// </summary>
	inline RectNumberStepperRects AiEditorUnitWeightDesiredStepperRects(const RectF& row)
	{
		return AiEditorUnitWeightStepperRects(AiEditorUnitWeightDeleteRect(row).x - 6.0, row, 26.0, 28.0);
	}

	/// <summary>
	/// AI Editor の unit priority 行で weight stepper の矩形を返します。
	/// </summary>
	inline RectNumberStepperRects AiEditorUnitWeightValueStepperRects(const RectF& row)
	{
		return AiEditorUnitWeightStepperRects(AiEditorUnitWeightDesiredStepperRects(row).minus.x - 6.0, row, 34.0, 34.0);
	}

	/// <summary>
	/// AI Editor の unit priority 行でユニット選択ボタンの矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightUnitButtonRect(const RectF& row)
	{
		const double left = AiEditorRowValueX(row);
		const double right = AiEditorUnitWeightValueStepperRects(row).minus.x - 8.0;
		return RectF{ left, row.y + 4.0, Max(56.0, right - left), 26.0 };
	}

	/// <summary>
	/// AI Editor の unit priority 行で候補ユニット一覧パネルの矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightPickerRect(const Vec2& anchor, int32 visibleRows)
	{
		const RectF detail = AiEditorDetailRect();
		const double width = 236.0;
		const double height = 8.0 + visibleRows * 24.0;
		const double x = Clamp(anchor.x, detail.x + 8.0, detail.x + detail.w - width - 8.0);
		const double y = Clamp(anchor.y, detail.y + 8.0, detail.y + detail.h - height - 8.0);
		return RectF{ x, y, width, height };
	}

	/// <summary>
	/// AI Editor の unit priority 行で候補ユニット一覧アイテムの矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightPickerItemRect(const RectF& panel, int32 index, double scroll)
	{
		return RectF{ panel.x + 4.0, panel.y + 4.0 + index * 24.0 - scroll, panel.w - 8.0, 22.0 };
	}

	/// <summary>
	/// AI Editor の unit priority 行で step メニューの矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightStepMenuRect(const Vec2& anchor, int32 itemCount)
	{
		const RectF detail = AiEditorDetailRect();
		const double width = 88.0;
		const double height = 8.0 + itemCount * 22.0;
		const double x = Clamp(anchor.x, detail.x + 8.0, detail.x + detail.w - width - 8.0);
		const double y = Clamp(anchor.y, detail.y + 8.0, detail.y + detail.h - height - 8.0);
		return RectF{ x, y, width, height };
	}

	/// <summary>
	/// AI Editor の unit priority 行で step メニュー項目の矩形を返します。
	/// </summary>
	inline RectF AiEditorUnitWeightStepMenuItemRect(const Vec2& anchor, int32 index)
	{
		const RectF menuRect = AiEditorUnitWeightStepMenuRect(anchor, 1);
		return RectF{ menuRect.x + 4.0, menuRect.y + 4.0 + index * 22.0, menuRect.w - 8.0, 20.0 };
	}

	/// <summary>
	/// AI Editor の unit priority 行で使う weight step 候補を返します。
	/// </summary>
	inline const Array<double>& AiEditorUnitWeightStepOptions()
	{
		static const Array<double> steps = { 0.1, 0.5, 1.0, 2.0, 5.0 };
		return steps;
	}

	/// <summary>
	/// AI Editor の unit priority 行で使う desired count step 候補を返します。
	/// </summary>
	inline const Array<int32>& AiEditorUnitDesiredStepOptions()
	{
		static const Array<int32> steps = { 1, 2, 5, 10 };
		return steps;
	}
}
