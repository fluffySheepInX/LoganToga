#pragma once
# include <Siv3D.hpp>
# include "DefinitionStores.h"
# include "UnitCatalog.h"

namespace LT3
{
    inline String ReadQuotedTomlValue(const String& line)
    {
        const size_t firstQuote = line.indexOf(U'"');
        if (firstQuote == String::npos)
        {
            return U"";
        }

        const size_t secondQuote = line.indexOf(U'"', firstQuote + 1);
        if (secondQuote == String::npos || secondQuote <= firstQuote)
        {
            return U"";
        }

        return line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }

    inline HashTable<String, String> LoadBuildMenuSpawnFallbackMap(const FilePath& path)
    {
        HashTable<String, String> result;
        TextReader reader{ path };
        if (!reader)
        {
            return result;
        }

        String currentOwnerTag;
        String currentId;
        String currentSpawnTag;
        bool currentIsUnit = false;

        const auto flushCurrent = [&]()
        {
            if (!currentOwnerTag.isEmpty() && !currentId.isEmpty() && currentIsUnit && !currentSpawnTag.isEmpty())
            {
                result[U"{}:{}"_fmt(currentOwnerTag, currentId)] = currentSpawnTag;
            }
        };

        String line;
        while (reader.readLine(line))
        {
            const String trimmed = line.trimmed();
            if (trimmed.isEmpty() || trimmed.starts_with(U"#"))
            {
                continue;
            }

            if (trimmed == U"[[commands]]")
            {
                flushCurrent();
                currentOwnerTag.clear();
                currentId.clear();
                currentSpawnTag.clear();
                currentIsUnit = false;
                continue;
            }

            if (trimmed.starts_with(U"owner_tag"))
            {
                currentOwnerTag = ReadQuotedTomlValue(trimmed);
                continue;
            }
            if (trimmed.starts_with(U"id"))
            {
                currentId = ReadQuotedTomlValue(trimmed);
                continue;
            }
            if (trimmed.starts_with(U"result"))
            {
                currentIsUnit = trimmed.includes(U"type = \"unit\"") || trimmed.includes(U"type=\"unit\"");

                const size_t spawnPos = trimmed.indexOf(U"spawn");
                if (spawnPos != String::npos)
                {
                    currentSpawnTag = ReadQuotedTomlValue(trimmed.substr(spawnPos));
                }
            }
        }

        flushCurrent();
        return result;
    }

    inline FilePath ResolveBuildMenuTomlPath()
    {
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildMenu.toml";
        if (FileSystem::Exists(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildMenu.toml";
        if (FileSystem::Exists(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
    }

    inline UnitDefId EnsureBuildMenuSpawnUnit(DefinitionStores& defs, const String& tag)
    {
        if (tag.isEmpty())
        {
            return InvalidUnitDefId;
        }
        if (defs.unitByTag.contains(tag))
        {
            return defs.unitByTag.at(tag);
        }

        SkillDefId skill = InvalidSkillDefId;
        if (defs.skillByTag.contains(U"sword"))
        {
            skill = defs.skillByTag.at(U"sword");
        }
        return defs.addUnit({ tag, tag, UnitRole::Soldier, 62, 8, 1, 78.0, 14.0, 0, 0, skill, Palette::Limegreen });
    }

    inline void LoadBuildMenuDefinitions(DefinitionStores& defs)
    {
        const FilePath buildMenuPath = ResolveBuildMenuTomlPath();
        const TOMLReader toml{ buildMenuPath };
        if (!toml)
        {
            return;
        }

        const HashTable<String, String> spawnFallbackByCommand = LoadBuildMenuSpawnFallbackMap(buildMenuPath);

        defs.buildActions.clear();
        defs.buildActionByTag.clear();

        for (const auto commandValue : toml[U"commands"].tableArrayView())
        {
            const String ownerTag = commandValue[U"owner_tag"].getOr<String>(U"");
            const String id = commandValue[U"id"].getOr<String>(U"");
            if (ownerTag.isEmpty() || id.isEmpty())
            {
                continue;
            }

            String spawnTag;
            const TOMLValue resultArray = commandValue[U"result"];
            if (resultArray.isArray())
            {
                for (const auto resultValue : resultArray.arrayView())
                {
                    if (resultValue[U"type"].getOr<String>(U"") == U"unit")
                    {
                        spawnTag = resultValue[U"spawn"].getOr<String>(U"");
                        break;
                    }
                }
            }

            if (spawnTag.isEmpty())
            {
                const String commandKey = U"{}:{}"_fmt(ownerTag, id);
                if (spawnFallbackByCommand.contains(commandKey))
                {
                    spawnTag = spawnFallbackByCommand.at(commandKey);
                }
            }

            BuildActionDef action;
            action.tag = U"{}:{}"_fmt(ownerTag, id);
            action.ownerTag = ownerTag;
            action.name = commandValue[U"name"].getOr<String>(id);
            action.icon = commandValue[U"icon"].getOr<String>(U"");
            action.spawnUnit = EnsureBuildMenuSpawnUnit(defs, spawnTag);

            int32 costGold = 0;
            const TOMLValue costTable = commandValue[U"cost"];
            if (costTable.isTable())
            {
                costGold = costTable[U"wood"].getOr<int32>(costTable[U"gold"].getOr<int32>(0));
            }
            action.costGold = costGold;
            action.buildTimeSec = commandValue[U"build_time"].getOr<double>(0.0);

            if (action.spawnUnit != InvalidUnitDefId)
            {
                defs.addBuildAction(action);
            }
        }
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
                    Palette::Slategray
                };
            }
        }

        return UnitDef{ U"Home", U"Command Base", UnitRole::Base, 260, 10, 2, 0.0, 26.0, 0, 0, baseSkill, Palette::Slategray };
    }

    inline DefinitionStores CreateDefaultDefinitions(const UnitCatalog& unitCatalog)
    {
        DefinitionStores defs;

        const SkillDefId workerHit = defs.addSkill({ U"worker_hit", U"Tool Strike", 70.0, 0.75, 4, Palette::Khaki });
        const SkillDefId sword = defs.addSkill({ U"sword", U"Sword", 82.0, 0.65, 9, Palette::Orange });
        const SkillDefId arrow = defs.addSkill({ U"arrow", U"Arrow", 210.0, 1.05, 7, Palette::Skyblue });
        const SkillDefId baseShot = defs.addSkill({ U"base_shot", U"Base Shot", 230.0, 1.35, 10, Palette::Violet });

        defs.addUnit({ U"worker", U"Worker", UnitRole::Worker, 42, 4, 0, 92.0, 12.0, 0, 7, workerHit, Palette::Dodgerblue });
        defs.addUnit({ U"soldier", U"Soldier", UnitRole::Soldier, 70, 9, 1, 78.0, 15.0, 35, 0, sword, Palette::Limegreen });
        defs.addUnit({ U"archer", U"Archer", UnitRole::Archer, 48, 7, 0, 74.0, 13.0, 45, 0, arrow, ColorF{ 0.0, 0.75, 1.0 } });
        defs.addUnit(BuildCommandBaseFromCatalog(unitCatalog, baseShot));

        defs.addResource({ U"gold", U"Gold", ResourceKind::Gold, Palette::Gold });
        LoadBuildMenuDefinitions(defs);
        return defs;
    }

    inline DefinitionStores CreateDefaultDefinitions()
    {
        return CreateDefaultDefinitions(LoadUnitCatalog());
    }
}
