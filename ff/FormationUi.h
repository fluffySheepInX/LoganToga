# pragma once
# include "GameConstants.h"

const Array<ff::AllyBehavior>& GetFormationUnitTypes();
String GetAllyBehaviorLabel(ff::AllyBehavior behavior);
ColorF GetAllyBehaviorColor(ff::AllyBehavior behavior);
void DrawFormationUnitButton(const RectF& rect, const Font& font, ff::AllyBehavior behavior, bool selected);
String GetFormationSlotLabel(const Optional<ff::AllyBehavior>& behavior, size_t index);
