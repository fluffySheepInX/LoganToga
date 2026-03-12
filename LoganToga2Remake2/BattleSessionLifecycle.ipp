void BattleSession::cleanupDeadUnits()
{
	Array<int32> deadUnitIds;
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			unit.isSelected = false;
			deadUnitIds << unit.id;
		}
	}

	cancelPendingConstructionOrders(deadUnitIds, true);

	m_state.units.remove_if([](const UnitState& unit)
	{
		return !unit.isAlive && !IsBuildingArchetype(unit.archetype);
	});

	invalidateUnitIndex();

	m_state.buildings.remove_if([this](const BuildingState& building)
	{
		const auto* unit = findCachedUnit(building.unitId);
		return !(unit && unit->isAlive);
	});

	cleanupSquads();
}

void BattleSession::updateVictoryState()
{
	bool hasPlayerBase = false;
	bool hasEnemyBase = false;

	for (const auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.archetype != UnitArchetype::Base))
		{
			continue;
		}

		if (unit.owner == Owner::Player)
		{
			hasPlayerBase = true;
		}
		else
		{
			hasEnemyBase = true;
		}
	}

	if (!hasEnemyBase)
	{
		m_state.winner = Owner::Player;
	}
	else if (!hasPlayerBase)
	{
		m_state.winner = Owner::Enemy;
	}
}

int32 BattleSession::spawnUnit(const Owner owner, const UnitArchetype archetype, const Vec2& position)
{
	const int32 id = m_state.nextUnitId++;
	m_state.units << makeUnit(id, owner, archetype, position);
	invalidateUnitIndex();
	if (IsBuildingArchetype(archetype))
	{
		m_state.buildings << BuildingState{ id };
	}
	return id;
}

UnitState BattleSession::makeUnit(const int32 id, const Owner owner, const UnitArchetype archetype, const Vec2& position) const
{
	const auto& definition = getUnitDefinition(archetype);

	UnitState unit;
	unit.id = id;
	unit.owner = owner;
	unit.archetype = archetype;
	unit.position = position;
	unit.moveTarget = position;
	unit.radius = definition.radius;
	unit.moveSpeed = definition.moveSpeed;
	unit.attackRange = definition.attackRange;
	unit.attackCooldown = definition.attackCooldown;
	unit.attackPower = definition.attackPower;
	unit.hp = definition.hp;
	unit.maxHp = definition.hp;
	unit.canMove = definition.canMove;
	if ((owner == Owner::Player) && FindPlayerUnitModifier(m_config, archetype))
	{
		const auto& modifier = *FindPlayerUnitModifier(m_config, archetype);
		unit.moveSpeed += modifier.moveSpeedDelta;
		unit.attackRange += modifier.attackRangeDelta;
		unit.attackPower += modifier.attackPowerDelta;
		unit.hp += modifier.hpDelta;
		unit.maxHp += modifier.hpDelta;
	}

	return unit;
}
