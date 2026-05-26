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
			editor.decalEditorTabIndex = 0;
			editor.statusText = U"Decal editor closed";
			return true;
		}

		for (int32 tabIndex = 0; tabIndex < 2; ++tabIndex)
		{
			if (EditorDecalEditorTabRect(tabIndex).leftClicked())
			{
				editor.decalEditorTabIndex = tabIndex;
				editor.decalShadowEditorScroll = 0.0;
				editor.statusText = (tabIndex == 0) ? U"Decal editor" : U"Decal shadow editor (preview)";
				return true;
			}
		}

		if (editor.decalEditorTabIndex != 0)
		{
			const RectF viewport = EditorDecalShadowViewportRect();
			const double maxScroll = Max(0.0, EditorDecalShadowContentHeight() - viewport.h);
			if (viewport.mouseOver())
			{
				editor.decalShadowEditorScroll = Clamp(editor.decalShadowEditorScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
				if (Mouse::Wheel() != 0.0)
				{
					return true;
				}
			}

			const double scroll = editor.decalShadowEditorScroll;
			if (EditorDecalShadowEnabledRect(scroll).leftClicked())
			{
				asset.useDecalShadow = !asset.useDecalShadow;
				editor.statusText = asset.useDecalShadow ? U"Decal shadow ON" : U"Decal shadow OFF";
				return true;
			}
			if (EditorDecalShadowModeRect(0, scroll).leftClicked())
			{
				asset.decalShadowMode = DecalShadowMode::Circle;
				editor.statusText = U"Shadow mode: circle";
				return true;
			}
			if (EditorDecalShadowModeRect(1, scroll).leftClicked())
			{
				asset.decalShadowMode = DecalShadowMode::Silhouette;
				editor.statusText = U"Shadow mode: silhouette";
				return true;
			}
			for (int32 i = 0; i < 8; ++i)
			{
				if (EditorDecalShadowDirectionButtonRect(i, scroll).leftClicked())
				{
					asset.decalShadowDirection = static_cast<DecalShadowDirection>(i);
					editor.statusText = U"Shadow direction updated";
					return true;
				}
			}

			const auto stepValueRow = [&](const RectF& row, double& value, double step, double minValue, double maxValue, StringView label) -> bool
			{
				if (EditorDecalShadowValueDecRect(row).leftClicked())
				{
					value = Clamp(value - step, minValue, maxValue);
					editor.statusText = U"{}: {:.2f}"_fmt(label, value);
					return true;
				}
				if (EditorDecalShadowValueIncRect(row).leftClicked())
				{
					value = Clamp(value + step, minValue, maxValue);
					editor.statusText = U"{}: {:.2f}"_fmt(label, value);
					return true;
				}
				return false;
			};

			if (stepValueRow(EditorDecalShadowLengthRowRect(scroll), asset.decalShadowLength, 2.0, 0.0, 120.0, U"Shadow length")) return true;
			if (stepValueRow(EditorDecalShadowOpacityRowRect(scroll), asset.decalShadowOpacity, 0.05, 0.0, 1.0, U"Shadow opacity")) return true;
			if (stepValueRow(EditorDecalShadowBlurRowRect(scroll), asset.decalShadowBlur, 1.0, 0.0, 32.0, U"Shadow blur")) return true;

			return EditorDecalEditorPanelRect().mouseOver();
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
		for (int32 kindIndex = 0; kindIndex < 3; ++kindIndex)
		{
			if (EditorDecalRenderKindButtonRect(kindIndex).leftClicked())
			{
				asset.decalRenderKind = static_cast<DecalRenderKind>(kindIndex);
				editor.statusText = U"Decal render kind: {}"_fmt(DecalRenderKindToTomlValue(asset.decalRenderKind));
				consumed = true;
			}
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
