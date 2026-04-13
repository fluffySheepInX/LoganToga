# pragma once
# include <Siv3D.hpp>

namespace MainSupport
{
	using AppCamera3D = BasicCamera3D;

	inline constexpr FilePathView CameraSettingsPath = U"settings/camera_settings.toml";
	inline constexpr FilePathView MapDataPath = U"settings/map_data.toml";
	inline constexpr FilePathView ModelHeightSettingsPath = U"settings/model_height_settings.toml";
    inline constexpr FilePathView EditorTextColorSettingsPath = U"settings/editor_text_colors.toml";
	inline constexpr FilePathView UnitSettingsPath = U"settings/unit_settings.toml";
	inline constexpr FilePathView UiLayoutSettingsPath = U"settings/ui_layout_settings.toml";
	inline constexpr FilePathView BirdModelPath = U"model/bird.glb";
	inline constexpr FilePathView AshigaruModelPath = U"model/ashigaru_v2.1.glb";
	inline constexpr FilePathView SugoiCarModelPath = U"model/sugoiCar.glb";
	inline constexpr Vec3 DefaultCameraEye{ 0, 8, -16 };
	inline constexpr Vec3 DefaultCameraFocus{ 0, 0, 0 };
	inline constexpr Vec3 BirdDisplayPosition{ -10.5, 0, -2.5 };
	inline constexpr Vec3 AshigaruDisplayPosition{ -5.5, 0, -2.5 };
	inline constexpr Vec3 SugoiCarDisplayPosition{ -0.5, 0, -2.5 };
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
	inline constexpr double SugoiCarCost = 140.0;
	inline constexpr double TierUpgradeBaseCost = 100.0;
	inline constexpr int32 MaximumSapperTier = 10;
	inline constexpr double SapperTierStatBonusRate = 0.10;
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
}
