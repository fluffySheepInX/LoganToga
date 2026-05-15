#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	inline void DrawNineSliceTexture(const Texture& texture, const RectF& rect, const double cornerSize)
	{
		if (rect.w <= 0.0 || rect.h <= 0.0)
		{
			return;
		}

		const double left = Min(cornerSize, rect.w * 0.5);
		const double right = left;
		const double top = Min(cornerSize, rect.h * 0.5);
		const double bottom = top;
		const double centerW = Max(0.0, rect.w - left - right);
		const double centerH = Max(0.0, rect.h - top - bottom);
		constexpr int32 sourceCorner = 16;
		constexpr int32 sourceCenter = 64;

		texture(Rect{ 0, 0, sourceCorner, sourceCorner }).resized(left, top).draw(rect.pos);
		texture(Rect{ 80, 0, sourceCorner, sourceCorner }).resized(right, top).draw(rect.pos.movedBy(rect.w - right, 0));
		texture(Rect{ 0, 80, sourceCorner, sourceCorner }).resized(left, bottom).draw(rect.pos.movedBy(0, rect.h - bottom));
		texture(Rect{ 80, 80, sourceCorner, sourceCorner }).resized(right, bottom).draw(rect.pos.movedBy(rect.w - right, rect.h - bottom));

		if (centerW > 0.0)
		{
			texture(Rect{ sourceCorner, 0, sourceCenter, sourceCorner }).resized(centerW, top).draw(rect.pos.movedBy(left, 0));
			texture(Rect{ sourceCorner, 80, sourceCenter, sourceCorner }).resized(centerW, bottom).draw(rect.pos.movedBy(left, rect.h - bottom));
		}
		if (centerH > 0.0)
		{
			texture(Rect{ 0, sourceCorner, sourceCorner, sourceCenter }).resized(left, centerH).draw(rect.pos.movedBy(0, top));
			texture(Rect{ 80, sourceCorner, sourceCorner, sourceCenter }).resized(right, centerH).draw(rect.pos.movedBy(rect.w - right, top));
		}
		if (centerW > 0.0 && centerH > 0.0)
		{
			texture(Rect{ sourceCorner, sourceCorner, sourceCenter, sourceCenter }).resized(centerW, centerH).draw(rect.pos.movedBy(left, top));
		}
	}
}
