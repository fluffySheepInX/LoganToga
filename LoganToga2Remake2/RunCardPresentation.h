#pragma once

#include "Localization.h"
#include "RunTypes.h"

[[nodiscard]] inline String GetRewardCardRarityLabel(const RewardCardRarity rarity)
{
	switch (rarity)
	{
	case RewardCardRarity::Rare:
      return Localization::GetText(U"reward.card.rarity.rare");
	case RewardCardRarity::Epic:
        return Localization::GetText(U"reward.card.rarity.epic");
	case RewardCardRarity::Common:
	default:
       return Localization::GetText(U"reward.card.rarity.common");
	}
}

[[nodiscard]] inline String GetRewardCardName(const RewardCardDefinition& card)
{
 return Localization::GetText(U"reward.card." + card.id + U".name");
}

[[nodiscard]] inline String GetRewardCardDescription(const RewardCardDefinition& card)
{
 return Localization::GetText(U"reward.card." + card.id + U".description");
}

[[nodiscard]] inline ColorF GetRewardCardRarityColor(const RewardCardRarity rarity)
{
	switch (rarity)
	{
	case RewardCardRarity::Rare:
		return ColorF{ 0.28, 0.56, 1.0 };
	case RewardCardRarity::Epic:
		return ColorF{ 0.78, 0.40, 1.0 };
	case RewardCardRarity::Common:
	default:
		return ColorF{ 0.78, 0.82, 0.90 };
	}
}
