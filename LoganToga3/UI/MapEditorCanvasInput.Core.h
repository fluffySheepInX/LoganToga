#pragma once
# include "MapEditorCanvasInput.Palette.h"

namespace LT3
{
	inline bool ProcessMapEditorCanvasInput(MapEditorState& editor, const Vec2& screenMouse)
	{
		if (KeySpace.pressed())
		{
			return false;
		}

		if (MouseL.up())
		{
			editor.lastPaintCell.reset();
			editor.lastPaintAsset = InvalidMapEditorAsset;
		}
		if (MouseR.up())
		{
			editor.lastEraseCell.reset();
		}

		const Optional<Point> cell = PickMapEditorCell(editor, screenMouse);
		const Vec2 worldMouse = ToQuarterWorld(screenMouse);

		if (MouseL.down())
		{
			if (Circle{ ToQuarterViewportScreen(editor.playerHomePosition), 22.0 }.intersects(screenMouse))
			{
				editor.draggingPlayerHome = true;
				editor.draggingEnemyHome = false;
				editor.statusText = U"Dragging player Home";
				return true;
			}
			if (Circle{ ToQuarterViewportScreen(editor.enemyHomePosition), 22.0 }.intersects(screenMouse))
			{
				editor.draggingEnemyHome = true;
				editor.draggingPlayerHome = false;
				editor.statusText = U"Dragging enemy Home";
				return true;
			}
		}

		if (editor.draggingPlayerHome && MouseL.pressed())
		{
			const Point cell = QuarterWorldToBattleCell(worldMouse, editor.mapWidth, editor.mapHeight);
			editor.playerHomePosition = QuarterBattleCellCenter(cell.x, cell.y);
			return true;
		}
		if (editor.draggingEnemyHome && MouseL.pressed())
		{
			const Point cell = QuarterWorldToBattleCell(worldMouse, editor.mapWidth, editor.mapHeight);
			editor.enemyHomePosition = QuarterBattleCellCenter(cell.x, cell.y);
			return true;
		}
		if (MouseL.up() && (editor.draggingPlayerHome || editor.draggingEnemyHome))
		{
			editor.draggingPlayerHome = false;
			editor.draggingEnemyHome = false;
			SaveMapEditorToml(editor, false);
			editor.statusText = U"Home position updated";
			return true;
		}

		if (!cell)
		{
			if (editor.resourcePlacementDragKind && MouseL.up())
			{
				editor.resourcePlacementDragKind.reset();
				return true;
			}
			if (MouseR.down())
			{
				editor.showDecalEditor = false;
				editor.decalEditorAssetIndex = InvalidMapEditorAsset;
			}
			return false;
		}

		bool consumed = false;

		if (editor.resourcePlacementDragKind && MouseL.up())
		{
			CommitDraggedResourcePlacement(editor, *cell);
			return true;
		}

		if (editor.resourcePlacementDragKind)
		{
			return true;
		}

		if (editor.zOrderMode)
		{
			if (MouseL.down())
			{
				editor.zOrderDragStartCell = *cell;
				editor.zOrderSelectionRect.reset();
				editor.statusText = U"Z-order drag start: ({}, {})"_fmt(cell->x, cell->y);
				return true;
			}

			if (MouseL.pressed() && editor.zOrderDragStartCell)
			{
				editor.zOrderSelectionRect = NormalizeMapEditorCellRect(*editor.zOrderDragStartCell, *cell);
				return true;
			}

			if (MouseL.up() && editor.zOrderDragStartCell)
			{
				editor.zOrderSelectionRect = NormalizeMapEditorCellRect(*editor.zOrderDragStartCell, *cell);
				editor.zOrderDragStartCell.reset();
				editor.zOrderSelectedStackIndex = 0;
				const int32 maxStackSize = MaxDecalStackSizeInRect(editor, *editor.zOrderSelectionRect);
				editor.statusText = U"Z-order range: ({}, {}) - ({}, {}), stack {}"_fmt(
					editor.zOrderSelectionRect->x,
					editor.zOrderSelectionRect->y,
					editor.zOrderSelectionRect->x + editor.zOrderSelectionRect->w - 1,
					editor.zOrderSelectionRect->y + editor.zOrderSelectionRect->h - 1,
					maxStackSize);
				return true;
			}

			return true;
		}

		if (MouseL.down())
		{
			if (const Optional<size_t> existing = FindResourceNodeAtCell(editor, *cell))
			{
				if (PassesResourceNodeFilter(editor, editor.resourceNodes[*existing].kind))
				{
					SelectResourceNodeIndex(editor, static_cast<int32>(*existing));
					editor.statusText = U"Selected resource node: {}"_fmt(ResourceKindLabel(editor.resourceNodes[*existing].kind));
					return true;
				}
			}
		}

		if (editor.resourceNodeDragging && MouseL.pressed() && IsValidSelectedResourceNodeIndex(editor))
		{
			editor.resourceNodes[editor.selectedResourceNodeIndex].cell = *cell;
			SortMapEditorResourceNodes(editor);
			if (const Optional<size_t> index = FindResourceNodeAtCell(editor, *cell))
			{
				editor.selectedResourceNodeIndex = static_cast<int32>(*index);
			}
			consumed = true;
		}
		if (editor.resourceNodeDragging && MouseL.up())
		{
			editor.resourceNodeDragging = false;
			if (IsValidSelectedResourceNodeIndex(editor))
			{
				const auto& node = editor.resourceNodes[editor.selectedResourceNodeIndex];
				editor.statusText = U"Moved resource node to ({}, {})"_fmt(node.cell.x, node.cell.y);
			}
			consumed = true;
		}

		if (MouseL.pressed() && 0 <= editor.selectedAsset && editor.selectedAsset < static_cast<int32>(editor.assets.size()))
		{
			MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
			const bool samePaintTarget = editor.lastPaintCell
				&& (*editor.lastPaintCell == *cell)
				&& (editor.lastPaintAsset == editor.selectedAsset);
			if (editor.assets[editor.selectedAsset].kind == MapEditorAssetKind::Terrain)
			{
				if (!samePaintTarget)
				{
					target.terrainAsset = editor.selectedAsset;
					editor.lastPaintCell = *cell;
					editor.lastPaintAsset = editor.selectedAsset;
					consumed = true;
				}
			}
			else if (IsMapEditorDecalAsset(editor, editor.selectedAsset))
			{
				if (!samePaintTarget)
				{
					AddDecalAssetToCell(target, editor.selectedAsset, editor.assets[editor.selectedAsset]);
					editor.lastPaintCell = *cell;
					editor.lastPaintAsset = editor.selectedAsset;
					consumed = true;
				}
			}
			else
			{
				if (!samePaintTarget)
				{
					target.objectAsset = editor.selectedAsset;
					target.decalOpacity = 1.0;
					target.decalScale = 1.0;
					editor.lastPaintCell = *cell;
					editor.lastPaintAsset = editor.selectedAsset;
					consumed = true;
				}
			}
		}
		if (MouseR.pressed())
		{
			MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
			const bool sameEraseTarget = editor.lastEraseCell && (*editor.lastEraseCell == *cell);
			if (!sameEraseTarget)
			{
				if (!target.decals.isEmpty())
				{
					target.decals.pop_back();
					SyncLegacyDecalFieldsFromStack(target);
				}
				else
				{
					target.objectAsset = InvalidMapEditorAsset;
					target.decalOpacity = 1.0;
					target.decalScale = 1.0;
				}
				editor.lastEraseCell = *cell;
				consumed = true;
			}
		}

		return consumed;
	}
}
