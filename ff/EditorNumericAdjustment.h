# pragma once
# include <Siv3D.hpp>

namespace ff
{
	// 浮動小数点の編集値を指定した最小値とstepで調整します。
	inline void AdjustNumericValue(double& value, const double minimum, const double step, const double direction)
	{
		value = Max(minimum, (value + (step * direction)));
	}

	// 整数の編集値を指定した最小値とstepで調整します。
	inline void AdjustNumericValue(int32& value, const int32 minimum, const int32 step, const double direction)
	{
		value = Max(minimum, (value + (step * static_cast<int32>(direction))));
	}
}
