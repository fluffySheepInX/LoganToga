#pragma once
# include <Siv3D.hpp>
# include "RectNumberStepperTypes.h"

namespace LT3
{
	inline RectF RectFromPanel(const RectF& panel, double x, double y, double w, double h)
	{
		return RectF{ panel.x + x, panel.y + y, w, h };
	}

	inline RectF RectFromPanelRight(const RectF& panel, double rightOffset, double y, double w, double h)
	{
		return RectF{ panel.x + panel.w - rightOffset, panel.y + y, w, h };
	}

	inline RectF RectFromPanelBottomRight(const RectF& panel, double rightOffset, double bottomOffset, double w, double h)
	{
		return RectF{ panel.x + panel.w - rightOffset, panel.y + panel.h - bottomOffset, w, h };
	}

	inline RectF RectInset(const RectF& rect, double left, double top, double right, double bottom)
	{
		return RectF{ rect.x + left, rect.y + top, rect.w - left - right, rect.h - top - bottom };
	}

	inline RectF RectRow(const RectF& viewport, int32 index, double rowHeight, double rowStride, double scroll = 0.0, double insetX = 0.0, double insetY = 0.0)
	{
		return RectF{ viewport.x + insetX, viewport.y + insetY + index * rowStride - scroll, viewport.w - insetX * 2.0, rowHeight };
	}

	inline RectF RectLinearItem(const Vec2& origin, int32 index, const SizeF& size, double strideX, double strideY = 0.0)
	{
		return RectF{ origin.x + index * strideX, origin.y + index * strideY, size.x, size.y };
	}

	inline RectF RectGridItem(const Vec2& origin, int32 index, int32 columns, const SizeF& size, const Vec2& stride)
	{
		const int32 safeColumns = Max(1, columns);
		const int32 col = index % safeColumns;
		const int32 row = index / safeColumns;
		return RectF{ origin.x + col * stride.x, origin.y + row * stride.y, size.x, size.y };
	}

	inline RectF RectCloseButton(const RectF& panel, double rightOffset = 42.0, double topOffset = 10.0, const SizeF& size = SizeF{ 28.0, 28.0 })
	{
		return RectFromPanelRight(panel, rightOffset, topOffset, size.x, size.y);
	}

	inline RectF RectDragHandle(const RectF& panel, double rightOffset = 24.0, double topOffset = 6.0, const SizeF& size = SizeF{ 18.0, 18.0 })
	{
		return RectFromPanelRight(panel, rightOffset, topOffset, size.x, size.y);
	}

	inline RectF RectStepMenu(const Vec2& pos, int32 itemCount, double width = 86.0, double topBottomPadding = 8.0, double itemStride = 24.0)
	{
		return RectF{ pos.x, pos.y, width, topBottomPadding + itemCount * itemStride };
	}

	inline RectF RectStepMenuItem(const Vec2& pos, int32 index, double xInset = 4.0, double yInset = 4.0, const SizeF& size = SizeF{ 78.0, 20.0 }, double itemStride = 24.0)
	{
		return RectF{ pos.x + xInset, pos.y + yInset + index * itemStride, size.x, size.y };
	}

	inline RectNumberStepperRects RectNumberStepperFromRow(const RectF& row, double valueX, double valueY, double valueW, double valueH, double minusX, double plusX, double stepX, double buttonW = 26.0, double stepW = 46.0)
	{
		return RectNumberStepperRects{
			.minus = RectF{ row.x + minusX, row.y + valueY, buttonW, valueH },
			.value = RectF{ row.x + valueX, row.y + valueY, valueW, valueH },
			.plus = RectF{ row.x + plusX, row.y + valueY, buttonW, valueH },
			.step = RectF{ row.x + stepX, row.y + valueY, stepW, valueH },
		};
	}

	inline RectNumberStepperRects RectNumberStepperFromValueRect(const RectF& valueRect, const RectF& minusRect, double plusGap, double plusW, const RectF& stepRect)
	{
		return RectNumberStepperRects{
			.minus = minusRect,
			.value = valueRect,
			.plus = RectF{ valueRect.x + valueRect.w + plusGap, valueRect.y, plusW, valueRect.h },
			.step = stepRect,
		};
	}
}
