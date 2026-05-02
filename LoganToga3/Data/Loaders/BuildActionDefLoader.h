#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"

namespace LT3
{
    inline Array<String> ReadTomlStringArrayValue(const TOMLValue& value)
    {
        Array<String> result;
        if (!value.isArray())
        {
            return result;
        }

        for (const auto item : value.arrayView())
        {
            if (const Optional<String> text = item.getOpt<String>())
            {
                result << *text;
            }
        }

        return result;
    }

    inline BuildActionResultType ParseBuildActionResultType(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"unit")
        {
            return BuildActionResultType::Unit;
        }
        if (lowered == U"obj")
        {
            return BuildActionResultType::Object;
        }
        if (lowered == U"carrier")
        {
            return BuildActionResultType::Carrier;
        }

        return BuildActionResultType::None;
    }

    inline bool TryReadBuildActionResult(const TOMLValue& resultValue, BuildActionResultType& resultType, String& spawnTag)
    {
        resultType = BuildActionResultType::None;
        spawnTag.clear();

        for (const auto resultTable : resultValue.tableArrayView())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultTable[U"type"].getOr<String>(U""));
            if (parsedType == BuildActionResultType::Unit)
            {
                resultType = parsedType;
                spawnTag = resultTable[U"spawn"].getOr<String>(U"");
                return !spawnTag.isEmpty();
            }
        }

        if (resultValue.isTable())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultValue[U"type"].getOr<String>(U""));
            if (parsedType == BuildActionResultType::Unit)
            {
                resultType = parsedType;
                spawnTag = resultValue[U"spawn"].getOr<String>(U"");
                return !spawnTag.isEmpty();
            }
        }

        return false;
    }

    inline FilePath ResolveBuildActionTomlPath()
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

    inline void LoadBuildActionDefinitions(DefinitionStores& defs)
    {
        const FilePath buildActionPath = ResolveBuildActionTomlPath();
        const TOMLReader toml{ buildActionPath };
        if (!toml)
        {
            return;
        }

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
            BuildActionResultType resultType = BuildActionResultType::None;
            const TOMLValue resultArray = commandValue[U"result"];
            TryReadBuildActionResult(resultArray, resultType, spawnTag);

            if (resultType != BuildActionResultType::Unit || spawnTag.isEmpty())
            {
                continue;
            }

            const auto unitIt = defs.unitByTag.find(spawnTag);
            if (unitIt == defs.unitByTag.end())
            {
                continue;
            }

            BuildActionDef action;
            action.tag = U"{}:{}"_fmt(ownerTag, id);
            action.ownerTag = ownerTag;
            action.name = commandValue[U"name"].getOr<String>(id);
            action.description = commandValue[U"description"].getOr<String>(U"");
            action.icon = commandValue[U"icon"].getOr<String>(U"");
            action.category = commandValue[U"category"].getOr<String>(U"");
            action.requirements = ReadTomlStringArrayValue(commandValue[U"requires"]);
            action.spawnTag = spawnTag;
            action.resultType = resultType;
            action.spawnUnit = unitIt->second;
            action.createCount = Max(1, commandValue[U"create_count"].getOr<int32>(1));

            int32 costGold = 0;
            const TOMLValue costTable = commandValue[U"cost"];
            if (costTable.isTable())
            {
                costGold = costTable[U"wood"].getOr<int32>(costTable[U"gold"].getOr<int32>(0));
            }
            action.costGold = costGold;
            action.buildTimeSec = commandValue[U"build_time"].getOr<double>(0.0);
            action.isMove = commandValue[U"is_move"].getOr<bool>(false);

            if (action.spawnUnit != InvalidUnitDefId)
            {
                defs.addBuildAction(action);
            }
        }
    }
}
