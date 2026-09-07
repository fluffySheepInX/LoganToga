#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionStores.h"
# include "BattleWorldMap.h"

namespace LT3
{
	struct AudioAssetCache;
	struct ModContext;
	struct BattleWorld;
	struct UnitRuntimeStore;
	struct CooldownStore;
	struct BuildQueueStore;
	struct PathRuntimeStore;
	struct CarrierStore;

	// ユニットIDと同じ添字空間を共有する列。構造変更は対応ストアだけに限定する。
	template <class T>
	class UnitAlignedArray
	{
	public:
		[[nodiscard]] size_t size() const
		{
			return m_values.size();
		}

		decltype(auto) operator[](size_t index)
		{
			return m_values[index];
		}

		decltype(auto) operator[](size_t index) const
		{
			return m_values[index];
		}

		auto begin()
		{
			return m_values.begin();
		}

		auto end()
		{
			return m_values.end();
		}

		auto begin() const
		{
			return m_values.begin();
		}

		auto end() const
		{
			return m_values.end();
		}

	private:
		void append(const T& value)
		{
			m_values << value;
		}

		void append(T&& value)
		{
			m_values << std::move(value);
		}

		void clear()
		{
			m_values.clear();
		}

		void truncate(size_t size)
		{
			m_values.resize(size);
		}

		Array<T> m_values;

		friend struct UnitRuntimeStore;
		friend struct CooldownStore;
		friend struct BuildQueueStore;
		friend struct PathRuntimeStore;
		friend struct CarrierStore;
	};

	struct UnitSpatialIndexStore
	{
		int32 width = 0;
		int32 height = 0;
		Array<Array<UnitId>> unitsByCell;

		// マップのセル寸法に合わせて生存ユニット用バケットを初期化する。
		void init(int32 mapWidth, int32 mapHeight)
		{
			width = Max(0, mapWidth);
			height = Max(0, mapHeight);
			unitsByCell.assign(static_cast<size_t>(width * height), Array<UnitId>{});
		}

		// 指定セルに対応する生存ユニットバケットを返す。
		const Array<UnitId>& get(int32 row, int32 col) const
		{
			static const Array<UnitId> empty;
			if (row < 0 || col < 0 || row >= height || col >= width)
			{
				return empty;
			}

			return unitsByCell[static_cast<size_t>(row * width + col)];
		}
	};

	struct BuildCellReservation
	{
		UnitId builder = InvalidUnitId;
		BuildActionDefId actionId = InvalidBuildActionDefId;
		Point cell{ 0, 0 };
	};

		enum class BattleSkillFilterKind : uint8
		{
			All,
			Heal,
			ResourceCost,
		};

	inline constexpr int32 DefaultBattleMapWidth = 12;
	inline constexpr int32 DefaultBattleMapHeight = 8;

	struct QueuedBuildAction
	{
		BuildActionDefId actionId = InvalidBuildActionDefId;
		Vec2 targetPosition{ 0, 0 };
		bool hasTargetPosition = false;
		String iconOverride;
		Faction costFaction = Faction::Neutral;
		Point reservedCell{ 0, 0 };
		bool hasReservedCell = false;
		ResourceDefId paidGoldResource = InvalidResourceDefId;
		ResourceDefId paidTrustResource = InvalidResourceDefId;
		ResourceDefId paidFoodResource = InvalidResourceDefId;
		int32 paidGold = 0;
		int32 paidTrust = 0;
		int32 paidFood = 0;
	};

	struct PathRequest
	{
		UnitId unit = InvalidUnitId;
		Vec2 destination{ 0, 0 };
		Point startCell{ 0, 0 };
		Point goalCell{ 0, 0 };
		uint32 mapRevision = 0;
	};

	struct PathResult
	{
		UnitId unit = InvalidUnitId;
		Vec2 destination{ 0, 0 };
		Array<Vec2> waypoints;
		bool success = false;
		uint32 mapRevision = 0;
	};

	struct UnitRuntimeStore
	{
		UnitAlignedArray<UnitDefId> defId;
		UnitAlignedArray<Faction> faction;
		UnitAlignedArray<bool> alive;
		UnitAlignedArray<UnitTask> task;
		UnitAlignedArray<Vec2> position;
		UnitAlignedArray<Vec2> targetPosition;
		UnitAlignedArray<UnitId> attackTarget;
		UnitAlignedArray<int32> hp;
		UnitAlignedArray<int32> resourceTargetNode;
		UnitAlignedArray<bool> ignoreCombatWhileMoving;
		UnitAlignedArray<Vec2> formationFinalTarget;
		UnitAlignedArray<bool> hasFormationFinalTarget;
		UnitAlignedArray<String> iconOverride;

		[[nodiscard]] size_t size() const
		{
			return defId.size();
		}

	private:
		friend struct BattleWorld;

		UnitId add(UnitDefId unitDef, Faction unitFaction, const Vec2& pos, const DefinitionStores& defs)
		{
			const UnitId id = static_cast<UnitId>(defId.size());
			defId.append(unitDef);
			faction.append(unitFaction);
			alive.append(true);
			task.append(UnitTask::Idle);
			position.append(pos);
			targetPosition.append(pos);
			attackTarget.append(InvalidUnitId);
			hp.append(defs.units[unitDef].hp);
			resourceTargetNode.append(-1);
			ignoreCombatWhileMoving.append(false);
			formationFinalTarget.append(pos);
			hasFormationFinalTarget.append(false);
			iconOverride.append(U"");
			return id;
		}

		void truncate(size_t size)
		{
			defId.truncate(size);
			faction.truncate(size);
			alive.truncate(size);
			task.truncate(size);
			position.truncate(size);
			targetPosition.truncate(size);
			attackTarget.truncate(size);
			hp.truncate(size);
			resourceTargetNode.truncate(size);
			ignoreCombatWhileMoving.truncate(size);
			formationFinalTarget.truncate(size);
			hasFormationFinalTarget.truncate(size);
			iconOverride.truncate(size);
		}
	};

	struct PathRuntimeStore
	{
		UnitAlignedArray<Array<Vec2>> waypoints;
		UnitAlignedArray<int32> waypointIndex;
		UnitAlignedArray<Vec2> destination;
		UnitAlignedArray<uint32> pathMapRevision;
		UnitAlignedArray<bool> hasPath;
		UnitAlignedArray<bool> requestPending;
		UnitAlignedArray<double> repathCooldownSec;
		Array<PathRequest> requests;
		Array<PathResult> results;
		int32 maxRequestsPerFrame = 8;

