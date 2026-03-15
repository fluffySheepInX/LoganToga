void BattleSession::updateResourcePoints(const double deltaTime)
{
	for (auto& resourcePoint : m_state.resourcePoints)
	{
		const double captureRadiusSq = (resourcePoint.radius * resourcePoint.radius);
		bool playerPresent = false;
		bool enemyPresent = false;

		gatherNearbyUnitIndices(Owner::Player, resourcePoint.position, resourcePoint.radius, m_nearbyUnitIndicesScratch);
		for (const auto unitIndex : m_nearbyUnitIndicesScratch)
		{
			const auto& unit = m_state.units[unitIndex];
			if (!unit.isAlive || IsBuildingArchetype(unit.archetype))
			{
				continue;
			}

			if (unit.position.distanceFromSq(resourcePoint.position) <= captureRadiusSq)
			{
				playerPresent = true;
				break;
			}
		}

		gatherNearbyUnitIndices(Owner::Enemy, resourcePoint.position, resourcePoint.radius, m_nearbyUnitIndicesScratch);
		for (const auto unitIndex : m_nearbyUnitIndicesScratch)
		{
			const auto& unit = m_state.units[unitIndex];
			if (!unit.isAlive || IsBuildingArchetype(unit.archetype))
			{
				continue;
			}

			if (unit.position.distanceFromSq(resourcePoint.position) <= captureRadiusSq)
			{
				enemyPresent = true;
				break;
			}
		}

		if (playerPresent == enemyPresent)
		{
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
			continue;
		}

		const Owner occupier = playerPresent ? Owner::Player : Owner::Enemy;
		if (resourcePoint.owner == occupier)
		{
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
			continue;
		}

		if (!resourcePoint.capturingOwner || (*resourcePoint.capturingOwner != occupier))
		{
			resourcePoint.capturingOwner = occupier;
			resourcePoint.captureProgress = 0.0;
		}

		resourcePoint.captureProgress = Min(resourcePoint.captureProgress + deltaTime, resourcePoint.captureTime);
		if (resourcePoint.captureProgress >= resourcePoint.captureTime)
		{
			resourcePoint.owner = occupier;
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
		}
	}
}
