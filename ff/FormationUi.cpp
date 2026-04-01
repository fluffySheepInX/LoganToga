# include "FormationUi.h"

const Array<ff::UnitId>& GetFormationUnitTypes()
{
  return ff::GetAvailableUnitIds();
}

String GetAllyBehaviorLabel(const ff::UnitId behavior)
{
   return ff::GetUnitDefinition(behavior).label;
}

StringView GetAllyBehaviorRoleDescription(const ff::UnitId behavior)
{
   return ff::GetUnitDefinition(behavior).roleDescription;
}

ColorF GetAllyBehaviorColor(const ff::UnitId behavior)
{
   return ff::GetUnitDefinition(behavior).color;
}

void DrawFormationUnitButton(const RectF& rect, const Font& font, const Font& infoFont, const ff::UnitDefinition& unitDefinition, const bool selected)
{
	const bool hovered = rect.mouseOver();
   const ColorF accent = unitDefinition.color;
	const ColorF fillColor = selected
		? accent.lerp(ColorF{ 0.20, 0.24, 0.34 }, 0.45)
		: (hovered ? ColorF{ 0.22, 0.27, 0.39, 0.96 } : ColorF{ 0.14, 0.18, 0.28, 0.92 });
	const ColorF frameColor = selected ? accent : ColorF{ 0.75, 0.82, 0.95, (hovered ? 0.90 : 0.58) };

   if (hovered)
	{
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	rect.rounded(12).draw(fillColor);
	rect.rounded(12).drawFrame(2, frameColor);
 font(unitDefinition.label).drawAt(19, rect.center().movedBy(0, -8), Palette::White);
	infoFont(unitDefinition.roleDescription).drawAt(12, rect.center().movedBy(0, 12), ColorF{ 0.90, 0.94, 1.0, 0.84 });
}

String GetFormationSlotLabel(const Optional<ff::UnitId>& behavior, const size_t index)
{
	if (behavior)
	{
		return U"Slot {}: {}"_fmt(index + 1, GetAllyBehaviorLabel(*behavior));
	}

	return U"Slot {}: Empty"_fmt(index + 1);
}
