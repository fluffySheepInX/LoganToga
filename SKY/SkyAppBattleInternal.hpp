# pragma once
# include "SkyAppLoopInternal.hpp"

namespace SkyAppFlow
{
	namespace BattleDetail
	{
		void ResetResourceState(SkyAppState& state);
     void SpawnEnemyUnit(SkyAppState& state, MainSupport::SapperUnitType unitType, bool moveImmediately);
		void SpawnEnemyReinforcement(SkyAppState& state, bool moveImmediately);
		void UpdateBaseCombat(Array<MainSupport::SpawnedSapper>& attackers, const Vec3& basePosition, double& baseHitPoints);
		void UpdateMillDefense(SkyAppState& state);
		void UpdateResourceAreas(SkyAppState& state);
	}
}
