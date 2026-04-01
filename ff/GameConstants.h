# pragma once
# include <array>
# include <Siv3D.hpp>

namespace ff
{
	inline constexpr Size MapSize{ 24, 24 };
	inline constexpr Vec2 TileHalfSize{ 48, 24 };
	inline constexpr int32 TileWidth = 96;
	inline constexpr int32 TileHeight = 48;
	inline constexpr int32 TileThickness = 17;
	inline constexpr double PlayerSpeed = 3.8;
	inline constexpr double PlayerFootOffsetY = 0.0;
  inline constexpr double PlayerMaxHp = 18.0;
  inline constexpr double PlayerHitInvincibilityDuration = 0.55;
  inline constexpr int32 InitialResources = 8;
	inline constexpr int32 ResourcePerEnemyKill = 2;
 inline constexpr int32 BonusTileResourcePerEnemyKill = 4;
	inline constexpr int32 PenaltyTileResourcePerEnemyKill = 1;
 inline constexpr double SpecialTileVisibleDuration = 7.0;
	inline constexpr double SpecialTileHiddenDuration = 5.0;
 inline constexpr double WaveStartDelay = 1.8;
	inline constexpr double WaveClearDelay = 4.0;
  inline constexpr double StageClearReturnDelay = 3.2;
	inline constexpr int32 BaseEnemiesPerWave = 5;
	inline constexpr int32 AdditionalEnemiesPerWave = 2;
	inline constexpr double EnemySpawnIntervalStepPerWave = 0.12;
	inline constexpr double MinEnemySpawnInterval = 0.55;
	inline constexpr double WaveBannerDuration = 1.6;
	inline constexpr double WaterBoundaryHalfWidth = 10.0;
	inline constexpr double EnemySpeed = 2.1;
	inline constexpr double EnemySpawnInterval = 1.8;
	inline constexpr double EnemyStopDistance = 0.35;
 inline constexpr double EnemyMaxHp = 4.0;
	inline constexpr double EnemyAttackRange = 0.9;
	inline constexpr double EnemyAttackInterval = 0.95;
	inline constexpr double EnemyAttackDamage = 1.0;
 inline constexpr size_t MaxEnemyCount = 24;
    inline constexpr int32 MidBossWaveInterval = 5;
	inline constexpr int32 FinalBossWave = 20;
	inline constexpr int32 MidBossBaseSupportEnemyCount = 3;
	inline constexpr double MidBossMaxHp = 24.0;
	inline constexpr double MidBossSpeed = 1.7;
	inline constexpr double MidBossAttackRange = 1.0;
	inline constexpr double MidBossAttackInterval = 0.88;
	inline constexpr double MidBossAttackDamage = 2.0;
	inline constexpr int32 MidBossRewardMultiplier = 6;
	inline constexpr double TrueBossMaxHp = 72.0;
	inline constexpr double TrueBossSpeed = 1.45;
	inline constexpr double TrueBossAttackRange = 1.2;
	inline constexpr double TrueBossAttackInterval = 0.72;
	inline constexpr double TrueBossAttackDamage = 3.0;
	inline constexpr int32 TrueBossRewardMultiplier = 20;
	inline constexpr double AllySpeed = 2.8;
	inline constexpr double AllyStopDistance = 0.2;
	inline constexpr double AllyOrbitRadius = 1.4;
   inline constexpr double AllyMaxHp = 6.0;
	inline constexpr double AllyAttackRange = 1.15;
	inline constexpr double AllyAttackInterval = 0.55;
	inline constexpr double AllyAttackDamage = 1.0;
    inline constexpr double PlayerCommandRadius = 2.35;
	inline constexpr double PlayerCommandAttackIntervalMultiplier = 0.82;
	inline constexpr double PlayerCommandDamageMultiplier = 1.25;
	inline constexpr double FixedTurretMaxHp = 10.0;
	inline constexpr double FixedTurretAttackRange = 3.2;
	inline constexpr double FixedTurretAttackInterval = 1.45;
	inline constexpr double FixedTurretAttackDamage = 3.0;

	enum class TerrainTile : uint8
	{
		Grass,
		Path,
		Sand,
		Water,
	};

	using TerrainGrid = Grid<TerrainTile>;

	enum class SpecialTileKind : uint8
	{
		None,
		Bonus,
		Penalty,
	};

	enum class WaveTrait : uint8
	{
		None,
		Reinforced,
		Assault,
		Bounty,
	};

	using SpecialTileGrid = Grid<SpecialTileKind>;

	inline constexpr size_t ToTextureIndex(const TerrainTile tile)
	{
		return static_cast<size_t>(tile);
	}

