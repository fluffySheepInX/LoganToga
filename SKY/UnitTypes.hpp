# pragma once
# include <array>
# include "MainConstants.hpp"

namespace MainSupport
{
	enum class UnitTeam
	{
		Player,
		Enemy,
	};

	enum class SapperUnitType
	{
		Infantry,
		ArcaneInfantry,
		SugoiCar,
	};

	enum class MovementType
	{
		Infantry,
		Tank,
	};

	enum class UniqueSkillType
	{
		BuildMill,
		Heal,
		Scout,
	};

	enum class UnitFootprintType
	{
		Circle,
		Capsule,
	};

	enum class ResourceType
	{
		Budget,
		Gunpowder,
		Mana,
	};

	enum class UnitAiRole
	{
		SecureResources,
		AssaultBase,
		Support,
	};

	struct ResourceStock
	{
		double budget = 0.0;
		double gunpowder = 0.0;
		double mana = 0.0;
	};

	struct ResourceAreaState
	{
		Optional<UnitTeam> ownerTeam;
		Optional<UnitTeam> capturingTeam;
		double captureProgress = 0.0;
		double incomeProgress = 0.0;
	};

	struct UnitParameters
	{
		MovementType movementType = MovementType::Infantry;
		UnitAiRole aiRole = UnitAiRole::SecureResources;
		double maxHitPoints = 100.0;
		double moveSpeed = 6.0;
		double attackRange = 3.2;
		double stopDistance = 0.2;
		double attackDamage = 12.0;
		double attackInterval = 0.8;
       double visionRange = 8.0;
		double manaCost = SapperCost;
		UnitFootprintType footprintType = UnitFootprintType::Circle;
		double footprintRadius = 1.2;
		double footprintHalfLength = 0.0;
	};

	struct ExplosionSkillParameters
	{
		double radius = SapperExplosionRadius;
		double unitDamage = SapperExplosionDamage;
		double baseDamage = SapperExplosionBaseDamage;
		double cooldownSeconds = SapperExplosionCooldownSeconds;
		double gunpowderCost = SapperExplosionGunpowderCost;
		double effectLifetime = 0.42;
		double effectThickness = 7.0;
		double effectOffsetY = 1.1;
		ColorF effectColor{ 1.0, 0.62, 0.24, 0.98 };
	};

	enum class UnitRenderModel
	{
		Bird,
		Ashigaru,
		SugoiCar,
		Count,
	};

	inline constexpr size_t UnitRenderModelCount = static_cast<size_t>(UnitRenderModel::Count);

	[[nodiscard]] inline constexpr size_t GetUnitRenderModelIndex(const UnitRenderModel renderModel)
	{
		return static_cast<size_t>(renderModel);
	}

	[[nodiscard]] inline constexpr const std::array<UnitRenderModel, UnitRenderModelCount>& GetUnitRenderModels()
	{
		static constexpr std::array<UnitRenderModel, UnitRenderModelCount> RenderModels{
			UnitRenderModel::Bird,
			UnitRenderModel::Ashigaru,
			UnitRenderModel::SugoiCar,
		};

		return RenderModels;
	}

	struct UnitDefinition
	{
		SapperUnitType unitType = SapperUnitType::Infantry;
       UniqueSkillType uniqueSkillType = UniqueSkillType::BuildMill;
		StringView settingsKeySuffix;
		StringView displayName;
		StringView playerEditorSectionLabel;
		StringView enemyEditorSectionLabel;
		UnitRenderModel playerRenderModel = UnitRenderModel::Bird;
		UnitRenderModel enemyRenderModel = UnitRenderModel::Ashigaru;
		Vec3 tintMultiplier{ 1.0, 1.0, 1.0 };
		Vec3 tintOffset{ 0.0, 0.0, 0.0 };
      StringView uniqueSkillLabel = U"固有スキル";
		StringView uniqueSkillDeniedMessage = U"この固有スキルは使用できない";
		bool canUseExplosionSkill = false;
		StringView explosionSkillUnavailableLabel = U"爆破スキル [兵専用]";
		StringView explosionSkillDeniedMessage = U"爆破スキルは兵専用";
		int32 enemyProductionPriority = 0;
		UnitParameters playerDefaults;
		UnitParameters enemyDefaults;
      ExplosionSkillParameters playerExplosionDefaults;
		ExplosionSkillParameters enemyExplosionDefaults;
	};
}
