#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	enum class BattleOutcome : uint8
	{
		InProgress,
		Victory,
		Defeat,
		Draw,
		Aborted,
	};

	enum class BattleEndCondition : uint8
	{
		DestroyEnemyBases,
		EliminateEnemyUnits,
		None,
	};

	struct BattleOutcomeRules
	{
		BattleEndCondition victoryCondition = BattleEndCondition::DestroyEnemyBases;
		BattleEndCondition defeatCondition = BattleEndCondition::DestroyEnemyBases;
		double timeLimitSec = 0.0;
		BattleOutcome timeoutOutcome = BattleOutcome::Draw;
	};

	// 戦闘結果が終局状態かを判定する。
	inline bool IsTerminalBattleOutcome(BattleOutcome outcome)
	{
		return outcome != BattleOutcome::InProgress;
	}

	// TOML の終局条件値を実行時の条件へ変換する。
	inline Optional<BattleEndCondition> ParseBattleEndCondition(StringView value)
	{
		const String normalized = String{ value }.lowercased();
		if (normalized == U"destroy_enemy_bases")
		{
			return BattleEndCondition::DestroyEnemyBases;
		}
		if (normalized == U"eliminate_enemy_units")
		{
			return BattleEndCondition::EliminateEnemyUnits;
		}
		if (normalized == U"none")
		{
			return BattleEndCondition::None;
		}
		return none;
	}

	// TOML の時間切れ結果値を実行時の結果へ変換する。
	inline Optional<BattleOutcome> ParseTimeoutBattleOutcome(StringView value)
	{
		const String normalized = String{ value }.lowercased();
		if (normalized == U"victory")
		{
			return BattleOutcome::Victory;
		}
		if (normalized == U"defeat")
		{
			return BattleOutcome::Defeat;
		}
		if (normalized == U"draw")
		{
			return BattleOutcome::Draw;
		}
		return none;
	}
}
