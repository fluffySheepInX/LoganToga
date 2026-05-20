#pragma once
# include <Siv3D.hpp>
# include "../BuildLineIconOverrides.h"
# include "BuildActionDefTomlHelpers.h"

namespace LT3
{
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

        for (const auto commandValue : toml[BuildActionToml::KeyCommands].tableArrayView())
        {
            const String ownerTag = commandValue[BuildActionToml::KeyOwnerTag].getOr<String>(U"");
            const Array<String> ownerTags = ReadOwnerTags(commandValue[BuildActionToml::KeyOwnerTags], ownerTag);
            const String id = commandValue[BuildActionToml::KeyId].getOr<String>(U"");
            if (ownerTags.isEmpty() || id.isEmpty())
            {
                continue;
            }

            String resultTag;
            Array<String> resultTags;
            BuildActionResultType resultType = BuildActionResultType::None;
            const TOMLValue resultArray = commandValue[BuildActionToml::KeyResult];
            if (!TryReadBuildActionResult(resultArray, resultType, resultTag, resultTags))
            {
                continue;
            }

            BuildActionDef action;
            action.tag = id;
            action.id = id;
            action.ownerTag = ownerTags.front();
            action.ownerTags = ownerTags;
            action.name = commandValue[BuildActionToml::KeyName].getOr<String>(id);
            action.description = commandValue[BuildActionToml::KeyDescription].getOr<String>(U"");
            action.iconLayers = ReadIconLayers(commandValue[BuildActionToml::KeyIcons], commandValue[BuildActionToml::KeyIcon].getOr<String>(U""));
            action.icon = action.iconLayers.isEmpty() ? U"" : action.iconLayers.front();
            action.lineIconHorizontal = commandValue[BuildActionToml::KeyIconHorizontal].getOr<String>(action.icon);
            action.lineIconDiagUpRight = commandValue[BuildActionToml::KeyIconDiagUpRight].getOr<String>(action.lineIconHorizontal);
            action.lineIconDiagUpLeft = commandValue[BuildActionToml::KeyIconDiagUpLeft].getOr<String>(action.lineIconHorizontal);
            action.category = commandValue[BuildActionToml::KeyCategory].getOr<String>(U"");
            action.requirements = ReadTomlStringArrayValue(commandValue[BuildActionToml::KeyRequires]);
            action.spawnTag = resultTag;
            action.spawnTags = NormalizeSpawnTags(resultTags);
            action.resultTag = resultTag;
            action.resultType = resultType;
            action.createCount = Max<int32>(1, commandValue[BuildActionToml::KeyCreateCount].getOr<int32>(1));

            int32 costGold = 0;
            const TOMLValue costTable = commandValue[BuildActionToml::KeyCost];
            if (costTable.isTable())
            {
                costGold = costTable[String{ BuildActionToml::KeyWood }].getOr<int32>(costTable[String{ BuildActionToml::KeyGold }].getOr<int32>(0));
            }
            action.costGold = costGold;
            action.buildTimeSec = commandValue[BuildActionToml::KeyBuildTime].getOr<double>(0.0);
            action.isMove = commandValue[BuildActionToml::KeyIsMove].getOr<bool>(false);
            action.placementMode = ParseBuildPlacementMode(commandValue[BuildActionToml::KeyPlacementMode].getOr<String>(U"point"));
            action.lineAxisMode = ParseBuildLineAxisMode(commandValue[BuildActionToml::KeyLineAxisMode].getOr<String>(U"auto"));
            action.lineThicknessCells = Max<int32>(1, commandValue[BuildActionToml::KeyLineThicknessCells].getOr<int32>(1));
            action.maxLineCells = Max<int32>(1, commandValue[BuildActionToml::KeyMaxLineCells].getOr<int32>(12));
            action.useRightDragPlacement = commandValue[BuildActionToml::KeyUseRightDragPlacement].getOr<bool>(action.placementMode == BuildPlacementMode::Line);

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

            Array<String> iconLayers = NormalizeIconLayers(action.iconLayers);
            if (iconLayers.isEmpty() && !action.icon.isEmpty())
            {
                iconLayers << action.icon;
            }
            const String primaryIcon = iconLayers.isEmpty() ? action.icon : iconLayers.front();

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
            tomlText += U"owner_tags = " + BuildTomlStringArrayValue(ownerTags) + U"\n";
            tomlText += U"id = \"" + BuildActionTomlEscape(action.id) + U"\"\n";
            tomlText += U"name = \"" + BuildActionTomlEscape(action.name) + U"\"\n";
            tomlText += U"description = \"" + BuildActionTomlEscape(action.description) + U"\"\n";
            tomlText += U"icon = \"" + BuildActionTomlEscape(primaryIcon) + U"\"\n";
            if (iconLayers.size() > 1)
            {
                tomlText += U"icons = " + BuildTomlStringArrayValue(iconLayers) + U"\n";
            }
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
