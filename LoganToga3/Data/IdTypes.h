#pragma once
# include <Siv3D.hpp>

namespace LT3
{
    using UnitId = uint32;
    using UnitDefId = uint32;
    using SkillDefId = uint32;
    using BuildActionDefId = uint32;
    using ResourceDefId = uint32;
    using TileIndex = uint32;

    inline constexpr UnitId InvalidUnitId = UINT32_MAX;
    inline constexpr UnitDefId InvalidUnitDefId = UINT32_MAX;
    inline constexpr SkillDefId InvalidSkillDefId = UINT32_MAX;
    inline constexpr BuildActionDefId InvalidBuildActionDefId = UINT32_MAX;
    inline constexpr ResourceDefId InvalidResourceDefId = UINT32_MAX;

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
}
