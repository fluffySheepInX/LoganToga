#pragma once
# include <algorithm>
# include <Siv3D.hpp>
# include "MapEditorDrawBase.h"

namespace LT3
{
	enum class MapEditorObjectLayerPass
	{
		Ground,
		Tall,
		Overlay,
	};

	struct MapEditorObjectRenderEntry
	{
		Vec2 bottomCenter;
		int32 assetIndex = InvalidMapEditorAsset;
		int32 cellX = 0;
		int32 cellY = 0;
		double scale = 1.0;
		double opacity = 1.0;
		double depth = 0.0;
		int32 diagonal = 0;
		int32 cellZIndex = 0;
		int32 insertionOrder = 0;
	};

	inline DecalRenderKind MapEditorObjectLayerPassToRenderKind(MapEditorObjectLayerPass pass)
	{
		switch (pass)
		{
		case MapEditorObjectLayerPass::Tall:
			return DecalRenderKind::Tall;
		case MapEditorObjectLayerPass::Overlay:
			return DecalRenderKind::Overlay;
		case MapEditorObjectLayerPass::Ground:
		default:
			return DecalRenderKind::Ground;
		}
	}

	inline void SortMapEditorObjectRenderEntries(Array<MapEditorObjectRenderEntry>& renderList)
	{
		std::stable_sort(renderList.begin(), renderList.end(), [](const MapEditorObjectRenderEntry& a, const MapEditorObjectRenderEntry& b)
		{
			if (a.depth != b.depth)
			{
				return a.depth < b.depth;
			}
			if (a.diagonal != b.diagonal)
			{
				return a.diagonal < b.diagonal;
			}
			if (a.cellZIndex != b.cellZIndex)
			{
				return a.cellZIndex < b.cellZIndex;
			}
			return a.insertionOrder < b.insertionOrder;
		});
	}

	inline void DrawMapEditorObjectRenderEntry(const MapEditorState& editor, const MapEditorObjectRenderEntry& object)
	{
		const MapEditorAsset& asset = editor.assets[object.assetIndex];
		const double animationTimeSec = Scene::Time() + ComputeMapEditorGifPhaseOffsetSec(asset, object.cellX, object.cellY);
		DrawPlacedMapAsset(asset, object.bottomCenter, object.scale, object.opacity, animationTimeSec);
	}

	inline void CollectMapEditorObjectRenderEntries(const MapEditorState& editor, MapEditorObjectLayerPass pass, Array<MapEditorObjectRenderEntry>& renderList)
	{
		if (editor.cells.isEmpty())
		{
			return;
		}

		const DecalRenderKind targetRenderKind = MapEditorObjectLayerPassToRenderKind(pass);
		int32 insertionOrder = 0;
		for (int32 diagonal = 0; diagonal < (editor.mapWidth + editor.mapHeight - 1); ++diagonal)
		{
			const int32 xBegin = Max(0, diagonal - (editor.mapHeight - 1));
			const int32 xEnd = Min(editor.mapWidth - 1, diagonal);
			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = diagonal - x;
				const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
				if (!cell.decals.isEmpty())
				{
					const Vec2 bottomCenter = ToQuarterScreen(MapEditorCellCenter(x, y));
					size_t decalIndex = 0;
					for (const MapEditorDecalPlacement& decal : cell.decals)
					{
						if (0 <= decal.assetIndex && decal.assetIndex < static_cast<int32>(editor.assets.size()))
						{
							const DecalRenderKind renderKind = editor.assets[decal.assetIndex].decalRenderKind;
							if (renderKind == targetRenderKind)
							{
								const Vec2 drawBottomCenter = bottomCenter.movedBy(0, -QuarterTileThickness);
								renderList << MapEditorObjectRenderEntry{ drawBottomCenter, decal.assetIndex, x, y, decal.scale, decal.opacity, drawBottomCenter.y, diagonal, static_cast<int32>(decalIndex), insertionOrder };
							}
							++insertionOrder;
						}
						++decalIndex;
					}
				}
				else if (pass == MapEditorObjectLayerPass::Ground && 0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(editor.assets.size()))
				{
					const Vec2 bottomCenter = ToQuarterScreen(MapEditorCellCenter(x, y));
					const Vec2 drawBottomCenter = bottomCenter.movedBy(0, -QuarterTileThickness);
					renderList << MapEditorObjectRenderEntry{ drawBottomCenter, cell.objectAsset, x, y, cell.decalScale, cell.decalOpacity, drawBottomCenter.y, diagonal, 0, insertionOrder++ };
				}
			}
		}
	}

	inline void DrawMapEditorTerrainLayer(const MapEditorState& editor, bool drawGrid)
	{
		if (editor.cells.isEmpty())
		{
			return;
		}

		for (int32 diagonal = 0; diagonal < (editor.mapWidth + editor.mapHeight - 1); ++diagonal)
		{
			const int32 xBegin = Max(0, diagonal - (editor.mapHeight - 1));
			const int32 xEnd = Min(editor.mapWidth - 1, diagonal);
			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = diagonal - x;
				const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
				const Vec2 worldCenter = MapEditorCellCenter(x, y);
				const Vec2 bottomCenter = ToQuarterScreen(worldCenter);
				const Quad tile = ToQuarterTile(worldCenter);

				tile.draw(ColorF{ 1, 1, 1, 0.08 });
				if (0 <= cell.terrainAsset && cell.terrainAsset < static_cast<int32>(editor.assets.size()))
				{
					DrawPlacedMapAsset(editor.assets[cell.terrainAsset], bottomCenter);
				}
				if (drawGrid)
				{
					tile.drawFrame(2.0, ColorF{ 0.0, 0.75, 1.0, 0.22 });
				}
			}
		}
	}

	inline void DrawMapEditorObjectLayer(const MapEditorState& editor, MapEditorObjectLayerPass pass = MapEditorObjectLayerPass::Ground)
	{
		Array<MapEditorObjectRenderEntry> renderList;
		CollectMapEditorObjectRenderEntries(editor, pass, renderList);
		SortMapEditorObjectRenderEntries(renderList);
		for (const auto& object : renderList)
		{
			DrawMapEditorObjectRenderEntry(editor, object);
		}
	}
}
