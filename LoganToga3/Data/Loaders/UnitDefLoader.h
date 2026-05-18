#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../UnitCatalog.h"

namespace LT3
{
    inline SkillDefId ResolveCatalogSkill(const DefinitionStores& defs, const UnitCatalogEntry& entry, UnitRole role)
    {
        for (const auto& skillTag : entry.skills)
        {
            if (defs.skillByTag.contains(skillTag))
            {
                return defs.skillByTag.at(skillTag);
            }
        }

        if (role == UnitRole::Base)
        {
            return defs.skillByTag.contains(U"base_shot") ? defs.skillByTag.at(U"base_shot") : InvalidSkillDefId;
        }

        if (entry.maintainRange > 150)
        {
            return defs.skillByTag.contains(U"arrow") ? defs.skillByTag.at(U"arrow") : InvalidSkillDefId;
        }

        return defs.skillByTag.contains(U"sword") ? defs.skillByTag.at(U"sword") : InvalidSkillDefId;
    }

    inline UnitRole ResolveCatalogUnitRole(const UnitCatalogEntry& entry)
    {
        if (entry.kind.lowercased() == U"building" || entry.classBuild.lowercased() == U"home")
        {
            return UnitRole::Base;
        }

        const String loweredTag = entry.tag.lowercased();
        if (loweredTag.includes(U"worker") || loweredTag.includes(U"kouhei"))
        {
            return UnitRole::Worker;
        }

        if (entry.maintainRange > 150)
        {
            return UnitRole::Archer;
        }

        return UnitRole::Soldier;
    }

    inline bool IsBarrierUnitEntry(const UnitCatalogEntry& entry)
    {
        const String loweredTag = entry.tag.lowercased();
        return loweredTag.includes(U"ironwall") || loweredTag.includes(U"barbed") || loweredTag.includes(U"wire");
    }

    inline UnitDef BuildCommandBaseFromCatalog(const UnitCatalog& catalog, SkillDefId baseSkill)
    {
        for (const auto& entry : catalog.entries)
        {
            if (entry.tag.lowercased() == U"home")
            {
                const int32 hp = (entry.buildingHp > 0) ? entry.buildingHp : Max(1, entry.hp);
                return UnitDef{
                    entry.tag,
                    entry.name.isEmpty() ? U"Command Base" : entry.name,
                    UnitRole::Base,
                    hp,
                    entry.attack,
                    entry.defense,
                    0.0,
                    26.0,
                    entry.goldCost,
                    0,
                    Max(0, entry.visionRadius),
                    baseSkill,
                    Palette::Slategray,
                    entry.visualScale,
                    entry.classBuild,
                    entry.classTag,
                    false
                };
            }
        }

        return UnitDef{ U"Home", U"Command Base", UnitRole::Base, 260, 10, 2, 0.0, 26.0, 0, 0, 6, baseSkill, Palette::Slategray, 1.0, U"home", U"", false };
    }

    inline void LoadUnitDefinitions(DefinitionStores& defs, const UnitCatalog& unitCatalog)
    {
        defs.units.clear();
        defs.unitByTag.clear();

        const SkillDefId workerHit = defs.skillByTag.contains(U"worker_hit") ? defs.skillByTag.at(U"worker_hit") : InvalidSkillDefId;
        const SkillDefId sword = defs.skillByTag.contains(U"sword") ? defs.skillByTag.at(U"sword") : InvalidSkillDefId;
        const SkillDefId arrow = defs.skillByTag.contains(U"arrow") ? defs.skillByTag.at(U"arrow") : InvalidSkillDefId;
        const SkillDefId baseShot = defs.skillByTag.contains(U"base_shot") ? defs.skillByTag.at(U"base_shot") : InvalidSkillDefId;

        for (const auto& entry : unitCatalog.entries)
        {
            if (entry.tag.isEmpty() || defs.unitByTag.contains(entry.tag))
            {
                continue;
            }

            UnitRole role = ResolveCatalogUnitRole(entry);
            if (IsBarrierUnitEntry(entry))
            {
                role = UnitRole::Barrier;
            }
            const SkillDefId skill = ResolveCatalogSkill(defs, entry, role);
            const int32 hp = (role == UnitRole::Base && entry.buildingHp > 0) ? entry.buildingHp : Max(1, entry.hp);
            const double speed = (role == UnitRole::Base || role == UnitRole::Barrier)
                ? 0.0
                : Max(24.0, static_cast<double>((entry.move > 0) ? entry.move : entry.speed));
            const double radius = (role == UnitRole::Base || role == UnitRole::Barrier) ? 26.0 : 14.0;
            const bool blocksTileMovement = (role == UnitRole::Barrier);

            defs.addUnit({
                entry.tag,
                entry.name.isEmpty() ? entry.tag : entry.name,
                role,
                hp,
                entry.attack,
                entry.defense,
                speed,
                radius,
                entry.goldCost,
                1,
                Max(0, entry.visionRadius),
                skill,
                (role == UnitRole::Base) ? Palette::Slategray : Palette::Seagreen,
                entry.visualScale,
                entry.classBuild,
                entry.classTag,
                blocksTileMovement
            });
        }
    }
}
