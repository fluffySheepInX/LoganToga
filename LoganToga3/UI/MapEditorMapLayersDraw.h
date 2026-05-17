#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBase.h"

namespace LT3
{
	enum class MapEditorObjectLayerPass
	{
		Back,
		Front,
	};

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

	inline void DrawMapEditorObjectLayer(const MapEditorState& editor, MapEditorObjectLayerPass pass = MapEditorObjectLayerPass::Back)
	{
		if (editor.cells.isEmpty())
		{
			return;
		}

		struct ObjectRenderEntry
		{
			Vec2 bottomCenter;
			int32 assetIndex = InvalidMapEditorAsset;
			int32 cellX = 0;
			int32 cellY = 0;
			double scale = 1.0;
			double opacity = 1.0;
		};

		Array<ObjectRenderEntry> backRenderList;
		Array<ObjectRenderEntry> frontRenderList;
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
					const size_t splitIndex = (cell.decals.size() + 1) / 2;
					size_t decalIndex = 0;
					for (const MapEditorDecalPlacement& decal : cell.decals)
					{
						if (0 <= decal.assetIndex && decal.assetIndex < static_cast<int32>(editor.assets.size()))
						{
							ObjectRenderEntry entry{ bottomCenter.movedBy(0, -QuarterTileThickness), decal.assetIndex, x, y, decal.scale, decal.opacity };
							((decalIndex < splitIndex) ? backRenderList : frontRenderList) << entry;
						}
						++decalIndex;
					}
				}
				else if (0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(editor.assets.size()))
				{
					const Vec2 bottomCenter = ToQuarterScreen(MapEditorCellCenter(x, y));
					backRenderList << ObjectRenderEntry{ bottomCenter.movedBy(0, -QuarterTileThickness), cell.objectAsset, x, y, cell.decalScale, cell.decalOpacity };
				}
			}
		}

		const Array<ObjectRenderEntry>& renderList = (pass == MapEditorObjectLayerPass::Back) ? backRenderList : frontRenderList;
		for (const auto& object : renderList)
		{
			const MapEditorAsset& asset = editor.assets[object.assetIndex];
			const double animationTimeSec = Scene::Time() + ComputeMapEditorGifPhaseOffsetSec(asset, object.cellX, object.cellY);
			DrawPlacedMapAsset(asset, object.bottomCenter, object.scale, object.opacity, animationTimeSec);
		}
	}
}
