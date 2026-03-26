# include "FormationUi.h"

const Array<ff::AllyBehavior>& GetFormationUnitTypes()
{
	static const Array<ff::AllyBehavior> UnitTypes = {
		ff::AllyBehavior::ChaseEnemies,
		ff::AllyBehavior::HoldPosition,
		ff::AllyBehavior::GuardPlayer,
		ff::AllyBehavior::OrbitPlayer,
		ff::AllyBehavior::FixedTurret,
	};

	return UnitTypes;
}

String GetAllyBehaviorLabel(const ff::AllyBehavior behavior)
{
	switch (behavior)
	{
	case ff::AllyBehavior::ChaseEnemies:
		return U"追撃";

	case ff::AllyBehavior::HoldPosition:
		return U"固定";

	case ff::AllyBehavior::GuardPlayer:
		return U"護衛";

	case ff::AllyBehavior::OrbitPlayer:
		return U"周回";

	case ff::AllyBehavior::FixedTurret:
		return U"砲台";
	}

	return U"不明";
}

ColorF GetAllyBehaviorColor(const ff::AllyBehavior behavior)
{
	switch (behavior)
	{
	case ff::AllyBehavior::ChaseEnemies:
		return ColorF{ 0.88, 0.42, 0.36 };

	case ff::AllyBehavior::HoldPosition:
		return ColorF{ 0.95, 0.76, 0.34 };

	case ff::AllyBehavior::GuardPlayer:
		return ColorF{ 0.38, 0.78, 0.56 };

	case ff::AllyBehavior::OrbitPlayer:
		return ColorF{ 0.50, 0.62, 0.94 };

	case ff::AllyBehavior::FixedTurret:
		return ColorF{ 0.72, 0.54, 0.22 };
	}

	return Palette::White;
}

void DrawFormationUnitButton(const RectF& rect, const Font& font, const ff::AllyBehavior behavior, const bool selected)
{
	const bool hovered = rect.mouseOver();
	const ColorF accent = GetAllyBehaviorColor(behavior);
	const ColorF fillColor = selected
		? accent.lerp(ColorF{ 0.20, 0.24, 0.34 }, 0.45)
		: (hovered ? ColorF{ 0.22, 0.27, 0.39, 0.96 } : ColorF{ 0.14, 0.18, 0.28, 0.92 });
	const ColorF frameColor = selected ? accent : ColorF{ 0.75, 0.82, 0.95, (hovered ? 0.90 : 0.58) };

	rect.rounded(12).draw(fillColor);
	rect.rounded(12).drawFrame(2, frameColor);
	font(GetAllyBehaviorLabel(behavior)).drawAt(20, rect.center(), Palette::White);
}

String GetFormationSlotLabel(const Optional<ff::AllyBehavior>& behavior, const size_t index)
{
	if (behavior)
	{
		return U"Slot {}: {}"_fmt(index + 1, GetAllyBehaviorLabel(*behavior));
	}

	return U"Slot {}: Empty"_fmt(index + 1);
}
