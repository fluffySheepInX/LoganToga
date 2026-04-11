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
     double stopDistance = 0.2;
		double attackDamage = 12.0;
		double attackInterval = 0.8;
		double manaCost = SapperCost;
      UnitFootprintType footprintType = UnitFootprintType::Circle;
		double footprintRadius = 1.2;
		double footprintHalfLength = 0.0;
	};

	enum class UnitRenderModel
	{
		Bird,
		Ashigaru,
		SugoiCar,
	};

	struct UnitDefinition
	{
		SapperUnitType unitType = SapperUnitType::Infantry;
		StringView settingsKeySuffix;
		StringView displayName;
		StringView playerEditorSectionLabel;
		StringView enemyEditorSectionLabel;
      UnitRenderModel playerRenderModel = UnitRenderModel::Bird;
		UnitRenderModel enemyRenderModel = UnitRenderModel::Ashigaru;
		UnitParameters playerDefaults;
		UnitParameters enemyDefaults;
	};

	[[nodiscard]] inline const Array<UnitDefinition>& GetUnitDefinitions()
	{
		static const Array<UnitDefinition> Definitions{
			UnitDefinition{
				.unitType = SapperUnitType::Infantry,
				.settingsKeySuffix = U"Infantry",
				.displayName = U"兵",
				.playerEditorSectionLabel = U"Player Infantry",
				.enemyEditorSectionLabel = U"Enemy Infantry",
               .playerRenderModel = UnitRenderModel::Bird,
				.enemyRenderModel = UnitRenderModel::Ashigaru,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 100.0,
					.moveSpeed = 6.0,
					.attackRange = 3.2,
					.stopDistance = 0.15,
					.attackDamage = 12.0,
					.attackInterval = 0.8,
					.manaCost = SapperCost,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 120.0,
					.moveSpeed = 6.0,
					.attackRange = 3.0,
					.stopDistance = 0.15,
					.attackDamage = 10.0,
					.attackInterval = 0.95,
					.manaCost = SapperCost,
				},
			},
			UnitDefinition{
				.unitType = SapperUnitType::ArcaneInfantry,
				.settingsKeySuffix = U"ArcaneInfantry",
				.displayName = U"魔導兵(仮)",
				.playerEditorSectionLabel = U"Player Arcane",
				.enemyEditorSectionLabel = U"Enemy Arcane",
               .playerRenderModel = UnitRenderModel::Bird,
				.enemyRenderModel = UnitRenderModel::Ashigaru,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 150.0,
					.moveSpeed = 5.4,
					.attackRange = 3.8,
					.stopDistance = 0.35,
					.attackDamage = 20.0,
					.attackInterval = 1.05,
					.manaCost = ArcaneInfantryCost,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Infantry,
					.maxHitPoints = 180.0,
					.moveSpeed = 5.4,
					.attackRange = 3.8,
					.stopDistance = 0.35,
					.attackDamage = 19.0,
					.attackInterval = 1.05,
					.manaCost = ArcaneInfantryCost,
				},
			},
			UnitDefinition{
				.unitType = SapperUnitType::SugoiCar,
				.settingsKeySuffix = U"SugoiCar",
				.displayName = U"すごい車(仮)",
				.playerEditorSectionLabel = U"Player SugoiCar",
				.enemyEditorSectionLabel = U"Enemy SugoiCar",
               .playerRenderModel = UnitRenderModel::SugoiCar,
				.enemyRenderModel = UnitRenderModel::SugoiCar,
				.playerDefaults = UnitParameters{
					.movementType = MovementType::Tank,
					.maxHitPoints = 260.0,
					.moveSpeed = 4.8,
					.attackRange = 4.6,
					.stopDistance = 0.5,
					.attackDamage = 28.0,
					.attackInterval = 1.15,
					.manaCost = SugoiCarCost,
					.footprintType = UnitFootprintType::Capsule,
					.footprintRadius = 0.95,
					.footprintHalfLength = 2.05,
				},
				.enemyDefaults = UnitParameters{
					.movementType = MovementType::Tank,
					.maxHitPoints = 300.0,
					.moveSpeed = 4.8,
					.attackRange = 4.6,
					.stopDistance = 0.5,
					.attackDamage = 24.0,
					.attackInterval = 1.15,
					.manaCost = SugoiCarCost,
					.footprintType = UnitFootprintType::Capsule,
					.footprintRadius = 0.95,
					.footprintHalfLength = 2.05,
				},
			},
		};

		return Definitions;
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

		return GetUnitDefinitions().front();
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

		return 0;
	}

	[[nodiscard]] inline size_t GetUnitParameterSlotIndex(const UnitTeam team, const SapperUnitType unitType)
	{
		return (GetUnitTeamIndex(team) * GetUnitDefinitions().size() + GetUnitDefinitionIndex(unitType));
	}

	[[nodiscard]] inline UnitParameters MakeDefaultUnitParameters(const UnitTeam team, const SapperUnitType unitType)
	{
       return GetDefaultUnitParameters(team, unitType);
	}

	struct UnitEditorSettings
	{
      Array<UnitParameters> parameters;

		UnitEditorSettings()
		{
			parameters.resize(GetUnitDefinitions().size() * 2);

			for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
			{
				for (const auto& definition : GetUnitDefinitions())
				{
					parameters[GetUnitParameterSlotIndex(team, definition.unitType)] = MakeDefaultUnitParameters(team, definition.unitType);
				}
			}
		}
	};

    struct UnitEditorSelection
	{
		UnitTeam team = UnitTeam::Player;
		SapperUnitType unitType = SapperUnitType::Infantry;

		[[nodiscard]] bool operator==(const UnitEditorSelection& other) const
		{
			return ((team == other.team) && (unitType == other.unitType));
		}
	};

	enum class UnitEditorPage
	{
		Basic,
		Combat,
		Footprint,
	};

	[[nodiscard]] inline const UnitParameters& GetUnitParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
        return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitParameters& GetUnitParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
        return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
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
     Vec3 retreatReturnPosition{ 0, 0, 0 };
		double spawnedAt = 0.0;
      double moveStartedAt = 0.0;
		double moveDuration = 0.0;
        double facingYaw = BirdDisplayYaw;
		UnitTeam team = UnitTeam::Player;
        SapperUnitType unitType = SapperUnitType::Infantry;
        MovementType movementType = MovementType::Infantry;
        int32 tier = 1;
		double maxHitPoints = 100.0;
		double hitPoints = 100.0;
       double moveSpeed = 6.0;
		double attackRange = 3.2;
        double stopDistance = 0.2;
     double baseAttackDamage = 12.0;
		double baseAttackInterval = 0.8;
       UnitFootprintType footprintType = UnitFootprintType::Circle;
		double footprintRadius = 1.2;
		double footprintHalfLength = 0.0;
		double suppressedUntil = -1000.0;
		double suppressedMoveSpeedMultiplier = 1.0;
		double suppressedAttackDamageMultiplier = 1.0;
		double suppressedAttackIntervalMultiplier = 1.0;
		double lastAttackAt = -1000.0;
      double explosionSkillCooldownUntil = -1000.0;
      double retreatDisappearAt = -1000.0;
		double retreatReturnAt = -1000.0;
	};

	struct ModelHeightSettings
	{
		double birdOffsetY = 0.0;
		double ashigaruOffsetY = 0.0;
      double sugoiCarOffsetY = 0.0;
      double birdScale = 1.0;
		double ashigaruScale = 1.0;
		double sugoiCarScale = 1.0;
	};

	inline constexpr double ModelScaleMin = 0.25;
	inline constexpr double ModelScaleMax = 4.0;

	enum class ModelHeightTarget
	{
		Bird,
		Ashigaru,
		SugoiCar,
	};

	[[nodiscard]] inline ModelHeightTarget ToModelHeightTarget(const UnitRenderModel renderModel)
	{
		switch (renderModel)
		{
		case UnitRenderModel::Ashigaru:
			return ModelHeightTarget::Ashigaru;

		case UnitRenderModel::SugoiCar:
			return ModelHeightTarget::SugoiCar;

		case UnitRenderModel::Bird:
		default:
			return ModelHeightTarget::Bird;
		}
	}

	[[nodiscard]] inline double& GetModelScale(ModelHeightSettings& settings, const ModelHeightTarget target)
	{
		switch (target)
		{
		case ModelHeightTarget::Ashigaru:
			return settings.ashigaruScale;

		case ModelHeightTarget::SugoiCar:
			return settings.sugoiCarScale;

		case ModelHeightTarget::Bird:
		default:
			return settings.birdScale;
		}
	}

	[[nodiscard]] inline double GetModelScale(const ModelHeightSettings& settings, const ModelHeightTarget target)
	{
		switch (target)
		{
		case ModelHeightTarget::Ashigaru:
			return settings.ashigaruScale;

		case ModelHeightTarget::SugoiCar:
			return settings.sugoiCarScale;

		case ModelHeightTarget::Bird:
		default:
			return settings.birdScale;
		}
	}

	[[nodiscard]] inline double GetModelScaleForUnit(const ModelHeightSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
       return GetModelScale(settings, ToModelHeightTarget(GetUnitRenderModel(team, unitType)));
	}

	[[nodiscard]] inline UnitRenderModel GetSpawnedSapperRenderModel(const SpawnedSapper& sapper)
	{
		return GetUnitRenderModel(sapper.team, sapper.unitType);
	}

	[[nodiscard]] inline double GetSpawnedSapperModelScale(const ModelHeightSettings& settings, const SpawnedSapper& sapper)
	{
		return GetModelScaleForUnit(settings, sapper.team, sapper.unitType);
	}

	struct UiLayoutSettings
	{
		Point miniMapPosition{ 0, 0 };
		Point resourcePanelPosition{ 0, 0 };
     Point modelHeightPosition{ 0, 0 };
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
