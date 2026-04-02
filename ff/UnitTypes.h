# pragma once
# include "WaveTypes.h"

namespace ff
{
	enum class AllyBehavior : uint8
	{
		ChaseEnemies,
		HoldPosition,
		GuardPlayer,
		OrbitPlayer,
		FixedTurret,
	};

	using UnitId = AllyBehavior;
	inline constexpr size_t AllyBehaviorCount = 5;
	inline constexpr int32 WaveTraitSummonDiscountAmount = 1;
	using SummonDiscountTraitConfig = std::array<Array<WaveTrait>, AllyBehaviorCount>;

	struct UnitDefinition
	{
		UnitId id = UnitId::GuardPlayer;
		String stableId;
		String label;
		String roleDescription;
		ColorF color = Palette::White;
		int32 summonCost = 1;
		double maxHp = 1.0;
		double attackRange = 1.0;
		double attackInterval = 1.0;
		double attackDamage = 1.0;
	};

	inline constexpr size_t ToIndex(const AllyBehavior behavior)
	{
		return static_cast<size_t>(behavior);
	}

	[[nodiscard]] inline StringView GetUnitStableId(const UnitId unitId)
	{
		switch (unitId)
		{
		case UnitId::ChaseEnemies:
			return U"chase_enemies";

		case UnitId::HoldPosition:
			return U"hold_position";

		case UnitId::GuardPlayer:
			return U"guard_player";

		case UnitId::OrbitPlayer:
			return U"orbit_player";

		case UnitId::FixedTurret:
			return U"fixed_turret";
		}

		return U"guard_player";
	}

	[[nodiscard]] inline Optional<UnitId> ParseUnitId(const StringView stableId)
	{
		if (stableId == U"chase_enemies")
		{
			return UnitId::ChaseEnemies;
		}

		if (stableId == U"hold_position")
		{
			return UnitId::HoldPosition;
		}

		if (stableId == U"guard_player")
		{
			return UnitId::GuardPlayer;
		}

		if (stableId == U"orbit_player")
		{
			return UnitId::OrbitPlayer;
		}

		if (stableId == U"fixed_turret")
		{
			return UnitId::FixedTurret;
		}

		return none;
	}

	[[nodiscard]] inline bool IsWithinPlayerCommandRange(const Vec2& unitPos, const Vec2& playerPos)
	{
		return (unitPos.distanceFrom(playerPos) <= PlayerCommandRadius);
	}

	[[nodiscard]] inline String GetBundledUnitDefinitionsPath()
	{
		const String runtimeRelativePath = U"unitDefinitions.toml";
		if (FileSystem::Exists(runtimeRelativePath))
		{
			return runtimeRelativePath;
		}

		const String projectRelativePath = U"App/unitDefinitions.toml";
		if (FileSystem::Exists(projectRelativePath))
		{
			return projectRelativePath;
		}

		return runtimeRelativePath;
	}

	[[nodiscard]] inline String GetUserUnitDefinitionsPath()
	{
		return U"save/unitDefinitions.toml";
	}

	[[nodiscard]] inline String GetUnitDefinitionsPath()
	{
		const String userPath = GetUserUnitDefinitionsPath();
		if (FileSystem::Exists(userPath))
		{
			return userPath;
		}

		return GetBundledUnitDefinitionsPath();
	}

	[[nodiscard]] inline std::array<UnitDefinition, AllyBehaviorCount> MakeDefaultUnitDefinitions()
	{
		return {{
			{ UnitId::ChaseEnemies, U"chase_enemies", U"追撃", U"敵を追って前線を押す", ColorF{ 0.88, 0.42, 0.36 }, 3, AllyMaxHp, AllyAttackRange, AllyAttackInterval, AllyAttackDamage },
			{ UnitId::HoldPosition, U"hold_position", U"固定", U"その場で迎撃して足止め", ColorF{ 0.95, 0.76, 0.34 }, 2, AllyMaxHp, AllyAttackRange, AllyAttackInterval, AllyAttackDamage },
			{ UnitId::GuardPlayer, U"guard_player", U"護衛", U"主人公の近くを守る", ColorF{ 0.38, 0.78, 0.56 }, 2, AllyMaxHp, AllyAttackRange, AllyAttackInterval, AllyAttackDamage },
			{ UnitId::OrbitPlayer, U"orbit_player", U"周回", U"周囲を巡回して素早く接敵", ColorF{ 0.50, 0.62, 0.94 }, 4, AllyMaxHp, AllyAttackRange, AllyAttackInterval, AllyAttackDamage },
			{ UnitId::FixedTurret, U"fixed_turret", U"砲台", U"動かず遠距離火力を出す", ColorF{ 0.72, 0.54, 0.22 }, 7, FixedTurretMaxHp, FixedTurretAttackRange, FixedTurretAttackInterval, FixedTurretAttackDamage },
		}};
	}

