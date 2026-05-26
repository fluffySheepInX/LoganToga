#pragma once
# include <Siv3D.hpp>
# include "RectLayoutPrimitives.h"
# include "RectNumberStepperTypes.h"

namespace LT3
{
	struct RectValueRowLayoutSpec
	{
		double rowHeight = 34.0;
		double rowStride = 40.0;
		double fieldY = 5.0;
		double fieldHeightInset = 10.0;
		double nameX = 8.0;
		double nameW = 128.0;
		double valueX = 146.0;
		double valueW = 72.0;
		double minusX = 224.0;
		double plusX = 254.0;
		double stepX = 284.0;
		double stepW = 48.0;
		double buttonX = 338.0;
		double buttonW = 24.0;
		double buttonStride = 28.0;
		double stepperButtonW = 26.0;
	};

	inline RectF RectValueRow(const RectF& viewport, int32 index, const RectValueRowLayoutSpec& spec, double scroll = 0.0)
	{
		return RectRow(viewport, index, spec.rowHeight, spec.rowStride, scroll);
	}

	inline RectF RectValueRowName(const RectF& row, const RectValueRowLayoutSpec& spec)
	{
		return RectF{ row.x + spec.nameX, row.y + spec.fieldY, spec.nameW, row.h - spec.fieldHeightInset };
	}

	inline RectF RectValueRowValue(const RectF& row, const RectValueRowLayoutSpec& spec)
	{
		return RectF{ row.x + spec.valueX, row.y + spec.fieldY, spec.valueW, row.h - spec.fieldHeightInset };
	}

	inline RectNumberStepperRects RectValueRowStepper(const RectF& row, const RectValueRowLayoutSpec& spec)
	{
		return RectNumberStepperFromRow(row, spec.valueX, spec.fieldY, spec.valueW, row.h - spec.fieldHeightInset, spec.minusX, spec.plusX, spec.stepX, spec.stepperButtonW, spec.stepW);
	}

	inline RectF RectValueRowButton(const RectF& row, int32 buttonIndex, const RectValueRowLayoutSpec& spec)
	{
		return RectF{ row.x + spec.buttonX + buttonIndex * spec.buttonStride, row.y + spec.fieldY, spec.buttonW, row.h - spec.fieldHeightInset };
	}

	inline RectF RectValueRowStepMenu(const Vec2& pos, int32 itemCount)
	{
		return RectStepMenu(pos, itemCount);
	}

	inline RectF RectValueRowStepMenuItem(const Vec2& pos, int32 index)
	{
		return RectStepMenuItem(pos, index);
	}

}
