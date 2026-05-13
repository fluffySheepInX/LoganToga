#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"

namespace LT3
{
    inline FilePath ResolveResourceNodeTomlPath();

    inline String ResourceKindToTag(ResourceKind kind)
    {
        switch (kind)
        {
        case ResourceKind::Trust:
            return U"trust";
        case ResourceKind::Food:
            return U"food";
        default:
            return U"gold";
        }
    }

    inline ColorF ResourceKindColor(ResourceKind kind)
    {
        switch (kind)
        {
        case ResourceKind::Trust:
            return Palette::Violet;
        case ResourceKind::Food:
            return Palette::Yellowgreen;
        default:
            return Palette::Gold;
        }
    }

    inline bool PassesResourceNodeFilter(const MapEditorState& editor, ResourceKind kind)
    {
        return (editor.resourceNodeFilterKind < 0) || (editor.resourceNodeFilterKind == static_cast<int32>(kind));
    }

    inline String ResourceKindLabel(ResourceKind kind)
    {
        switch (kind)
        {
        case ResourceKind::Trust:
            return U"Trust";
        case ResourceKind::Food:
            return U"Food";
        default:
            return U"Gold";
        }
    }

    inline ResourceKind NextResourceKind(ResourceKind kind)
    {
        switch (kind)
        {
        case ResourceKind::Gold:
            return ResourceKind::Trust;
        case ResourceKind::Trust:
            return ResourceKind::Food;
        default:
            return ResourceKind::Gold;
        }
    }

    inline void SortMapEditorResourceNodes(MapEditorState& editor)
    {
        editor.resourceNodes.sort_by([](const ResourceNodeEditData& a, const ResourceNodeEditData& b)
        {
            if (a.cell.y != b.cell.y)
            {
                return a.cell.y < b.cell.y;
            }
            if (a.cell.x != b.cell.x)
            {
                return a.cell.x < b.cell.x;
            }
            return static_cast<int32>(a.kind) < static_cast<int32>(b.kind);
        });
    }

    inline void LoadMapEditorResourceNodes(MapEditorState& editor)
    {
        editor.resourceNodes.clear();
        editor.selectedResourceNodeIndex = -1;
        const TOMLReader toml{ editor.resourceNodeSavePath };
        if (!toml)
        {
            return;
        }

        const TOMLValue resourceNodes = toml[U"resource_nodes"];
        if (resourceNodes.isEmpty())
        {
            return;
        }

        try
        {
            for (const auto& nodeValue : resourceNodes.tableArrayView())
            {
                Array<double> positionValues;
                const TOMLValue position = nodeValue[U"position"];
                if (position.isEmpty() || !position.isArray())
                {
                    continue;
                }

                for (const auto& positionValue : position.arrayView())
                {
                    if (const Optional<double> coordinate = positionValue.getOpt<double>())
                    {
                        positionValues << *coordinate;
                    }
                }
                if (positionValues.size() < 2)
                {
                    continue;
                }

                const String kindText = nodeValue[U"resource_kind"].getOr<String>(U"gold").lowercased();
                const ResourceKind kind = (kindText == U"trust")
                    ? ResourceKind::Trust
                    : (kindText == U"food") ? ResourceKind::Food : ResourceKind::Gold;
                const int32 x = Round(positionValues[0] / QuarterTileStep);
                const int32 y = Round(positionValues[1] / QuarterTileStep);
                editor.resourceNodes << ResourceNodeEditData{
                    kind,
                    Point{ x, y },
                    Max(0, nodeValue[U"amount"].getOr<int32>(700)),
                    Max(0, nodeValue[U"income_per_sec"].getOr<int32>(5))
                };
            }
        }
        catch (const std::exception&)
        {
        }

        SortMapEditorResourceNodes(editor);
    }

    inline Array<String> ValidateMapEditorResourceNodes(const MapEditorState& editor)
    {
        Array<String> issues;
        HashSet<Point> occupiedCells;
        for (const auto& node : editor.resourceNodes)
        {
            if ((node.cell.x < 0) || (node.cell.y < 0) || (node.cell.x >= editor.mapWidth) || (node.cell.y >= editor.mapHeight))
            {
                issues << U"Node at ({}, {}) is out of bounds"_fmt(node.cell.x, node.cell.y);
            }
            if (occupiedCells.contains(node.cell))
            {
                issues << U"Duplicate resource node cell: ({}, {})"_fmt(node.cell.x, node.cell.y);
            }
            occupiedCells.insert(node.cell);
            if (node.amount <= 0)
            {
                issues << U"Node at ({}, {}) has zero amount"_fmt(node.cell.x, node.cell.y);
            }
            if (node.incomePerSec <= 0)
            {
                issues << U"Node at ({}, {}) has zero income"_fmt(node.cell.x, node.cell.y);
            }
        }
        return issues;
    }

    inline void SaveMapEditorResourceNodes(const MapEditorState& editor)
    {
        FileSystem::CreateDirectories(FileSystem::ParentPath(editor.resourceNodeSavePath));
        TextWriter resourceWriter{ editor.resourceNodeSavePath };
        if (!resourceWriter)
        {
            return;
        }

        for (const auto& resourceNode : editor.resourceNodes)
        {
            const Vec2 world = Vec2{ resourceNode.cell.x * QuarterTileStep, resourceNode.cell.y * QuarterTileStep };
            resourceWriter << U"[[resource_nodes]]\n";
            resourceWriter << U"resource_kind = \"" << ResourceKindToTag(resourceNode.kind) << U"\"\n";
            resourceWriter << U"position = [" << world.x << U", " << world.y << U"]\n";
            resourceWriter << U"amount = " << resourceNode.amount << U"\n";
            resourceWriter << U"income_per_sec = " << resourceNode.incomePerSec << U"\n\n";
        }
    }
}
