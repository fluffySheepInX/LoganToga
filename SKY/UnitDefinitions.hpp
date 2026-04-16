# pragma once
# include "UnitTypes.hpp"

namespace MainSupport
{
	[[nodiscard]] inline const Array<UnitDefinition>& GetUnitDefinitions()
	{
		static const Array<UnitDefinition> Definitions{
			UnitDefinition{
				.unitType = SapperUnitType::Infantry,
               .uniqueSkillType = UniqueSkillType::BuildMill,
				.settingsKeySuffix = U"Infantry",
				.displayName = U"兵",
				.playerEditorSectionLabel = U"Player Infantry",
				.enemyEditorSectionLabel = U"Enemy Infantry",
				.playerRenderModel = UnitRenderModel::Bird,
				.enemyRenderModel = UnitRenderModel::Ashigaru,
               .uniqueSkillLabel = U"建築: Mill",
				.uniqueSkillDeniedMessage = U"建築スキルは歩兵専用",
				.canUseExplosionSkill = true,
				.enemyProductionPriority = 10,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 100.0,
					.moveSpeed = 6.0,
					.attackRange = 3.2,
					.stopDistance = 0.15,
					.attackDamage = 12.0,
					.attackInterval = 0.8,
                 .visionRange = 8.0,
					.manaCost = SapperCost,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.aiRole = UnitAiRole::SecureResources,
					.maxHitPoints = 120.0,
					.moveSpeed = 6.0,
					.attackRange = 3.0,
					.stopDistance = 0.15,
					.attackDamage = 10.0,
					.attackInterval = 0.95,
                 .visionRange = 8.0,
					.manaCost = SapperCost,
				},
			},
			UnitDefinition{
				.unitType = SapperUnitType::ArcaneInfantry,
             .uniqueSkillType = UniqueSkillType::Heal,
				.settingsKeySuffix = U"ArcaneInfantry",
				.displayName = U"魔導兵(仮)",
				.playerEditorSectionLabel = U"Player Arcane",
				.enemyEditorSectionLabel = U"Enemy Arcane",
				.playerRenderModel = UnitRenderModel::Bird,
				.enemyRenderModel = UnitRenderModel::Ashigaru,
				.tintMultiplier = Vec3{ 0.62, 0.82, 1.30 },
				.tintOffset = Vec3{ 0.18, 0.10, 0.18 },
              .uniqueSkillLabel = U"回復",
				.uniqueSkillDeniedMessage = U"回復スキルは魔導兵専用",
				.enemyProductionPriority = 20,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 150.0,
					.moveSpeed = 5.4,
					.attackRange = 3.8,
					.stopDistance = 0.35,
					.attackDamage = 20.0,
					.attackInterval = 1.05,
                 .visionRange = 9.0,
					.manaCost = ArcaneInfantryCost,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.aiRole = UnitAiRole::AssaultBase,
					.maxHitPoints = 180.0,
					.moveSpeed = 5.4,
					.attackRange = 3.8,
					.stopDistance = 0.35,
					.attackDamage = 19.0,
					.attackInterval = 1.05,
                 .visionRange = 9.0,
					.manaCost = ArcaneInfantryCost,
				},
			},
			UnitDefinition{
				.unitType = SapperUnitType::SugoiCar,
               .uniqueSkillType = UniqueSkillType::Scout,
				.settingsKeySuffix = U"SugoiCar",
				.displayName = U"すごい車(仮)",
				.playerEditorSectionLabel = U"Player SugoiCar",
				.enemyEditorSectionLabel = U"Enemy SugoiCar",
				.playerRenderModel = UnitRenderModel::SugoiCar,
				.enemyRenderModel = UnitRenderModel::SugoiCar,
				.tintMultiplier = Vec3{ 1.15, 1.00, 0.62 },
				.tintOffset = Vec3{ 0.12, 0.08, 0.02 },
              .uniqueSkillLabel = U"偵察",
				.uniqueSkillDeniedMessage = U"偵察スキルは車専用",
				.enemyProductionPriority = 30,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Tank,
					.maxHitPoints = 260.0,
					.moveSpeed = 4.8,
					.attackRange = 4.6,
					.stopDistance = 0.5,
					.attackDamage = 28.0,
					.attackInterval = 1.15,
                   .visionRange = 10.0,
					.manaCost = SugoiCarCost,
					.footprintType = UnitFootprintType::Capsule,
					.footprintRadius = 0.95,
					.footprintHalfLength = 2.05,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Tank,
					.aiRole = UnitAiRole::AssaultBase,
					.maxHitPoints = 300.0,
					.moveSpeed = 4.8,
					.attackRange = 4.6,
					.stopDistance = 0.5,
					.attackDamage = 24.0,
					.attackInterval = 1.15,
                   .visionRange = 10.0,
					.manaCost = SugoiCarCost,
					.footprintType = UnitFootprintType::Capsule,
					.footprintRadius = 0.95,
					.footprintHalfLength = 2.05,
				},
			},
		};

		return Definitions;
	}

	[[noreturn]] inline void ThrowInvalidUnitType(const SapperUnitType unitType, const StringView context)
	{
		throw Error{ U"Invalid SapperUnitType in {}: {}"_fmt(context, static_cast<int32>(unitType)) };
	}

	[[nodiscard]] inline const UnitDefinition& GetUnitDefinition(const SapperUnitType unitType)
	{
		for (const auto& definition : GetUnitDefinitions())
		{
			if (definition.unitType == unitType)
			{
				return definition;
			}
		}

        ThrowInvalidUnitType(unitType, U"GetUnitDefinition");
	}

	[[nodiscard]] inline const UnitParameters& GetDefaultUnitParameters(const UnitTeam team, const SapperUnitType unitType)
	{
		const UnitDefinition& definition = GetUnitDefinition(unitType);
		return ((team == UnitTeam::Enemy) ? definition.enemyDefaults : definition.playerDefaults);
	}

	[[nodiscard]] inline StringView GetUnitDisplayName(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).displayName;
	}

	[[nodiscard]] inline UniqueSkillType GetUnitUniqueSkillType(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).uniqueSkillType;
	}

	[[nodiscard]] inline StringView GetUnitUniqueSkillLabel(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).uniqueSkillLabel;
	}

	[[nodiscard]] inline StringView GetUnitUniqueSkillDeniedMessage(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).uniqueSkillDeniedMessage;
	}

	[[nodiscard]] inline StringView GetUnitEditorSectionLabel(const UnitTeam team, const SapperUnitType unitType)
	{
		const UnitDefinition& definition = GetUnitDefinition(unitType);
		return ((team == UnitTeam::Enemy) ? definition.enemyEditorSectionLabel : definition.playerEditorSectionLabel);
	}

	[[nodiscard]] inline UnitRenderModel GetUnitRenderModel(const UnitTeam team, const SapperUnitType unitType)
	{
		const UnitDefinition& definition = GetUnitDefinition(unitType);
		return ((team == UnitTeam::Enemy) ? definition.enemyRenderModel : definition.playerRenderModel);
	}

	[[nodiscard]] inline ColorF ApplyUnitTint(const SapperUnitType unitType, const ColorF& baseColor)
	{
		const UnitDefinition& definition = GetUnitDefinition(unitType);
		return ColorF{
			Clamp(baseColor.r * definition.tintMultiplier.x + definition.tintOffset.x, 0.0, 1.0),
			Clamp(baseColor.g * definition.tintMultiplier.y + definition.tintOffset.y, 0.0, 1.0),
			Clamp(baseColor.b * definition.tintMultiplier.z + definition.tintOffset.z, 0.0, 1.0),
			baseColor.a,
		};
	}

	[[nodiscard]] inline bool CanUnitUseExplosionSkill(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).canUseExplosionSkill;
	}

	[[nodiscard]] inline StringView GetExplosionSkillUnavailableLabel(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).explosionSkillUnavailableLabel;
	}

	[[nodiscard]] inline StringView GetExplosionSkillDeniedMessage(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).explosionSkillDeniedMessage;
	}

	[[nodiscard]] inline int32 GetEnemyProductionPriority(const SapperUnitType unitType)
	{
		return GetUnitDefinition(unitType).enemyProductionPriority;
	}

	[[nodiscard]] inline UnitAiRole GetUnitAiRole(const SapperUnitType unitType)
	{
		return GetDefaultUnitParameters(UnitTeam::Enemy, unitType).aiRole;
	}

	[[nodiscard]] inline String GetUnitSettingsGroupKey(const UnitTeam team, const SapperUnitType unitType)
	{
		return (((team == UnitTeam::Enemy) ? U"enemy" : U"player") + String{ GetUnitDefinition(unitType).settingsKeySuffix });
	}

	[[nodiscard]] inline size_t GetUnitTeamIndex(const UnitTeam team)
	{
		return ((team == UnitTeam::Enemy) ? 1 : 0);
	}

	[[nodiscard]] inline size_t GetUnitDefinitionIndex(const SapperUnitType unitType)
	{
		const auto& definitions = GetUnitDefinitions();

		for (size_t index = 0; index < definitions.size(); ++index)
		{
			if (definitions[index].unitType == unitType)
			{
				return index;
			}
		}

       ThrowInvalidUnitType(unitType, U"GetUnitDefinitionIndex");
	}

	[[nodiscard]] inline size_t GetUnitParameterSlotIndex(const UnitTeam team, const SapperUnitType unitType)
	{
		return (GetUnitTeamIndex(team) * GetUnitDefinitions().size() + GetUnitDefinitionIndex(unitType));
	}

	[[nodiscard]] inline UnitParameters MakeDefaultUnitParameters(const UnitTeam team, const SapperUnitType unitType)
	{
		return GetDefaultUnitParameters(team, unitType);
	}

	[[nodiscard]] inline const ExplosionSkillParameters& GetDefaultExplosionSkillParameters(const UnitTeam team, const SapperUnitType unitType)
	{
		const UnitDefinition& definition = GetUnitDefinition(unitType);
		return ((team == UnitTeam::Enemy) ? definition.enemyExplosionDefaults : definition.playerExplosionDefaults);
	}

	[[nodiscard]] inline ExplosionSkillParameters MakeDefaultExplosionSkillParameters(const UnitTeam team, const SapperUnitType unitType)
	{
		return GetDefaultExplosionSkillParameters(team, unitType);
	}
}
