#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionStores.h"
# include "BattleWorldMap.h"

namespace LT3
{
	inline constexpr int32 DefaultBattleMapWidth = 12;
	inline constexpr int32 DefaultBattleMapHeight = 8;

	struct QueuedBuildAction
	{
		BuildActionDefId actionId = InvalidBuildActionDefId;
		Vec2 targetPosition{ 0, 0 };
		bool hasTargetPosition = false;
		String iconOverride;
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
		Array<UnitDefId> defId;
		Array<Faction> faction;
		Array<bool> alive;
		Array<UnitTask> task;
		Array<Vec2> position;
		Array<Vec2> targetPosition;
		Array<UnitId> attackTarget;
		Array<int32> hp;
		Array<int32> resourceTargetNode;
		Array<String> iconOverride;

		UnitId add(UnitDefId unitDef, Faction unitFaction, const Vec2& pos, const DefinitionStores& defs)
		{
			const UnitId id = static_cast<UnitId>(defId.size());
			defId << unitDef;
			faction << unitFaction;
			alive << true;
			task << UnitTask::Idle;
			position << pos;
			targetPosition << pos;
			attackTarget << InvalidUnitId;
			hp << defs.units[unitDef].hp;
			resourceTargetNode << -1;
			iconOverride << U"";
			return id;
		}

		[[nodiscard]] size_t size() const
		{
			return defId.size();
		}
	};

	struct PathRuntimeStore
	{
		Array<Array<Vec2>> waypoints;
		Array<int32> waypointIndex;
		Array<Vec2> destination;
		Array<uint32> pathMapRevision;
		Array<bool> hasPath;
		Array<bool> requestPending;
		Array<double> repathCooldownSec;
		Array<PathRequest> requests;
		Array<PathResult> results;
		int32 maxRequestsPerFrame = 8;

		void addUnit(const Vec2& initialPosition)
		{
			waypoints << Array<Vec2>{};
			waypointIndex << 0;
			destination << initialPosition;
			pathMapRevision << 0u;
			hasPath << false;
			requestPending << false;
			repathCooldownSec << 0.0;
		}

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
		Array<double> attackLeftSec;

		void addUnit()
		{
			attackLeftSec << Random(0.0, 0.25);
		}
	};

	struct BuildQueueStore
	{
		Array<double> progressSec;
		Array<Array<QueuedBuildAction>> entries;
		Array<QueuedBuildAction> pendingEntry;
		Array<bool> hasPendingEntry;
		Array<bool> locked;

		void addUnit()
		{
			progressSec << 0.0;
			entries << Array<QueuedBuildAction>{};
			pendingEntry << QueuedBuildAction{};
			hasPendingEntry << false;
			locked << false;
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
		Array<Array<UnitId>> storedUnits;

		void addUnit()
		{
			storedUnits << Array<UnitId>{};
		}
	};

	struct ResourceNodeStore
	{
		Array<ResourceDefId> defId;
		Array<Vec2> position;
		Array<int32> amount;
		Array<int32> incomePerSec;
		Array<Faction> owner;
		Array<double> captureProgress;

		void add(ResourceDefId resourceDef, const Vec2& pos, int32 value, int32 income)
		{
			defId << resourceDef;
			position << pos;
			amount << value;
			incomePerSec << income;
			owner << Faction::Neutral;
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
		Array<SkillDefId> skill;
		Array<SkillProjectileMotion> motion;
		Array<double> lifeSec;
		Array<double> ageSec;
		Array<double> maxLifeSec;
		Array<double> height;
		Array<double> angleRad;

		void add(const Vec2& pos, const Vec2& vel, UnitId targetUnit, Faction ownerFaction, SkillDefId skillDef)
		{
			add(pos, vel, pos, pos + vel, targetUnit, InvalidUnitId, ownerFaction, skillDef, SkillProjectileMotion::Direct, 2.5, 0.0);
		}

		void add(const Vec2& pos, const Vec2& vel, const Vec2& start, const Vec2& end, UnitId targetUnit, UnitId ownerUnit, Faction ownerFaction, SkillDefId skillDef, SkillProjectileMotion projectileMotion, double maxLife, double initialAngleRad)
		{
			position << pos;
			velocity << vel;
			startPosition << start;
			endPosition << end;
			target << targetUnit;
			owner << ownerUnit;
			faction << ownerFaction;
			skill << skillDef;
			motion << projectileMotion;
			lifeSec << maxLife;
			ageSec << 0.0;
			maxLifeSec << maxLife;
			height << 0.0;
			angleRad << initialAngleRad;
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
				skill[index]         = skill[last];
				motion[index]        = motion[last];
				lifeSec[index]       = lifeSec[last];
				ageSec[index]        = ageSec[last];
				maxLifeSec[index]    = maxLifeSec[last];
				height[index]        = height[last];
				angleRad[index]      = angleRad[last];
			}
			position.pop_back();
			velocity.pop_back();
			startPosition.pop_back();
			endPosition.pop_back();
			target.pop_back();
			owner.pop_back();
			faction.pop_back();
			skill.pop_back();
			motion.pop_back();
			lifeSec.pop_back();
			ageSec.pop_back();
			maxLifeSec.pop_back();
			height.pop_back();
			angleRad.pop_back();
		}
	};

	struct ResourceRuntimeStore
	{
		Array<int32> playerAmounts;
		Array<int32> enemyAmounts;
		double incomeTickAccumSec = 0.0;
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
	};

	struct BattleWorld
	{
		int32 mapWidth  = DefaultBattleMapWidth;
		int32 mapHeight = DefaultBattleMapHeight;
		UnitRuntimeStore  units;
		CooldownStore cooldowns;
		BuildQueueStore buildQueues;
		ResourceNodeStore resourceNodes;
		ProjectileStore   projectiles;
		PlacedObjectStore placedObjects;
		CarrierStore carriers;
		PathRuntimeStore pathing;
		ResourceRuntimeStore resources;
		SelectionStore    selection;
		BattleMapStore    map;
		bool enemyDirectorPaused = false;
		double enemySpawnTimerSec = 0.0;
		double elapsedSec         = 0.0;
		bool victory = false;
		bool defeat  = false;
	};
}
