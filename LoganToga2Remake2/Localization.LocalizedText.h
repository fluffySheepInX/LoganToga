#pragma once

#include "Localization.Text.h"
#include <utility>

struct LocalizedText
{
	String key;

	LocalizedText() = default;

	LocalizedText(String keyValue)
		: key{ std::move(keyValue) }
	{
	}

	[[nodiscard]] String get() const
	{
       return Localization::GetText(key);
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
