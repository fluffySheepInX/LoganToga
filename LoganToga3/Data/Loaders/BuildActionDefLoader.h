#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../BuildLineIconOverrides.h"

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

    inline BuildPlacementMode ParseBuildPlacementMode(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"line")
        {
            return BuildPlacementMode::Line;
        }

        return BuildPlacementMode::Point;
    }

    inline BuildLineAxisMode ParseBuildLineAxisMode(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"horizontal" || lowered == U"horizontal_only")
        {
            return BuildLineAxisMode::HorizontalOnly;
        }
        if (lowered == U"vertical" || lowered == U"vertical_only")
        {
            return BuildLineAxisMode::VerticalOnly;
        }

        return BuildLineAxisMode::Auto;
    }

    inline bool TryReadBuildActionResult(const TOMLValue& resultValue, BuildActionResultType& resultType, String& resultTag)
    {
        resultType = BuildActionResultType::None;
        resultTag.clear();

        for (const auto resultTable : resultValue.tableArrayView())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultTable[U"type"].getOr<String>(U""));
            if (parsedType != BuildActionResultType::None)
            {
                resultType = parsedType;
                resultTag = resultTable[U"spawn"].getOr<String>(U"");
                return true;
            }
        }

        if (resultValue.isTable())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultValue[U"type"].getOr<String>(U""));
            if (parsedType != BuildActionResultType::None)
            {
                resultType = parsedType;
                resultTag = resultValue[U"spawn"].getOr<String>(U"");
                return true;
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

            String resultTag;
            BuildActionResultType resultType = BuildActionResultType::None;
            const TOMLValue resultArray = commandValue[U"result"];
            if (!TryReadBuildActionResult(resultArray, resultType, resultTag))
            {
                continue;
            }

            BuildActionDef action;
            action.tag = U"{}:{}"_fmt(ownerTag, id);
            action.id = id;
            action.ownerTag = ownerTag;
            action.name = commandValue[U"name"].getOr<String>(id);
            action.description = commandValue[U"description"].getOr<String>(U"");
            action.icon = commandValue[U"icon"].getOr<String>(U"");
            action.lineIconHorizontal = commandValue[U"icon_horizontal"].getOr<String>(action.icon);
            action.lineIconDiagUpRight = commandValue[U"icon_diag_up_right"].getOr<String>(action.lineIconHorizontal);
            action.lineIconDiagUpLeft = commandValue[U"icon_diag_up_left"].getOr<String>(action.lineIconHorizontal);
            action.category = commandValue[U"category"].getOr<String>(U"");
            action.requirements = ReadTomlStringArrayValue(commandValue[U"requires"]);
            action.spawnTag = resultTag;
            action.resultTag = resultTag;
            action.resultType = resultType;
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
            action.placementMode = ParseBuildPlacementMode(commandValue[U"placement_mode"].getOr<String>(U"point"));
            action.lineAxisMode = ParseBuildLineAxisMode(commandValue[U"line_axis_mode"].getOr<String>(U"auto"));
            action.lineThicknessCells = Max(1, commandValue[U"line_thickness_cells"].getOr<int32>(1));
            action.maxLineCells = Max(1, commandValue[U"max_line_cells"].getOr<int32>(12));
            action.useRightDragPlacement = commandValue[U"use_right_drag_placement"].getOr<bool>(action.placementMode == BuildPlacementMode::Line);

            if (action.resultType == BuildActionResultType::Unit && defs.unitByTag.contains(resultTag))
            {
                action.spawnUnit = defs.unitByTag.at(resultTag);
            }

            if (action.resultType == BuildActionResultType::Carrier || !action.resultTag.isEmpty())
            {
                defs.addBuildAction(action);
            }
        }

        ApplyBuildLineIconOverrides(defs.buildActions);
    }
}
