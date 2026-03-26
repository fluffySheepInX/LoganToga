# pragma once
# include "GameConstants.h"

namespace ff
{
  Optional<AllyBehavior> CheckSummonAllyButtonPressed();
    void DrawSummonAllyButtons(const Font& font, int32 resourceCount);
}
