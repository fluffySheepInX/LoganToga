# pragma once
# include <Siv3D.hpp>

namespace MainSupport
{
    using AppCamera3D = BasicCamera3D;

	inline constexpr FilePathView CameraSettingsPath = U"settings/camera_settings.toml";
	inline constexpr FilePathView MapDataPath = U"settings/map_data.toml";
	inline constexpr FilePathView ModelHeightSettingsPath = U"settings/model_height_settings.toml";
    inline constexpr FilePathView UnitSettingsPath = U"settings/unit_settings.toml";
    inline constexpr FilePathView UiLayoutSettingsPath = U"settings/ui_layout_settings.toml";
	inline constexpr FilePathView BirdModelPath = U"model/bird.glb";
	inline constexpr FilePathView AshigaruModelPath = U"model/ashigaru_v2.1.glb";
    inline constexpr Vec3 DefaultCameraEye{ 0, 8, -16 };
	inline constexpr Vec3 DefaultCameraFocus{ 0, 0, 0 };
	inline constexpr Vec3 BirdDisplayPosition{ -10.5, 0, -2.5 };
	inline constexpr Vec3 AshigaruDisplayPosition{ -5.5, 0, -2.5 };
	inline constexpr double BirdDisplayYaw = 0_deg;
    inline constexpr double SapperFacingYawOffset = 180_deg;
	inline constexpr Vec3 BlacksmithPosition{ 8, 0, 4 };
 inline constexpr Vec3 EnemyBasePosition{ -2.0, 0, 13.0 };
	inline constexpr Sphere BlacksmithInteractionSphere{ BlacksmithPosition + Vec3{ 0, 4.0, 0 }, 4.5 };
	inline constexpr Vec3 BlacksmithSelectionBoxSize{ 8.0, 8.0, 8.0 };
	inline constexpr Vec3 BlacksmithSelectionBoxPadding{ 1.2, 0.8, 1.2 };
    inline constexpr double BaseMaxHitPoints = 260.0;
	inline constexpr double BaseCombatRadius = 4.2;
 inline constexpr double StartingResources = 120.0;
 inline constexpr double ManaIncomePerSecond = 18.0;
	inline constexpr double ResourceIncomePerSecond = 18.0;
	inline constexpr double SapperCost = 60.0;
   inline constexpr double ArcaneInfantryCost = 90.0;
 inline constexpr double TierUpgradeBaseCost = 100.0;
   inline constexpr double ResourceAreaDefaultRadius = 5.0;
	inline constexpr double ResourceAreaCaptureSeconds = 2.5;
	inline constexpr double ResourceAreaIncomeIntervalSeconds = 3.0;
	inline constexpr double BudgetAreaIncome = 45.0;
	inline constexpr double GunpowderAreaIncome = 25.0;
	inline constexpr double ManaAreaIncome = 20.0;
   inline constexpr double SapperExplosionGunpowderCost = 20.0;
	inline constexpr double SapperExplosionRadius = 3.6;
	inline constexpr double SapperExplosionDamage = 48.0;
	inline constexpr double SapperExplosionBaseDamage = 36.0;
	inline constexpr double SapperExplosionCooldownSeconds = 3.0;
	inline constexpr double EnemyReinforcementInterval = 6.0;
 inline constexpr double EnemyAiDecisionInterval = 1.0;
	inline constexpr double EnemyStrongUnitProductionCooldown = 3.0;
	inline constexpr double EnemyAdvanceStopDistance = 4.8;
    inline constexpr double MillDefenseRange = 6.5;
	inline constexpr double MillDefenseDamage = 18.0;
	inline constexpr double MillDefenseInterval = 1.2;
  inline constexpr int32 MillDefenseTargetCount = 2;
    inline constexpr double MillSuppressionDuration = 2.4;
	inline constexpr double MillSuppressionMoveSpeedMultiplier = 0.45;
	inline constexpr double MillSuppressionAttackDamageMultiplier = 0.35;
	inline constexpr double MillSuppressionAttackIntervalMultiplier = 2.4;
    inline constexpr double MinimumCameraEyeFocusDistance = 0.5;
	inline constexpr double CameraZoomMinDistance = 3.0;
	inline constexpr double CameraZoomMaxDistance = 80.0;
	inline constexpr double CameraZoomFactorPerWheelStep = 0.85;
	inline constexpr double BirdDisplayHeight = 3.6;
	inline constexpr double ModelHeightOffsetMin = -10.0;
	inline constexpr double ModelHeightOffsetMax = 10.0;

   enum class UnitTeam
	{
		Player,
		Enemy,
	};

	enum class SapperUnitType
	{
		Infantry,
		ArcaneInfantry,
	};

	enum class MovementType
	{
		Infantry,
		Tank,
	};