	inline constexpr bool IsWaterTile(const TerrainTile tile)
	{
		return (tile == TerrainTile::Water);
	}

	inline constexpr bool IsLandTile(const TerrainTile tile)
	{
		return (not IsWaterTile(tile));
	}

	inline constexpr int32 GetResourceRewardPerEnemyKill(const SpecialTileKind tileKind)
	{
		switch (tileKind)
		{
		case SpecialTileKind::Bonus:
			return BonusTileResourcePerEnemyKill;

		case SpecialTileKind::Penalty:
			return PenaltyTileResourcePerEnemyKill;

		case SpecialTileKind::None:
		default:
			return ResourcePerEnemyKill;
		}
	}

	inline bool IsWithinPlayerCommandRange(const Vec2& unitPos, const Vec2& playerPos)
	{
		return (unitPos.distanceFrom(playerPos) <= PlayerCommandRadius);
	}

	inline StringView GetWaveTraitLabel(const WaveTrait trait)
	{
		switch (trait)
		{
		case WaveTrait::Reinforced:
			return U"装甲";

		case WaveTrait::Assault:
			return U"急襲";

		case WaveTrait::Bounty:
			return U"収穫";

		case WaveTrait::None:
		default:
			return U"";
		}
	}

	inline StringView GetWaveTraitDescription(const WaveTrait trait)
	{
		switch (trait)
		{
		case WaveTrait::Reinforced:
			return U"敵HP増加";

		case WaveTrait::Assault:
			return U"敵加速・攻撃短縮";

		case WaveTrait::Bounty:
			return U"撃破資源 +1";

		case WaveTrait::None:
		default:
			return U"";
		}
	}

	enum class EnemyKind : uint8
	{
		Normal,
		MidBoss,
		TrueBoss,
	};

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

	inline constexpr double GetEnemyMaxHp(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossMaxHp;

		case EnemyKind::TrueBoss:
			return TrueBossMaxHp;

		case EnemyKind::Normal:
		default:
			return EnemyMaxHp;
		}
	}

	inline constexpr double GetEnemySpeed(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossSpeed;

		case EnemyKind::TrueBoss:
			return TrueBossSpeed;

		case EnemyKind::Normal:
		default:
			return EnemySpeed;
		}
	}

	inline constexpr double GetEnemyAttackRange(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackRange;

		case EnemyKind::TrueBoss:
			return TrueBossAttackRange;

		case EnemyKind::Normal:
		default:
			return EnemyAttackRange;
		}
	}

	inline constexpr double GetEnemyAttackInterval(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackInterval;

		case EnemyKind::TrueBoss:
			return TrueBossAttackInterval;

		case EnemyKind::Normal:
		default:
			return EnemyAttackInterval;
		}
	}

	inline constexpr double GetEnemyAttackDamage(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackDamage;

		case EnemyKind::TrueBoss:
			return TrueBossAttackDamage;

		case EnemyKind::Normal:
		default:
			return EnemyAttackDamage;
		}
	}

	inline constexpr int32 GetEnemyRewardMultiplier(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossRewardMultiplier;

		case EnemyKind::TrueBoss:
			return TrueBossRewardMultiplier;

		case EnemyKind::Normal:
		default:
			return 1;
		}
	}

	enum class WaveType : uint8
	{
		Standard,
		MidBoss,
		TrueBoss,
	};

	struct WaveDefinition
	{
		int32 waveNumber = 1;
		WaveType type = WaveType::Standard;
		String label;
		String description;
		ColorF accentColor = Palette::White;
		WaveTrait trait = WaveTrait::None;
		double enemyHpMultiplier = 1.0;
		double enemySpeedMultiplier = 1.0;
		double enemyAttackIntervalMultiplier = 1.0;
		int32 rewardBonusPerKill = 0;
		double spawnInterval = EnemySpawnInterval;
		Array<EnemyKind> spawnQueue;
	};

	struct WaveConfig
	{
		double waveStartDelay = WaveStartDelay;
		double waveClearDelay = WaveClearDelay;
		double waveBannerDuration = WaveBannerDuration;
		Array<WaveDefinition> waves;
	};

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
  inline constexpr int32 WaveDefinitionSchemaVersion = 1;
   inline constexpr int32 UnitDefinitionSchemaVersion = 1;
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

	struct WaterEdgeMask
	{
		bool upperRight = false;
		bool lowerRight = false;
		bool lowerLeft = false;
		bool upperLeft = false;
		bool topCornerOnly = false;
		bool bottomCornerOnly = false;
		bool rightCornerOnly = false;
		bool leftCornerOnly = false;
	};
}
