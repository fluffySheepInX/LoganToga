# pragma once
# include "BalanceConstants.h"

namespace ff
{
	template <class TomlLike>
	[[nodiscard]] inline String ReadTomlString(const TomlLike& toml, const String& key, const String& fallback)
	{
		try
		{
			const String value = toml[key].get<String>();
			return value.isEmpty() ? fallback : value;
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	template <class TomlLike>
	[[nodiscard]] inline int32 ReadTomlInt(const TomlLike& toml, const String& key, const int32 fallback)
	{
		try
		{
			return toml[key].get<int32>();
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	template <class TomlLike>
	[[nodiscard]] inline double ReadTomlDouble(const TomlLike& toml, const String& key, const double fallback)
	{
		try
		{
			return toml[key].get<double>();
		}
		catch (const std::exception&)
		{
			try
			{
				return static_cast<double>(toml[key].get<int32>());
			}
			catch (const std::exception&)
			{
				return fallback;
			}
		}
	}

	template <class TomlLike>
	[[nodiscard]] inline ColorF ReadTomlColor(const TomlLike& toml, const String& key, const ColorF& fallback)
	{
		try
		{
			Array<double> channels;
			channels.reserve(4);

			for (const auto& value : toml[key].arrayView())
			{
				try
				{
					channels << Clamp(value.get<double>(), 0.0, 1.0);
				}
				catch (const std::exception&)
				{
					try
					{
						channels << Clamp((static_cast<double>(value.get<int32>()) / 255.0), 0.0, 1.0);
					}
					catch (const std::exception&)
					{
						channels << ((channels.size() == 0) ? fallback.r
							: (channels.size() == 1) ? fallback.g
							: (channels.size() == 2) ? fallback.b
							: fallback.a);
					}
				}

				if (channels.size() >= 4)
				{
					break;
				}
			}

			if (channels.size() < 3)
			{
				return fallback;
			}

			const double alpha = (channels.size() >= 4) ? channels[3] : fallback.a;
			return ColorF{ channels[0], channels[1], channels[2], alpha };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	[[nodiscard]] inline String EscapeTomlBasicString(const String& value)
	{
		String escaped;
		escaped.reserve(value.size());

		for (const auto ch : value)
		{
			if (ch == U'\\')
			{
				escaped += U"\\\\";
			}
			else if (ch == U'\"')
			{
				escaped += U"\\\"";
			}
			else
			{
				escaped.push_back(ch);
			}
		}

		return escaped;
	}

	[[nodiscard]] inline String BuildTomlColorArray(const ColorF& color)
	{
		return U"[{:.3f}, {:.3f}, {:.3f}, {:.3f}]"_fmt(color.r, color.g, color.b, color.a);
	}
}