	private:
		friend struct BattleWorld;

		void addUnit(const Vec2& initialPosition)
		{
			waypoints.append(Array<Vec2>{});
			waypointIndex.append(0);
			destination.append(initialPosition);
			pathMapRevision.append(0u);
			hasPath.append(false);
			requestPending.append(false);
			repathCooldownSec.append(0.0);
		}

		void truncate(size_t size)
		{
			waypoints.truncate(size);
			waypointIndex.truncate(size);
			destination.truncate(size);
			pathMapRevision.truncate(size);
			hasPath.truncate(size);
			requestPending.truncate(size);
			repathCooldownSec.truncate(size);
		}

	public:

		void clearUnitPath(UnitId unit)
		{
			if (unit >= waypoints.size())
			{
				return;
			}

			waypoints[unit].clear();
			waypointIndex[unit] = 0;
			pathMapRevision[unit] = 0u;
			hasPath[unit] = false;
			requestPending[unit] = false;
			repathCooldownSec[unit] = 0.0;
		}
	};

	struct CooldownStore
	{
		UnitAlignedArray<double> attackLeftSec;
		UnitAlignedArray<int32> burstShotsLeft;
		UnitAlignedArray<double> burstShotTimerSec;
		UnitAlignedArray<UnitId> burstTarget;
		UnitAlignedArray<SkillDefId> burstSkill;
		UnitAlignedArray<Array<int32>> burstOrder;
		UnitAlignedArray<double> skillCastFailureDisplayLeftSec;

	private:
		friend struct BattleWorld;

		void addUnit()
		{
			attackLeftSec.append(Random(0.0, 0.25));
			burstShotsLeft.append(0);
			burstShotTimerSec.append(0.0);
			burstTarget.append(InvalidUnitId);
			burstSkill.append(InvalidSkillDefId);
			burstOrder.append(Array<int32>{});
			skillCastFailureDisplayLeftSec.append(0.0);
		}

		void truncate(size_t size)
		{
			attackLeftSec.truncate(size);
			burstShotsLeft.truncate(size);
			burstShotTimerSec.truncate(size);
			burstTarget.truncate(size);
			burstSkill.truncate(size);
			burstOrder.truncate(size);
			skillCastFailureDisplayLeftSec.truncate(size);
		}
	};

	struct BuildQueueStore
	{
		UnitAlignedArray<double> progressSec;
		UnitAlignedArray<Array<QueuedBuildAction>> entries;
		UnitAlignedArray<QueuedBuildAction> pendingEntry;
		UnitAlignedArray<bool> hasPendingEntry;
		UnitAlignedArray<bool> locked;

	private:
		friend struct BattleWorld;

		void addUnit()
		{
			progressSec.append(0.0);
			entries.append(Array<QueuedBuildAction>{});
			pendingEntry.append(QueuedBuildAction{});
			hasPendingEntry.append(false);
			locked.append(false);
		}

		void truncate(size_t size)
		{
			progressSec.truncate(size);
			entries.truncate(size);
			pendingEntry.truncate(size);
			hasPendingEntry.truncate(size);
			locked.truncate(size);
		}
	};

	struct PlacedObjectStore
	{
		Array<Vec2> position;
		Array<String> tag;
		Array<String> icon;

		void add(const Vec2& placedPosition, const String& objectTag, const String& iconName)
		{
			position << placedPosition;
			tag << objectTag;
			icon << iconName;
		}
	};

	struct CarrierStore
	{
		UnitAlignedArray<Array<UnitId>> storedUnits;

	private:
		friend struct BattleWorld;

		void addUnit()
		{
			storedUnits.append(Array<UnitId>{});
		}

		void truncate(size_t size)
		{
			storedUnits.truncate(size);
		}
	};

	struct ResourceNodeStore
	{
		Array<ResourceDefId> defId;
		Array<Vec2> position;
		Array<int32> amount;
		Array<int32> incomePerSec;
		Array<bool> oneShot;
		Array<bool> collected;
		Array<double> captureTimeSec;
		Array<Faction> owner;
		Array<Faction> capturingFaction;
		Array<double> captureProgress;

		void add(ResourceDefId resourceDef, const Vec2& pos, int32 value, int32 income, bool isOneShot, double captureTime)
		{
			defId << resourceDef;
			position << pos;
			amount << value;
			incomePerSec << income;
			oneShot << isOneShot;
			collected << false;
			captureTimeSec << Max(0.1, captureTime);
			owner << Faction::Neutral;
			capturingFaction << Faction::Neutral;
			captureProgress << 0.0;
		}
	};

	struct ProjectileStore
	{
		Array<Vec2> position;
		Array<Vec2> velocity;
		Array<Vec2> startPosition;
		Array<Vec2> endPosition;
		Array<UnitId> target;
		Array<UnitId> owner;
		Array<Faction> faction;
		Array<int32> ownerAttack;
		Array<SkillDefId> skill;
		Array<SkillProjectileMotion> motion;
		Array<double> lifeSec;
		Array<double> ageSec;
		Array<double> maxLifeSec;
		Array<double> height;
		Array<double> angleRad;
		Array<double> baseAngleRad;
		Array<Vec2> firedTargetPosition;
		Array<bool> hasImageAngleOverride;
		Array<double> imageAngleOverrideRad;
		Array<int32> chainDepth;
		Array<Array<UnitId>> swingHitUnits;

		void add(const Vec2& pos, const Vec2& vel, const Vec2& start, const Vec2& end, UnitId targetUnit, UnitId ownerUnit, Faction ownerFaction, int32 firedOwnerAttack, SkillDefId skillDef, SkillProjectileMotion projectileMotion, double maxLife, double initialAngleRad, const Vec2& firedTargetPos = Vec2{ 0.0, 0.0 }, bool imageAngleOverrideEnabled = false, double imageAngleOverride = 0.0, int32 nextChainDepth = 0)
		{
			position << pos;
			velocity << vel;
			startPosition << start;
			endPosition << end;
			target << targetUnit;
			owner << ownerUnit;
			faction << ownerFaction;
			ownerAttack << firedOwnerAttack;
			skill << skillDef;
			motion << projectileMotion;
			lifeSec << maxLife;
			ageSec << 0.0;
			maxLifeSec << maxLife;
			height << 0.0;
			angleRad << initialAngleRad;
			baseAngleRad << initialAngleRad;
			firedTargetPosition << firedTargetPos;
			hasImageAngleOverride << imageAngleOverrideEnabled;
			imageAngleOverrideRad << imageAngleOverride;
			chainDepth << nextChainDepth;
			swingHitUnits << Array<UnitId>{};
		}

