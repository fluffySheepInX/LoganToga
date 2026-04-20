# pragma once
# include "TomlSchema.hpp"
# include "UnitTypes.hpp"

namespace MainSupport::UnitParameterSchemas
{
    // --- Enum codecs (TOML <-> enum) ------------------------------------

    inline constexpr TomlSchema::EnumCodec<MovementType> MovementTypeCodec{
        .parse = +[](StringView s) -> MovementType
        {
            return ((s == U"Tank") ? MovementType::Tank : MovementType::Infantry);
        },
        .format = +[](MovementType v) -> StringView
        {
            return ((v == MovementType::Tank) ? StringView{ U"Tank" } : StringView{ U"Infantry" });
        },
    };

    inline constexpr TomlSchema::EnumCodec<UnitAiRole> UnitAiRoleCodec{
        .parse = +[](StringView s) -> UnitAiRole
        {
            if (s == U"AssaultBase")
            {
                return UnitAiRole::AssaultBase;
            }
            if (s == U"Support")
            {
                return UnitAiRole::Support;
            }
            return UnitAiRole::SecureResources;
        },
        .format = +[](UnitAiRole v) -> StringView
        {
            switch (v)
            {
            case UnitAiRole::AssaultBase: return StringView{ U"AssaultBase" };
            case UnitAiRole::Support:     return StringView{ U"Support" };
            case UnitAiRole::SecureResources:
            default:                      return StringView{ U"SecureResources" };
            }
        },
    };

    inline constexpr TomlSchema::EnumCodec<UnitFootprintType> UnitFootprintTypeCodec{
        .parse = +[](StringView s) -> UnitFootprintType
        {
            return ((s == U"Capsule") ? UnitFootprintType::Capsule : UnitFootprintType::Circle);
        },
        .format = +[](UnitFootprintType v) -> StringView
        {
            return ((v == UnitFootprintType::Capsule) ? StringView{ U"Capsule" } : StringView{ U"Circle" });
        },
    };

    // --- Per-struct schemas ---------------------------------------------
    // Each field appears exactly ONCE. Adding / renaming a field is a
    // single-line change instead of touching Load + Save in two cpp files.
    //
    // IMPORTANT: The on-disk key order and formatting here must match the
    // previous hand-written Save*Group functions to keep TOML output
    // byte-compatible. Order also matters for human-readable diffs.

    template <class V, class P>
    void VisitUnitParameters(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec d{};

        v(U"MovementType",        p.movementType,        MovementTypeCodec);
        v(U"AiRole",              p.aiRole,              UnitAiRoleCodec);
        v(U"MaxHitPoints",        p.maxHitPoints,        d);
        v(U"MoveSpeed",           p.moveSpeed,           d);
        v(U"AttackRange",         p.attackRange,         d);
        v(U"StopDistance",        p.stopDistance,        d);
        v(U"AttackDamage",        p.attackDamage,        d);
        v(U"AttackInterval",      p.attackInterval,      d);
        v(U"VisionRange",         p.visionRange,         d);
        v(U"ManaCost",            p.manaCost,            d);
        v(U"FootprintType",       p.footprintType,       UnitFootprintTypeCodec);
        v(U"FootprintRadius",     p.footprintRadius,     d);
        v(U"FootprintHalfLength", p.footprintHalfLength, d);
    }

    template <class V, class P>
    void VisitExplosionSkillParameters(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec d{};
        constexpr TomlSchema::ColorFCodec c{};

        v(U"Radius",          p.radius,          d);
        v(U"UnitDamage",      p.unitDamage,      d);
        v(U"BaseDamage",      p.baseDamage,      d);
        v(U"CooldownSeconds", p.cooldownSeconds, d);
        v(U"GunpowderCost",   p.gunpowderCost,   d);
        v(U"EffectLifetime",  p.effectLifetime,  d);
        v(U"EffectThickness", p.effectThickness, d);
        v(U"EffectOffsetY",   p.effectOffsetY,   d);
        v(U"EffectColor",     p.effectColor,     c);
    }

    template <class V, class P>
    void VisitBuildMillSkillParameters(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec d{};

        v(U"ManaCost",      p.manaCost,      d);
        v(U"GunpowderCost", p.gunpowderCost, d);
        v(U"ForwardOffset", p.forwardOffset, d);
    }

    template <class V, class P>
    void VisitHealSkillParameters(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec d{};

        v(U"ManaCost", p.manaCost, d);
        v(U"Radius",   p.radius,   d);
        v(U"Amount",   p.amount,   d);
    }

    template <class V, class P>
    void VisitScoutSkillParameters(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec d{};

        v(U"GunpowderCost",    p.gunpowderCost,    d);
        v(U"DurationSeconds",  p.durationSeconds,  d);
        v(U"VisionMultiplier", p.visionMultiplier, d);
    }
}
