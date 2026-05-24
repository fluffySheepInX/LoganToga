#pragma once
# include "MapEditorCanvasInput.ZOrder.h"

namespace LT3
{
	inline bool ProcessMapEditorPaletteInput(MapEditorState& editor)
	{
		const RectF palettePanel = EditorPalettePanelRect();
		const RectF paletteViewport = EditorPaletteViewportRect();

		for (int32 tabIndex = 0; tabIndex < 2; ++tabIndex)
		{
			if (EditorPaletteTabRect(tabIndex).leftClicked())
			{
				editor.paletteTabIndex = tabIndex;
				editor.paletteScroll = 0.0;
				if ((editor.selectedAsset < 0)
					|| (editor.selectedAsset >= static_cast<int32>(editor.assets.size()))
					|| (editor.assets[editor.selectedAsset].kind != CurrentMapEditorPaletteKind(editor)))
				{
					editor.selectedAsset = InvalidMapEditorAsset;
				}
				editor.statusText = U"MapAssets tab: {}"_fmt(MapEditorPaletteTabLabel(tabIndex));
				return true;
			}
		}

		if (!palettePanel.mouseOver())
		{
			return false;
		}

		const double maxScroll = Max(0.0, MapEditorPaletteContentHeight(editor) - paletteViewport.h);
		editor.paletteScroll = Clamp(editor.paletteScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);

		const MapEditorAssetKind activeKind = CurrentMapEditorPaletteKind(editor);
		double y = paletteViewport.y - editor.paletteScroll;
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			const MapEditorAsset& asset = editor.assets[i];
			if (asset.kind != activeKind)
			{
				continue;
			}

			const RectF itemRect{ paletteViewport.x, y, paletteViewport.w, 46.0 };
			if (paletteViewport.intersects(itemRect) && IsMapEditorDecalAsset(editor, i))
			{
				const RectF checkRect = EditorPaletteDecalPassageCheckboxRect(itemRect);
				if (checkRect.leftClicked())
				{
					editor.assets[i].decalBlocksPassage = !editor.assets[i].decalBlocksPassage;
					editor.statusText = U"Decal {} passage: {}"_fmt(
						editor.assets[i].fileName,
						editor.assets[i].decalBlocksPassage ? U"blocked" : U"passable");
					return true;
				}
			}
			if (paletteViewport.intersects(itemRect) && itemRect.leftClicked())
			{
				editor.selectedAsset = i;
				editor.statusText = U"Selected: {}"_fmt(asset.fileName);
			}
			if (paletteViewport.intersects(itemRect) && itemRect.rightClicked() && asset.kind == MapEditorAssetKind::Object)
			{
				if (PromoteMapEditorAssetToDecal(editor, i))
				{
					NormalizeDecalSettings(editor.assets[i]);
					ApplyDecalAssetToPlacedCells(editor, i);
					editor.showDecalEditor = true;
					editor.decalEditorAssetIndex = i;
					editor.statusText = U"Decal editor: {}"_fmt(editor.assets[i].fileName);
				}
				return true;
			}

			y += 54.0;
		}

		return true;
	}
}
