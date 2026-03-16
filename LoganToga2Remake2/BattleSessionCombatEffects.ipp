void BattleSession::applyUnitHpDelta(UnitState& target, const int32 hpDelta, const UnitArchetype sourceArchetype)
{
	if (!target.isAlive)
	{
		return;
	}

	const int32 previousHp = target.hp;
	target.hp = Clamp(target.hp - hpDelta, 0, target.maxHp);
	if ((hpDelta > 0) && (target.hp < previousHp))
	{
		target.damageFlashTime = 0.18;
		m_state.battleAudioEvents << BattleAudioEvent{
			.position = target.position,
			.targetOwner = target.owner,
			.sourceArchetype = sourceArchetype,
			.isBuilding = IsBuildingArchetype(target.archetype),
			.kind = ((target.hp <= 0) ? BattleAudioEventKind::Death : BattleAudioEventKind::Hit),
		};
	}

	if ((hpDelta > 0) && (target.hp <= 0))
	{
		if (target.archetype == UnitArchetype::Goliath)
		{
			triggerGoliathExplosion(target);
			return;
		}

		m_state.deathVisualEffects << DeathVisualEffect{
			.position = target.position,
			.radius = target.radius,
			.owner = target.owner,
			.archetype = target.archetype,
			.isBuilding = IsBuildingArchetype(target.archetype),
			.remainingTime = 0.36,
			.totalTime = 0.36,
		};

		target.hp = 0;
		target.isAlive = false;
		target.damageFlashTime = 0.0;
	}
}

void BattleSession::triggerGoliathExplosion(UnitState& unit)
{
	if (!unit.isAlive)
	{
		return;
	}

	const Vec2 explosionCenter = unit.position;
	const int32 effectFrames = GetAttackEffectFrames(UnitArchetype::Goliath);
	m_state.battleAudioEvents << BattleAudioEvent{
		.position = explosionCenter,
		.targetOwner = unit.owner,
		.sourceArchetype = UnitArchetype::Goliath,
		.isBuilding = false,
		.kind = BattleAudioEventKind::Explosion,
	};

	unit.hp = 0;
	unit.isAlive = false;
	unit.isSelected = false;
	unit.isDetonating = false;
	unit.detonationFramesRemaining = 0;
	unit.order.type = UnitOrderType::Idle;
	unit.order.targetUnitId.reset();
	unit.order.targetPoint = explosionCenter;
	unit.moveTarget = explosionCenter;
	BattleSessionInternal::ClearNavigationPath(unit);

	m_state.attackVisualEffects << AttackVisualEffect{
		.sourceUnitId = unit.id,
		.start = explosionCenter,
		.end = explosionCenter,
		.owner = unit.owner,
		.sourceArchetype = UnitArchetype::Goliath,
		.framesRemaining = effectFrames,
		.totalFrames = effectFrames,
		.areaRadius = GoliathExplosionRadius,
	};

	const Owner targetOwner = (unit.owner == Owner::Enemy) ? Owner::Player : Owner::Enemy;
	gatherNearbyUnitIndices(targetOwner, explosionCenter, GoliathExplosionRadius + m_spatialQueryCellSize, m_nearbyUnitIndicesScratch);
	for (const auto targetIndex : m_nearbyUnitIndicesScratch)
	{
		auto& target = m_state.units[targetIndex];
		if (!target.isAlive || (target.owner == unit.owner) || (target.id == unit.id))
		{
			continue;
		}

		const double splashRadius = GoliathExplosionRadius + (target.radius * 0.35);
		if (target.position.distanceFromSq(explosionCenter) > (splashRadius * splashRadius))
		{
			continue;
		}

		const int32 damage = IsBuildingArchetype(target.archetype)
			? Max(1, static_cast<int32>(std::round(GoliathExplosionDamage * GoliathBuildingDamageMultiplier)))
			: GoliathExplosionDamage;
		applyUnitHpDelta(target, damage, UnitArchetype::Goliath);
	}
}
