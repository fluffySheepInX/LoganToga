void BattleSession::assignFormationMove(const Array<int32>& unitIds, const Vec2& destination, const FormationType formation, const Vec2& facingDirection)
{
	removeUnitsFromSquads(unitIds);

	Array<int32> movableUnitIds;
	for (const auto unitId : unitIds)
	{
		if (const auto* unit = m_state.findUnit(unitId))
		{
			if (unit->isAlive && unit->canMove)
			{
				movableUnitIds << unitId;
			}
		}
	}

	if (movableUnitIds.isEmpty())
	{
		return;
	}

	const Array<Vec2> offsets = BattleSessionInternal::MakeFormationOffsets(movableUnitIds, m_state, destination, formation, facingDirection);
	const int32 squadId = m_state.nextSquadId++;
	SquadState squad;
	squad.id = squadId;
	squad.owner = Owner::Player;
	squad.formation = formation;
	squad.destination = destination;
	squad.unitIds = movableUnitIds;
	m_state.squads << squad;

	for (int32 i = 0; i < movableUnitIds.size(); ++i)
	{
		if (auto* unit = m_state.findUnit(movableUnitIds[i]))
		{
			unit->squadId = squadId;
			unit->formationOffset = (i < offsets.size()) ? offsets[i] : Vec2::Zero();
			unit->order.type = UnitOrderType::Move;
			unit->order.targetUnitId.reset();
			unit->order.targetPoint = destination + unit->formationOffset;
			unit->moveTarget = unit->order.targetPoint;
			BattleSessionInternal::InvalidateNavigationPath(*unit);
		}
	}
}

void BattleSession::removeUnitsFromSquads(const Array<int32>& unitIds)
{
	for (auto& squad : m_state.squads)
	{
		squad.unitIds.remove_if([&unitIds](const int32 squadUnitId)
		{
			return unitIds.contains(squadUnitId);
		});
	}

	for (const auto unitId : unitIds)
	{
		if (auto* unit = m_state.findUnit(unitId))
		{
			unit->squadId.reset();
			unit->formationOffset = Vec2::Zero();
		}
	}

	cleanupSquads();
}

void BattleSession::cleanupSquads()
{
	for (auto& squad : m_state.squads)
	{
		const int32 currentSquadId = squad.id;
		squad.unitIds.remove_if([this, currentSquadId](const int32 unitId)
		{
			const auto* unit = m_state.findUnit(unitId);
			return !(unit && unit->isAlive && unit->squadId && (*unit->squadId == currentSquadId));
		});
	}

	m_state.squads.remove_if([](const SquadState& squad)
	{
		return squad.unitIds.isEmpty();
	});
}
