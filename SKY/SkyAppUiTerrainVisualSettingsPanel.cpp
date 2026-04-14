# include "SkyAppUiSettingsInternal.hpp"
# include "SkyAppText.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	using namespace UiSettingsDetail;
	using SkyAppText::TextId;
	using SkyAppText::Tr;

	void DrawTerrainVisualSettingsPanel(TerrainVisualSettings& settings,
		const bool uiEditMode,
		bool& isExpanded,
		const SkyAppPanels& panels)
	{
		enum class TerrainVisualPage
		{
			Surface,
			Grounding,
			Noise,
		};

		static TerrainVisualPage currentPage = TerrainVisualPage::Surface;
     static Optional<int32> activeValueRowId;
		constexpr int32 TerrainHelpBoxHeight = 122;

		if (not isExpanded)
		{
			return;
		}

		const Rect& panelRect = panels.terrainSettings;
		Optional<RectF> hoveredRect;
		String hoveredTitle;
		String hoveredDescription;
      const double valueRowHeight = 64.0;
		const double valueRowGap = 10.0;
		const double contentWidth = (panelRect.w - 24.0);
		const auto registerHelp = [&](const RectF& rect, const StringView title, const StringView description)
			{
				if (rect.mouseOver())
				{
					hoveredRect = rect;
					hoveredTitle = title;
					hoveredDescription = description;
				}
			};
     const auto sectionRect = [&](const int32 yOffset, const int32 height = 54)
			{
				return RectF{ static_cast<double>(panelRect.x + 12), static_cast<double>(panelRect.y + yOffset), static_cast<double>(panelRect.w - 24), static_cast<double>(height) };
			};
		const Rect dragHandleRect = SkyAppUiLayout::TerrainVisualPanelDragHandle(panelRect);
		const Rect pageButton0{ (panelRect.x + 14), (panelRect.y + 72), 122, 28 };
		const Rect pageButton1{ (pageButton0.rightX() + 8), pageButton0.y, 122, 28 };
		const Rect pageButton2{ (pageButton1.rightX() + 8), pageButton0.y, 122, 28 };
        const Rect resetButtonRect = SkyAppUiLayout::TerrainVisualPanelButtonRect(panelRect, (panelRect.w - 100), (panelRect.h - 48));
		const int32 helpPanelWidth = 236;
		const int32 helpPanelGap = 12;
		const RectF helpRect{
			Min(static_cast<double>(Scene::Width() - helpPanelWidth - 8), static_cast<double>(panelRect.rightX() + helpPanelGap)),
			static_cast<double>(panelRect.y + 40),
			static_cast<double>(helpPanelWidth),
			static_cast<double>(Max(180, panelRect.h - 84)) };
		const ColorF sectionFill{ 0.10, 0.12, 0.16, 0.50 };
		const ColorF sectionFrame{ 0.72, 0.80, 0.90, 0.28 };
		const ColorF sectionTitleColor{ 0.96, 0.98, 1.0, 1.0 };
		const ColorF sectionBodyColor{ 0.84, 0.90, 0.98, 0.92 };
     const auto formatValue = [](const double value, const int32 decimals)
			{
				switch (decimals)
				{
				case 0:
					return U"{:.0f}"_fmt(value);

				case 1:
					return U"{:.1f}"_fmt(value);

				default:
					return U"{:.2f}"_fmt(value);
				}
			};
		const auto drawValueRow = [&](const RectF& rect,
			const int32 rowId,
			const StringView title,
			const StringView summary,
			double& value,
			const double minValue,
			const double maxValue,
			const int32 decimals,
			const StringView helpTitle,
			const StringView helpDescription)
			{
                value = Clamp(value, minValue, maxValue);
				const bool hovered = rect.mouseOver();

				if (hovered && MouseL.down())
				{
					activeValueRowId = rowId;
				}

				const bool active = (activeValueRowId && (*activeValueRowId == rowId));
				const RectF sliderTrackRect{ (rect.x + 14.0), (rect.bottomY() - 14.0), (rect.w - 28.0), 8.0 };
				if (active)
				{
					if (MouseL.pressed())
					{
						const double cursorRatio = Math::Saturate((Cursor::PosF().x - sliderTrackRect.x) / Max(1.0, sliderTrackRect.w));
						value = Clamp((minValue + (maxValue - minValue) * cursorRatio), minValue, maxValue);
					}
					else
					{
						activeValueRowId.reset();
					}
				}

				if (hovered || active)
				{
					hoveredRect = rect;
					hoveredTitle = helpTitle;
					hoveredDescription = helpDescription;
				}

				const double ratio = Math::Saturate((value - minValue) / Max(0.0001, (maxValue - minValue)));
				rect.rounded(8).draw(active
					? ColorF{ 0.16, 0.20, 0.28, 0.84 }
					: (hovered ? ColorF{ 0.14, 0.17, 0.24, 0.76 } : sectionFill))
					.drawFrame(1.0, 0.0, active ? ColorF{ 0.58, 0.76, 0.98, 0.92 } : sectionFrame);
				SimpleGUI::GetFont()(title).draw(Vec2{ rect.x + 12.0, rect.y + 8.0 }, sectionTitleColor);
				SimpleGUI::GetFont()(formatValue(value, decimals)).draw(Vec2{ rect.rightX() - 86.0, rect.y + 8.0 }, sectionTitleColor);
				SimpleGUI::GetFont()(summary).draw(Vec2{ rect.x + 12.0, rect.y + 30.0 }, sectionBodyColor);

				if (active)
				{
					sliderTrackRect.rounded(4).draw(ColorF{ 0.12, 0.14, 0.17, 0.92 });
					RectF{ sliderTrackRect.pos, (sliderTrackRect.w * ratio), sliderTrackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, 0.95 });
					sliderTrackRect.rounded(4).drawFrame(1.0, ColorF{ 0.82, 0.88, 0.95, 0.65 });
					RectF{ Arg::center = Vec2{ (sliderTrackRect.x + sliderTrackRect.w * ratio), sliderTrackRect.centerY() }, EditorSliderKnobWidth, EditorSliderKnobHeight }
						.rounded(4)
						.draw(ColorF{ 0.96, 0.98, 1.0 })
						.drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
				}
				else
				{
                  SimpleGUI::GetFont()(Tr(TextId::CommonDragToAdjust)).draw(Vec2{ rect.rightX() - 128.0, rect.y + 30.0 }, UiInternal::EditorTextOnLightSecondaryColor());
				}
			};
		const auto drawToggleRow = [&](const RectF& rect,
			const StringView title,
			const StringView summary,
			bool& enabled,
			const StringView helpTitle,
			const StringView helpDescription)
			{
				const bool hovered = rect.mouseOver();
				if (hovered)
				{
					hoveredRect = rect;
					hoveredTitle = helpTitle;
					hoveredDescription = helpDescription;
				}

				if (hovered && MouseL.down())
				{
					enabled = not enabled;
				}

				rect.rounded(8).draw(hovered ? ColorF{ 0.14, 0.17, 0.24, 0.76 } : sectionFill)
					.drawFrame(1.0, 0.0, sectionFrame);
				SimpleGUI::GetFont()(title).draw(Vec2{ rect.x + 12.0, rect.y + 8.0 }, sectionTitleColor);
				SimpleGUI::GetFont()(summary).draw(Vec2{ rect.x + 12.0, rect.y + 30.0 }, sectionBodyColor);
				const RectF stateRect{ (rect.rightX() - 72.0), (rect.y + 18.0), 56.0, 24.0 };
				stateRect.rounded(6).draw(enabled ? ColorF{ 0.33, 0.53, 0.82 } : ColorF{ 0.40, 0.44, 0.52 })
					.drawFrame(1.0, 0.0, enabled ? ColorF{ 0.74, 0.86, 0.98 } : ColorF{ 0.62, 0.68, 0.76 });
              SimpleGUI::GetFont()(enabled ? Tr(TextId::CommonOn) : Tr(TextId::CommonOff)).drawAt(stateRect.center(), ColorF{ 0.98 });
			};
		DrawTerrainVisualSettingsPanelFrame(panelRect);

		if (uiEditMode)
		{
         dragHandleRect.rounded(6).draw(ColorF{ 0.80, 0.86, 0.96, 0.32 }).drawFrame(1.0, 0.0, ColorF{ 0.56, 0.66, 0.82, 0.42 });
         SimpleGUI::GetFont()(Tr(TextId::CommonDrag)).drawAt(dragHandleRect.center(), ColorF{ 0.92, 0.96, 1.0, 0.88 });
			registerHelp(dragHandleRect,
             Tr(TextId::TerrainPanelMove),
				Tr(TextId::TerrainPanelMoveDescription));
		}

     if (DrawTerrainPageButton(pageButton0, Tr(TextId::TerrainPageSurface), (currentPage == TerrainVisualPage::Surface)))
		{
			currentPage = TerrainVisualPage::Surface;
		}
        if (DrawTerrainPageButton(pageButton1, Tr(TextId::TerrainPageGrounding), (currentPage == TerrainVisualPage::Grounding)))
		{
			currentPage = TerrainVisualPage::Grounding;
		}
      if (DrawTerrainPageButton(pageButton2, Tr(TextId::TerrainPageNoise), (currentPage == TerrainVisualPage::Noise)))
		{
			currentPage = TerrainVisualPage::Noise;
		}

       registerHelp(pageButton0, Tr(TextId::TerrainPageSurface), Tr(TextId::TerrainPageSurfaceDescription));
		registerHelp(pageButton1, Tr(TextId::TerrainPageGrounding), Tr(TextId::TerrainPageGroundingDescription));
		registerHelp(pageButton2, Tr(TextId::TerrainPageNoise), Tr(TextId::TerrainPageNoiseDescription));

		if (currentPage == TerrainVisualPage::Surface)
		{
            drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0, contentWidth, valueRowHeight },
				40,
				U"材質混合",
				U"境界をにじませ、セルの角張りを崩します。",
				settings.materialBlendStrength,
				0.0,
				1.0,
				2,
				U"blend",
				U"隣接4セルから取り込む材質の量です。\n上げる: 草地の縁や土道の肩が自然に混ざります。\n下げる: 塗り分けが明快になりますが、タイルっぽさが出やすくなります。");

			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap), contentWidth, valueRowHeight },
				41,
				U"配置物の浸食",
				U"建物・木・道が周辺地面を書き換える強さです。",
				settings.placementImprintStrength,
				0.0,
				1.5,
				2,
				U"imprint",
				U"配置物が地面側へ書き込む量です。\n上げる: 道肩・建物足元・木の根元の変化が強まります。\n下げる: 配置物の周辺地面は元のセル状態に近づき、0 で隣接浸食は止まります。");
		}
		else if (currentPage == TerrainVisualPage::Grounding)
		{
          drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0, contentWidth, valueRowHeight },
				42,
				U"摩耗",
				U"道・建物周囲の踏み荒らしや削れを強調します。",
				settings.wearStrength,
				0.0,
				2.0,
				2,
				U"wear",
				U"摩耗マスクの強度です。\n上げる: 土道や建物足元が濃く締まり、使い込まれた印象になります。\n下げる: 道や拠点周囲の削れが減り、きれいな地面に戻ります。");

			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap), contentWidth, valueRowHeight },
				43,
				U"AO / 接地暗がり",
				U"木・岩・建物まわりの暗がりを増やします。",
				settings.ambientOcclusionStrength,
				0.0,
				2.0,
				2,
				U"ao",
				U"オブジェクト足元の暗がりの効き具合です。\n上げる: 根元や建物まわりが締まり、立体感が増します。\n下げる: 絵は軽くなりますが、やや浮いて見えやすくなります。");
		}
		else
		{
          drawToggleRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0, contentWidth, valueRowHeight },
				U"surface noise",
				U"広域ノイズと細部ノイズをまとめて有効化します。",
				settings.noiseEnabled,
				U"surface noise",
				U"広域ノイズと細部ノイズの両方をまとめて有効化します。\n上げる/ON: 土色のムラ、乾き、色温度差が見えやすくなります。\n下げる/OFF: 地表が均一になり、セル塗りの印象が強まります。");

			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap) * 1, contentWidth, valueRowHeight },
				44,
				U"macro strength",
				U"マップ全体の乾き・色ムラの振れ幅です。",
				settings.macroNoiseStrength,
				0.0,
				1.0,
				2,
				U"macro strength",
				U"広域ノイズの振れ幅です。\n上げる: 土色の温度差や乾湿差がはっきりします。\n下げる: 地面全体のトーンが揃い、整理された印象になります。");
			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap) * 2, contentWidth, valueRowHeight },
				45,
				U"macro scale",
				U"マップ全体にかかる大きな波のサイズです。",
				settings.macroNoiseScale,
				0.01,
				0.20,
				3,
				U"macro scale",
				U"広域ノイズの大きさです。\n上げる: ムラの周期が細かくなり、まだらな地面になります。\n下げる: 大きくゆったりした地形差になり、地帯ごとの差として見えます。");
			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap) * 3, contentWidth, valueRowHeight },
				46,
				U"detail strength",
				U"近距離で見える細かな色揺れの強さです。",
				settings.noiseStrength,
				0.0,
				1.0,
				2,
				U"detail strength",
				U"細部ノイズの強さです。\n上げる: 地面表面の粒感と色揺れが増えます。\n下げる: 滑らかになりますが、セル塗り感が戻りやすくなります。");
			drawValueRow(RectF{ panelRect.x + 12.0, panelRect.y + 116.0 + (valueRowHeight + valueRowGap) * 4, contentWidth, valueRowHeight },
				47,
				U"detail scale",
				U"近距離で見える粒度の大きさです。",
				settings.noiseScale,
				0.04,
				0.60,
				2,
				U"detail scale",
				U"細部ノイズの粒度です。\n上げる: 細かな変化が短い間隔で現れ、ざらついた印象になります。\n下げる: 変化がゆるやかになり、より大きな面として見えます。");
		}

      if (not hoveredDescription.isEmpty())
		{
         helpRect.rounded(8).draw(ColorF{ 0.96, 0.97, 0.99, 0.96 })
				.drawFrame(1.0, 0.0, ColorF{ 0.72, 0.79, 0.88, 0.92 });
			SimpleGUI::GetFont()(hoveredTitle).draw((helpRect.x + 12), (helpRect.y + 8), UiInternal::EditorTextOnCardSecondaryColor());
			const Array<String> helpLines = WrapTooltipText(hoveredDescription, (helpRect.w - 24));
			for (size_t i = 0; i < Min<size_t>(8, helpLines.size()); ++i)
			{
				SimpleGUI::GetFont()(helpLines[i]).draw((helpRect.x + 12), (helpRect.y + 32 + static_cast<int32>(i) * 20), UiInternal::EditorTextOnCardPrimaryColor());
			}
		}

     if (DrawTextButton(resetButtonRect, Tr(TextId::CommonReset)))
		{
			settings = TerrainVisualSettings{};
		}

	}
}
