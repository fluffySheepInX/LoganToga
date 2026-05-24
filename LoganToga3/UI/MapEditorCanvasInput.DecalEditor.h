#pragma once
# include "MapEditorCanvasInputCommon.h"

namespace LT3
{
	inline bool ProcessMapEditorDecalEditorInput(MapEditorState& editor)
	{
		if (!HasOpenDecalEditorTarget(editor))
		{
			return false;
		}

		bool consumed = false;
		MapEditorAsset& asset = editor.assets[editor.decalEditorAssetIndex];
		if (EditorDecalEditorCloseRect().leftClicked())
		{
			editor.showDecalEditor = false;
			editor.decalEditorAssetIndex = InvalidMapEditorAsset;
			editor.statusText = U"Decal editor closed";
			return true;
		}

		if (EditorDecalOpacityDecRect().leftClicked())
		{
			asset.decalOpacity = Clamp(asset.decalOpacity - 0.05, 0.0, 1.0);
			editor.statusText = U"Decal opacity: {:.2f}"_fmt(asset.decalOpacity);
			consumed = true;
		}
		if (EditorDecalOpacityIncRect().leftClicked())
		{
			asset.decalOpacity = Clamp(asset.decalOpacity + 0.05, 0.0, 1.0);
			editor.statusText = U"Decal opacity: {:.2f}"_fmt(asset.decalOpacity);
			consumed = true;
		}
		if (EditorDecalScaleDecRect().leftClicked())
		{
			asset.decalScale = Clamp(asset.decalScale - 0.05, 0.1, 4.0);
			editor.statusText = U"Decal scale: {:.2f}"_fmt(asset.decalScale);
			consumed = true;
		}
		if (EditorDecalScaleIncRect().leftClicked())
		{
			asset.decalScale = Clamp(asset.decalScale + 0.05, 0.1, 4.0);
			editor.statusText = U"Decal scale: {:.2f}"_fmt(asset.decalScale);
			consumed = true;
		}
		if (EditorDecalOpacityRandomToggleRect().leftClicked())
		{
			asset.useRandomDecalOpacity = !asset.useRandomDecalOpacity;
			editor.statusText = asset.useRandomDecalOpacity ? U"Random opacity ON" : U"Random opacity OFF";
			consumed = true;
		}
		if (EditorDecalScaleRandomToggleRect().leftClicked())
		{
			asset.useRandomDecalScale = !asset.useRandomDecalScale;
			editor.statusText = asset.useRandomDecalScale ? U"Random scale ON" : U"Random scale OFF";
			consumed = true;
		}
		if (EditorDecalApplyRect().leftClicked())
		{
			NormalizeDecalSettings(asset);
			const int32 appliedCount = ApplyDecalAssetToPlacedCells(editor, editor.decalEditorAssetIndex);
			editor.statusText = U"Applied decal settings to {} placements"_fmt(appliedCount);
			return true;
		}

		const auto stepRange = [&](const RectF& rect, double& value, double delta, double minValue, double maxValue, StringView label) -> bool
		{
			if (!rect.leftClicked())
			{
				return false;
			}

			value = Clamp(value + delta, minValue, maxValue);
			editor.statusText = U"{}: {}"_fmt(label, value);
			return true;
		};

		consumed = stepRange(EditorDecalOpacityMinDecRect(), asset.decalOpacityMin, -0.05, 0.0, 1.0, U"Opacity min") || consumed;
		consumed = stepRange(EditorDecalOpacityMinIncRect(), asset.decalOpacityMin, 0.05, 0.0, 1.0, U"Opacity min") || consumed;
		consumed = stepRange(EditorDecalOpacityMaxDecRect(), asset.decalOpacityMax, -0.05, 0.0, 1.0, U"Opacity max") || consumed;
		consumed = stepRange(EditorDecalOpacityMaxIncRect(), asset.decalOpacityMax, 0.05, 0.0, 1.0, U"Opacity max") || consumed;
		consumed = stepRange(EditorDecalScaleMinDecRect(), asset.decalScaleMin, -0.05, 0.1, 4.0, U"Scale min") || consumed;
		consumed = stepRange(EditorDecalScaleMinIncRect(), asset.decalScaleMin, 0.05, 0.1, 4.0, U"Scale min") || consumed;
		consumed = stepRange(EditorDecalScaleMaxDecRect(), asset.decalScaleMax, -0.05, 0.1, 4.0, U"Scale max") || consumed;
		consumed = stepRange(EditorDecalScaleMaxIncRect(), asset.decalScaleMax, 0.05, 0.1, 4.0, U"Scale max") || consumed;

		if (consumed)
		{
			NormalizeDecalSettings(asset);
		}

		return consumed || EditorDecalEditorPanelRect().mouseOver();
	}
}
