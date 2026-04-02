# pragma once
# include "TomlUtils.h"

namespace ff
{
	enum class EnemyKind : uint8
	{
		Normal,
		MidBoss,
		TrueBoss,
	};

	inline constexpr size_t EnemyKindCount = 3;

	struct Enemy
	{
		Vec2 pos;
		EnemyKind kind = EnemyKind::Normal;
		double hp = EnemyMaxHp;
		double maxHp = EnemyMaxHp;
		double speedMultiplier = 1.0;
		double attackIntervalMultiplier = 1.0;
		double attackCooldown = 0.0;
	};

	struct EnemyDefinition
	{
		EnemyKind kind = EnemyKind::Normal;
		String stableId;
		String label;
		String roleDescription;
		ColorF color = Palette::White;
		double maxHp = EnemyMaxHp;
		double speed = EnemySpeed;
		double attackRange = EnemyAttackRange;
		double attackInterval = EnemyAttackInterval;
		double attackDamage = EnemyAttackDamage;
		int32 rewardMultiplier = 1;
	};

	[[nodiscard]] inline size_t ToIndex(const EnemyKind kind)
	{
		return static_cast<size_t>(kind);
	}

	[[nodiscard]] inline StringView GetEnemyStableId(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return U"mid_boss";

		case EnemyKind::TrueBoss:
			return U"true_boss";

		case EnemyKind::Normal:
		default:
			return U"normal";
		}
	}

	[[nodiscard]] inline Optional<EnemyKind> ParseEnemyStableId(const StringView stableId)
	{
		if (stableId == U"normal")
		{
			return EnemyKind::Normal;
		}

		if (stableId == U"mid_boss")
		{
			return EnemyKind::MidBoss;
		}

		if (stableId == U"true_boss")
		{
			return EnemyKind::TrueBoss;
		}

		return none;
	}

	[[nodiscard]] inline String GetBundledEnemyDefinitionsPath()
	{
		const String runtimeRelativePath = U"enemyDefinitions.toml";
		if (FileSystem::Exists(runtimeRelativePath))
		{
			return runtimeRelativePath;
		}

		const String projectRelativePath = U"App/enemyDefinitions.toml";
		if (FileSystem::Exists(projectRelativePath))
		{
			return projectRelativePath;
		}

		return runtimeRelativePath;
	}

	[[nodiscard]] inline String GetUserEnemyDefinitionsPath()
	{
		return U"save/enemyDefinitions.toml";
	}

	[[nodiscard]] inline String GetEnemyDefinitionsPath()
	{
		const String userPath = GetUserEnemyDefinitionsPath();
		if (FileSystem::Exists(userPath))
		{
			return userPath;
		}

		return GetBundledEnemyDefinitionsPath();
	}

	[[nodiscard]] inline std::array<EnemyDefinition, EnemyKindCount> MakeDefaultEnemyDefinitions()
	{
		return {{
			{ EnemyKind::Normal, U"normal", U"Enemy", U"標準的な歩兵", ColorF{ 0.82, 0.28, 0.32 }, EnemyMaxHp, EnemySpeed, EnemyAttackRange, EnemyAttackInterval, EnemyAttackDamage, 1 },
			{ EnemyKind::MidBoss, U"mid_boss", U"Mid Boss", U"高耐久の中型ボス", ColorF{ 0.74, 0.18, 0.26 }, MidBossMaxHp, MidBossSpeed, MidBossAttackRange, MidBossAttackInterval, MidBossAttackDamage, MidBossRewardMultiplier },
			{ EnemyKind::TrueBoss, U"true_boss", U"True Boss", U"最終ウェーブの大型ボス", ColorF{ 0.44, 0.18, 0.66 }, TrueBossMaxHp, TrueBossSpeed, TrueBossAttackRange, TrueBossAttackInterval, TrueBossAttackDamage, TrueBossRewardMultiplier },
		}};
	}

	inline void NormalizeEnemyDefinition(EnemyDefinition& definition, const EnemyDefinition& fallback)
	{
		definition.kind = fallback.kind;
		definition.stableId = fallback.stableId;

		if (definition.label.isEmpty())
		{
			definition.label = fallback.label;
		}

		if (definition.roleDescription.isEmpty())
		{
			definition.roleDescription = fallback.roleDescription;
		}

		if (definition.maxHp <= 0.0)
		{
			definition.maxHp = fallback.maxHp;
		}

		if (definition.speed <= 0.0)
		{
			definition.speed = fallback.speed;
		}

		if (definition.attackRange <= 0.0)
		{
			definition.attackRange = fallback.attackRange;
		}

		if (definition.attackInterval <= 0.0)
		{
			definition.attackInterval = fallback.attackInterval;
		}

		if (definition.attackDamage <= 0.0)
		{
			definition.attackDamage = fallback.attackDamage;
		}

		if (definition.rewardMultiplier <= 0)
		{
			definition.rewardMultiplier = fallback.rewardMultiplier;
		}
	}

	[[nodiscard]] inline std::array<EnemyDefinition, EnemyKindCount> LoadEnemyDefinitions()
	{
		auto definitions = MakeDefaultEnemyDefinitions();
		const TOMLReader toml{ GetEnemyDefinitionsPath() };
		if (!toml)
		{
			return definitions;
		}

		const int32 schemaVersion = ReadTomlInt(toml, U"schemaVersion", 0);
		if ((schemaVersion <= 0) || (schemaVersion > EnemyDefinitionSchemaVersion) || (not toml[U"enemies"].isTableArray()))
		{
			return definitions;
		}

		Array<EnemyKind> loadedKinds;
		for (const auto& table : toml[U"enemies"].tableArrayView())
		{
			const auto kind = ParseEnemyStableId(ReadTomlString(table, U"id", U""));
			if (!kind || loadedKinds.contains(*kind))
			{
				continue;
			}

			const EnemyDefinition fallback = definitions[ToIndex(*kind)];
			EnemyDefinition definition = fallback;
			definition.label = ReadTomlString(table, U"label", definition.label);
			definition.roleDescription = ReadTomlString(table, U"role_description", definition.roleDescription);
			definition.color = ReadTomlColor(table, U"color", definition.color);
			definition.maxHp = ReadTomlDouble(table, U"max_hp", definition.maxHp);
			definition.speed = ReadTomlDouble(table, U"speed", definition.speed);
			definition.attackRange = ReadTomlDouble(table, U"attack_range", definition.attackRange);
			definition.attackInterval = ReadTomlDouble(table, U"attack_interval", definition.attackInterval);
			definition.attackDamage = ReadTomlDouble(table, U"attack_damage", definition.attackDamage);
			definition.rewardMultiplier = ReadTomlInt(table, U"reward_multiplier", definition.rewardMultiplier);
			NormalizeEnemyDefinition(definition, fallback);
			definitions[ToIndex(*kind)] = std::move(definition);
			loadedKinds << *kind;
		}

		return definitions;
	}

	[[nodiscard]] inline std::array<EnemyDefinition, EnemyKindCount>& GetMutableEnemyDefinitions()
	{
		static std::array<EnemyDefinition, EnemyKindCount> Definitions = LoadEnemyDefinitions();
		return Definitions;
	}

	[[nodiscard]] inline const std::array<EnemyDefinition, EnemyKindCount>& GetEnemyDefinitions()
	{
		return GetMutableEnemyDefinitions();
	}

	[[nodiscard]] inline const EnemyDefinition& GetEnemyDefinition(const EnemyKind kind)
	{
		return GetEnemyDefinitions()[ToIndex(kind)];
	}

	[[nodiscard]] inline const EnemyDefinition& GetDefaultEnemyDefinition(const EnemyKind kind)
	{
		static const std::array<EnemyDefinition, EnemyKindCount> Definitions = MakeDefaultEnemyDefinitions();
		return Definitions[ToIndex(kind)];
	}

	[[nodiscard]] inline const Array<EnemyKind>& GetAvailableEnemyKinds()
	{
		static const Array<EnemyKind> EnemyKinds = {
			EnemyKind::Normal,
			EnemyKind::MidBoss,
			EnemyKind::TrueBoss,
		};

		return EnemyKinds;
	}

	[[nodiscard]] inline String BuildEnemyDefinitionsToml(const std::array<EnemyDefinition, EnemyKindCount>& definitions)
	{
		String content;
		content += U"schemaVersion = {}\n"_fmt(EnemyDefinitionSchemaVersion);

		for (const auto& definition : definitions)
		{
			const String stableId = definition.stableId.isEmpty() ? String{ GetEnemyStableId(definition.kind) } : definition.stableId;
			content += U"\n[[enemies]]\n";
			content += U"id = \"{}\"\n"_fmt(EscapeTomlBasicString(stableId));
			content += U"label = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.label));
			content += U"role_description = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.roleDescription));
			content += U"color = {}\n"_fmt(BuildTomlColorArray(definition.color));
			content += U"max_hp = {:.3f}\n"_fmt(definition.maxHp);
			content += U"speed = {:.3f}\n"_fmt(definition.speed);
			content += U"attack_range = {:.3f}\n"_fmt(definition.attackRange);
			content += U"attack_interval = {:.3f}\n"_fmt(definition.attackInterval);
			content += U"attack_damage = {:.3f}\n"_fmt(definition.attackDamage);
			content += U"reward_multiplier = {}\n"_fmt(definition.rewardMultiplier);
		}

		return content;
	}

	[[nodiscard]] inline bool SaveEnemyDefinitionsToDisk(const std::array<EnemyDefinition, EnemyKindCount>& definitions)
	{
		const String savePath = GetUserEnemyDefinitionsPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(savePath));

		TextWriter writer{ savePath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildEnemyDefinitionsToml(definitions));
		return true;
	}

	inline void SetEnemyDefinition(const EnemyDefinition& definition)
	{
		auto normalized = definition;
		NormalizeEnemyDefinition(normalized, GetDefaultEnemyDefinition(normalized.kind));
		GetMutableEnemyDefinitions()[ToIndex(normalized.kind)] = std::move(normalized);
	}

	[[nodiscard]] inline bool SaveCurrentEnemyDefinitionsToDisk()
	{
		return SaveEnemyDefinitionsToDisk(GetEnemyDefinitions());
	}

	inline void ReloadEnemyDefinitionsFromDisk()
	{
		GetMutableEnemyDefinitions() = LoadEnemyDefinitions();
	}

	inline double GetEnemyMaxHp(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).maxHp;
	}

	inline double GetEnemySpeed(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).speed;
	}

	inline double GetEnemyAttackRange(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).attackRange;
	}

	inline double GetEnemyAttackInterval(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).attackInterval;
	}

	inline double GetEnemyAttackDamage(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).attackDamage;
	}

	inline int32 GetEnemyRewardMultiplier(const EnemyKind kind)
	{
		return GetEnemyDefinition(kind).rewardMultiplier;
	}
}
