#include "BattleSession.h"

void BattleSession::updateCombat()
{
	struct DamageEvent
	{
		int32 targetId = -1;
		int32 damage = 0;
	};

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

		damageEvents << DamageEvent{ target->id, unit.attackPower };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : damageEvents)
	{
		if (auto* target = m_state.findUnit(event.targetId))
		{
			target->hp -= event.damage;
			if (target->hp <= 0)
			{
				target->hp = 0;
				target->isAlive = false;
			}
		}
	}
}
