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

[[nodiscard]] inline String GetRewardCardLocalizedFallback(const String& key, const String& japanese, const String& english)
{
	if (!(japanese.isEmpty() && english.isEmpty()))
	{
		const String localizedValue = Localization::TryGetText(key);
		if (!localizedValue.isEmpty())
		{
			return localizedValue;
		}

     return Localization::Legacy::SelectFallbackText(japanese, english);
	}

	return Localization::GetText(key);
}

[[nodiscard]] inline String GetRewardCardName(const RewardCardDefinition& card)
{
   const String japanese = card.nameJa.isEmpty() ? card.name : card.nameJa;
	const String english = card.nameEn.isEmpty() ? card.name : card.nameEn;
  return GetRewardCardLocalizedFallback(U"reward.card." + card.id + U".name", japanese, english);
}

[[nodiscard]] inline String GetRewardCardDescription(const RewardCardDefinition& card)
{
  const String japanese = card.descriptionJa.isEmpty() ? card.description : card.descriptionJa;
	const String english = card.descriptionEn.isEmpty() ? card.description : card.descriptionEn;
    return GetRewardCardLocalizedFallback(U"reward.card." + card.id + U".description", japanese, english);
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