		void add(const Vec2& pos, const Vec2& vel, UnitId targetUnit, Faction ownerFaction, SkillDefId skillDef)
		{
			add(pos, vel, pos, pos + vel, targetUnit, InvalidUnitId, ownerFaction, 0, skillDef, SkillProjectileMotion::Direct, 2.5, 0.0);
		}

		void removeAt(size_t index)
		{
			const size_t last = position.size() - 1;
			if (index != last)
			{
				position[index]      = position[last];
				velocity[index]      = velocity[last];
				startPosition[index] = startPosition[last];
				endPosition[index]   = endPosition[last];
				target[index]        = target[last];
				owner[index]         = owner[last];
				faction[index]       = faction[last];
				ownerAttack[index]   = ownerAttack[last];
				skill[index]         = skill[last];
				motion[index]        = motion[last];
				lifeSec[index]       = lifeSec[last];
				ageSec[index]        = ageSec[last];
				maxLifeSec[index]    = maxLifeSec[last];
				height[index]        = height[last];
				angleRad[index]      = angleRad[last];
				baseAngleRad[index]  = baseAngleRad[last];
				firedTargetPosition[index] = firedTargetPosition[last];
				hasImageAngleOverride[index] = hasImageAngleOverride[last];
				imageAngleOverrideRad[index] = imageAngleOverrideRad[last];
				chainDepth[index] = chainDepth[last];
				swingHitUnits[index] = swingHitUnits[last];
			}
			position.pop_back();
			velocity.pop_back();
			startPosition.pop_back();
			endPosition.pop_back();
			target.pop_back();
			owner.pop_back();
			faction.pop_back();
			ownerAttack.pop_back();
			skill.pop_back();
			motion.pop_back();
			lifeSec.pop_back();
			ageSec.pop_back();
			maxLifeSec.pop_back();
			height.pop_back();
			angleRad.pop_back();
			baseAngleRad.pop_back();
			firedTargetPosition.pop_back();
			hasImageAngleOverride.pop_back();
			imageAngleOverrideRad.pop_back();
			chainDepth.pop_back();
			swingHitUnits.pop_back();
		}
	};

	struct ResourceRuntimeStore
	{
		Array<int32> playerAmounts;
		Array<int32> enemyAmounts;
		double incomeTickAccumSec = 0.0;
	};

	struct BomVisualEffectStore
	{
		Array<Vec2> position;
		Array<double> leftSec;
		Array<double> durationSec;
		Array<double> scale;
		Array<double> radius;
		Array<SkillBomVisual> visual;
		Array<SkillKind> kind;
		Array<bool> friendlyFire;
		Array<String> image;

		void add(const Vec2& effectPosition, double effectDurationSec, double effectScale, double effectRadius, SkillBomVisual effectVisual, SkillKind effectKind, bool effectFriendlyFire, const String& effectImage)
		{
			position << effectPosition;
			leftSec << effectDurationSec;
			durationSec << effectDurationSec;
			scale << effectScale;
			radius << effectRadius;
			visual << effectVisual;
			kind << effectKind;
			friendlyFire << effectFriendlyFire;
			image << effectImage;
		}

		void removeAt(size_t index)
		{
			const size_t last = position.size() - 1;
			if (index != last)
			{
				position[index] = position[last];
				leftSec[index] = leftSec[last];
				durationSec[index] = durationSec[last];
				scale[index] = scale[last];
				radius[index] = radius[last];
				visual[index] = visual[last];
				kind[index] = kind[last];
				friendlyFire[index] = friendlyFire[last];
				image[index] = image[last];
			}
			position.pop_back();
			leftSec.pop_back();
			durationSec.pop_back();
			scale.pop_back();
			radius.pop_back();
			visual.pop_back();
			kind.pop_back();
			friendlyFire.pop_back();
			image.pop_back();
		}
	};

	enum class AiRuntimePhase : uint8
	{
		Opening,
		BuildUp,
		AttackWave,
		Recover,
	};

	struct AiRuntimeStore
	{
		AiProfileDefId profileId = InvalidAiProfileDefId;
		String profileTag = U"balanced";
		AiRuntimePhase phase = AiRuntimePhase::Opening;
		double phaseTimerSec = 0.0;
		double spawnTimerSec = 0.0;
		double productionTimerSec = 0.0;
		double attackWaveTimerSec = 0.0;
		double tacticalTimerSec = 0.0;
		double nonWaveRoleReassignmentTimerSec = 0.0;
		int32 attackWaveIndex = 0;
		Array<UnitId> attackWaveUnits;
		Array<UnitId> resourceReclaimUnits;
		Array<UnitId> guardUnits;
		Array<UnitId> skirmishUnits;
		Vec2 rallyPosition{ 0.0, 0.0 };
		Vec2 attackTargetPosition{ 0.0, 0.0 };
		Vec2 resourceTargetPosition{ 0.0, 0.0 };
		Vec2 guardAnchorPosition{ 0.0, 0.0 };
		Vec2 skirmishAnchorPosition{ 0.0, 0.0 };
		UnitId attackTargetUnit = InvalidUnitId;
		bool hasRallyPosition = false;
		bool hasAttackTargetPosition = false;
		bool hasResourceTargetPosition = false;
		bool hasGuardAnchorPosition = false;
		bool hasSkirmishAnchorPosition = false;
		double battleTimeLimitSec = 25.0 * 60.0;

		void resetForProfile(AiProfileDefId id, const String& tag)
		{
			profileId = id;
			profileTag = tag;
			phase = AiRuntimePhase::Opening;
			phaseTimerSec = 0.0;
			spawnTimerSec = 0.0;
			productionTimerSec = 0.0;
			attackWaveTimerSec = 0.0;
			tacticalTimerSec = 0.0;
			nonWaveRoleReassignmentTimerSec = 0.0;
			attackWaveIndex = 0;
			attackWaveUnits.clear();
			resourceReclaimUnits.clear();
			guardUnits.clear();
			skirmishUnits.clear();
			rallyPosition = Vec2{ 0.0, 0.0 };
			attackTargetPosition = Vec2{ 0.0, 0.0 };
			resourceTargetPosition = Vec2{ 0.0, 0.0 };
			guardAnchorPosition = Vec2{ 0.0, 0.0 };
			skirmishAnchorPosition = Vec2{ 0.0, 0.0 };
			attackTargetUnit = InvalidUnitId;
			hasRallyPosition = false;
			hasAttackTargetPosition = false;
			hasResourceTargetPosition = false;
			hasGuardAnchorPosition = false;
			hasSkirmishAnchorPosition = false;
			battleTimeLimitSec = 25.0 * 60.0;
		}
	};

