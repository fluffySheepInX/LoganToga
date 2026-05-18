#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../BuildLineIconOverrides.h"

namespace LT3
{
    inline bool EqualsIgnoreCaseOwnerTag(const String& a, const String& b)
    {
        return !a.isEmpty() && !b.isEmpty() && a.lowercased() == b.lowercased();
    }

    inline Array<String> NormalizeOwnerTags(const Array<String>& source)
    {
        Array<String> normalized;
        for (const auto& tag : source)
        {
            if (tag.isEmpty())
            {
                continue;
            }

            bool alreadyExists = false;
            for (const auto& existing : normalized)
            {
                if (EqualsIgnoreCaseOwnerTag(existing, tag))
                {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists)
            {
                normalized << tag;
            }
        }

        return normalized;
    }

    inline String BuildActionTomlEscape(StringView text)
    {
        String result;
        for (const char32 ch : text)
        {
            if (ch == U'\\')
            {
                result += U"\\\\";
            }
            else if (ch == U'\"')
            {
                result += U"\\\"";
            }
            else
            {
                result += ch;
            }
        }

        return result;
    }

    inline void WriteTomlStringArrayValue(TextWriter& writer, const Array<String>& values)
    {
        writer << U"[";
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                writer << U", ";
            }
            writer << U"\"" << BuildActionTomlEscape(values[i]) << U"\"";
        }
        writer << U"]";
    }

    inline String BuildTomlStringArrayValue(const Array<String>& values)
    {
        String result = U"[";
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                result += U", ";
            }

            result += U"\"" + BuildActionTomlEscape(values[i]) + U"\"";
        }

        result += U"]";
        return result;
    }

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

    inline Array<String> ReadOwnerTags(const TOMLValue& ownerTagsValue, const String& fallbackOwnerTag)
    {
        Array<String> ownerTags = NormalizeOwnerTags(ReadTomlStringArrayValue(ownerTagsValue));
        if (ownerTags.isEmpty() && !fallbackOwnerTag.isEmpty())
        {
            ownerTags << fallbackOwnerTag;
        }

        return NormalizeOwnerTags(ownerTags);
    }

    inline Array<String> NormalizeSpawnTags(const Array<String>& source)
    {
        return NormalizeOwnerTags(source);
    }

    inline Array<String> ReadSpawnTags(const TOMLValue& spawnTagsValue, const String& fallbackSpawnTag)
    {
        Array<String> spawnTags = NormalizeSpawnTags(ReadTomlStringArrayValue(spawnTagsValue));
        if (spawnTags.isEmpty() && !fallbackSpawnTag.isEmpty())
        {
            spawnTags << fallbackSpawnTag;
        }

        return NormalizeSpawnTags(spawnTags);
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

    inline bool TryReadBuildActionResult(const TOMLValue& resultValue, BuildActionResultType& resultType, String& resultTag, Array<String>& resultTags)
    {
        resultType = BuildActionResultType::None;
        resultTag.clear();
        resultTags.clear();

        for (const auto resultTable : resultValue.tableArrayView())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultTable[U"type"].getOr<String>(U""));
            if (parsedType != BuildActionResultType::None)
            {
                resultTags = ReadSpawnTags(resultTable[U"spawns"], resultTable[U"spawn"].getOr<String>(U""));
                resultTag = resultTags.isEmpty() ? U"" : resultTags.front();
                resultType = parsedType;
                return true;
            }
        }

        if (resultValue.isTable())
        {
            const BuildActionResultType parsedType = ParseBuildActionResultType(resultValue[U"type"].getOr<String>(U""));
            if (parsedType != BuildActionResultType::None)
            {
                resultTags = ReadSpawnTags(resultValue[U"spawns"], resultValue[U"spawn"].getOr<String>(U""));
                resultTag = resultTags.isEmpty() ? U"" : resultTags.front();
                resultType = parsedType;
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
            const Array<String> ownerTags = ReadOwnerTags(commandValue[U"owner_tags"], ownerTag);
            const String id = commandValue[U"id"].getOr<String>(U"");
            if (ownerTags.isEmpty() || id.isEmpty())
            {
                continue;
            }

            String resultTag;
            Array<String> resultTags;
            BuildActionResultType resultType = BuildActionResultType::None;
            const TOMLValue resultArray = commandValue[U"result"];
            if (!TryReadBuildActionResult(resultArray, resultType, resultTag, resultTags))
            {
                continue;
            }

            BuildActionDef action;
            action.tag = U"{}:{}"_fmt(ownerTags.front(), id);
            action.id = id;
            action.ownerTag = ownerTags.front();
            action.ownerTags = ownerTags;
            action.name = commandValue[U"name"].getOr<String>(id);
            action.description = commandValue[U"description"].getOr<String>(U"");
            action.icon = commandValue[U"icon"].getOr<String>(U"");
            action.lineIconHorizontal = commandValue[U"icon_horizontal"].getOr<String>(action.icon);
            action.lineIconDiagUpRight = commandValue[U"icon_diag_up_right"].getOr<String>(action.lineIconHorizontal);
            action.lineIconDiagUpLeft = commandValue[U"icon_diag_up_left"].getOr<String>(action.lineIconHorizontal);
            action.category = commandValue[U"category"].getOr<String>(U"");
            action.requirements = ReadTomlStringArrayValue(commandValue[U"requires"]);
            action.spawnTag = resultTag;
            action.spawnTags = NormalizeSpawnTags(resultTags);
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

            if (action.resultType == BuildActionResultType::Unit)
            {
                action.spawnUnits.clear();
                for (const auto& spawnTag : action.spawnTags)
                {
                    if (defs.unitByTag.contains(spawnTag))
                    {
                        action.spawnUnits << defs.unitByTag.at(spawnTag);
                    }
                }

                if (!action.spawnUnits.isEmpty())
                {
                    action.spawnUnit = action.spawnUnits.front();
                }
            }

            if (action.resultType == BuildActionResultType::Carrier || !action.resultTag.isEmpty())
            {
                defs.addBuildAction(action);
            }
        }

        ApplyBuildLineIconOverrides(defs.buildActions);
    }

    inline String BuildActionResultTypeToTomlValue(BuildActionResultType type)
    {
        switch (type)
        {
        case BuildActionResultType::Unit:
            return U"unit";
        case BuildActionResultType::Object:
            return U"obj";
        case BuildActionResultType::Carrier:
            return U"Carrier";
        default:
            return U"none";
        }
    }

    inline String BuildPlacementModeToTomlValue(BuildPlacementMode mode)
    {
        return (mode == BuildPlacementMode::Line) ? U"line" : U"point";
    }

    inline String BuildLineAxisModeToTomlValue(BuildLineAxisMode mode)
    {
        switch (mode)
        {
        case BuildLineAxisMode::HorizontalOnly:
            return U"horizontal_only";
        case BuildLineAxisMode::VerticalOnly:
            return U"vertical_only";
        default:
            return U"auto";
        }
    }

    inline bool SaveBuildActionDefinitions(const DefinitionStores& defs, String& statusText)
    {
        const FilePath buildActionPath = ResolveBuildActionTomlPath();
        FileSystem::CreateDirectories(FileSystem::ParentPath(buildActionPath));
        TextWriter writer{ buildActionPath };
        if (!writer)
        {
            statusText = U"Build action save failed: {}"_fmt(buildActionPath);
            return false;
        }

        String tomlText;
        bool firstCommand = true;

        for (const auto& action : defs.buildActions)
        {
            if (action.id.isEmpty())
            {
                continue;
            }

            Array<String> ownerTags = NormalizeOwnerTags(action.ownerTags);
            if (ownerTags.isEmpty() && !action.ownerTag.isEmpty())
            {
                ownerTags << action.ownerTag;
            }
            ownerTags = NormalizeOwnerTags(ownerTags);
            if (ownerTags.isEmpty())
            {
                statusText = U"Build action save failed: owner not set ({})"_fmt(action.name.isEmpty() ? action.id : action.name);
                return false;
            }

            Array<String> spawnTags = NormalizeSpawnTags(action.spawnTags);
            if (spawnTags.isEmpty() && !action.resultTag.isEmpty())
            {
                spawnTags << action.resultTag;
            }

            if (action.resultType == BuildActionResultType::Unit && spawnTags.isEmpty())
            {
                statusText = U"Build action save failed: spawn not set ({})"_fmt(action.name.isEmpty() ? action.id : action.name);
                return false;
            }

            if (!firstCommand)
            {
                tomlText += U"\n";
            }
            firstCommand = false;

            tomlText += U"[[commands]]\n";
            tomlText += U"owner_tag = \"" + BuildActionTomlEscape(ownerTags.front()) + U"\"\n";
            tomlText += U"owner_tags = " + BuildTomlStringArrayValue(ownerTags) + U"\n";
            tomlText += U"id = \"" + BuildActionTomlEscape(action.id) + U"\"\n";
            tomlText += U"name = \"" + BuildActionTomlEscape(action.name) + U"\"\n";
            tomlText += U"description = \"" + BuildActionTomlEscape(action.description) + U"\"\n";
            tomlText += U"icon = \"" + BuildActionTomlEscape(action.icon) + U"\"\n";
            tomlText += U"icon_horizontal = \"" + BuildActionTomlEscape(action.lineIconHorizontal) + U"\"\n";
            tomlText += U"icon_diag_up_right = \"" + BuildActionTomlEscape(action.lineIconDiagUpRight) + U"\"\n";
            tomlText += U"icon_diag_up_left = \"" + BuildActionTomlEscape(action.lineIconDiagUpLeft) + U"\"\n";
            tomlText += U"create_count = {}\n"_fmt(Max(1, action.createCount));
            tomlText += U"cost = { wood = " + Format(Max(0, action.costGold)) + U" }\n";
            tomlText += U"build_time = {}\n"_fmt(Max(0.0, action.buildTimeSec));
            tomlText += U"category = \"" + BuildActionTomlEscape(action.category) + U"\"\n";
            tomlText += U"requires = " + BuildTomlStringArrayValue(action.requirements) + U"\n";
            tomlText += U"is_move = {}\n"_fmt(action.isMove ? U"true" : U"false");
            tomlText += U"placement_mode = \"" + BuildPlacementModeToTomlValue(action.placementMode) + U"\"\n";
            tomlText += U"line_axis_mode = \"" + BuildLineAxisModeToTomlValue(action.lineAxisMode) + U"\"\n";
            tomlText += U"line_thickness_cells = {}\n"_fmt(Max(1, action.lineThicknessCells));
            tomlText += U"max_line_cells = {}\n"_fmt(Max(1, action.maxLineCells));
            tomlText += U"use_right_drag_placement = {}\n"_fmt(action.useRightDragPlacement ? U"true" : U"false");
            tomlText += U"[[commands.result]]\n";
            tomlText += U"type = \"" + BuildActionResultTypeToTomlValue(action.resultType) + U"\"\n";
            if (!spawnTags.isEmpty())
            {
                tomlText += U"spawn = \"" + BuildActionTomlEscape(spawnTags.front()) + U"\"\n";
                if (spawnTags.size() > 1)
                {
                    tomlText += U"spawns = " + BuildTomlStringArrayValue(spawnTags) + U"\n";
                }
            }
        }

        // TOML への保存では、行ごとに自動で改行コードが付与される書き方を使わず、必要な改行だけを文字列側で明示する。
        writer.write(tomlText);

        statusText = U"Saved build actions: {}"_fmt(buildActionPath);
        return true;
    }
}
