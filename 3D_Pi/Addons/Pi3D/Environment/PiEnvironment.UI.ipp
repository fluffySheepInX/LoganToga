namespace Pi3D
{
	namespace
	{
		inline constexpr double EnvironmentCollapsedBodyHeight = 94.0;
		inline constexpr double EnvironmentPanelTopOffset = (ui::layout::HeaderHeight + 4.0);
		inline constexpr double EnvironmentPanelInnerPadding = 8.0;

		[[nodiscard]] inline constexpr double GetExpandedEnvironmentSectionHeight()
		{
			const double iconRowY = EnvironmentPanelInnerPadding;
			const double groundIconY = iconRowY;
			const double rainIconY = (groundIconY + 40.0);
			const double rainAmountIconBottomY = (rainIconY + 38.0);
			const double rainControlStartY = (rainAmountIconBottomY + 14.0);
			const double fogRectY = (rainControlStartY + 132.0);
			const double underwaterRectY = (fogRectY + 410.0);
			const double deepWaterRectBottomY = (underwaterRectY + 482.0 + ui::layout::AddButtonHeight);
			return (deepWaterRectBottomY + EnvironmentPanelInnerPadding);
		}

		[[nodiscard]] inline constexpr double GetExpandedEnvironmentBodyHeight()
		{
			return (EnvironmentPanelTopOffset + GetExpandedEnvironmentSectionHeight());
		}
	}

	// 環境パネル本体の高さを返す
	inline double PiEnvironment::getUIBodyHeight() const
	{
		return m_environmentPanelOpened ? GetExpandedEnvironmentBodyHeight() : EnvironmentCollapsedBodyHeight;
	}

	// 環境 UI を描画
	inline void PiEnvironment::drawUI(const Font& font, Vec2& uiPos, const double contentWidth)
	{
		const double bodyHeight = getUIBodyHeight();
		const double panelTopOffset = EnvironmentPanelTopOffset;
		const double panelInnerPadding = EnvironmentPanelInnerPadding;
		const RectF environmentSectionRect{ uiPos.x, uiPos.y + panelTopOffset, contentWidth, GetExpandedEnvironmentSectionHeight() };
		ui::Section(environmentSectionRect);
		const Vec2 panelPos = environmentSectionRect.pos;

		String hoverTooltip;
		const auto drawIconButton = [&](const RectF& rect, const Texture& icon)
		{
			const bool hovered = rect.mouseOver();
			const bool pressed = hovered && MouseL.pressed();
			rect.rounded(6).draw(pressed ? ui::GetTheme().itemPressed : (hovered ? ui::GetTheme().itemHovered : ui::GetTheme().item));
			rect.rounded(6).drawFrame(1.0, ui::GetTheme().panelBorder);
			if (icon)
			{
				icon.resized(static_cast<int32>(rect.w), static_cast<int32>(rect.h)).draw(rect.pos);
			}
			return rect.leftClicked();
		};

		const auto drawSliderNoButtons = [&](StringView label, double& value, const double min, const double max, const Vec2& pos)
		{
			const double labelWidth = 110.0;
			const double trackWidth = contentWidth - 126.0;

			const RectF labelRect{ pos, labelWidth, 34.0 };
			const RectF trackRect{ pos.x + labelWidth + 10.0, pos.y + 14.0, trackWidth, 6.0 };

			const double clampedValue = Clamp(value, min, max);
			const double t = (max > min) ? ((clampedValue - min) / (max - min)) : 0.0;
			const Vec2 knobCenter{ trackRect.x + trackRect.w * t, trackRect.centerY() };
			const Circle knob{ knobCenter, 10.0 };

			static Optional<size_t> activeSlider;
			const size_t sliderId = reinterpret_cast<size_t>(&value);
			const RectF hitRect = trackRect.stretched(12, 12);

			if (MouseL.down() && (labelRect.mouseOver() || hitRect.mouseOver() || knob.mouseOver()))
			{
				activeSlider = sliderId;
			}

			if (activeSlider && (*activeSlider == sliderId))
			{
				if (MouseL.pressed())
				{
					const double nt = Clamp((Cursor::PosF().x - trackRect.x) / trackRect.w, 0.0, 1.0);
					value = Min(max, Max(min, (min + (max - min) * nt)));
				}
				else
				{
					activeSlider.reset();
				}
			}

			const bool hovered = labelRect.mouseOver() || hitRect.mouseOver() || knob.mouseOver() || (activeSlider && (*activeSlider == sliderId));

			labelRect.rounded(6).draw(ui::GetTheme().item);
			labelRect.rounded(6).drawFrame(1.0, ui::GetTheme().panelBorder);
			font(label).draw(labelRect.x + 10, labelRect.y + 6, ui::GetTheme().text);

			trackRect.rounded(3).draw(ColorF{ 0.78, 0.82, 0.88, 1.0 });
			RectF{ trackRect.pos, Max(0.0, knobCenter.x - trackRect.x), trackRect.h }.rounded(3).draw(ui::GetTheme().accent);
			knob.draw(hovered ? ColorF{ 1.0, 1.0, 1.0, 1.0 } : ColorF{ 0.96, 0.98, 1.0, 1.0 });
			knob.drawFrame(2.0, ui::GetTheme().panelBorder);
		};

		const auto drawRainAmountSlider = [&](double& value, const double min, const double max, const RectF& iconRect)
		{
			const double trackLeft = (iconRect.rightX() + 12.0);
			const double trackWidth = Max(44.0, (panelPos.x + contentWidth - 16.0) - trackLeft);
			const RectF trackRect{ trackLeft, iconRect.y + 22.0, trackWidth, 6.0 };

			const double clampedValue = Clamp(value, min, max);
			const double t = (max > min) ? ((clampedValue - min) / (max - min)) : 0.0;
			const Vec2 knobCenter{ trackRect.x + trackRect.w * t, trackRect.centerY() };
			const Circle knob{ knobCenter, 10.0 };

			static Optional<size_t> activeSlider;
			const size_t sliderId = reinterpret_cast<size_t>(&value);
			const RectF hitRect = trackRect.stretched(12, 12);

			if (MouseL.down() && (hitRect.mouseOver() || knob.mouseOver()))
			{
				activeSlider = sliderId;
			}

			if (activeSlider && (*activeSlider == sliderId))
			{
				if (MouseL.pressed())
				{
					const double nt = Clamp((Cursor::PosF().x - trackRect.x) / trackRect.w, 0.0, 1.0);
					value = Min(max, Max(min, (min + (max - min) * nt)));
				}
				else
				{
					activeSlider.reset();
				}
			}

			const bool hovered = hitRect.mouseOver() || knob.mouseOver() || (activeSlider && (*activeSlider == sliderId));

			drawIconButton(iconRect, m_rainAmountIcon);
			font(U"雨量: {:.0f}"_fmt(value)).draw(trackLeft, iconRect.y - 2.0, ui::GetTheme().text);
			trackRect.rounded(3).draw(ColorF{ 0.78, 0.82, 0.88, 1.0 });
			RectF{ trackRect.pos, Max(0.0, knobCenter.x - trackRect.x), trackRect.h }.rounded(3).draw(ui::GetTheme().accent);
			knob.draw(hovered ? ColorF{ 1.0, 1.0, 1.0, 1.0 } : ColorF{ 0.96, 0.98, 1.0, 1.0 });
			knob.drawFrame(2.0, ui::GetTheme().panelBorder);
		};

		const double iconRowY = (panelPos.y + panelInnerPadding);
		const RectF envHelpRect{ panelPos.x + contentWidth - 80, iconRowY, 32, 32 };
		drawIconButton(envHelpRect, m_environmentHelpIcon);
		if (envHelpRect.mouseOver())
		{
			hoverTooltip = U"環境パネルの説明";
		}

		const RectF openCloseRect{ panelPos.x + contentWidth - 40, iconRowY, 32, 32 };
		if (drawIconButton(openCloseRect, m_environmentOpenCloseIcon))
		{
			m_environmentPanelOpened = (not m_environmentPanelOpened);
		}
		if (openCloseRect.mouseOver())
		{
			hoverTooltip = m_environmentPanelOpened ? U"環境パネルを閉じる" : U"環境パネルを開く";
		}

		if (not m_environmentPanelOpened)
		{
			if (not hoverTooltip.isEmpty())
			{
				ui::Tooltip(font, hoverTooltip, Cursor::PosF().movedBy(18, 20));
			}
			uiPos.y += bodyHeight + ui::layout::SectionGap;
			return;
		}

		const RectF groundIconRect{ panelPos.x + panelInnerPadding, iconRowY, 32, 32 };
		if (drawIconButton(groundIconRect, m_groundModeIcon))
		{
			cycleGroundMode();
		}
		if (m_groundMode != PiGroundMode::Texture)
		{
			groundIconRect.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
		}
		if (groundIconRect.mouseOver())
		{
			hoverTooltip = U"地面テクスチャ切り替え";
		}

		const RectF rainIconRect{ panelPos.x + panelInnerPadding, groundIconRect.y + 40, 32, 32 };
		if (drawIconButton(rainIconRect, m_rainToggleIcon))
		{
			m_rain.toggle();
		}
		if (m_rain.isEnabled())
		{
			rainIconRect.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
		}
		if (rainIconRect.mouseOver())
		{
			hoverTooltip = U"雨 ON/OFF";
		}

		const RectF rainAmountIconRect{ rainIconRect.br().movedBy(-10, 6), 32, 32 };
		if (rainAmountIconRect.mouseOver())
		{
			hoverTooltip = U"雨量";
		}

		double dropCount = static_cast<double>(m_rain.getDropCount());
		drawRainAmountSlider(dropCount, 0.0, 1200.0, rainAmountIconRect);
		m_rain.setDropCount(static_cast<int32>(std::round(dropCount)));

		const double rainControlStartY = (rainAmountIconRect.bottomY() + 14.0);

		double fallSpeed = m_rain.getFallSpeed();
		drawSliderNoButtons(U"速度: {:.1f}"_fmt(fallSpeed), fallSpeed, 1.0, 60.0, Vec2{ panelPos.x + 8, rainControlStartY });
		m_rain.setFallSpeed(fallSpeed);

		double windStrength = m_rain.getWindStrength();
		drawSliderNoButtons(U"風: {:.2f}"_fmt(windStrength), windStrength, -0.5, 0.5, Vec2{ panelPos.x + 8, rainControlStartY + 40.0 });
		m_rain.setWindStrength(windStrength);

		double streakLength = m_rain.getStreakLength();
		drawSliderNoButtons(U"線長: {:.2f}"_fmt(streakLength), streakLength, 0.4, 4.0, Vec2{ panelPos.x + 8, rainControlStartY + 80.0 });
		m_rain.setStreakLength(streakLength);

		const RectF fogRect{ panelPos.x + 8, rainControlStartY + 132.0, contentWidth - 16, ui::layout::AddButtonHeight };
		if (ui::Button(font, m_fogEnabled ? U"フォグ: ON" : U"フォグ: OFF", fogRect))
		{
			m_fogEnabled = (not m_fogEnabled);
		}
		if (m_fogEnabled)
		{
			fogRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
		}

		ui::SliderH(U"Fog 開始: {:.1f}"_fmt(m_fogStartDistance), m_fogStartDistance, 0.0, 180.0,
			Vec2{ panelPos.x + 8, fogRect.y + 50.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"Fog 終端: {:.1f}"_fmt(m_fogEndDistance), m_fogEndDistance, 1.0, 260.0,
			Vec2{ panelPos.x + 8, fogRect.y + 90.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"Fog 濃度: {:.2f}"_fmt(m_fogDensity), m_fogDensity, 0.0, 1.0,
			Vec2{ panelPos.x + 8, fogRect.y + 130.0 }, 110.0, contentWidth - 126.0);
		if (m_fogEndDistance < (m_fogStartDistance + 1.0))
		{
			m_fogEndDistance = m_fogStartDistance + 1.0;
		}

		font(U"Fog 色").draw(Vec2{ panelPos.x + 8, fogRect.y + 170.0 }, ui::GetTheme().text);
		ui::SliderH(U"R: {:.2f}"_fmt(m_fogColor.r), m_fogColor.r, 0.0, 1.0,
			Vec2{ panelPos.x + 8, fogRect.y + 198.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"G: {:.2f}"_fmt(m_fogColor.g), m_fogColor.g, 0.0, 1.0,
			Vec2{ panelPos.x + 8, fogRect.y + 238.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"B: {:.2f}"_fmt(m_fogColor.b), m_fogColor.b, 0.0, 1.0,
			Vec2{ panelPos.x + 8, fogRect.y + 278.0 }, 110.0, contentWidth - 126.0);

		const double presetButtonW = (contentWidth - 24) * 0.5;
		const RectF morningFogRect{ panelPos.x + 8, fogRect.y + 324.0, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"朝霧", morningFogRect))
		{
			applyFogPreset(ColorF{ 0.74, 0.82, 0.90, 1.0 }, 14.0, 82.0, 0.52);
		}

		const RectF sunsetFogRect{ morningFogRect.rightX() + 8, morningFogRect.y, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"夕焼け", sunsetFogRect))
		{
			applyFogPreset(ColorF{ 0.92, 0.58, 0.44, 1.0 }, 18.0, 100.0, 0.45);
		}

		const RectF dustFogRect{ panelPos.x + 8, fogRect.y + 362.0, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"砂埃", dustFogRect))
		{
			applyFogPreset(ColorF{ 0.72, 0.61, 0.42, 1.0 }, 10.0, 74.0, 0.62);
		}

		const RectF nightFogRect{ dustFogRect.rightX() + 8, dustFogRect.y, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"夜霧", nightFogRect))
		{
			applyFogPreset(ColorF{ 0.25, 0.34, 0.55, 1.0 }, 12.0, 86.0, 0.58);
		}

		const double underwaterY = fogRect.y + 410.0;
		const RectF underwaterRect{ panelPos.x + 8, underwaterY, contentWidth - 16, ui::layout::AddButtonHeight };
		if (ui::Button(font, m_underwaterEnabled ? U"水中: ON" : U"水中: OFF", underwaterRect))
		{
			m_underwaterEnabled = (not m_underwaterEnabled);
		}
		if (m_underwaterEnabled)
		{
			underwaterRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
		}

		ui::SliderH(U"水中 Fog 開始: {:.1f}"_fmt(m_underwaterFogStartDistance), m_underwaterFogStartDistance, 0.0, 80.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 50.0 }, 132.0, contentWidth - 148.0);
		ui::SliderH(U"水中 Fog 終端: {:.1f}"_fmt(m_underwaterFogEndDistance), m_underwaterFogEndDistance, 1.0, 180.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 90.0 }, 132.0, contentWidth - 148.0);
		ui::SliderH(U"水中 Fog 濃度: {:.2f}"_fmt(m_underwaterFogDensity), m_underwaterFogDensity, 0.0, 1.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 130.0 }, 132.0, contentWidth - 148.0);
		if (m_underwaterFogEndDistance < (m_underwaterFogStartDistance + 1.0))
		{
			m_underwaterFogEndDistance = m_underwaterFogStartDistance + 1.0;
		}

		font(U"水中 Fog 色").draw(Vec2{ panelPos.x + 8, underwaterRect.y + 170.0 }, ui::GetTheme().text);
		ui::SliderH(U"R: {:.2f}"_fmt(m_underwaterFogColor.r), m_underwaterFogColor.r, 0.0, 1.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 198.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"G: {:.2f}"_fmt(m_underwaterFogColor.g), m_underwaterFogColor.g, 0.0, 1.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 238.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"B: {:.2f}"_fmt(m_underwaterFogColor.b), m_underwaterFogColor.b, 0.0, 1.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 278.0 }, 110.0, contentWidth - 126.0);

		ui::SliderH(U"屈折: {:.3f}"_fmt(m_underwaterDistortionStrength), m_underwaterDistortionStrength, 0.0, 0.025,
			Vec2{ panelPos.x + 8, underwaterRect.y + 318.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"揺れ速度: {:.2f}"_fmt(m_underwaterDistortionSpeed), m_underwaterDistortionSpeed, 0.0, 2.5,
			Vec2{ panelPos.x + 8, underwaterRect.y + 358.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"揺れ幅: {:.1f}"_fmt(m_underwaterDistortionScale), m_underwaterDistortionScale, 4.0, 60.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 398.0 }, 110.0, contentWidth - 126.0);
		ui::SliderH(U"浮遊物: {:.2f}"_fmt(m_underwaterParticleAmount), m_underwaterParticleAmount, 0.0, 1.0,
			Vec2{ panelPos.x + 8, underwaterRect.y + 438.0 }, 110.0, contentWidth - 126.0);

		const RectF deepWaterRect{ panelPos.x + 8, underwaterRect.y + 482.0, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"深海", deepWaterRect))
		{
			applyUnderwaterPreset(ColorF{ 0.04, 0.18, 0.27, 1.0 }, 2.0, 52.0, 0.90, 0.009, 0.56, 22.0, 0.82);
		}

		const RectF murkyWaterRect{ deepWaterRect.rightX() + 8, deepWaterRect.y, presetButtonW, ui::layout::AddButtonHeight };
		if (ui::Button(font, U"濁水", murkyWaterRect))
		{
			applyUnderwaterPreset(ColorF{ 0.10, 0.30, 0.28, 1.0 }, 1.0, 38.0, 0.96, 0.012, 0.42, 15.0, 1.0);
		}

		if (not hoverTooltip.isEmpty())
		{
			ui::Tooltip(font, hoverTooltip, Cursor::PosF().movedBy(18, 20));
		}

		uiPos.y += bodyHeight + ui::layout::SectionGap;
	}

	// 地面モードを保存用文字列へ変換
	inline String PiEnvironment::groundModeToString(const PiGroundMode mode)
	{
		switch (mode)
		{
		case PiGroundMode::None:
			return U"None";
		case PiGroundMode::Gray:
			return U"Gray";
		case PiGroundMode::White:
			return U"White";
		case PiGroundMode::Grid:
			return U"Grid";
		case PiGroundMode::Texture:
		default:
			return U"Texture";
		}
	}

	// 保存文字列を地面モードへ変換
	inline PiGroundMode PiEnvironment::stringToGroundMode(const String& mode)
	{
		if (mode == U"None")
		{
			return PiGroundMode::None;
		}
		if (mode == U"Gray")
		{
			return PiGroundMode::Gray;
		}
		if (mode == U"White")
		{
			return PiGroundMode::White;
		}
		if (mode == U"Grid")
		{
			return PiGroundMode::Grid;
		}
		return PiGroundMode::Texture;
	}

	// 現在の地面モード表示名を返す
	inline StringView PiEnvironment::getGroundModeLabel() const
	{
		switch (m_groundMode)
		{
		case PiGroundMode::None:
			return U"なし";
		case PiGroundMode::Gray:
			return U"灰色";
		case PiGroundMode::White:
			return U"白";
		case PiGroundMode::Grid:
			return U"グリッド";
		case PiGroundMode::Texture:
		default:
			return U"テクスチャ";
		}
	}

	// 地面モードを順送りで切り替え
	inline void PiEnvironment::cycleGroundMode()
	{
		switch (m_groundMode)
		{
		case PiGroundMode::Texture:
			m_groundMode = PiGroundMode::Gray;
			break;
		case PiGroundMode::Gray:
			m_groundMode = PiGroundMode::White;
			break;
		case PiGroundMode::White:
			m_groundMode = PiGroundMode::Grid;
			break;
		case PiGroundMode::Grid:
			m_groundMode = PiGroundMode::None;
			break;
		case PiGroundMode::None:
		default:
			m_groundMode = PiGroundMode::Texture;
			break;
		}
	}

	// 通常フォグプリセットを適用
	inline void PiEnvironment::applyFogPreset(const ColorF& color, const double startDistance, const double endDistance, const double density)
	{
		m_fogEnabled = true;
		m_fogColor = color;
		m_fogStartDistance = Max(0.0, startDistance);
		m_fogEndDistance = Max(m_fogStartDistance + 1.0, endDistance);
		m_fogDensity = Clamp(density, 0.0, 1.0);
	}

	// 水中プリセットを適用
	inline void PiEnvironment::applyUnderwaterPreset(const ColorF& color, const double startDistance, const double endDistance, const double density,
		const double distortionStrength, const double distortionSpeed, const double distortionScale, const double particleAmount)
	{
		m_underwaterEnabled = true;
		m_underwaterFogColor = color;
		m_underwaterFogStartDistance = Max(0.0, startDistance);
		m_underwaterFogEndDistance = Max(m_underwaterFogStartDistance + 1.0, endDistance);
		m_underwaterFogDensity = Clamp(density, 0.0, 1.0);
		m_underwaterDistortionStrength = Clamp(distortionStrength, 0.0, 0.05);
		m_underwaterDistortionSpeed = Clamp(distortionSpeed, 0.0, 5.0);
		m_underwaterDistortionScale = Clamp(distortionScale, 1.0, 80.0);
		m_underwaterParticleAmount = Clamp(particleAmount, 0.0, 1.0);
	}
}
