#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	inline String EscapeTomlBasicString(StringView text)
	{
		String result;
		for (const char32 ch : text)
		{
			if (ch == U'\\')
			{
				result += U"\\\\";
			}
			else if (ch == U'\"')
			{
				result += U"\\\"";
			}
			else
			{
				result += ch;
			}
		}

		return result;
	}

	inline Array<String> ReadTomlStringArrayValue(const TOMLValue& value)
	{
		Array<String> result;
		if (!value.isArray())
		{
			return result;
		}

		for (const auto item : value.arrayView())
		{
			if (const Optional<String> text = item.getOpt<String>())
			{
				result << *text;
			}
		}

		return result;
	}

	inline String BuildTomlStringArrayValue(const Array<String>& values)
	{
		String result = U"[";
		for (size_t i = 0; i < values.size(); ++i)
		{
			if (i > 0)
			{
				result += U", ";
			}

			result += U"\"" + EscapeTomlBasicString(values[i]) + U"\"";
		}

		result += U"]";
		return result;
	}

	inline FilePath ResolveFirstExistingPath(const Array<FilePath>& candidates)
	{
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}

		return candidates.isEmpty() ? FilePath{} : candidates.back();
	}
}
