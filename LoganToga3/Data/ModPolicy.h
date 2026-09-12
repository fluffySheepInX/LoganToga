#pragma once
# include <cstddef>

namespace LT3::ModPolicy
{
	struct TextView
	{
		const char32_t* data = nullptr;
		std::size_t size = 0;
	};

	enum class QuickBattleTargetKind
	{
		Unsupported,
		Skirmish,
	};

	struct QuickBattleTarget
	{
		QuickBattleTargetKind kind = QuickBattleTargetKind::Unsupported;
		TextView battleId;
	};

	// ASCII英大文字を小文字へ変換します。
	constexpr char32_t ToLowerAscii(const char32_t value)
	{
		return ((U'A' <= value) && (value <= U'Z'))
			? (value - U'A' + U'a')
			: value;
	}

	// ASCII stable IDとして許可する文字かを判定します。
	constexpr bool IsAsciiStableIdCharacter(const char32_t value)
	{
		return ((U'a' <= value) && (value <= U'z'))
			|| ((U'A' <= value) && (value <= U'Z'))
			|| ((U'0' <= value) && (value <= U'9'))
			|| (value == U'-');
	}

	// 大文字小文字を区別せずASCII文字列が一致するかを判定します。
	constexpr bool EqualsAsciiIgnoreCase(const TextView left, const TextView right)
	{
		if (!left.data || !right.data || (left.size != right.size))
		{
			return false;
		}

		for (std::size_t index = 0; index < left.size; ++index)
		{
			if (ToLowerAscii(left.data[index]) != ToLowerAscii(right.data[index]))
			{
				return false;
			}
		}

		return true;
	}

	// 入力IDを小文字のASCII stable IDへ正規化します。
	constexpr bool TryNormalizeAsciiStableId(const TextView input, char32_t* output, const std::size_t outputCapacity)
	{
		if (!input.data || !output || (input.size == 0) || (outputCapacity < input.size))
		{
			return false;
		}

		for (std::size_t index = 0; index < input.size; ++index)
		{
			if (!IsAsciiStableIdCharacter(input.data[index]))
			{
				return false;
			}

			output[index] = ToLowerAscii(input.data[index]);
		}

		return true;
	}

	// 相対パス要素の区切り文字かを判定します。
	constexpr bool IsPathSeparator(const char32_t value)
	{
		return (value == U'/') || (value == U'\\');
	}

	// 既定ゲームrootからの字句的逸脱がない相対パスかを判定します。
	constexpr bool IsLexicallySafeRelativePath(const TextView path)
	{
		if (!path.data || (path.size == 0) || IsPathSeparator(path.data[0]))
		{
			return false;
		}
		if ((path.size >= 2)
			&& (((U'a' <= path.data[0]) && (path.data[0] <= U'z')) || ((U'A' <= path.data[0]) && (path.data[0] <= U'Z')))
			&& (path.data[1] == U':'))
		{
			return false;
		}

		std::size_t depth = 0;
		std::size_t segmentStart = 0;
		while (segmentStart < path.size)
		{
			while ((segmentStart < path.size) && IsPathSeparator(path.data[segmentStart]))
			{
				++segmentStart;
			}
			std::size_t segmentEnd = segmentStart;
			while ((segmentEnd < path.size) && !IsPathSeparator(path.data[segmentEnd]))
			{
				++segmentEnd;
			}
			if (segmentStart == segmentEnd)
			{
				break;
			}

			const TextView segment{ path.data + segmentStart, segmentEnd - segmentStart };
			constexpr char32_t CurrentDirectory[] = U".";
			constexpr char32_t ParentDirectory[] = U"..";
			if (EqualsAsciiIgnoreCase(segment, TextView{ CurrentDirectory, 1 }))
			{
				// no-op
			}
			else if (EqualsAsciiIgnoreCase(segment, TextView{ ParentDirectory, 2 }))
			{
				if (depth == 0)
				{
					return false;
				}
				--depth;
			}
			else
			{
				++depth;
			}
			segmentStart = segmentEnd;
		}

		return true;
	}

	// 正規化済みIDが既定ゲームIDかを判定します。
	constexpr bool IsDefaultGameId(const TextView normalizedId)
	{
		constexpr char32_t DefaultGameId[] = U"000-default-game";
		return EqualsAsciiIgnoreCase(normalizedId, TextView{ DefaultGameId, 16 });
	}

	// manifestで明示された継承が既定ゲームフォールバックを許可するかを判定します。
	constexpr bool CanInheritDefaultGame(const bool manifestRequestsInheritance, const TextView normalizedModId)
	{
		return manifestRequestsInheritance && !IsDefaultGameId(normalizedModId);
	}

	// quick-battle引数を対応するスカーミッシュターゲットへ解釈します。
	constexpr QuickBattleTarget ParseQuickBattleTarget(const TextView argument)
	{
		constexpr char32_t Skirmish[] = U"skirmish";
		constexpr char32_t DefaultBattle[] = U"default";
		const TextView skirmish{ Skirmish, 8 };
		if (!argument.data || (argument.size == 0) || EqualsAsciiIgnoreCase(argument, skirmish))
		{
			return QuickBattleTarget{ QuickBattleTargetKind::Skirmish, TextView{ DefaultBattle, 7 } };
		}

		std::size_t separator = argument.size;
		for (std::size_t index = 0; index < argument.size; ++index)
		{
			if (argument.data[index] == U'/')
			{
				if (separator != argument.size)
				{
					return {};
				}
				separator = index;
			}
		}
		if ((separator == argument.size) || !EqualsAsciiIgnoreCase(TextView{ argument.data, separator }, skirmish))
		{
			return {};
		}

		return QuickBattleTarget{ QuickBattleTargetKind::Skirmish, TextView{ argument.data + separator + 1, argument.size - separator - 1 } };
	}

	static_assert(IsLexicallySafeRelativePath(TextView{ U"future/content.toml", (sizeof(U"future/content.toml") / sizeof(char32_t)) - 1 }));
	static_assert(!IsLexicallySafeRelativePath(TextView{ U"../content.toml", (sizeof(U"../content.toml") / sizeof(char32_t)) - 1 }));
	static_assert(!IsLexicallySafeRelativePath(TextView{ U"C:/content.toml", (sizeof(U"C:/content.toml") / sizeof(char32_t)) - 1 }));
	static_assert(CanInheritDefaultGame(true, TextView{ U"example-mod", (sizeof(U"example-mod") / sizeof(char32_t)) - 1 }));
	static_assert(!CanInheritDefaultGame(true, TextView{ U"000-default-game", (sizeof(U"000-default-game") / sizeof(char32_t)) - 1 }));
	static_assert(ParseQuickBattleTarget(TextView{ U"SkIrMiSh/default", (sizeof(U"SkIrMiSh/default") / sizeof(char32_t)) - 1 }).kind == QuickBattleTargetKind::Skirmish);
}
