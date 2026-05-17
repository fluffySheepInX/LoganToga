#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseAssetHelpers.h"
# include "MapEditorResourceDraw.h"
# include "BuildingEditor.h"

namespace LT3
{
	inline void DrawMapEditorDecalEditor(const MapEditorState& editor, const Font& uiFont)
	{
		if (!HasOpenDecalEditorTarget(editor))
		{
			return;
		}

		const MapEditorAsset& asset = editor.assets[editor.decalEditorAssetIndex];
		const RectF panel = EditorDecalEditorPanelRect();
		const RectF closeRect = EditorDecalEditorCloseRect();
		const RectF opacityDecRect = EditorDecalOpacityDecRect();
		const RectF opacityIncRect = EditorDecalOpacityIncRect();
		const RectF scaleDecRect = EditorDecalScaleDecRect();
		const RectF scaleIncRect = EditorDecalScaleIncRect();
		const RectF applyRect = EditorDecalApplyRect();
		const RectF opacityRandomRect = EditorDecalOpacityRandomToggleRect();
		const RectF opacityMinDecRect = EditorDecalOpacityMinDecRect();
		const RectF opacityMinIncRect = EditorDecalOpacityMinIncRect();
		const RectF opacityMaxDecRect = EditorDecalOpacityMaxDecRect();
		const RectF opacityMaxIncRect = EditorDecalOpacityMaxIncRect();
		const RectF scaleRandomRect = EditorDecalScaleRandomToggleRect();
		const RectF scaleMinDecRect = EditorDecalScaleMinDecRect();
		const RectF scaleMinIncRect = EditorDecalScaleMinIncRect();
		const RectF scaleMaxDecRect = EditorDecalScaleMaxDecRect();
		const RectF scaleMaxIncRect = EditorDecalScaleMaxIncRect();

		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Decal Editor").draw(panel.x + 24.0, panel.y + 16.0, Palette::White);
		closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);
		applyRect.draw(ColorF{ 0.12, 0.22, 0.18, 0.96 }).drawFrame(2, applyRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Apply / Reroll").drawAt(12, applyRect.center(), Palette::White);

		DrawAssetPreview(asset, Vec2{ panel.x + 56.0, panel.y + 64.0 }, SizeF{ 56.0, 56.0 });
		uiFont(asset.fileName).draw(13, panel.x + 96.0, panel.y + 46.0, Palette::White);
		uiFont(U"Palette asset setting / Apply affects all placed decals").draw(11, panel.x + 96.0, panel.y + 70.0, Palette::Lightgray);

		opacityDecRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, opacityDecRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		opacityIncRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, opacityIncRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Opacity").draw(13, panel.x + 24.0, panel.y + 78.0, Palette::Gold);
		uiFont(U"-").drawAt(24, opacityDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(24, opacityIncRect.center(), Palette::White);
		uiFont(U"{:.2f}"_fmt(asset.decalOpacity)).drawAt(26, Vec2{ panel.x + panel.w * 0.5, opacityDecRect.center().y }, Palette::White);

		scaleDecRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, scaleDecRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		scaleIncRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, scaleIncRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Scale").draw(13, panel.x + 24.0, panel.y + 128.0, Palette::Gold);
		uiFont(U"-").drawAt(24, scaleDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(24, scaleIncRect.center(), Palette::White);
		uiFont(U"{:.2f}"_fmt(asset.decalScale)).drawAt(26, Vec2{ panel.x + panel.w * 0.5, scaleDecRect.center().y }, Palette::White);

		opacityRandomRect.draw(asset.useRandomDecalOpacity ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, opacityRandomRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Random opacity range").draw(12, opacityRandomRect.x + 12.0, opacityRandomRect.y + 5.0, asset.useRandomDecalOpacity ? Palette::White : Palette::Lightgray);
		uiFont(U"min {:.2f} / max {:.2f}"_fmt(asset.decalOpacityMin, asset.decalOpacityMax)).draw(11, panel.x + 24.0, panel.y + 222.0, Palette::Lightgray);

		for (const RectF& rect : { opacityMinDecRect, opacityMinIncRect, opacityMaxDecRect, opacityMaxIncRect, scaleMinDecRect, scaleMinIncRect, scaleMaxDecRect, scaleMaxIncRect })
		{
			rect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		}
		uiFont(U"-").drawAt(18, opacityMinDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(18, opacityMinIncRect.center(), Palette::White);
		uiFont(U"-").drawAt(18, opacityMaxDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(18, opacityMaxIncRect.center(), Palette::White);
		uiFont(U"Min").drawAt(11, Vec2{ (opacityMinDecRect.x + opacityMinIncRect.x + opacityMinIncRect.w) * 0.5, opacityMinDecRect.y - 10.0 }, Palette::Gold);
		uiFont(U"Max").drawAt(11, Vec2{ (opacityMaxDecRect.x + opacityMaxIncRect.x + opacityMaxIncRect.w) * 0.5, opacityMaxDecRect.y - 10.0 }, Palette::Gold);

		scaleRandomRect.draw(asset.useRandomDecalScale ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, scaleRandomRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Random scale range").draw(12, scaleRandomRect.x + 12.0, scaleRandomRect.y + 5.0, asset.useRandomDecalScale ? Palette::White : Palette::Lightgray);
		uiFont(U"min {:.2f} / max {:.2f}"_fmt(asset.decalScaleMin, asset.decalScaleMax)).draw(11, panel.x + 24.0, panel.y + 296.0, Palette::Lightgray);
		uiFont(U"-").drawAt(18, scaleMinDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(18, scaleMinIncRect.center(), Palette::White);
		uiFont(U"-").drawAt(18, scaleMaxDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(18, scaleMaxIncRect.center(), Palette::White);
		uiFont(U"Min").drawAt(11, Vec2{ (scaleMinDecRect.x + scaleMinIncRect.x + scaleMinIncRect.w) * 0.5, scaleMinDecRect.y - 10.0 }, Palette::Gold);
		uiFont(U"Max").drawAt(11, Vec2{ (scaleMaxDecRect.x + scaleMaxIncRect.x + scaleMaxIncRect.w) * 0.5, scaleMaxDecRect.y - 10.0 }, Palette::Gold);
	}

	inline void DrawHomeMarker(const Vec2& worldPos, const String& label, const ColorF& color, const Font& uiFont)
	{
		const Vec2 screenPos = ToQuarterViewportScreen(worldPos);
		Circle{ screenPos, 13.0 }.draw(color).drawFrame(2.0, ColorF{ 1, 1, 1, 0.9 });
		uiFont(label).drawAt(12, screenPos.movedBy(0, -22), Palette::White);
	}
}
