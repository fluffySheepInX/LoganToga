# pragma once
# include "GameConstants.h"

const Array<ff::UnitId>& GetFormationUnitTypes();
String GetAllyBehaviorLabel(ff::UnitId behavior);
StringView GetAllyBehaviorRoleDescription(ff::UnitId behavior);
ColorF GetAllyBehaviorColor(ff::UnitId behavior);
void DrawFormationUnitButton(const RectF& rect, const Font& font, const Font& infoFont, const ff::UnitDefinition& unitDefinition, bool selected);
String GetFormationSlotLabel(const Optional<ff::UnitId>& behavior, size_t index);