	enum class ResourceType
	{
		Budget,
		Gunpowder,
		Mana,
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
		double maxHitPoints = 100.0;
		double moveSpeed = 6.0;
		double attackRange = 3.2;
		double attackDamage = 12.0;
		double attackInterval = 0.8;
		double manaCost = SapperCost;
	};

	[[nodiscard]] inline UnitParameters MakeDefaultUnitParameters(const UnitTeam team, const SapperUnitType unitType)
	{
		switch (unitType)
		{
		case SapperUnitType::ArcaneInfantry:
			return UnitParameters{
				.movementType = MovementType::Infantry,
				.maxHitPoints = ((team == UnitTeam::Enemy) ? 180.0 : 150.0),
				.moveSpeed = 5.4,
				.attackRange = 3.8,
				.attackDamage = ((team == UnitTeam::Enemy) ? 19.0 : 20.0),
				.attackInterval = 1.05,
				.manaCost = ArcaneInfantryCost,
			};

		case SapperUnitType::Infantry:
		default:
			return UnitParameters{
				.movementType = MovementType::Infantry,
				.maxHitPoints = ((team == UnitTeam::Enemy) ? 120.0 : 100.0),
				.moveSpeed = 6.0,
				.attackRange = ((team == UnitTeam::Enemy) ? 3.0 : 3.2),
				.attackDamage = ((team == UnitTeam::Enemy) ? 10.0 : 12.0),
				.attackInterval = ((team == UnitTeam::Enemy) ? 0.95 : 0.8),
				.manaCost = SapperCost,
			};
		}
	}

	struct UnitEditorSettings
	{
		UnitParameters playerInfantry = MakeDefaultUnitParameters(UnitTeam::Player, SapperUnitType::Infantry);
		UnitParameters playerArcaneInfantry = MakeDefaultUnitParameters(UnitTeam::Player, SapperUnitType::ArcaneInfantry);
		UnitParameters enemyInfantry = MakeDefaultUnitParameters(UnitTeam::Enemy, SapperUnitType::Infantry);
		UnitParameters enemyArcaneInfantry = MakeDefaultUnitParameters(UnitTeam::Enemy, SapperUnitType::ArcaneInfantry);
	};

	enum class UnitEditorSection
	{
		PlayerInfantry,
		PlayerArcaneInfantry,
		EnemyInfantry,
		EnemyArcaneInfantry,
	};

	[[nodiscard]] inline const UnitParameters& GetUnitParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		if (team == UnitTeam::Enemy)
		{
			return (unitType == SapperUnitType::ArcaneInfantry)
				? settings.enemyArcaneInfantry
				: settings.enemyInfantry;
		}

		return (unitType == SapperUnitType::ArcaneInfantry)
			? settings.playerArcaneInfantry
			: settings.playerInfantry;
	}

	[[nodiscard]] inline UnitParameters& GetUnitParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		if (team == UnitTeam::Enemy)
		{
			return (unitType == SapperUnitType::ArcaneInfantry)
				? settings.enemyArcaneInfantry
				: settings.enemyInfantry;
		}

		return (unitType == SapperUnitType::ArcaneInfantry)
			? settings.playerArcaneInfantry
			: settings.playerInfantry;
	}

	struct CameraSettings
	{
		Vec3 eye = DefaultCameraEye;
		Vec3 focus = DefaultCameraFocus;
	};

	struct SpawnedSapper
	{
		Vec3 startPosition;
		Vec3 position;
		Vec3 targetPosition;
     Vec3 destinationPosition;
		double spawnedAt = 0.0;
      double moveStartedAt = 0.0;
		double moveDuration = 0.0;
        double facingYaw = BirdDisplayYaw;
		UnitTeam team = UnitTeam::Player;
        SapperUnitType unitType = SapperUnitType::Infantry;
        MovementType movementType = MovementType::Infantry;
		double maxHitPoints = 100.0;
		double hitPoints = 100.0;
       double moveSpeed = 6.0;
		double attackRange = 3.2;
     double baseAttackDamage = 12.0;
		double baseAttackInterval = 0.8;
		double suppressedUntil = -1000.0;
		double suppressedMoveSpeedMultiplier = 1.0;
		double suppressedAttackDamageMultiplier = 1.0;
		double suppressedAttackIntervalMultiplier = 1.0;
		double lastAttackAt = -1000.0;
      double explosionSkillCooldownUntil = -1000.0;
	};

	struct ModelHeightSettings
	{
		double birdOffsetY = 0.0;
		double ashigaruOffsetY = 0.0;
	};

	struct UiLayoutSettings
	{
		Point miniMapPosition{ 0, 0 };
		Point resourcePanelPosition{ 0, 0 };
      Point unitEditorPosition{ 0, 0 };
		Point unitEditorListPosition{ 0, 0 };
	};

	enum class AppMode
	{
		Play,
		EditMap,
	};

	enum class EnemyBattlePlan
	{
		SecureResources,
		AssaultBase,
	};
}
