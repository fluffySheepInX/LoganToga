# include "SkyAppUiSettingsInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	using namespace UiSettingsDetail;

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
		constexpr int32 TerrainHelpBoxHeight = 122;

		if (not isExpanded)
		{
			return;
		}

		const Rect& panelRect = panels.terrainSettings;
		Optional<RectF> hoveredRect;
		String hoveredTitle;
		String hoveredDescription;
		const auto registerHelp = [&](const RectF& rect, const StringView title, const StringView description)
			{
				if (rect.mouseOver())
				{
					hoveredRect = rect;
					hoveredTitle = title;
					hoveredDescription = description;
				}
			};
		const auto sliderRowRect = [&](const int32 yOffset)
			{
				return RectF{ static_cast<double>(panelRect.x + 12), static_cast<double>(panelRect.y + yOffset), static_cast<double>(panelRect.w - 24), EditorSliderRowHeight };
			};
		const auto sectionRect = [&](const int32 yOffset, const int32 height = 54)
			{
				return RectF{ static_cast<double>(panelRect.x + 12), static_cast<double>(panelRect.y + yOffset), static_cast<double>(panelRect.w - 24), static_cast<double>(height) };
			};
		const Rect dragHandleRect = SkyAppUiLayout::TerrainVisualPanelDragHandle(panelRect);
		const Rect pageButton0{ (panelRect.x + 14), (panelRect.y + 72), 122, 28 };
		const Rect pageButton1{ (pageButton0.rightX() + 8), pageButton0.y, 122, 28 };
		const Rect pageButton2{ (pageButton1.rightX() + 8), pageButton0.y, 122, 28 };
     const ColorF sectionFill{ 0.10, 0.12, 0.16, 0.50 };
		const ColorF sectionFrame{ 0.72, 0.80, 0.90, 0.28 };
		const ColorF sectionTitleColor{ 0.96, 0.98, 1.0, 1.0 };
		const ColorF sectionBodyColor{ 0.84, 0.90, 0.98, 0.92 };
		const auto drawSectionCard = [&](const RectF& rect)
			{
				rect.rounded(8).draw(sectionFill).drawFrame(1.0, 0.0, sectionFrame);
			};
		const auto currentPageDefaultHelp = [&]() -> String
			{
				switch (currentPage)
				{
				case TerrainVisualPage::Surface:
					return U"混合 では地面の境界のなじみ、浸食 では建物や道が周囲地面へ書き込む強さを調整します。見た目のつながりを作るページです。";

				case TerrainVisualPage::Grounding:
					return U"接地 では踏み荒らしと接地暗がりを調整します。重さ、使用感、足元の落ち着きを作るページです。";

				case TerrainVisualPage::Noise:
				default:
					return U"ノイズ では広域の色ムラと近距離の粒感を調整します。面の単調さを崩し、自然なゆらぎを作るページです。";
				}
			};
		DrawTerrainVisualSettingsPanelFrame(panelRect);
       SimpleGUI::GetFont()(U"ページ切替で項目を整理しています。説明は下部ヘルプに表示されます。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 38), sectionBodyColor);

		if (uiEditMode)
		{
         dragHandleRect.rounded(6).draw(ColorF{ 0.80, 0.86, 0.96, 0.32 }).drawFrame(1.0, 0.0, ColorF{ 0.56, 0.66, 0.82, 0.42 });
			SimpleGUI::GetFont()(U"drag").drawAt(dragHandleRect.center(), ColorF{ 0.92, 0.96, 1.0, 0.88 });
			registerHelp(dragHandleRect,
              U"パネル移動",
				U"`UI+` が ON の間、この drag ハンドルをドラッグすると Terrain Surface パネルを移動できます。項目操作と競合しにくいよう、移動開始位置を右上に分けています。");
		}

        if (DrawTerrainPageButton(pageButton0, U"混合", (currentPage == TerrainVisualPage::Surface)))
		{
			currentPage = TerrainVisualPage::Surface;
		}
       if (DrawTerrainPageButton(pageButton1, U"接地", (currentPage == TerrainVisualPage::Grounding)))
		{
			currentPage = TerrainVisualPage::Grounding;
		}
        if (DrawTerrainPageButton(pageButton2, U"ノイズ", (currentPage == TerrainVisualPage::Noise)))
		{
			currentPage = TerrainVisualPage::Noise;
		}

      registerHelp(pageButton0, U"混合", U"材質混合と配置物の浸食を調整します。境界のなじみと、建物や道が周囲地面へ書き込む強さを確認するページです。");
		registerHelp(pageButton1, U"接地", U"摩耗と AO を調整します。接地感、踏み荒らし、重さの見え方を詰めるページです。");
		registerHelp(pageButton2, U"ノイズ", U"広域ノイズと細部ノイズを調整します。マップ全体のムラと近距離の粒感を扱います。");

		if (currentPage == TerrainVisualPage::Surface)
		{
            drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 116.0, panelRect.w - 24.0, 86.0 });
			SimpleGUI::GetFont()(U"材質混合").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 120), sectionTitleColor);
			SimpleGUI::GetFont()(U"隣接セルから色と材質をにじませ、境界の角張りを崩します。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 142), sectionBodyColor);
			registerHelp(sectionRect(116), U"材質混合", U"草・土・砂・岩の境界をどれだけ周囲へブレンドするかを決めます。\n上げる: 境界がなだらかになり、地面が一枚の連続した面に見えます。\n下げる: 材質の切り替わりがくっきりし、セル感が残ります。");
			DrawEditorSlider(40, U"blend: {:.2f}"_fmt(settings.materialBlendStrength), settings.materialBlendStrength, 0.0, 1.0, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 162), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(162), U"blend", U"隣接4セルから取り込む材質の量です。\n上げる: 草地の縁や土道の肩が自然に混ざります。\n下げる: 塗り分けが明快になりますが、タイルっぽさが出やすくなります。");

          drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 216.0, panelRect.w - 24.0, 86.0 });
			SimpleGUI::GetFont()(U"配置物の浸食").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 220), sectionTitleColor);
			SimpleGUI::GetFont()(U"建物・木・道が周辺地面を書き換える強さです。全部切りたい時はここを 0 にします。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 242), sectionBodyColor);
			registerHelp(sectionRect(216), U"配置物の浸食", U"`PlacedModel` が周囲セルへ与える材質変化・踏み固め・被覆の総量です。\n上げる: 建物や道が周辺地面により強く馴染みます。\n下げる: 影響が弱まり、0 で配置物由来の隣接浸食は止まります。");
			DrawEditorSlider(41, U"imprint: {:.2f}"_fmt(settings.placementImprintStrength), settings.placementImprintStrength, 0.0, 1.5, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 262), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(262), U"imprint", U"配置物が地面側へ書き込む量です。\n上げる: 道肩・建物足元・木の根元の変化が強まります。\n下げる: 配置物の周辺地面は元のセル状態に近づき、0 で隣接浸食は止まります。");
		}
		else if (currentPage == TerrainVisualPage::Grounding)
		{
          drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 116.0, panelRect.w - 24.0, 86.0 });
			SimpleGUI::GetFont()(U"摩耗").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 120), sectionTitleColor);
			SimpleGUI::GetFont()(U"道・建物周囲の踏み荒らしや削れを、土色への寄りとして強調します。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 142), sectionBodyColor);
			registerHelp(sectionRect(116), U"摩耗", U"`Road` や `Mill` などが周囲の地面をどれだけ使い込んだ見た目にするかを決めます。\n上げる: 入口前や道路中央が踏み固められ、戦場の履歴が見えやすくなります。\n下げる: 配置物は残っていても地面の生活痕が薄くなります。");
			DrawEditorSlider(42, U"wear: {:.2f}"_fmt(settings.wearStrength), settings.wearStrength, 0.0, 2.0, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 162), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(162), U"wear", U"摩耗マスクの強度です。\n上げる: 土道や建物足元が濃く締まり、使い込まれた印象になります。\n下げる: 道や拠点周囲の削れが減り、きれいな地面に戻ります。");

          drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 216.0, panelRect.w - 24.0, 86.0 });
			SimpleGUI::GetFont()(U"AO / 接地暗がり").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 220), sectionTitleColor);
			SimpleGUI::GetFont()(U"木・岩・建物まわりの暗がりを増やして、接地感と重さを出します。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 242), sectionBodyColor);
			registerHelp(sectionRect(216), U"AO / 接地暗がり", U"地面に落ちる局所的な暗がりの強さです。\n上げる: オブジェクトが地面に沈み、足元の重さが出ます。\n下げる: 接地感は弱まりますが、全体はフラットで見通し良くなります。");
			DrawEditorSlider(43, U"ao: {:.2f}"_fmt(settings.ambientOcclusionStrength), settings.ambientOcclusionStrength, 0.0, 2.0, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 262), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(262), U"ao", U"オブジェクト足元の暗がりの効き具合です。\n上げる: 根元や建物まわりが締まり、立体感が増します。\n下げる: 絵は軽くなりますが、やや浮いて見えやすくなります。");
		}
		else
		{
			const RectF noiseToggleRow{ panelRect.x + 16.0, panelRect.y + 118.0, 210.0, EditorCheckBoxRowHeight };
          drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 114.0, panelRect.w - 24.0, 60.0 });
			DrawEditorCheckBox(settings.noiseEnabled, U"surface noise", noiseToggleRow.pos, noiseToggleRow.w);
			registerHelp(noiseToggleRow, U"surface noise", U"広域ノイズと細部ノイズの両方をまとめて有効化します。\n上げる/ON: 土色のムラ、乾き、色温度差が見えやすくなります。\n下げる/OFF: 地表が均一になり、セル塗りの印象が強まります。");

           drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 158.0, panelRect.w - 24.0, 126.0 });
			SimpleGUI::GetFont()(U"広域ノイズ").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 162), sectionTitleColor);
			SimpleGUI::GetFont()(U"マップ全体の乾き・色ムラ・土の温度差を決める大きな波です。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 184), sectionBodyColor);
			registerHelp(sectionRect(158), U"広域ノイズ", U"地面全体にかかる大きなスケールの色ムラです。\n上げる: 乾いた場所・くすんだ場所の差が出て、平面感が減ります。\n下げる: 地面は均一になりますが、空気感は薄くなります。");
			DrawEditorSlider(44, U"macro strength: {:.2f}"_fmt(settings.macroNoiseStrength), settings.macroNoiseStrength, 0.0, 1.0, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 204), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(204), U"macro strength", U"広域ノイズの振れ幅です。\n上げる: 土色の温度差や乾湿差がはっきりします。\n下げる: 地面全体のトーンが揃い、整理された印象になります。");
			DrawEditorSlider(45, U"macro scale: {:.3f}"_fmt(settings.macroNoiseScale), settings.macroNoiseScale, 0.01, 0.20, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 244), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(244), U"macro scale", U"広域ノイズの大きさです。\n上げる: ムラの周期が細かくなり、まだらな地面になります。\n下げる: 大きくゆったりした地形差になり、地帯ごとの差として見えます。");

           drawSectionCard(RectF{ panelRect.x + 12.0, panelRect.y + 298.0, panelRect.w - 24.0, 126.0 });
			SimpleGUI::GetFont()(U"細部ノイズ").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 302), sectionTitleColor);
			SimpleGUI::GetFont()(U"近距離で見える細かな色揺れです。広域ノイズより小さい粒度で効きます。").draw(SkyAppUiLayout::TerrainVisualPanelTextPosition(panelRect, 16, 324), sectionBodyColor);
			registerHelp(sectionRect(298), U"細部ノイズ", U"セル内部の単調さを崩す細かな色揺れです。\n上げる: 近くで見たときの情報量が増えます。\n下げる: 面はきれいになりますが、のっぺり見えやすくなります。");
			DrawEditorSlider(46, U"detail strength: {:.2f}"_fmt(settings.noiseStrength), settings.noiseStrength, 0.0, 1.0, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 344), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(344), U"detail strength", U"細部ノイズの強さです。\n上げる: 地面表面の粒感と色揺れが増えます。\n下げる: 滑らかになりますが、セル塗り感が戻りやすくなります。");
			DrawEditorSlider(47, U"detail scale: {:.2f}"_fmt(settings.noiseScale), settings.noiseScale, 0.04, 0.60, SkyAppUiLayout::TerrainVisualPanelSliderPosition(panelRect, 384), SkyAppUiLayout::TerrainVisualPanelSliderLabelWidth(), SkyAppUiLayout::TerrainVisualPanelSliderWidth());
			registerHelp(sliderRowRect(384), U"detail scale", U"細部ノイズの粒度です。\n上げる: 細かな変化が短い間隔で現れ、ざらついた印象になります。\n下げる: 変化がゆるやかになり、より大きな面として見えます。");
		}

		const RectF helpRect{ static_cast<double>(panelRect.x + 12), static_cast<double>(panelRect.bottomY() - TerrainHelpBoxHeight - 12), static_cast<double>(panelRect.w - 116), static_cast<double>(TerrainHelpBoxHeight) };
		helpRect.rounded(8).draw(ColorF{ 0.96, 0.97, 0.99, 0.96 })
			.drawFrame(1.0, 0.0, ColorF{ 0.72, 0.79, 0.88, 0.92 });
       const String helpTitle = hoveredTitle.isEmpty() ? U"Terrain Surface Help" : hoveredTitle;
		const String helpBody = hoveredDescription.isEmpty()
           ? currentPageDefaultHelp()
			: hoveredDescription;
		SimpleGUI::GetFont()(helpTitle).draw((helpRect.x + 12), (helpRect.y + 8), UiInternal::EditorTextOnCardSecondaryColor());
		const Array<String> helpLines = WrapTooltipText(helpBody, (helpRect.w - 24));
		for (size_t i = 0; i < Min<size_t>(4, helpLines.size()); ++i)
		{
			SimpleGUI::GetFont()(helpLines[i]).draw((helpRect.x + 12), (helpRect.y + 32 + static_cast<int32>(i) * 20), UiInternal::EditorTextOnCardPrimaryColor());
		}

		if (DrawTextButton(SkyAppUiLayout::TerrainVisualPanelButtonRect(panelRect, (panelRect.w - 100), (panelRect.h - 48)), U"Reset"))
		{
			settings = TerrainVisualSettings{};
		}

	}
}
