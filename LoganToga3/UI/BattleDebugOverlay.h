# pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"

namespace LT3
{
    inline String BuildMenuDebugText(const BattleWorld& world, const DefinitionStores& defs)
    {
        const FilePath buildMenuPath = ResolveBuildMenuTomlPath();
        const bool buildMenuFileExists = FileSystem::Exists(buildMenuPath);
        const bool buildMenuTomlReadable = buildMenuFileExists && static_cast<bool>(TOMLReader{ buildMenuPath });

        int32 visibleActionCount = 0;
        if (IsValidUnit(world, world.selection.selected))
        {
            for (const auto& action : defs.buildActions)
            {
                if (CanUseBuildAction(world, defs, world.selection.selected, action))
                {
                    ++visibleActionCount;
                }
            }
        }

        return U"BuildMenu: actions={} visible={} file={} readable={}"_fmt(
            defs.buildActions.size(),
            visibleActionCount,
            buildMenuFileExists ? U"yes" : U"no",
            buildMenuTomlReadable ? U"yes" : U"no");
    }

    inline Array<String> BuildMenuDebugLines(const BattleWorld& world, const DefinitionStores& defs)
    {
        Array<String> lines;
        const FilePath buildMenuPath = ResolveBuildMenuTomlPath();
        lines << U"Path: {}"_fmt(buildMenuPath);

        const bool buildMenuFileExists = FileSystem::Exists(buildMenuPath);
        if (!buildMenuFileExists)
        {
            lines << U"BuildMenu file does not exist.";
            return lines;
        }

        const TOMLReader toml{ buildMenuPath };
        if (!toml)
        {
            lines << U"BuildMenu TOMLReader open failed.";
            return lines;
        }

        const auto commands = toml[U"commands"].tableArrayView();
        size_t commandCount = 0;

        int32 visibleActionCount = 0;
        if (IsValidUnit(world, world.selection.selected))
        {
            for (const auto& action : defs.buildActions)
            {
                if (CanUseBuildAction(world, defs, world.selection.selected, action))
                {
                    ++visibleActionCount;
                }
            }
        }

        for (const auto commandValue : commands)
        {
            const size_t i = commandCount;
            ++commandCount;
            const String ownerTag = commandValue[U"owner_tag"].getOr<String>(U"");
            const String id = commandValue[U"id"].getOr<String>(U"");

            String spawnTag;
            const TOMLValue resultArray = commandValue[U"result"];
            const bool resultIsArray = resultArray.isArray();
            if (resultIsArray)
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

            if (i < 4)
            {
                lines << U"[{}] owner={} id={} resultArray={} spawn={} known={}"_fmt(
                    i,
                    ownerTag.isEmpty() ? U"<empty>" : ownerTag,
                    id.isEmpty() ? U"<empty>" : id,
                    resultIsArray ? U"yes" : U"no",
                    spawnTag.isEmpty() ? U"<empty>" : spawnTag,
                    (!spawnTag.isEmpty() && defs.unitByTag.contains(spawnTag)) ? U"yes" : U"no");
            }
        }

        lines.insert(lines.begin() + 1, U"Summary: actions={} visible={} commands={}"_fmt(defs.buildActions.size(), visibleActionCount, commandCount));

        return lines;
    }

    inline void DrawBattleDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const RectF& panelRect)
    {
        uiFont(BuildMenuDebugText(world, defs)).draw(panelRect.x + 18, panelRect.y + 94, Palette::Orange);
        const Array<String> buildMenuDebugLines = BuildMenuDebugLines(world, defs);
        for (size_t i = 0; i < buildMenuDebugLines.size(); ++i)
        {
            uiFont(buildMenuDebugLines[i]).draw(11, panelRect.x + 18, panelRect.y + 118 + static_cast<int32>(i) * 16, Palette::Lightgray);
        }
    }
}