	inline void NormalizeUnitDefinition(UnitDefinition& definition, const UnitDefinition& fallback)
	{
		definition.id = fallback.id;
		definition.stableId = fallback.stableId;

		if (definition.label.isEmpty())
		{
			definition.label = fallback.label;
		}

		if (definition.roleDescription.isEmpty())
		{
			definition.roleDescription = fallback.roleDescription;
		}

		if (definition.summonCost <= 0)
		{
			definition.summonCost = fallback.summonCost;
		}

		if (definition.maxHp <= 0.0)
		{
			definition.maxHp = fallback.maxHp;
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
	}

	[[nodiscard]] inline std::array<UnitDefinition, AllyBehaviorCount> LoadUnitDefinitions()
	{
		auto definitions = MakeDefaultUnitDefinitions();
		const TOMLReader toml{ GetUnitDefinitionsPath() };
		if (!toml)
		{
			return definitions;
		}

		const int32 schemaVersion = ReadTomlInt(toml, U"schemaVersion", 0);
		if ((schemaVersion <= 0) || (schemaVersion > UnitDefinitionSchemaVersion) || (not toml[U"units"].isTableArray()))
		{
			return definitions;
		}

		Array<UnitId> loadedUnitIds;
		for (const auto& table : toml[U"units"].tableArrayView())
		{
			const auto unitId = ParseUnitId(ReadTomlString(table, U"id", U""));
			if (!unitId || loadedUnitIds.contains(*unitId))
			{
				continue;
			}

			const UnitDefinition fallback = definitions[ToIndex(*unitId)];
			UnitDefinition definition = fallback;
			definition.label = ReadTomlString(table, U"label", definition.label);
			definition.roleDescription = ReadTomlString(table, U"role_description", definition.roleDescription);
			definition.color = ReadTomlColor(table, U"color", definition.color);
			definition.summonCost = ReadTomlInt(table, U"summon_cost", definition.summonCost);
			definition.maxHp = ReadTomlDouble(table, U"max_hp", definition.maxHp);
			definition.attackRange = ReadTomlDouble(table, U"attack_range", definition.attackRange);
			definition.attackInterval = ReadTomlDouble(table, U"attack_interval", definition.attackInterval);
			definition.attackDamage = ReadTomlDouble(table, U"attack_damage", definition.attackDamage);
			NormalizeUnitDefinition(definition, fallback);
			definitions[ToIndex(*unitId)] = std::move(definition);
			loadedUnitIds << *unitId;
		}

		return definitions;
	}

	[[nodiscard]] inline std::array<UnitDefinition, AllyBehaviorCount>& GetMutableUnitDefinitions()
	{
		static std::array<UnitDefinition, AllyBehaviorCount> Definitions = LoadUnitDefinitions();
		return Definitions;
	}

	[[nodiscard]] inline const std::array<UnitDefinition, AllyBehaviorCount>& GetUnitDefinitions()
	{
		return GetMutableUnitDefinitions();
	}

	[[nodiscard]] inline const UnitDefinition& GetUnitDefinition(const UnitId unitId)
	{
		return GetUnitDefinitions()[ToIndex(unitId)];
	}

	[[nodiscard]] inline const UnitDefinition& GetDefaultUnitDefinition(const UnitId unitId)
	{
		static const std::array<UnitDefinition, AllyBehaviorCount> Definitions = MakeDefaultUnitDefinitions();
		return Definitions[ToIndex(unitId)];
	}

	[[nodiscard]] inline const Array<UnitId>& GetAvailableUnitIds()
	{
		static const Array<UnitId> UnitIds = {
			UnitId::ChaseEnemies,
			UnitId::HoldPosition,
			UnitId::GuardPlayer,
			UnitId::OrbitPlayer,
			UnitId::FixedTurret,
		};

		return UnitIds;
	}

	[[nodiscard]] inline String BuildUnitDefinitionsToml(const std::array<UnitDefinition, AllyBehaviorCount>& definitions)
	{
		String content;
		content += U"schemaVersion = {}\n"_fmt(UnitDefinitionSchemaVersion);

		for (const auto& definition : definitions)
		{
			const String stableId = definition.stableId.isEmpty() ? String{ GetUnitStableId(definition.id) } : definition.stableId;
			content += U"\n[[units]]\n";
			content += U"id = \"{}\"\n"_fmt(EscapeTomlBasicString(stableId));
			content += U"label = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.label));
			content += U"role_description = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.roleDescription));
			content += U"color = {}\n"_fmt(BuildTomlColorArray(definition.color));
			content += U"summon_cost = {}\n"_fmt(definition.summonCost);
			content += U"max_hp = {:.3f}\n"_fmt(definition.maxHp);
			content += U"attack_range = {:.3f}\n"_fmt(definition.attackRange);
			content += U"attack_interval = {:.3f}\n"_fmt(definition.attackInterval);
			content += U"attack_damage = {:.3f}\n"_fmt(definition.attackDamage);
		}

		return content;
	}

	[[nodiscard]] inline bool SaveUnitDefinitionsToDisk(const std::array<UnitDefinition, AllyBehaviorCount>& definitions)
	{
		const String savePath = GetUserUnitDefinitionsPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(savePath));

		TextWriter writer{ savePath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildUnitDefinitionsToml(definitions));
		return true;
	}

