#pragma once

#include "Localization.Runtime.h"

namespace Localization
{
	[[nodiscard]] inline String TryReadLocalizedValue(const s3d::TOMLReader* toml, const String& key)
	{
		if (!toml)
		{
			return U"";
		}

		try
		{
			const String value = (*toml)[key].get<String>();
			if (!value.isEmpty())
			{
				return value;
			}
		}
		catch (const std::exception&)
		{
		}

		return U"";
	}

	[[nodiscard]] inline String TryGetText(const String& key)
	{
		const String localizedValue = TryReadLocalizedValue(LanguageTomlStorage().get(), key);
		if (!localizedValue.isEmpty())
		{
			return localizedValue;
		}

		const String defaultLanguageValue = TryReadLocalizedValue(GetDefaultLanguageToml(), key);
		if (!defaultLanguageValue.isEmpty())
		{
			return defaultLanguageValue;
		}

		return U"";
	}

	[[nodiscard]] inline String GetText(const String& key)
	{
		const String localizedValue = TryGetText(key);
		return localizedValue.isEmpty() ? key : localizedValue;
	}

	template <class... Args>
	[[nodiscard]] inline String FormatText(const String& key, const Args&... args)
	{
		String text = GetText(key);
		const Array<String> replacements = { s3d::Format(args)... };
		for (size_t i = 0; i < replacements.size(); ++i)
		{
			text.replace((U"{" + s3d::Format(i) + U"}"), replacements[i]);
		}
		return text;
	}
}
