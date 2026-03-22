#pragma once

#include "Localization.Legacy.h"
#include <utility>

struct LocalizedText
{
	String key;
	String legacyJapanese;
	String legacyEnglish;

	LocalizedText() = default;

	LocalizedText(String keyValue)
		: key{ std::move(keyValue) }
	{
	}

	LocalizedText(String keyValue, String japaneseValue, String englishValue)
		: key{ std::move(keyValue) }
		, legacyJapanese{ std::move(japaneseValue) }
		, legacyEnglish{ std::move(englishValue) }
	{
	}

	[[nodiscard]] bool hasLegacyFallbackText() const
	{
		return !(legacyJapanese.isEmpty() && legacyEnglish.isEmpty());
	}

	[[nodiscard]] String get() const
	{
		if (!hasLegacyFallbackText())
		{
			return Localization::GetText(key);
		}

		return Localization::Legacy::GetText(key, legacyJapanese, legacyEnglish);
	}

	operator String() const
	{
		return get();
	}
};

namespace s3d
{
	inline void Formatter(FormatData& formatData, const ::LocalizedText& value)
	{
		Formatter(formatData, value.get());
	}
}
