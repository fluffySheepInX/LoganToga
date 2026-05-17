#pragma once
# include <Siv3D.hpp>
# include "MapEditorLayoutRects.h"
# include "MapEditorAssetUtils.h"

namespace LT3
{
	inline double EditorUnitListContentHeight(const UnitCatalog& catalog)
	{
		return catalog.entries.size() * 86.0;
	}

	inline MapEditorAssetKind CurrentMapEditorPaletteKind(const MapEditorState& editor)
	{
		return (editor.paletteTabIndex == 0) ? MapEditorAssetKind::Terrain : MapEditorAssetKind::Object;
	}

	inline String MapEditorPaletteTabLabel(int32 tabIndex)
	{
		return (tabIndex == 0) ? U"Terrain" : U"Object / decal / building";
	}

	inline double MapEditorPaletteContentHeight(const MapEditorState& editor)
	{
		const MapEditorAssetKind activeKind = CurrentMapEditorPaletteKind(editor);
		int32 count = 0;
		for (const auto& asset : editor.assets)
		{
			if (asset.kind == activeKind)
			{
				++count;
			}
		}

		return count * 54.0;
	}

	inline String MapEditorAssetKindLabel(MapEditorAssetKind kind)
	{
		return kind == MapEditorAssetKind::Terrain ? U"Terrain / base size" : U"Object / decal / building";
	}

	inline Vec2 MapEditorCellCenter(int32 x, int32 y)
	{
		return QuarterBattleCellCenter(x, y);
	}

	inline Optional<Point> PickMapEditorCell(const MapEditorState& editor, const Vec2& screenPos)
	{
		const Vec2 preCameraScreenPos = ToQuarterPreCameraScreen(screenPos);

		for (int32 y = 0; y < editor.mapHeight; ++y)
		{
			for (int32 x = 0; x < editor.mapWidth; ++x)
			{
				if (ToQuarterTile(MapEditorCellCenter(x, y)).intersects(preCameraScreenPos))
				{
					return Point{ x, y };
				}
			}
		}

		return none;
	}

	inline size_t MapEditorCellIndex(const MapEditorState& editor, int32 x, int32 y)
	{
		return static_cast<size_t>(y * editor.mapWidth + x);
	}

	inline bool IsValidSelectedResourceNodeIndex(const MapEditorState& editor)
	{
		return 0 <= editor.selectedResourceNodeIndex
			&& editor.selectedResourceNodeIndex < static_cast<int32>(editor.resourceNodes.size());
	}

	inline bool HasOpenDecalEditorTarget(const MapEditorState& editor)
	{
		if (!editor.showDecalEditor)
		{
			return false;
		}

		return IsMapEditorDecalAsset(editor, editor.decalEditorAssetIndex);
	}

	inline double EditorResourceNodeListContentHeight(const MapEditorState& editor)
	{
		int32 visibleCount = 0;
		for (const auto& node : editor.resourceNodes)
		{
			if ((editor.resourceNodeFilterKind < 0) || (editor.resourceNodeFilterKind == static_cast<int32>(node.kind)))
			{
				++visibleCount;
			}
		}
		return visibleCount * 54.0;
	}
}
