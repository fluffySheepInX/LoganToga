#pragma once
# include <Siv3D.hpp>

namespace LT3
{
    using UnitId = uint32;
    using UnitDefId = uint32;
    using SkillDefId = uint32;
    using BuildActionDefId = uint32;
    using ResourceDefId = uint32;
    using AiProfileDefId = uint32;
    using TileIndex = uint32;

    inline constexpr UnitId InvalidUnitId = UINT32_MAX;
    inline constexpr UnitDefId InvalidUnitDefId = UINT32_MAX;
    inline constexpr SkillDefId InvalidSkillDefId = UINT32_MAX;
    inline constexpr BuildActionDefId InvalidBuildActionDefId = UINT32_MAX;
    inline constexpr ResourceDefId InvalidResourceDefId = UINT32_MAX;
    inline constexpr AiProfileDefId InvalidAiProfileDefId = UINT32_MAX;

    enum class Faction : uint8
    {
        Player,
        Enemy,
        Neutral,
    };

    enum class UnitRole : uint8
    {
        Worker,
        Soldier,
        Archer,
        Base,
        Barrier,
    };

    enum class UnitTask : uint8
    {
        Idle,
        Moving,
        Attacking,
        Gathering,
        Building,
    };

    enum class ResourceKind : uint8
    {
        Gold,
        Trust,
        Food,
    };

    enum class SkillKind : uint8
    {
        Missile,
        Sword,
        Heal,
        Summon,
        Charge,
        Status,
    };

    enum class SkillProjectileMotion : uint8
    {
        Direct,
        Static,
        Arc,
        Parabola,
        Drop,
        Orbit,
        Swing,
    };

    enum class SkillProjectileCenter : uint8
    {
        Off,
        On,
        End,
    };

    enum class SkillSwingHitMode : uint8
    {
        Stop,
        MultiHitOnce,
    };

    enum class SkillBomVisual : uint8
    {
        Circle,
        Image,
    };

    enum class SkillBurstFireMode : uint8
    {
        Simultaneous,
        Staggered,
    };

    enum class SkillBurstOrderMode : uint8
    {
        Sequential,
        Random,
    };

    enum class SkillRayMode : uint8
    {
        None,
        Image,
        Line,
    };

    enum class BuildActionResultType : uint8
    {
        None,
        Unit,
        Object,
        Carrier,
    };

    enum class CarrierActionKind : uint8
    {
        Store,
        Release,
    };

    enum class BuildPlacementMode : uint8
    {
        Point,
        Line,
    };

    enum class BuildLineAxisMode : uint8
    {
        Auto,
        HorizontalOnly,
        VerticalOnly,
    };
}
