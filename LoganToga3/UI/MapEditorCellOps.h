#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "MapEditorAssetUtils.h"

namespace LT3
{
	inline void InitializeMapEditorCells(MapEditorState& editor)
	{
		editor.cells.assign(editor.mapWidth * editor.mapHeight, MapEditorCell{});

		int32 defaultTerrain = InvalidMapEditorAsset;
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			if (editor.assets[i].fileName == U"grass1.png")
			{
				defaultTerrain = i;
				break;
			}
		}
		if (defaultTerrain == InvalidMapEditorAsset)
		{
			for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
			{
				if (editor.assets[i].kind == MapEditorAssetKind::Terrain)
				{
					defaultTerrain = i;
					break;
				}
			}
		}

		for (auto& cell : editor.cells)
		{
			cell.terrainAsset = defaultTerrain;
		}
		editor.selectedAsset = defaultTerrain;
	}

	inline void ResizeMapEditorCells(MapEditorState& editor, int32 newWidth, int32 newHeight)
	{
		newWidth = Clamp(newWidth, 4, 40);
		newHeight = Clamp(newHeight, 4, 40);
		if (newWidth == editor.mapWidth && newHeight == editor.mapHeight)
		{
			return;
		}

		const int32 oldWidth = editor.mapWidth;
		const int32 oldHeight = editor.mapHeight;
		const Array<MapEditorCell> oldCells = editor.cells;

		editor.mapWidth = newWidth;
		editor.mapHeight = newHeight;
		editor.cells.assign(editor.mapWidth * editor.mapHeight, MapEditorCell{});

		const int32 copyWidth = Min(oldWidth, editor.mapWidth);
		const int32 copyHeight = Min(oldHeight, editor.mapHeight);
		for (int32 y = 0; y < copyHeight; ++y)
		{
			for (int32 x = 0; x < copyWidth; ++x)
			{
				editor.cells[MapEditorCellIndex(editor, x, y)] = oldCells[static_cast<size_t>(y * oldWidth + x)];
			}
		}

		size_t removedNodeCount = 0;
		editor.resourceNodes.remove_if([&](const ResourceNodeEditData& node)
		{
			const bool outOfBounds = (node.cell.x < 0)
				|| (node.cell.y < 0)
				|| (node.cell.x >= editor.mapWidth)
				|| (node.cell.y >= editor.mapHeight);
			if (outOfBounds)
			{
				++removedNodeCount;
			}
			return outOfBounds;
		});
		if (!IsValidSelectedResourceNodeIndex(editor))
		{
			editor.selectedResourceNodeIndex = -1;
		}

		editor.statusText = U"Map size: {} x {}{}"_fmt(
			editor.mapWidth,
			editor.mapHeight,
			(removedNodeCount > 0) ? U"  removed {} resource nodes"_fmt(removedNodeCount) : U"");
	}
}
