# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline void ApplyDirectDamageToUnit(BattleWorld& world, UnitId unit, int32 damage);

    /// <summary>
    /// 攻撃者と対象、防御値から最終ダメージを計算します。
    /// </summary>
    inline int32 ComputeSkillDamageAgainstUnit(const UnitDef& attackerDef, const UnitDef& targetDef, const SkillDef& skill)
    {
        const double rawDamage = (static_cast<double>(attackerDef.attack) * skill.damage) - static_cast<double>(targetDef.defense);
        return Max(1, static_cast<int32>(Math::Round(rawDamage)));
    }

    inline void ApplyDirectDamageToUnit(BattleWorld& world, UnitId unit, int32 damage)
    {
        if (!IsValidUnit(world, unit) || damage <= 0)
        {
            return;
        }

        world.units.hp[unit] -= damage;
        if (world.units.hp[unit] <= 0)
        {
            SetUnitAlive(world, unit, false);
            ClearSelectionIfUnitSelected(world, unit);
        }
    }

    /// <summary>
    /// 対象ユニットへ直接回復を適用します。
    /// </summary>
    inline void ApplyDirectHealToUnit(BattleWorld& world, const DefinitionStores& defs, UnitId unit, int32 amount)
    {
        if (!IsValidUnit(world, unit) || amount <= 0)
        {
            return;
        }

        const UnitDef& unitDef = defs.units[world.units.defId[unit]];
        world.units.hp[unit] = Min(world.units.hp[unit] + amount, unitDef.hp);
    }

    /// <summary>
    /// Heal 系スキルの回復量を計算します。
    /// </summary>
    inline int32 ComputeSkillHealAgainstUnit(const UnitDef& attackerDef, const SkillDef& skill)
    {
        const double rawHeal = static_cast<double>(attackerDef.attack) * Max(0.01, skill.damage);
        return Max(1, static_cast<int32>(Math::Round(rawHeal)));
    }

    /// <summary>
    /// 爆風対象に含めるべきユニットかを判定します。
    /// </summary>
    inline bool ShouldApplyBomDamageToUnit(const BattleWorld& world, UnitId attacker, UnitId unit, bool friendlyFire)
    {
        if (!IsValidUnit(world, unit) || unit == attacker)
        {
            return false;
        }

        if (friendlyFire)
        {
            return true;
        }

        return world.units.faction[unit] != world.units.faction[attacker];
    }

    /// <summary>
    /// 爆風半径内の対象へ一定ダメージを適用します。
    /// </summary>
    inline void ApplyBomDamage(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, const SkillDef& skill, const Vec2& impactPos)
    {
        if (!IsValidUnit(world, attacker))
        {
            return;
        }

        const auto applyDamage = [&](UnitId unit, int32 damage)
        {
            if (!IsValidUnit(world, unit) || damage <= 0)
            {
                return;
            }

            world.units.hp[unit] -= damage;
            if (world.units.hp[unit] <= 0)
            {
                SetUnitAlive(world, unit, false);
                ClearSelectionIfUnitSelected(world, unit);
            }
        };

        const UnitDef& attackerDef = defs.units[world.units.defId[attacker]];
        for (UnitId unit = 0; unit < static_cast<UnitId>(world.units.position.size()); ++unit)
        {
            if (!ShouldApplyBomDamageToUnit(world, attacker, unit, skill.bomFriendlyFire))
            {
                continue;
            }

            if (world.units.position[unit].distanceFrom(impactPos) > skill.bomRadius)
            {
                continue;
            }

            const UnitDef& targetDef = defs.units[world.units.defId[unit]];
            applyDamage(unit, ComputeSkillDamageAgainstUnit(attackerDef, targetDef, skill));
        }

        const int32 bomSelfDamage = Max(0, static_cast<int32>(Math::Round(static_cast<double>(attackerDef.attack) * skill.damage * skill.bomSelfDamageScale)));
        applyDamage(attacker, static_cast<int32>(Math::Round(skill.selfDamageOnHit)) + bomSelfDamage);
    }

    /// <summary>
    /// 着弾地点とスキル設定に応じて単体または爆風ダメージを適用します。
    /// </summary>
    inline void ApplyProjectileHit(BattleWorld& world, const DefinitionStores& defs, size_t projectileIndex, UnitId target, const Vec2& impactPos)
    {
        const UnitId attacker = world.projectiles.owner[projectileIndex];
        if (!IsValidUnit(world, attacker))
        {
            return;
        }

        const UnitDef& attackerDef = defs.units[world.units.defId[attacker]];
        const SkillDef& skill = defs.skills[world.projectiles.skill[projectileIndex]];
        if (skill.kind == SkillKind::Heal)
        {
            ApplyDirectHealToUnit(world, defs, target, ComputeSkillHealAgainstUnit(attackerDef, skill));
            ApplyDirectDamageToUnit(world, attacker, static_cast<int32>(Math::Round(skill.selfDamageOnHit)));
            return;
        }

        if (skill.bom && skill.bomRadius > 0.0)
        {
            ApplyBomDamage(world, defs, attacker, skill, impactPos);
            return;
        }

        const UnitDef& targetDef = defs.units[world.units.defId[target]];
        const int32 finalDamage = ComputeSkillDamageAgainstUnit(attackerDef, targetDef, skill);
        ApplyDirectDamageToUnit(world, target, finalDamage);
        ApplyDirectDamageToUnit(world, attacker, static_cast<int32>(Math::Round(skill.selfDamageOnHit)));
    }
}