	inline ResourceRuntimeStore MakeResourceRuntimeStore(const DefinitionStores& defs)
	{
		ResourceRuntimeStore store;
		store.playerAmounts.assign(defs.resources.size(), 0);
		store.enemyAmounts.assign(defs.resources.size(), 0);

		for (ResourceDefId id = 0; id < defs.resources.size(); ++id)
		{
			const ResourceDef& def = defs.resources[id];
			store.playerAmounts[id] = def.initialAmount;
			store.enemyAmounts[id] = def.initialAmount;
		}
		return store;
	}

	struct SelectionStore
	{
		UnitId selected = InvalidUnitId;
		Array<UnitId> selectedUnits;
		bool areaDragging = false;
		Vec2 areaDragStartScreen{ 0, 0 };
		Vec2 areaDragCurrentScreen{ 0, 0 };
		bool formationPlacementActive = false;
		Array<UnitId> formationUnits;
		Vec2 formationDestinationWorld{ 0, 0 };
		Vec2 formationCurrentWorld{ 0, 0 };
		bool actionPlacementActive = false;
		UnitId actionBuilder = InvalidUnitId;
		BuildActionDefId actionId = InvalidBuildActionDefId;
		Vec2 actionTargetWorld{ 0, 0 };
		bool actionLineDragging = false;
		Vec2 actionLineStartWorld{ 0, 0 };
		Array<Vec2> actionLineTargets;
		int32 hoveredResourceNode = -1;
		BattleSkillFilterKind skillFilter = BattleSkillFilterKind::All;
		SkillDefId selectedSkill = InvalidSkillDefId;
	};

	struct BattleWorld
	{
		uint64 definitionGeneration = 0;
		int32 mapWidth  = DefaultBattleMapWidth;
		int32 mapHeight = DefaultBattleMapHeight;
		UnitRuntimeStore  units;
		CooldownStore cooldowns;
		BuildQueueStore buildQueues;
		Array<BuildCellReservation> buildCellReservations;
		ResourceNodeStore resourceNodes;
		ProjectileStore   projectiles;
		BomVisualEffectStore bomVisualEffects;
		PlacedObjectStore placedObjects;
		CarrierStore carriers;
		PathRuntimeStore pathing;
		ResourceRuntimeStore resources;
		AiRuntimeStore aiRuntime;
		SelectionStore    selection;
		BattleMapStore    map;
		Array<UnitId> liveUnits;
		UnitSpatialIndexStore unitSpatialIndex;
		BattleOutcomeRules outcomeRules;
		AudioAssetCache* audioAssets = nullptr;
		const ModContext* audioMod = nullptr;
		bool enemyDirectorPaused = false;
		double enemySpawnTimerSec = 0.0;
		double elapsedSec         = 0.0;
		BattleOutcome outcome = BattleOutcome::InProgress;

		// 全ユニット対応ストアへ1行を同時に追加し、追加された安定IDを返す。
		UnitId addUnit(UnitDefId unitDef, Faction faction, const Vec2& position, const DefinitionStores& defs, const String& iconOverride = U"");

		// ユニット対応列を縮めずに、指定ユニットと関連する一時状態を終了する。
		bool retireUnit(UnitId unit);

		// ユニットを生存対象の索引から外し、非生存状態へ遷移する。
		bool deactivateUnit(UnitId unit);

		// 非生存スロットを生存対象の索引へ復帰させる。
		bool activateUnit(UnitId unit);

		// 建築予定セルを全件確保できた場合にのみ予約する。
		bool reserveBuildCells(UnitId builder, BuildActionDefId actionId, const Array<Point>& cells);

		// 指定された建築キュー項目の予定セルを解放する。
		void releaseBuildCellReservation(UnitId builder, const QueuedBuildAction& entry);

		// 指定された未完了建築を返金して、対応する予定セルを解放する。
		void cancelQueuedBuildAction(UnitId builder, const QueuedBuildAction& entry);

		// 建築者の未完了キューを返金して、関連する予定セルをすべて解放する。
		void cancelBuildQueue(UnitId builder);

		// BattleWorld全体を初期状態へ戻す。
		void reset();
	};

	struct BattleWorldStoreInvariantResult
	{
		bool valid = true;
		String store;
		String column;
		size_t expectedSize = 0;
		size_t actualSize = 0;
		size_t index = 0;
		UnitId referencedUnit = InvalidUnitId;
		String expectedCondition;
	};

	// UnitIdがBattleWorldのユニットスロットを指すか判定する。
	inline bool HasBattleWorldUnitSlot(const BattleWorld& world, UnitId unit)
	{
		return unit != InvalidUnitId && unit < world.units.size();
	}

	// UnitIdが生存中のBattleWorldユニットを指すか判定する。
	inline bool HasLiveBattleWorldUnit(const BattleWorld& world, UnitId unit)
	{
		return HasBattleWorldUnitSlot(world, unit) && world.units.alive[unit];
	}

	// 生存対象の安定UnitId索引を返す。
	inline const Array<UnitId>& GetLiveBattleWorldUnits(const BattleWorld& world)
	{
		return world.liveUnits;
	}

	inline constexpr double BattleWorldSpatialIndexCellStep = 120.0;
	inline constexpr Vec2 BattleWorldSpatialIndexMapOrigin{ 200.0, 90.0 };

	// ワールド座標を共有空間索引用のマップセルへ変換する。
	inline Point BattleWorldPositionToSpatialIndexCell(const BattleWorld& world, const Vec2& position)
	{
		const Vec2 local = position - BattleWorldSpatialIndexMapOrigin;
		const int32 col = Clamp(static_cast<int32>(Math::Round(local.x / BattleWorldSpatialIndexCellStep)), 0, Max(0, world.mapWidth - 1));
		const int32 row = Clamp(static_cast<int32>(Math::Round(local.y / BattleWorldSpatialIndexCellStep)), 0, Max(0, world.mapHeight - 1));
		return Point{ col, row };
	}

	// 生存ユニットの現在位置からセル別空間索引を再構築する。
	inline void RebuildBattleWorldUnitSpatialIndex(BattleWorld& world)
	{
		world.unitSpatialIndex.init(world.mapWidth, world.mapHeight);
		for (const UnitId unit : GetLiveBattleWorldUnits(world))
		{
			if (!HasLiveBattleWorldUnit(world, unit))
			{
				continue;
			}

			const Point cell = BattleWorldPositionToSpatialIndexCell(world, world.units.position[unit]);
			world.unitSpatialIndex.unitsByCell[static_cast<size_t>(cell.y * world.unitSpatialIndex.width + cell.x)] << unit;
		}
	}

