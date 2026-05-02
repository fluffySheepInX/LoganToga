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
                    entry.cost,
                    0,
                    baseSkill,
                    Palette::Slategray,
                    entry.visualScale
                };
            }
        }

        return UnitDef{ U"Home", U"Command Base", UnitRole::Base, 260, 10, 2, 0.0, 26.0, 0, 0, baseSkill, Palette::Slategray };
    }

    inline void LoadUnitDefinitions(DefinitionStores& defs, const UnitCatalog& unitCatalog)
    {
        defs.units.clear();
        defs.unitByTag.clear();

        const SkillDefId workerHit = defs.skillByTag.contains(U"worker_hit") ? defs.skillByTag.at(U"worker_hit") : InvalidSkillDefId;
        const SkillDefId sword = defs.skillByTag.contains(U"sword") ? defs.skillByTag.at(U"sword") : InvalidSkillDefId;
        const SkillDefId arrow = defs.skillByTag.contains(U"arrow") ? defs.skillByTag.at(U"arrow") : InvalidSkillDefId;
        const SkillDefId baseShot = defs.skillByTag.contains(U"base_shot") ? defs.skillByTag.at(U"base_shot") : InvalidSkillDefId;

        defs.addUnit({ U"worker", U"Worker", UnitRole::Worker, 42, 4, 0, 92.0, 12.0, 0, 7, workerHit, Palette::Dodgerblue });
        defs.addUnit({ U"soldier", U"Soldier", UnitRole::Soldier, 70, 9, 1, 78.0, 15.0, 35, 0, sword, Palette::Limegreen });
        defs.addUnit({ U"archer", U"Archer", UnitRole::Archer, 48, 7, 0, 74.0, 13.0, 45, 0, arrow, ColorF{ 0.0, 0.75, 1.0 } });
        defs.addUnit(BuildCommandBaseFromCatalog(unitCatalog, baseShot));

        for (const auto& entry : unitCatalog.entries)
        {
            if (entry.tag.isEmpty() || defs.unitByTag.contains(entry.tag))
            {
                continue;
            }

            const UnitRole role = ResolveCatalogUnitRole(entry);
            const SkillDefId skill = ResolveCatalogSkill(defs, entry, role);
            const int32 hp = (role == UnitRole::Base && entry.buildingHp > 0) ? entry.buildingHp : Max(1, entry.hp);
            const double speed = (role == UnitRole::Base) ? 0.0 : Max(24.0, static_cast<double>(entry.speed));
            const double radius = (role == UnitRole::Base) ? 26.0 : 14.0;
            const int32 gatherPower = (role == UnitRole::Worker) ? 7 : 0;

            defs.addUnit({
                entry.tag,
                entry.name.isEmpty() ? entry.tag : entry.name,
                role,
                hp,
                entry.attack,
                entry.defense,
                speed,
                radius,
                entry.cost,
                gatherPower,
                skill,
                (role == UnitRole::Base) ? Palette::Slategray : Palette::Seagreen,
                entry.visualScale
            });
        }
    }
}
