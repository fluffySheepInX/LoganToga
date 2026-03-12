void BattleSession::updateResourcePoints(const double deltaTime)
{
	for (auto& resourcePoint : m_state.resourcePoints)
	{
		bool playerPresent = false;
		bool enemyPresent = false;

		for (const auto& unit : m_state.units)
		{
			if (!unit.isAlive || IsBuildingArchetype(unit.archetype))
			{
				continue;
			}

			if (!Circle{ resourcePoint.position, resourcePoint.radius }.intersects(unit.position))
			{
				continue;
			}

			if (unit.owner == Owner::Player)
			{
				playerPresent = true;
			}
			else if (unit.owner == Owner::Enemy)
			{
				enemyPresent = true;
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