	inline void ReloadUnitDefinitionsFromDisk()
	{
		GetMutableUnitDefinitions() = LoadUnitDefinitions();
	}

	inline void SetUnitDefinition(UnitDefinition definition)
	{
		const UnitDefinition fallback = GetDefaultUnitDefinition(definition.id);
		NormalizeUnitDefinition(definition, fallback);
		GetMutableUnitDefinitions()[ToIndex(definition.id)] = std::move(definition);
	}

	[[nodiscard]] inline bool SaveCurrentUnitDefinitionsToDisk()
	{
		return SaveUnitDefinitionsToDisk(GetUnitDefinitions());
	}

	inline SummonDiscountTraitConfig MakeDefaultSummonDiscountTraitConfig()
	{
		SummonDiscountTraitConfig config;
		config[ToIndex(AllyBehavior::ChaseEnemies)] = { WaveTrait::Reinforced };
		config[ToIndex(AllyBehavior::HoldPosition)] = { WaveTrait::Bounty };
		config[ToIndex(AllyBehavior::GuardPlayer)] = { WaveTrait::Assault };
		config[ToIndex(AllyBehavior::OrbitPlayer)] = { WaveTrait::Assault, WaveTrait::Bounty };
		config[ToIndex(AllyBehavior::FixedTurret)] = { WaveTrait::Reinforced };
		return config;
	}

	inline bool HasWaveTraitSummonDiscount(const SummonDiscountTraitConfig& config, const AllyBehavior behavior, const WaveTrait activeWaveTrait)
	{
		if (activeWaveTrait == WaveTrait::None)
		{
			return false;
		}

		const auto& discountedTraits = config[ToIndex(behavior)];
		return discountedTraits.contains(activeWaveTrait);
	}

	inline double GetAllyMaxHp(const AllyBehavior behavior)
	{
		return GetUnitDefinition(behavior).maxHp;
	}

	inline double GetAllyAttackRange(const AllyBehavior behavior)
	{
		return GetUnitDefinition(behavior).attackRange;
	}

	inline double GetAllyAttackInterval(const AllyBehavior behavior)
	{
		return GetUnitDefinition(behavior).attackInterval;
	}

	inline double GetAllyAttackDamage(const AllyBehavior behavior)
	{
		return GetUnitDefinition(behavior).attackDamage;
	}

	inline int32 GetSummonCost(const AllyBehavior behavior)
	{
		return GetUnitDefinition(behavior).summonCost;
	}

	inline int32 GetSummonCost(const AllyBehavior behavior, const WaveTrait activeWaveTrait, const SummonDiscountTraitConfig& config)
	{
		const int32 baseCost = GetSummonCost(behavior);

		if (HasWaveTraitSummonDiscount(config, behavior, activeWaveTrait))
		{
			return Max(1, (baseCost - WaveTraitSummonDiscountAmount));
		}

		return baseCost;
	}

	struct Ally
	{
		Vec2 pos;
		AllyBehavior behavior = AllyBehavior::GuardPlayer;
		double orbitAngle = 0.0;
		double hp = GetAllyMaxHp(AllyBehavior::GuardPlayer);
		double attackCooldown = 0.0;
	};
}
