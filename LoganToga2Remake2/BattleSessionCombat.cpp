#include "BattleSession.h"

void BattleSession::updateCombat()
{
	struct DamageEvent
	{
		Vec2 sourcePosition = Vec2::Zero();
		int32 targetId = -1;
		int32 damage = 0;
		Owner owner = Owner::Player;
		UnitArchetype sourceArchetype = UnitArchetype::Soldier;
	};

	for (auto& effect : m_state.attackVisualEffects)
	{
		effect.framesRemaining = Max(effect.framesRemaining - 1, 0);
	}

	m_state.attackVisualEffects.remove_if([](const AttackVisualEffect& effect)
	{
		return (effect.framesRemaining <= 0);
	});

	Array<DamageEvent> damageEvents;

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		const UnitState* target = nullptr;

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			target = m_state.findUnit(*unit.order.targetUnitId);
			if (!(target && target->isAlive && IsEnemy(unit, *target)))
			{
				target = nullptr;
				unit.order.type = UnitOrderType::Idle;
				unit.order.targetUnitId.reset();
			}
		}

		if (!target)
		{
			target = findNearestEnemy(unit);
		}

		if (!target)
		{
			continue;
		}

		if (unit.position.distanceFrom(target->position) > unit.attackRange)
		{
			continue;
		}

		if (unit.attackCooldownRemaining > 0.0)
		{
			continue;
		}

		damageEvents << DamageEvent{ unit.position, target->id, unit.attackPower, unit.owner, unit.archetype };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : damageEvents)
	{
		if (auto* target = m_state.findUnit(event.targetId))
		{
			if (event.sourceArchetype == UnitArchetype::Turret)
			{
				m_state.attackVisualEffects << AttackVisualEffect{
					.start = event.sourcePosition,
					.end = target->position,
					.owner = event.owner,
					.sourceArchetype = event.sourceArchetype,
					.framesRemaining = 5,
					.totalFrames = 5,
				};
			}

			target->hp -= event.damage;
			if (target->hp <= 0)
			{
				target->hp = 0;
				target->isAlive = false;
			}
		}
	}
}
