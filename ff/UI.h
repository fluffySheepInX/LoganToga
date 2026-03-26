# pragma once
# include "GameConstants.h"

namespace ff
{
    struct SummonInputResult
    {
        size_t slotIndex = 0;
        AllyBehavior behavior = AllyBehavior::GuardPlayer;
    };

    Optional<SummonInputResult> CheckSummonAllyButtonPressed(const Array<Optional<AllyBehavior>>& formationSlots);
    void DrawSummonAllyButtons(const Font& font, int32 resourceCount, const Array<Optional<AllyBehavior>>& formationSlots, const Optional<size_t>& deniedSlotIndex = none, double deniedFlashTimer = 0.0);
}
