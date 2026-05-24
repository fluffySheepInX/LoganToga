# pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"

namespace LT3
{
    inline RectF DebugEnemyMoveMarkersCheckboxRect(const RectF& anchorRect)
    {
        return RectF{ anchorRect.x, anchorRect.y, 18.0, 18.0 };
    }

    inline RectF DebugEnemyMoveMarkersRowRect(const RectF& anchorRect)
    {
        return RectF{ anchorRect.x - 6.0, anchorRect.y - 6.0, 232.0, 28.0 };
    }

    inline String BuildMenuDebugText(const BattleWorld& world, const DefinitionStores& defs)
    {
        const FilePath buildMenuPath = ResolveBuildActionTomlPath();
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

        const String pendingAction = (world.selection.actionPlacementActive && world.selection.actionId < defs.buildActions.size())
            ? defs.buildActions[world.selection.actionId].name
            : U"<none>";

        return U"BuildMenu: actions={} visible={} file={} readable={} pending={}"_fmt(
            defs.buildActions.size(),
            visibleActionCount,
            buildMenuFileExists ? U"yes" : U"no",
            buildMenuTomlReadable ? U"yes" : U"no",
            pendingAction);
    }

    inline Array<String> BuildMenuDebugLines(const BattleWorld& world, const DefinitionStores& defs)
    {
        Array<String> lines;
        const FilePath buildMenuPath = ResolveBuildActionTomlPath();
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
            Array<String> spawnTags;
            BuildActionResultType parsedResultType = BuildActionResultType::None;
            const TOMLValue resultArray = commandValue[U"result"];
            const bool hasReadableResult = TryReadBuildActionResult(resultArray, parsedResultType, spawnTag, spawnTags);

            String resultType = U"<none>";
            switch (parsedResultType)
            {
            case BuildActionResultType::Unit:
                resultType = U"unit";
                break;
            case BuildActionResultType::Object:
                resultType = U"obj";
                break;
            case BuildActionResultType::Carrier:
                resultType = U"carrier";
                break;
            default:
                break;
            }

            if (i < 4)
            {
                lines << U"[{}] owner={} id={} resultReadable={} type={} spawn={} known={}"_fmt(
                    i,
                    ownerTag.isEmpty() ? U"<empty>" : ownerTag,
                    id.isEmpty() ? U"<empty>" : id,
                    hasReadableResult ? U"yes" : U"no",
                    resultType,
                    spawnTag.isEmpty() ? U"<empty>" : spawnTag,
                    (!spawnTag.isEmpty() && defs.unitByTag.contains(spawnTag)) ? U"yes" : U"no");
            }
        }

        lines.insert(lines.begin() + 1, U"Summary: actions={} visible={} commands={}"_fmt(defs.buildActions.size(), visibleActionCount, commandCount));

        return lines;
    }

    inline void DrawBattleDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Font& uiFont, const RectF& panelRect)
    {
        uiFont(BuildMenuDebugText(world, defs)).draw(panelRect.x + 18, panelRect.y + 94, Palette::Orange);
        const Array<String> buildMenuDebugLines = BuildMenuDebugLines(world, defs);
        for (size_t i = 0; i < buildMenuDebugLines.size(); ++i)
        {
            uiFont(buildMenuDebugLines[i]).draw(11, panelRect.x + 18, panelRect.y + 118 + static_cast<int32>(i) * 16, Palette::Lightgray);
        }
    }

    inline void DrawDebugEnemyMoveMarkersToggle(const MapEditorState& mapEditor, const Font& uiFont, const RectF& anchorRect)
    {
        const RectF rowRect = DebugEnemyMoveMarkersRowRect(anchorRect);
        const RectF checkRect = DebugEnemyMoveMarkersCheckboxRect(anchorRect);
        rowRect.draw(ColorF{ 0.02, 0.03, 0.045, 0.72 }).drawFrame(1.0, ColorF{ 1, 1, 1, 0.12 });
        checkRect.draw(mapEditor.showEnemyMoveMarkers ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
            .drawFrame(2.0, checkRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.18 });
        if (mapEditor.showEnemyMoveMarkers)
        {
            Line{ checkRect.tl().movedBy(3.0, 10.0), checkRect.tl().movedBy(7.0, 14.0) }.draw(2.6, Palette::Orange);
            Line{ checkRect.tl().movedBy(7.0, 14.0), checkRect.tl().movedBy(15.0, 4.0) }.draw(2.6, Palette::Orange);
        }
        uiFont(U"Enemy move markers").draw(12, checkRect.x + 28.0, checkRect.y - 2.0, Palette::Lightgray);
    }
}
