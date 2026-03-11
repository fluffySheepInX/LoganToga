#include "BattleSession.h"

namespace
{
	[[nodiscard]] int32 GetAttackEffectFrames(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return 5;
		case UnitArchetype::Soldier:
			return 6;
		case UnitArchetype::Archer:
			return 9;
		case UnitArchetype::Turret:
			return 5;
		default:
			return 4;
		}
	}
}

void BattleSession::updateCombat()
{
	struct DamageEvent
	{
		int32 sourceUnitId = -1;
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

		damageEvents << DamageEvent{ unit.id, unit.position, target->id, unit.attackPower, unit.owner, unit.archetype };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : damageEvents)
	{
		if (auto* target = m_state.findUnit(event.targetId))
		{
			const int32 effectFrames = GetAttackEffectFrames(event.sourceArchetype);
			m_state.attackVisualEffects << AttackVisualEffect{
				.sourceUnitId = event.sourceUnitId,
				.start = event.sourcePosition,
				.end = target->position,
				.owner = event.owner,
				.sourceArchetype = event.sourceArchetype,
				.framesRemaining = effectFrames,
				.totalFrames = effectFrames,
			};

			target->hp -= event.damage;
			if (target->hp <= 0)
			{
				target->hp = 0;
				target->isAlive = false;
			}
		}
	}
}
