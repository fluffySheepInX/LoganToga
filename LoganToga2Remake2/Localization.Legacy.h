#pragma once

#include "Localization.Text.h"

namespace Localization::Legacy
{
	[[nodiscard]] inline bool PrefersJapaneseFallbackText(const AppLanguage& language)
	{
		return (ParsePersistenceLabel(language) == U"ja");
	}

	[[nodiscard]] inline String SelectFallbackText(const String& japanese, const String& english)
	{
		if (PrefersJapaneseFallbackText(GetLanguage()))
		{
			if (!japanese.isEmpty())
			{
				return japanese;
			}

			return english;
		}

		if (!english.isEmpty())
		{
			return english;
		}

		return japanese;
	}

	[[nodiscard]] inline String GetText(const String& key, const String& japanese, const String& english)
	{
		const String localizedValue = TryGetText(key);
		return localizedValue.isEmpty() ? SelectFallbackText(japanese, english) : localizedValue;
	}

	template <class... Args>
	[[nodiscard]] inline String FormatText(const String& key, const String& japanese, const String& english, const Args&... args)
	{
		String text = GetText(key, japanese, english);
		const Array<String> replacements = { s3d::Format(args)... };
		for (size_t i = 0; i < replacements.size(); ++i)
		{
			text.replace((U"{" + s3d::Format(i) + U"}"), replacements[i]);
		}
		return text;
	}
}
