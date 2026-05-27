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
		const Vec2 panelOffset = editor.uiDecalEditorPos - EditorDecalEditorPanelRect().pos;
		const auto decalRect = [&](const RectF& rect)
		{
			return rect.movedBy(panelOffset);
		};
		const RectF panelRect = EditorDecalEditorPanelRect(editor);
		if (decalRect(EditorDecalEditorCloseRect()).leftClicked())
		{
			editor.showDecalEditor = false;
			editor.decalEditorAssetIndex = InvalidMapEditorAsset;
			editor.decalEditorTabIndex = 0;
			editor.statusText = U"Decal editor closed";
			return true;
		}

		for (int32 tabIndex = 0; tabIndex < 2; ++tabIndex)
		{
			if (decalRect(EditorDecalEditorTabRect(tabIndex)).leftClicked())
			{
				editor.decalEditorTabIndex = tabIndex;
				editor.decalShadowEditorScroll = 0.0;
				editor.statusText = (tabIndex == 0) ? U"Decal editor" : U"Decal shadow editor (preview)";
				return true;
			}
		}

		if (editor.decalEditorTabIndex != 0)
		{
			const RectF viewport = decalRect(EditorDecalShadowViewportRect());
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
			if (decalRect(EditorDecalShadowEnabledRect(scroll)).leftClicked())
			{
				asset.useDecalShadow = !asset.useDecalShadow;
				editor.statusText = asset.useDecalShadow ? U"Decal shadow ON" : U"Decal shadow OFF";
				return true;
			}
			if (decalRect(EditorDecalShadowModeRect(0, scroll)).leftClicked())
			{
				asset.decalShadowMode = DecalShadowMode::Circle;
				editor.statusText = U"Shadow mode: circle";
				return true;
			}
			if (decalRect(EditorDecalShadowModeRect(1, scroll)).leftClicked())
			{
				asset.decalShadowMode = DecalShadowMode::Silhouette;
				editor.statusText = U"Shadow mode: silhouette";
				return true;
			}
			for (int32 i = 0; i < 8; ++i)
			{
				if (decalRect(EditorDecalShadowDirectionButtonRect(i, scroll)).leftClicked())
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

			if (stepValueRow(decalRect(EditorDecalShadowLengthRowRect(scroll)), asset.decalShadowLength, 2.0, 0.0, 120.0, U"Shadow length")) return true;
			if (stepValueRow(decalRect(EditorDecalShadowOpacityRowRect(scroll)), asset.decalShadowOpacity, 0.05, 0.0, 1.0, U"Shadow opacity")) return true;
			if (stepValueRow(decalRect(EditorDecalShadowBlurRowRect(scroll)), asset.decalShadowBlur, 1.0, 0.0, 32.0, U"Shadow blur")) return true;

			return panelRect.mouseOver();
		}

		if (decalRect(EditorDecalOpacityDecRect()).leftClicked())
		{
			asset.decalOpacity = Clamp(asset.decalOpacity - 0.05, 0.0, 1.0);
			editor.statusText = U"Decal opacity: {:.2f}"_fmt(asset.decalOpacity);
			consumed = true;
		}
		if (decalRect(EditorDecalOpacityIncRect()).leftClicked())
		{
			asset.decalOpacity = Clamp(asset.decalOpacity + 0.05, 0.0, 1.0);
			editor.statusText = U"Decal opacity: {:.2f}"_fmt(asset.decalOpacity);
			consumed = true;
		}
		if (decalRect(EditorDecalScaleDecRect()).leftClicked())
		{
			asset.decalScale = Clamp(asset.decalScale - 0.05, 0.1, 4.0);
			editor.statusText = U"Decal scale: {:.2f}"_fmt(asset.decalScale);
			consumed = true;
		}
		if (decalRect(EditorDecalScaleIncRect()).leftClicked())
		{
			asset.decalScale = Clamp(asset.decalScale + 0.05, 0.1, 4.0);
			editor.statusText = U"Decal scale: {:.2f}"_fmt(asset.decalScale);
			consumed = true;
		}
		for (int32 kindIndex = 0; kindIndex < 3; ++kindIndex)
		{
			if (decalRect(EditorDecalRenderKindButtonRect(kindIndex)).leftClicked())
			{
				asset.decalRenderKind = static_cast<DecalRenderKind>(kindIndex);
				editor.statusText = U"Decal render kind: {}"_fmt(DecalRenderKindToTomlValue(asset.decalRenderKind));
				consumed = true;
			}
		}
		if (decalRect(EditorDecalOpacityRandomToggleRect()).leftClicked())
		{
			asset.useRandomDecalOpacity = !asset.useRandomDecalOpacity;
			editor.statusText = asset.useRandomDecalOpacity ? U"Random opacity ON" : U"Random opacity OFF";
			consumed = true;
		}
		if (decalRect(EditorDecalScaleRandomToggleRect()).leftClicked())
		{
			asset.useRandomDecalScale = !asset.useRandomDecalScale;
			editor.statusText = asset.useRandomDecalScale ? U"Random scale ON" : U"Random scale OFF";
			consumed = true;
		}
		if (decalRect(EditorDecalAmbientSoundBrowseRect()).leftClicked())
		{
			const Array<FileFilter> audioFilters = { FileFilter::AllAudioFiles(), FileFilter::AllFiles() };
			const Optional<FilePath> sourcePath = Dialog::OpenFile(audioFilters);
			if (sourcePath)
			{
				asset.decalAmbientSound = FileSystem::FileName(*sourcePath);
				editor.statusText = U"Decal ambient sound: {}"_fmt(asset.decalAmbientSound);
			}
			return true;
		}
		if (decalRect(EditorDecalAmbientSoundClearRect()).leftClicked())
		{
			asset.decalAmbientSound.clear();
			editor.statusText = U"Decal ambient sound cleared";
			return true;
		}
		if (decalRect(EditorDecalAmbientVolumeDecRect()).leftClicked())
		{
			asset.decalAmbientVolume = Clamp(asset.decalAmbientVolume - 0.05, 0.0, 1.0);
			editor.statusText = U"Decal ambient volume: {:.2f}"_fmt(asset.decalAmbientVolume);
			return true;
		}
		if (decalRect(EditorDecalAmbientVolumeIncRect()).leftClicked())
		{
			asset.decalAmbientVolume = Clamp(asset.decalAmbientVolume + 0.05, 0.0, 1.0);
			editor.statusText = U"Decal ambient volume: {:.2f}"_fmt(asset.decalAmbientVolume);
			return true;
		}
		if (decalRect(EditorDecalApplyRect()).leftClicked())
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

		consumed = stepRange(decalRect(EditorDecalOpacityMinDecRect()), asset.decalOpacityMin, -0.05, 0.0, 1.0, U"Opacity min") || consumed;
		consumed = stepRange(decalRect(EditorDecalOpacityMinIncRect()), asset.decalOpacityMin, 0.05, 0.0, 1.0, U"Opacity min") || consumed;
		consumed = stepRange(decalRect(EditorDecalOpacityMaxDecRect()), asset.decalOpacityMax, -0.05, 0.0, 1.0, U"Opacity max") || consumed;
		consumed = stepRange(decalRect(EditorDecalOpacityMaxIncRect()), asset.decalOpacityMax, 0.05, 0.0, 1.0, U"Opacity max") || consumed;
		consumed = stepRange(decalRect(EditorDecalScaleMinDecRect()), asset.decalScaleMin, -0.05, 0.1, 4.0, U"Scale min") || consumed;
		consumed = stepRange(decalRect(EditorDecalScaleMinIncRect()), asset.decalScaleMin, 0.05, 0.1, 4.0, U"Scale min") || consumed;
		consumed = stepRange(decalRect(EditorDecalScaleMaxDecRect()), asset.decalScaleMax, -0.05, 0.1, 4.0, U"Scale max") || consumed;
		consumed = stepRange(decalRect(EditorDecalScaleMaxIncRect()), asset.decalScaleMax, 0.05, 0.1, 4.0, U"Scale max") || consumed;

		if (consumed)
		{
			NormalizeDecalSettings(asset);
		}

		return consumed || panelRect.mouseOver();
	}
}
