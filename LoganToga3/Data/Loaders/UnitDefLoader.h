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

        if (!entry.skills.isEmpty())
        {
            return InvalidSkillDefId;
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
        if (entry.kind.lowercased() == U"building" || entry.building_category.lowercased() == U"home")
        {
            return UnitRole::Base;
        }

        const String loweredTag = entry.unit_id.lowercased();
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
        const String loweredTag = entry.unit_id.lowercased();
        return loweredTag.includes(U"ironwall") || loweredTag.includes(U"barbed") || loweredTag.includes(U"wire");
    }

    inline UnitDef BuildCommandBaseFromCatalog(const UnitCatalog& catalog, SkillDefId baseSkill)
    {
        for (const auto& entry : catalog.entries)
        {
            if (entry.unit_id.lowercased() == U"home")
            {
                const int32 hp = (entry.buildingHp > 0) ? entry.buildingHp : Max(1, entry.hp);
                UnitDef def;
                def.unit_id = entry.unit_id;
                def.name = entry.name.isEmpty() ? U"Command Base" : entry.name;
                def.role = UnitRole::Base;
                def.hp = hp;
                def.attack = entry.attack;
                def.defense = entry.defense;
                def.speed = 0.0;
                def.radius = 26.0;
                def.gatherPower = 0;
                def.visionRadiusCells = Max(0, entry.visionRadius);
                def.skill = baseSkill;
                def.color = Palette::Slategray;
                def.visualScale = entry.visualScale;
                def.unique = entry.unique;
                def.uniqueRespawnAllowed = entry.uniqueRespawnAllowed;
                def.building_category = entry.building_category;
                def.unit_family = entry.unit_family;
                def.blocksTileMovement = false;
                return def;
            }
        }

        UnitDef fallback;
        fallback.unit_id = U"Home";
        fallback.name = U"Command Base";
        fallback.role = UnitRole::Base;
        fallback.hp = 260;
        fallback.attack = 10;
        fallback.defense = 2;
        fallback.speed = 0.0;
        fallback.radius = 26.0;
        fallback.gatherPower = 0;
        fallback.visionRadiusCells = 6;
        fallback.skill = baseSkill;
        fallback.color = Palette::Slategray;
        fallback.visualScale = 1.0;
        fallback.unique = false;
        fallback.uniqueRespawnAllowed = false;
        fallback.building_category = U"home";
        fallback.unit_family = U"";
        fallback.blocksTileMovement = false;
        return fallback;
    }

    inline void LoadUnitDefinitions(DefinitionStores& defs, const UnitCatalog& unitCatalog)
    {
        defs.units.clear();
        defs.unitByTag.clear();

        for (const auto& entry : unitCatalog.entries)
        {
            if (entry.unit_id.isEmpty() || defs.unitByTag.contains(entry.unit_id))
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
                entry.unit_id,
                entry.name.isEmpty() ? entry.unit_id : entry.name,
                role,
                hp,
                entry.attack,
                entry.defense,
                speed,
                radius,
                1,
                Max(0, entry.visionRadius),
                skill,
                (role == UnitRole::Base) ? Palette::Slategray : Palette::Seagreen,
                entry.visualScale,
                entry.unique,
                entry.uniqueRespawnAllowed,
                entry.building_category,
                entry.unit_family,
                blocksTileMovement
            });
        }
    }
}
