#pragma once

#include "RunTypes.h"

[[nodiscard]] inline String GetRewardCardRarityLabel(const RewardCardRarity rarity)
{
	switch (rarity)
	{
	case RewardCardRarity::Rare:
		return U"RARE";
	case RewardCardRarity::Epic:
		return U"EPIC";
	case RewardCardRarity::Common:
	default:
		return U"COMMON";
	}
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
