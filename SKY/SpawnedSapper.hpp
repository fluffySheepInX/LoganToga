# pragma once
# include "UnitTypes.hpp"

namespace MainSupport
{
	struct SpawnedSapper
	{
		Vec3 startPosition;
		Vec3 position;
		Vec3 targetPosition;
		Vec3 destinationPosition;
		Vec3 retreatReturnPosition{ 0, 0, 0 };
		double spawnedAt = 0.0;
		double moveStartedAt = 0.0;
		double moveDuration = 0.0;
		double facingYaw = BirdDisplayYaw;
		UnitTeam team = UnitTeam::Player;
		SapperUnitType unitType = SapperUnitType::Infantry;
     UnitAiRole aiRole = UnitAiRole::SecureResources;
		MovementType movementType = MovementType::Infantry;
		int32 tier = 1;
		double maxHitPoints = 100.0;
		double hitPoints = 100.0;
		double moveSpeed = 6.0;
		double attackRange = 3.2;
		double stopDistance = 0.2;
		double baseAttackDamage = 12.0;
		double baseAttackInterval = 0.8;
        double visionRange = 8.0;
		UnitFootprintType footprintType = UnitFootprintType::Circle;
		double footprintRadius = 1.2;
		double footprintHalfLength = 0.0;
		double suppressedUntil = -1000.0;
		double suppressedMoveSpeedMultiplier = 1.0;
		double suppressedAttackDamageMultiplier = 1.0;
		double suppressedAttackIntervalMultiplier = 1.0;
		double lastAttackAt = -1000.0;
		double explosionSkillCooldownUntil = -1000.0;
        double scoutingSkillUntil = -1000.0;
        double scoutingSkillVisionMultiplier = SapperScoutingSkillVisionMultiplier;
		double retreatDisappearAt = -1000.0;
		double retreatReturnAt = -1000.0;
      bool moveOrderActive = false;
	};
}
