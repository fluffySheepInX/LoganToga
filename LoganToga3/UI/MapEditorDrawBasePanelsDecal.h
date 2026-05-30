#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseAssetHelpers.h"
# include "MapEditorResourceDraw.h"
# include "BuildingEditor.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline void DrawMapEditorDecalEditor(const MapEditorState& editor, const Font& uiFont)
	{
		if (!HasOpenDecalEditorTarget(editor))
		{
			return;
		}

		const MapEditorAsset& asset = editor.assets[editor.decalEditorAssetIndex];
		const RectF panel = EditorDecalEditorPanelRect(editor);
		const Vec2 panelOffset = panel.pos - EditorDecalEditorPanelRect().pos;
		const auto decalRect = [&](const RectF& rect)
		{
			return rect.movedBy(panelOffset);
		};
		const RectF closeRect = decalRect(EditorDecalEditorCloseRect());
		const RectF tabBarRect = decalRect(EditorDecalEditorTabBarRect());
		const RectF decalTabRect = decalRect(EditorDecalEditorTabRect(0));
		const RectF shadowTabRect = decalRect(EditorDecalEditorTabRect(1));

		DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.94 }, ColorF{ 1, 1, 1, 0.18 });
		DrawRectPanelTitle(panel, U"Decal Editor", uiFont);
		DrawRectPanelCloseButton(closeRect, uiFont, 18);
		if (editor.uiLayoutEditEnabled)
		{
			DrawUiLayoutDragHandleOnly(EditorDecalEditorDragHandleRect(editor), editor.uiLayoutDraggingDecalEditor, uiFont, 11);
		}

		tabBarRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.90 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		DrawRectTabButton(decalTabRect, U"Decal Editor", editor.decalEditorTabIndex == 0, uiFont, 11);
		DrawRectTabButton(shadowTabRect, U"Decal Shadow Editor", editor.decalEditorTabIndex == 1, uiFont, 11);

		if (editor.decalEditorTabIndex == 1)
		{
			const RectF viewport = decalRect(EditorDecalShadowViewportRect());
			const double contentHeight = EditorDecalShadowContentHeight();
			const double maxScroll = Max(0.0, contentHeight - viewport.h);
			const double scroll = Clamp(editor.decalShadowEditorScroll, 0.0, maxScroll);
			const RectF previewShadowRect = decalRect(EditorDecalShadowPreviewRect(scroll));
			const RectF enabledRect = decalRect(EditorDecalShadowEnabledRect(scroll));
			const RectF circleModeRect = decalRect(EditorDecalShadowModeRect(0, scroll));
			const RectF silhouetteModeRect = decalRect(EditorDecalShadowModeRect(1, scroll));
			const RectF directionGridRect = decalRect(EditorDecalShadowDirectionGridRect(scroll));
			const RectF lengthRowRect = decalRect(EditorDecalShadowLengthRowRect(scroll));
			const RectF opacityRowRect = decalRect(EditorDecalShadowOpacityRowRect(scroll));
			const RectF blurRowRect = decalRect(EditorDecalShadowBlurRowRect(scroll));
			viewport.draw(ColorF{ 0.04, 0.05, 0.07, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });

			previewShadowRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			uiFont(U"Shadow Preview").draw(12, previewShadowRect.x + 12.0, previewShadowRect.y + 10.0, Palette::White);
			const Vec2 previewCenter = previewShadowRect.center().movedBy(0, 10);
			if (asset.useDecalShadow)
			{
				const Vec2 direction = DecalShadowDirectionVector(asset.decalShadowDirection);
				const Vec2 shadowOffset = direction * Min(42.0, asset.decalShadowLength);
				if (asset.decalShadowMode == DecalShadowMode::Circle)
				{
					Ellipse{ previewCenter + shadowOffset, 22.0 + asset.decalShadowLength * 0.25, 11.0 + asset.decalShadowLength * 0.12 }
						.draw(ColorF{ 0.0, 0.0, 0.0, Clamp(asset.decalShadowOpacity, 0.0, 1.0) * 0.7 });
				}
				else
				{
					RectF{ Arg::center = previewCenter + shadowOffset, 52.0, 24.0 }.draw(ColorF{ 0.0, 0.0, 0.0, Clamp(asset.decalShadowOpacity, 0.0, 1.0) * 0.7 });
				}
			}
			DrawAssetPreview(asset, previewCenter, SizeF{ 64.0, 64.0 });

			DrawRectCheckRow(enabledRect, asset.useDecalShadow ? U"Shadow Enabled" : U"Shadow Disabled", asset.useDecalShadow, uiFont, 12, 4.0, 11.0, true);

			uiFont(U"Shadow Mode").draw(12, circleModeRect.x, circleModeRect.y - 22.0, Palette::Gold);
			circleModeRect.draw(asset.decalShadowMode == DecalShadowMode::Circle ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, circleModeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			silhouetteModeRect.draw(asset.decalShadowMode == DecalShadowMode::Silhouette ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, silhouetteModeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"Circle").drawAt(11, circleModeRect.center(), Palette::White);
			uiFont(U"Silhouette").drawAt(11, silhouetteModeRect.center(), Palette::White);

			directionGridRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.70 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			uiFont(U"Direction").draw(12, directionGridRect.x, directionGridRect.y + 4.0, Palette::Gold);
			const Array<String> directionLabels = { U"N", U"NE", U"E", U"SE", U"S", U"SW", U"W", U"NW" };
			for (int32 i = 0; i < 8; ++i)
			{
				const RectF rect = decalRect(EditorDecalShadowDirectionButtonRect(i, scroll));
				const bool active = (static_cast<int32>(asset.decalShadowDirection) == i);
				rect.draw(active ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				uiFont(directionLabels[i]).drawAt(11, rect.center(), active ? Palette::White : Palette::Lightgray);
			}

			const auto drawValueRow = [&](const RectF& row, StringView label, double value)
			{
				const RectF decRect = EditorDecalShadowValueDecRect(row);
				const RectF incRect = EditorDecalShadowValueIncRect(row);
				DrawRectValueAdjustRow(row, label, U"{:.2f}"_fmt(value), decRect, incRect, uiFont, 12, 22, 22, 18.0);
			};

			drawValueRow(lengthRowRect, U"Shadow Length", asset.decalShadowLength);
			drawValueRow(opacityRowRect, U"Shadow Opacity", asset.decalShadowOpacity);
			drawValueRow(blurRowRect, U"Shadow Blur", asset.decalShadowBlur);

			DrawRectVerticalScrollbar(viewport, contentHeight, scroll, ColorF{ 1, 1, 1, 0.08 }, ColorF{ 1.0, 0.84, 0.0, 0.70 }, 6.0, -6.0, 32.0);
			return;
		}

		const RectF viewport = decalRect(EditorDecalBasicViewportRect());
		const double contentHeight = EditorDecalBasicContentHeight();
		const double maxScroll = Max(0.0, contentHeight - viewport.h);
		const double scroll = Clamp(editor.decalBasicEditorScroll, 0.0, maxScroll);
		const RectF previewRect = decalRect(EditorDecalEditorPreviewRect(scroll));
		const RectF opacityDecRect = decalRect(EditorDecalOpacityDecRect(scroll));
		const RectF opacityIncRect = decalRect(EditorDecalOpacityIncRect(scroll));
		const RectF scaleDecRect = decalRect(EditorDecalScaleDecRect(scroll));
		const RectF scaleIncRect = decalRect(EditorDecalScaleIncRect(scroll));
		const RectF applyRect = decalRect(EditorDecalApplyRect(scroll));
		const RectF opacityRowRect = decalRect(EditorDecalOpacityRowRect(scroll));
		const RectF scaleRowRect = decalRect(EditorDecalScaleRowRect(scroll));
		const RectF renderKindRowRect = decalRect(EditorDecalRenderKindRowRect(scroll));
		const RectF opacityRandomRowRect = decalRect(EditorDecalOpacityRandomRowRect(scroll));
		const RectF scaleRandomRowRect = decalRect(EditorDecalScaleRandomRowRect(scroll));
		const RectF opacityRandomRect = decalRect(EditorDecalOpacityRandomToggleRect(scroll));
		const RectF opacityRandomValueRect = decalRect(EditorDecalOpacityRandomValueRect(scroll));
		const RectF opacityMinDecRect = decalRect(EditorDecalOpacityMinDecRect(scroll));
		const RectF opacityMinIncRect = decalRect(EditorDecalOpacityMinIncRect(scroll));
		const RectF opacityMaxDecRect = decalRect(EditorDecalOpacityMaxDecRect(scroll));
		const RectF opacityMaxIncRect = decalRect(EditorDecalOpacityMaxIncRect(scroll));
		const RectF scaleRandomRect = decalRect(EditorDecalScaleRandomToggleRect(scroll));
		const RectF scaleRandomValueRect = decalRect(EditorDecalScaleRandomValueRect(scroll));
		const RectF scaleMinDecRect = decalRect(EditorDecalScaleMinDecRect(scroll));
		const RectF scaleMinIncRect = decalRect(EditorDecalScaleMinIncRect(scroll));
		const RectF scaleMaxDecRect = decalRect(EditorDecalScaleMaxDecRect(scroll));
		const RectF scaleMaxIncRect = decalRect(EditorDecalScaleMaxIncRect(scroll));
		const RectF ambientSoundRowRect = decalRect(EditorDecalAmbientSoundRowRect(scroll));
		const RectF ambientSoundBrowseRect = decalRect(EditorDecalAmbientSoundBrowseRect(scroll));
		const RectF ambientSoundClearRect = decalRect(EditorDecalAmbientSoundClearRect(scroll));
		const RectF ambientSoundNameRect = decalRect(EditorDecalAmbientSoundNameRect(scroll));
		const RectF ambientVolumeRowRect = decalRect(EditorDecalAmbientVolumeRowRect(scroll));
		const RectF ambientVolumeDecRect = decalRect(EditorDecalAmbientVolumeDecRect(scroll));
		const RectF ambientVolumeIncRect = decalRect(EditorDecalAmbientVolumeIncRect(scroll));

		viewport.draw(ColorF{ 0.04, 0.05, 0.07, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		{
			const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
			const Rect previousScissor = Graphics2D::GetScissorRect();
			Graphics2D::SetScissorRect(viewport.stretched(-2).asRect());

			applyRect.draw(ColorF{ 0.12, 0.22, 0.18, 0.96 }).drawFrame(2, applyRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"Apply / Reroll").drawAt(12, applyRect.center(), Palette::White);

			DrawAssetPreview(asset, previewRect.center(), SizeF{ previewRect.w, previewRect.h });
			uiFont(asset.fileName).draw(13, previewRect.x + previewRect.w + 16.0, previewRect.y + 6.0, Palette::White);
			uiFont(U"Palette asset setting / Apply affects all placed decals").draw(11, previewRect.x + previewRect.w + 16.0, previewRect.y + 30.0, Palette::Lightgray);

			DrawRectValueAdjustRow(opacityRowRect, U"Opacity", U"{:.2f}"_fmt(asset.decalOpacity), opacityDecRect, opacityIncRect, uiFont);
			DrawRectValueAdjustRow(scaleRowRect, U"Scale", U"{:.2f}"_fmt(asset.decalScale), scaleDecRect, scaleIncRect, uiFont);

			renderKindRowRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.70 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			uiFont(U"Render Kind").draw(13, renderKindRowRect.x, renderKindRowRect.y - 18.0, Palette::Gold);
			const Array<String> renderKindLabels = { U"Ground", U"Tall", U"Overlay" };
			for (int32 i = 0; i < 3; ++i)
			{
				const RectF rect = decalRect(EditorDecalRenderKindButtonRect(i, scroll));
				const bool active = (static_cast<int32>(asset.decalRenderKind) == i);
				DrawRectTabButton(rect, renderKindLabels[i], active, uiFont, 11);
			}

			opacityRandomRowRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.70 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			opacityRandomRect.draw(asset.useRandomDecalOpacity ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, opacityRandomRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"Random opacity range").draw(12, opacityRandomRect.x + 12.0, opacityRandomRect.y + 5.0, asset.useRandomDecalOpacity ? Palette::White : Palette::Lightgray);
			uiFont(U"min {:.2f} / max {:.2f}"_fmt(asset.decalOpacityMin, asset.decalOpacityMax)).draw(11, opacityRandomValueRect.x, opacityRandomValueRect.y, Palette::Lightgray);

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

			scaleRandomRowRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.70 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			scaleRandomRect.draw(asset.useRandomDecalScale ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, scaleRandomRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"Random scale range").draw(12, scaleRandomRect.x + 12.0, scaleRandomRect.y + 5.0, asset.useRandomDecalScale ? Palette::White : Palette::Lightgray);
			uiFont(U"min {:.2f} / max {:.2f}"_fmt(asset.decalScaleMin, asset.decalScaleMax)).draw(11, scaleRandomValueRect.x, scaleRandomValueRect.y, Palette::Lightgray);
			uiFont(U"-").drawAt(18, scaleMinDecRect.center(), Palette::White);
			uiFont(U"+").drawAt(18, scaleMinIncRect.center(), Palette::White);
			uiFont(U"-").drawAt(18, scaleMaxDecRect.center(), Palette::White);
			uiFont(U"+").drawAt(18, scaleMaxIncRect.center(), Palette::White);
			uiFont(U"Min").drawAt(11, Vec2{ (scaleMinDecRect.x + scaleMinIncRect.x + scaleMinIncRect.w) * 0.5, scaleMinDecRect.y - 10.0 }, Palette::Gold);
			uiFont(U"Max").drawAt(11, Vec2{ (scaleMaxDecRect.x + scaleMaxIncRect.x + scaleMaxIncRect.w) * 0.5, scaleMaxDecRect.y - 10.0 }, Palette::Gold);

			ambientSoundRowRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.70 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
			uiFont(U"Ambient Sound").draw(13, ambientSoundRowRect.x, ambientSoundRowRect.y - 18.0, Palette::Gold);
			DrawRectTabButton(ambientSoundBrowseRect, U"Sound...", false, uiFont, 11);
			DrawRectIconButton(ambientSoundClearRect, U"clr", uiFont, 10, ColorF{ 0.12, 0.05, 0.05, 0.90 }, 1.0, Palette::White);
			const String ambientSoundName = asset.decalAmbientSound.isEmpty() ? U"(none)" : asset.decalAmbientSound;
			uiFont(ambientSoundName).draw(11, ambientSoundNameRect.x, ambientSoundNameRect.y, asset.decalAmbientSound.isEmpty() ? Palette::Gray : Palette::Aqua);

			DrawRectValueAdjustRow(ambientVolumeRowRect, U"Ambient Volume", U"{:.2f}"_fmt(asset.decalAmbientVolume), ambientVolumeDecRect, ambientVolumeIncRect, uiFont);

			Graphics2D::SetScissorRect(previousScissor);
		}

		DrawRectVerticalScrollbar(viewport, contentHeight, scroll, ColorF{ 1, 1, 1, 0.08 }, ColorF{ 1.0, 0.84, 0.0, 0.70 }, 6.0, -6.0, 32.0);
	}

	inline void DrawHomeMarker(const Vec2& worldPos, const String& label, const ColorF& color, const Font& uiFont)
	{
		const Vec2 screenPos = ToQuarterViewportScreen(worldPos);
		Circle{ screenPos, 13.0 }.draw(color).drawFrame(2.0, ColorF{ 1, 1, 1, 0.9 });
		uiFont(label).drawAt(12, screenPos.movedBy(0, -22), Palette::White);
	}
}
