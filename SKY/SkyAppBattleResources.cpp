# include "SkyAppBattleInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		void AddResource(ResourceStock& stock, const ResourceType type, const double amount)
		{
			switch (type)
			{
			case ResourceType::Budget:
				stock.budget += amount;
				return;

			case ResourceType::Gunpowder:
				stock.gunpowder += amount;
				return;

			case ResourceType::Mana:
				stock.mana += amount;
				return;

			default:
				return;
			}
		}

		[[nodiscard]] double GetResourceIncomeAmount(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return BudgetAreaIncome;

			case ResourceType::Gunpowder:
				return GunpowderAreaIncome;

			case ResourceType::Mana:
				return ManaAreaIncome;

			default:
				return 0.0;
			}
		}
	}

	namespace BattleDetail
	{
		void ResetResourceState(SkyAppState& state)
		{
               state.playerResources = state.initialPlayerResources;
			state.enemyResources = {};
			state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
		}

		void UpdateResourceAreas(SkyAppState& state)
		{
			const double deltaTime = Scene::DeltaTime();

			if (state.resourceAreaStates.size() != state.mapData.resourceAreas.size())
			{
				state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
			}

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const ResourceArea& area = state.mapData.resourceAreas[i];
				ResourceAreaState& areaState = state.resourceAreaStates[i];
				int32 playerInside = 0;
				int32 enemyInside = 0;
				const double radiusSq = Square(area.radius);

				for (const auto& sapper : state.spawnedSappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++playerInside;
					}
				}

				for (const auto& sapper : state.enemySappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++enemyInside;
					}
				}

				Optional<UnitTeam> occupyingTeam;
				if ((0 < playerInside) && (enemyInside == 0))
				{
					occupyingTeam = UnitTeam::Player;
				}
				else if ((0 < enemyInside) && (playerInside == 0))
				{
					occupyingTeam = UnitTeam::Enemy;
				}

				if (occupyingTeam)
				{
					if (areaState.ownerTeam && (*areaState.ownerTeam == *occupyingTeam))
					{
						areaState.capturingTeam.reset();
						areaState.captureProgress = ResourceAreaCaptureSeconds;
					}
					else
					{
						if ((not areaState.capturingTeam) || (*areaState.capturingTeam != *occupyingTeam))
						{
							areaState.capturingTeam = *occupyingTeam;
							areaState.captureProgress = 0.0;
						}

						areaState.captureProgress += deltaTime;

						if (ResourceAreaCaptureSeconds <= areaState.captureProgress)
						{
							areaState.ownerTeam = *occupyingTeam;
							areaState.capturingTeam.reset();
							areaState.captureProgress = ResourceAreaCaptureSeconds;
							areaState.incomeProgress = 0.0;
						}
					}
				}
				else if (areaState.capturingTeam)
				{
					areaState.captureProgress = Max(0.0, (areaState.captureProgress - deltaTime));
					if (areaState.captureProgress <= 0.0)
					{
						areaState.capturingTeam.reset();
					}
				}

				if (areaState.ownerTeam)
				{
					areaState.incomeProgress += deltaTime;

					while (ResourceAreaIncomeIntervalSeconds <= areaState.incomeProgress)
					{
						areaState.incomeProgress -= ResourceAreaIncomeIntervalSeconds;
						if (*areaState.ownerTeam == UnitTeam::Player)
						{
							AddResource(state.playerResources, area.type, GetResourceIncomeAmount(area.type));
						}
						else
						{
							AddResource(state.enemyResources, area.type, GetResourceIncomeAmount(area.type));
						}
					}
				}
				else
				{
					areaState.incomeProgress = 0.0;
				}
			}
		}
	}
}