	// 中心位置と半径に交差し得るセル内の生存ユニットを列挙する。
	template <class Func>
	inline void ForEachBattleWorldUnitNearPosition(const BattleWorld& world, const Vec2& center, double radius, Func&& func)
	{
		if (world.unitSpatialIndex.width != world.mapWidth
			|| world.unitSpatialIndex.height != world.mapHeight)
		{
			return;
		}

		const Point centerCell = BattleWorldPositionToSpatialIndexCell(world, center);
		const int32 cellRadius = Max(0, static_cast<int32>(Math::Ceil(Max(0.0, radius) / BattleWorldSpatialIndexCellStep)) + 1);
		for (int32 row = centerCell.y - cellRadius; row <= centerCell.y + cellRadius; ++row)
		{
			for (int32 col = centerCell.x - cellRadius; col <= centerCell.x + cellRadius; ++col)
			{
				for (const UnitId unit : world.unitSpatialIndex.get(row, col))
				{
					func(unit);
				}
			}
		}
	}

	// ユニット対応SoA列と経路ワークキューの参照先が整合しているか検証する。
	inline BattleWorldStoreInvariantResult ValidateBattleWorldStoreInvariants(const BattleWorld& world)
	{
		const size_t expectedSize = world.units.size();
		const auto validateSize = [expectedSize](StringView store, StringView column, size_t actualSize)
		{
			BattleWorldStoreInvariantResult result;
			result.valid = (actualSize == expectedSize);
			result.store = store;
			result.column = column;
			result.expectedSize = expectedSize;
			result.actualSize = actualSize;
			return result;
		};

		const auto check = [&](StringView store, StringView column, size_t actualSize)
		{
			const auto result = validateSize(store, column, actualSize);
			return result.valid ? Optional<BattleWorldStoreInvariantResult>{} : Optional<BattleWorldStoreInvariantResult>{ result };
		};

		#define LT3_VALIDATE_UNIT_ALIGNED_COLUMN(store, column) if (const auto result = check(U#store, U#column, world.store.column.size())) return *result
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, defId);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, faction);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, alive);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, task);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, position);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, targetPosition);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, attackTarget);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, hp);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, resourceTargetNode);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, ignoreCombatWhileMoving);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, formationFinalTarget);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, hasFormationFinalTarget);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(units, iconOverride);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, attackLeftSec);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, burstShotsLeft);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, burstShotTimerSec);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, burstTarget);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, burstSkill);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, burstOrder);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(cooldowns, skillCastFailureDisplayLeftSec);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(buildQueues, progressSec);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(buildQueues, entries);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(buildQueues, pendingEntry);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(buildQueues, hasPendingEntry);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(buildQueues, locked);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, waypoints);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, waypointIndex);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, destination);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, pathMapRevision);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, hasPath);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, requestPending);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(pathing, repathCooldownSec);
		LT3_VALIDATE_UNIT_ALIGNED_COLUMN(carriers, storedUnits);
		#undef LT3_VALIDATE_UNIT_ALIGNED_COLUMN

		const size_t resourceNodeCount = world.resourceNodes.defId.size();
		#define LT3_VALIDATE_RESOURCE_NODE_COLUMN(column) if (world.resourceNodes.column.size() != resourceNodeCount) return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U#column, resourceNodeCount, world.resourceNodes.column.size() }
		if (world.resourceNodes.position.size() != resourceNodeCount) return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U"position", resourceNodeCount, world.resourceNodes.position.size() };
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(amount);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(incomePerSec);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(oneShot);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(collected);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(captureTimeSec);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(owner);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(capturingFaction);
		LT3_VALIDATE_RESOURCE_NODE_COLUMN(captureProgress);
		#undef LT3_VALIDATE_RESOURCE_NODE_COLUMN
		for (size_t node = 0; node < resourceNodeCount; ++node)
		{
			const double progress = world.resourceNodes.captureProgress[node];
			const Faction owner = world.resourceNodes.owner[node];
			const Faction capturingFaction = world.resourceNodes.capturingFaction[node];
			if (!(0.0 <= progress && progress <= 1.0))
			{
				return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U"captureProgress", 0, 0, node, InvalidUnitId, U"value in [0.0, 1.0]" };
			}
			if (progress == 0.0 && capturingFaction != Faction::Neutral)
			{
				return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U"capturingFaction", 0, 0, node, InvalidUnitId, U"Neutral when progress is zero" };
			}
			if (progress == 1.0 && (owner == Faction::Neutral || owner != capturingFaction))
			{
				return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U"capturingFaction", 0, 0, node, InvalidUnitId, U"owner when progress is complete" };
			}
			if (0.0 < progress && progress < 1.0 && (capturingFaction == Faction::Neutral || owner == capturingFaction))
			{
				return BattleWorldStoreInvariantResult{ false, U"resourceNodes", U"capturingFaction", 0, 0, node, InvalidUnitId, U"non-owner faction while capture is in progress" };
			}
		}

		for (const PathRequest& request : world.pathing.requests)
		{
			if (!HasLiveBattleWorldUnit(world, request.unit))
			{
				return BattleWorldStoreInvariantResult{ false, U"pathing", U"requests.unit", 0, 0, 0, request.unit, U"live unit" };
			}
		}
		for (const PathResult& result : world.pathing.results)
		{
			if (!HasLiveBattleWorldUnit(world, result.unit))
			{
				return BattleWorldStoreInvariantResult{ false, U"pathing", U"results.unit", 0, 0, 0, result.unit, U"live unit" };
			}
		}

		const auto invalidSelectionReference = [](StringView column, size_t index, UnitId unit)
		{
			return BattleWorldStoreInvariantResult{ false, U"selection", String{ column }, 0, 0, index, unit, U"live unit" };
		};
		if (world.selection.selected != InvalidUnitId && !HasLiveBattleWorldUnit(world, world.selection.selected))
		{
			return invalidSelectionReference(U"selected", 0, world.selection.selected);
		}
		if (world.selection.selectedUnits.isEmpty())
		{
			if (world.selection.selected != InvalidUnitId)
			{
				return BattleWorldStoreInvariantResult{ false, U"selection", U"selected", 0, 0, 0, world.selection.selected, U"InvalidUnitId when selectedUnits is empty" };
			}
		}
		else if (world.selection.selected != world.selection.selectedUnits.front())
		{
			return BattleWorldStoreInvariantResult{ false, U"selection", U"selected", 0, 0, 0, world.selection.selected, U"selectedUnits.front()" };
		}
		for (size_t i = 0; i < world.selection.selectedUnits.size(); ++i)
		{
			const UnitId unit = world.selection.selectedUnits[i];
			if (!HasLiveBattleWorldUnit(world, unit))
			{
				return invalidSelectionReference(U"selectedUnits", i, unit);
			}
			for (size_t previous = 0; previous < i; ++previous)
			{
				if (world.selection.selectedUnits[previous] == unit)
				{
					return BattleWorldStoreInvariantResult{ false, U"selection", U"selectedUnits", 0, 0, i, unit, U"unique live unit" };
				}
			}
		}
		for (size_t i = 0; i < world.selection.formationUnits.size(); ++i)
		{
			const UnitId unit = world.selection.formationUnits[i];
			if (!HasLiveBattleWorldUnit(world, unit))
			{
				return invalidSelectionReference(U"formationUnits", i, unit);
			}
			for (size_t previous = 0; previous < i; ++previous)
			{
				if (world.selection.formationUnits[previous] == unit)
				{
					return BattleWorldStoreInvariantResult{ false, U"selection", U"formationUnits", 0, 0, i, unit, U"unique live unit" };
				}
			}
		}
		if (world.selection.actionPlacementActive)
		{
			if (!HasLiveBattleWorldUnit(world, world.selection.actionBuilder))
			{
				return invalidSelectionReference(U"actionBuilder", 0, world.selection.actionBuilder);
			}
		}
		else if (world.selection.actionBuilder != InvalidUnitId)
		{
			return BattleWorldStoreInvariantResult{ false, U"selection", U"actionBuilder", 0, 0, 0, world.selection.actionBuilder, U"InvalidUnitId when action placement is inactive" };
		}

		Array<bool> liveUnitIndexed(expectedSize, false);
		for (size_t i = 0; i < world.liveUnits.size(); ++i)
		{
			const UnitId unit = world.liveUnits[i];
			if (!HasLiveBattleWorldUnit(world, unit))
			{
				return BattleWorldStoreInvariantResult{ false, U"liveUnits", U"unit", 0, 0, i, unit, U"live unit" };
			}
			if (liveUnitIndexed[unit])
			{
				return BattleWorldStoreInvariantResult{ false, U"liveUnits", U"unit", 0, 0, i, unit, U"unique unit" };
			}
			if (i > 0 && world.liveUnits[i - 1] >= unit)
			{
				return BattleWorldStoreInvariantResult{ false, U"liveUnits", U"unit", 0, 0, i, unit, U"strict ascending UnitId order" };
			}
			liveUnitIndexed[unit] = true;
		}
		for (size_t unit = 0; unit < expectedSize; ++unit)
		{
			if (world.units.alive[unit] != liveUnitIndexed[unit])
			{
				return BattleWorldStoreInvariantResult{ false, U"liveUnits", U"unit", 0, 0, unit, static_cast<UnitId>(unit), U"exactly all live units in UnitId order" };
			}
		}

		for (size_t i = 0; i < expectedSize; ++i)
		{
			const UnitId attackTarget = world.units.attackTarget[i];
			if (attackTarget != InvalidUnitId && !HasLiveBattleWorldUnit(world, attackTarget))
			{
				return BattleWorldStoreInvariantResult{ false, U"units", U"attackTarget", 0, 0, i, attackTarget, U"InvalidUnitId or live unit" };
			}

			const UnitId burstTarget = world.cooldowns.burstTarget[i];
			if (burstTarget != InvalidUnitId && !HasLiveBattleWorldUnit(world, burstTarget))
			{
				return BattleWorldStoreInvariantResult{ false, U"cooldowns", U"burstTarget", 0, 0, i, burstTarget, U"InvalidUnitId or live unit" };
			}
			if (world.cooldowns.burstShotsLeft[i] <= 0 && burstTarget != InvalidUnitId)
			{
				return BattleWorldStoreInvariantResult{ false, U"cooldowns", U"burstTarget", 0, 0, i, burstTarget, U"InvalidUnitId when no burst shots remain" };
			}
		}

		Array<UnitId> assignedAiUnits;
		const auto validateAiAssignments = [&](const Array<UnitId>& assignments, StringView column) -> Optional<BattleWorldStoreInvariantResult>
		{
			for (size_t i = 0; i < assignments.size(); ++i)
			{
				const UnitId unit = assignments[i];
				if (!HasLiveBattleWorldUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
				{
					return BattleWorldStoreInvariantResult{ false, U"aiRuntime", String{ column }, 0, 0, i, unit, U"live enemy unit" };
				}
				if (assignedAiUnits.contains(unit))
				{
					return BattleWorldStoreInvariantResult{ false, U"aiRuntime", String{ column }, 0, 0, i, unit, U"unit assigned to one AI role" };
				}
				assignedAiUnits << unit;
			}
			return none;
		};
		if (const auto result = validateAiAssignments(world.aiRuntime.attackWaveUnits, U"attackWaveUnits")) return *result;
		if (const auto result = validateAiAssignments(world.aiRuntime.resourceReclaimUnits, U"resourceReclaimUnits")) return *result;
		if (const auto result = validateAiAssignments(world.aiRuntime.guardUnits, U"guardUnits")) return *result;
		if (const auto result = validateAiAssignments(world.aiRuntime.skirmishUnits, U"skirmishUnits")) return *result;
		if (world.aiRuntime.attackTargetUnit != InvalidUnitId
			&& (!HasLiveBattleWorldUnit(world, world.aiRuntime.attackTargetUnit)
				|| world.units.faction[world.aiRuntime.attackTargetUnit] != Faction::Player))
		{
			return BattleWorldStoreInvariantResult{ false, U"aiRuntime", U"attackTargetUnit", 0, 0, 0, world.aiRuntime.attackTargetUnit, U"InvalidUnitId or live player unit" };
		}

		Array<UnitId> storedUnits;
		for (size_t carrier = 0; carrier < world.carriers.storedUnits.size(); ++carrier)
		{
			for (const UnitId unit : world.carriers.storedUnits[carrier])
			{
				if (!HasBattleWorldUnitSlot(world, unit) || world.units.alive[unit])
				{
					return BattleWorldStoreInvariantResult{ false, U"carriers", U"storedUnits", 0, 0, carrier, unit, U"non-live unit slot" };
				}
				if (storedUnits.contains(unit))
				{
					return BattleWorldStoreInvariantResult{ false, U"carriers", U"storedUnits", 0, 0, carrier, unit, U"unit stored by one carrier" };
				}
				storedUnits << unit;
			}
		}

		if (world.projectiles.owner.size() != world.projectiles.target.size())
		{
			return BattleWorldStoreInvariantResult{ false, U"projectiles", U"owner", world.projectiles.target.size(), world.projectiles.owner.size(), 0, InvalidUnitId, U"same size as target" };
		}
		if (world.projectiles.swingHitUnits.size() != world.projectiles.target.size())
		{
			return BattleWorldStoreInvariantResult{ false, U"projectiles", U"swingHitUnits", world.projectiles.target.size(), world.projectiles.swingHitUnits.size(), 0, InvalidUnitId, U"same size as target" };
		}
		for (size_t i = 0; i < world.projectiles.target.size(); ++i)
		{
			const auto validateProjectileReference = [&](StringView column, UnitId unit) -> Optional<BattleWorldStoreInvariantResult>
			{
				if (unit != InvalidUnitId && !HasBattleWorldUnitSlot(world, unit))
				{
					return BattleWorldStoreInvariantResult{ false, U"projectiles", String{ column }, 0, 0, i, unit, U"InvalidUnitId or unit slot" };
				}
				return none;
			};
			if (const auto result = validateProjectileReference(U"target", world.projectiles.target[i])) return *result;
			if (const auto result = validateProjectileReference(U"owner", world.projectiles.owner[i])) return *result;
			for (const UnitId unit : world.projectiles.swingHitUnits[i])
			{
				if (const auto result = validateProjectileReference(U"swingHitUnits", unit)) return *result;
			}
		}
		for (size_t i = 0; i < world.buildCellReservations.size(); ++i)
		{
			const BuildCellReservation& reservation = world.buildCellReservations[i];
			if (!HasLiveBattleWorldUnit(world, reservation.builder)
				|| reservation.actionId == InvalidBuildActionDefId
				|| !world.map.inBounds(reservation.cell.y, reservation.cell.x))
			{
				return BattleWorldStoreInvariantResult{ false, U"buildCellReservations", U"owner", 0, 0, i, reservation.builder, U"live builder, valid action ID, and in-bounds cell" };
			}
			for (size_t j = 0; j < i; ++j)
			{
				if (world.buildCellReservations[j].cell == reservation.cell)
				{
					return BattleWorldStoreInvariantResult{ false, U"buildCellReservations", U"cell", 0, 0, i, reservation.builder, U"unique reserved cell" };
				}
			}
		}

		return BattleWorldStoreInvariantResult{};
	}

	inline UnitId BattleWorld::addUnit(UnitDefId unitDef, Faction faction, const Vec2& position, const DefinitionStores& defs, const String& iconOverride)
	{
		if (unitDef >= defs.units.size() || !ValidateBattleWorldStoreInvariants(*this).valid)
		{
			return InvalidUnitId;
		}

		const size_t originalSize = units.size();
		try
		{
			const UnitId id = units.add(unitDef, faction, position, defs);
			cooldowns.addUnit();
			buildQueues.addUnit();
			carriers.addUnit();
			pathing.addUnit(position);
			units.iconOverride[id] = iconOverride;
			liveUnits << id;

			if (ValidateBattleWorldStoreInvariants(*this).valid)
			{
				return id;
			}
		}
		catch (const std::exception&)
		{
		}

		units.truncate(originalSize);
		cooldowns.truncate(originalSize);
		buildQueues.truncate(originalSize);
		carriers.truncate(originalSize);
		pathing.truncate(originalSize);
		liveUnits.remove_if([originalSize](const UnitId unit) { return unit >= originalSize; });
		return InvalidUnitId;
	}

	inline bool BattleWorld::deactivateUnit(UnitId unit)
	{
		if (!HasLiveBattleWorldUnit(*this, unit))
		{
			return false;
		}

		units.alive[unit] = false;
		liveUnits.remove(unit);
		return true;
	}

	inline bool BattleWorld::activateUnit(UnitId unit)
	{
		if (!HasBattleWorldUnitSlot(*this, unit) || units.alive[unit] || liveUnits.contains(unit))
		{
			return false;
		}

		units.alive[unit] = true;
		liveUnits << unit;
		liveUnits.sort();
		return true;
	}

	inline bool BattleWorld::retireUnit(UnitId unit)
	{
		if (!ValidateBattleWorldStoreInvariants(*this).valid || unit == InvalidUnitId || unit >= units.size() || !units.alive[unit])
		{
			return false;
		}

		cancelBuildQueue(unit);
		deactivateUnit(unit);
		units.task[unit] = UnitTask::Idle;
		units.targetPosition[unit] = units.position[unit];
		units.attackTarget[unit] = InvalidUnitId;
		units.resourceTargetNode[unit] = -1;
		units.ignoreCombatWhileMoving[unit] = false;
		units.formationFinalTarget[unit] = units.position[unit];
		units.hasFormationFinalTarget[unit] = false;
		cooldowns.attackLeftSec[unit] = 0.0;
		cooldowns.burstShotsLeft[unit] = 0;
		cooldowns.burstShotTimerSec[unit] = 0.0;
		cooldowns.burstTarget[unit] = InvalidUnitId;
		cooldowns.burstSkill[unit] = InvalidSkillDefId;
		cooldowns.burstOrder[unit].clear();
		cooldowns.skillCastFailureDisplayLeftSec[unit] = 0.0;
		carriers.storedUnits[unit].clear();
		pathing.clearUnitPath(unit);
		pathing.requests.remove_if([unit](const PathRequest& request) { return request.unit == unit; });
		pathing.results.remove_if([unit](const PathResult& result) { return result.unit == unit; });
		for (size_t i = 0; i < units.size(); ++i)
		{
			if (units.attackTarget[i] == unit)
			{
				units.attackTarget[i] = InvalidUnitId;
			}
			if (cooldowns.burstTarget[i] == unit)
			{
				cooldowns.burstTarget[i] = InvalidUnitId;
			}
		}
		for (auto& stored : carriers.storedUnits)
		{
			stored.remove(unit);
		}
		if (selection.selected == unit)
		{
			selection.selected = InvalidUnitId;
		}
		selection.selectedUnits.remove(unit);
		selection.formationUnits.remove(unit);
		if (selection.selectedUnits.isEmpty())
		{
			selection.selected = InvalidUnitId;
		}
		else
		{
			selection.selected = selection.selectedUnits.front();
		}
		if (selection.actionBuilder == unit)
		{
			selection.actionPlacementActive = false;
			selection.actionBuilder = InvalidUnitId;
			selection.actionId = InvalidBuildActionDefId;
			selection.actionLineDragging = false;
			selection.actionLineTargets.clear();
		}
		aiRuntime.attackWaveUnits.remove(unit);
		aiRuntime.resourceReclaimUnits.remove(unit);
		aiRuntime.guardUnits.remove(unit);
		aiRuntime.skirmishUnits.remove(unit);
		if (aiRuntime.attackTargetUnit == unit)
		{
			aiRuntime.attackTargetUnit = InvalidUnitId;
		}
		return true;
	}

	inline bool BattleWorld::reserveBuildCells(UnitId builder, BuildActionDefId actionId, const Array<Point>& cells)
	{
		if (!HasLiveBattleWorldUnit(*this, builder) || actionId == InvalidBuildActionDefId || cells.isEmpty())
		{
			return false;
		}

		HashSet<Point> uniqueCells;
		for (const Point& cell : cells)
		{
			if (!map.inBounds(cell.y, cell.x) || uniqueCells.contains(cell))
			{
				return false;
			}
			uniqueCells.insert(cell);
			for (const BuildCellReservation& reservation : buildCellReservations)
			{
				if (reservation.cell == cell)
				{
					return false;
				}
			}
		}

		for (const Point& cell : cells)
		{
			buildCellReservations << BuildCellReservation{ builder, actionId, cell };
		}
		return true;
	}

	inline void BattleWorld::releaseBuildCellReservation(UnitId builder, const QueuedBuildAction& entry)
	{
		if (!entry.hasTargetPosition)
		{
			return;
		}

		if (!entry.hasReservedCell)
		{
			return;
		}

		buildCellReservations.remove_if([builder, actionId = entry.actionId, cell = entry.reservedCell](const BuildCellReservation& reservation)
			{
				return reservation.builder == builder && reservation.actionId == actionId && reservation.cell == cell;
			});
	}

	inline void BattleWorld::cancelQueuedBuildAction(UnitId builder, const QueuedBuildAction& entry)
	{
		releaseBuildCellReservation(builder, entry);
		if (entry.costFaction != Faction::Player && entry.costFaction != Faction::Enemy)
		{
			return;
		}

		Array<int32>& amounts = (entry.costFaction == Faction::Enemy) ? resources.enemyAmounts : resources.playerAmounts;
		const auto add = [&amounts](ResourceDefId resourceId, int32 amount)
			{
				if (resourceId != InvalidResourceDefId && resourceId < amounts.size())
				{
					amounts[resourceId] += amount;
				}
			};
		add(entry.paidGoldResource, entry.paidGold);
		add(entry.paidTrustResource, entry.paidTrust);
		add(entry.paidFoodResource, entry.paidFood);
	}

	inline void BattleWorld::cancelBuildQueue(UnitId builder)
	{
		if (builder == InvalidUnitId || builder >= units.size())
		{
			return;
		}

		for (const QueuedBuildAction& entry : buildQueues.entries[builder])
		{
			cancelQueuedBuildAction(builder, entry);
		}
		if (buildQueues.hasPendingEntry[builder])
		{
			cancelQueuedBuildAction(builder, buildQueues.pendingEntry[builder]);
		}
		buildQueues.progressSec[builder] = 0.0;
		buildQueues.entries[builder].clear();
		buildQueues.pendingEntry[builder] = QueuedBuildAction{};
		buildQueues.hasPendingEntry[builder] = false;
		buildQueues.locked[builder] = false;
	}

	inline void BattleWorld::reset()
	{
		AudioAssetCache* const retainedAudioAssets = audioAssets;
		const ModContext* const retainedAudioMod = audioMod;
		*this = BattleWorld{};
		audioAssets = retainedAudioAssets;
		audioMod = retainedAudioMod;
	}

	// BattleWorld が保持する定義 index が指定スナップショット内で有効か検証する。
	inline bool HasValidBattleDefinitionIds(const BattleWorld& world, const DefinitionStores& defs)
	{
		if (!ValidateBattleWorldStoreInvariants(world).valid)
		{
			return false;
		}

		const auto isUnit = [&](UnitDefId id) { return id < defs.units.size(); };
		const auto isSkill = [&](SkillDefId id) { return id < defs.skills.size(); };
		const auto isAction = [&](BuildActionDefId id) { return id < defs.buildActions.size(); };
		const auto isResource = [&](ResourceDefId id) { return id < defs.resources.size(); };
		const auto isProfile = [&](AiProfileDefId id) { return id < defs.aiProfiles.size(); };

		for (const UnitDefId id : world.units.defId)
		{
			if (!isUnit(id)) return false;
		}
		for (const SkillDefId id : world.cooldowns.burstSkill)
		{
			if (id != InvalidSkillDefId && !isSkill(id)) return false;
		}
		for (const auto& queue : world.buildQueues.entries)
		{
			for (const auto& entry : queue)
			{
				if (!isAction(entry.actionId)) return false;
			}
		}
		for (size_t i = 0; i < world.buildQueues.pendingEntry.size(); ++i)
		{
			if (world.buildQueues.hasPendingEntry[i] && !isAction(world.buildQueues.pendingEntry[i].actionId)) return false;
		}
		for (const BuildCellReservation& reservation : world.buildCellReservations)
		{
			if (!isAction(reservation.actionId)) return false;
		}
		for (const SkillDefId id : world.projectiles.skill)
		{
			if (!isSkill(id)) return false;
		}
		for (const ResourceDefId id : world.resourceNodes.defId)
		{
			if (!isResource(id)) return false;
		}
		if (world.aiRuntime.profileId != InvalidAiProfileDefId && !isProfile(world.aiRuntime.profileId)) return false;
		if (world.selection.actionId != InvalidBuildActionDefId && !isAction(world.selection.actionId)) return false;
		if (world.selection.selectedSkill != InvalidSkillDefId && !isSkill(world.selection.selectedSkill)) return false;
		return true;
	}

	// BattleWorldのSoA形状、UnitId参照、定義IDがすべて有効か検証する。
	inline bool HasValidBattleWorldState(const BattleWorld& world, const DefinitionStores& defs)
	{
		return ValidateBattleWorldStoreInvariants(world).valid
			&& HasValidBattleDefinitionIds(world, defs);
	}
}
