# pragma once
# include <Siv3D.hpp>
# include "../PiSettings.hpp"
# include "../../../UI/Layout.hpp"
# include "../../../UI/RectUI.hpp"

namespace Pi3D
{
	class PiLighting
	{
	public:
		struct Preset
		{
			String name;
			String description;
			Vec3 sunDirection;
			ColorF sunColor;
			double sunIntensity = 1.0;
			ColorF ambientColor;
			ColorF backgroundColor;
		};

		PiLighting()
			: m_presets{ loadPresetsFromToml() }
		{
			if (m_presets.isEmpty())
			{
				m_presets = defaultPresets();
			}
			m_lightingPresetIndex = Clamp<size_t>(m_lightingPresetIndex, 0, (m_presets.size() - 1));
		}

		[[nodiscard]] const Array<Preset>& getPresets() const
		{
			return m_presets;
		}

		[[nodiscard]] size_t getPresetIndex() const
		{
			return m_lightingPresetIndex;
		}

		[[nodiscard]] String getCurrentPresetName() const
		{
			return m_presets[m_lightingPresetIndex].name;
		}

        [[nodiscard]] Pi3D::LightingSettings getSettings() const
		{
            Pi3D::LightingSettings settings;
			settings.presetIndex = m_lightingPresetIndex;
			settings.sunIntensityScale = m_sunIntensityScale;
			settings.ambientIntensityScale = m_ambientIntensityScale;
			settings.sunDirectionOverride = m_sunDirectionOverride;
			return settings;
		}

        void applySettings(const Pi3D::LightingSettings& settings)
		{
			m_lightingPresetIndex = Clamp<size_t>(settings.presetIndex, 0, (m_presets.size() - 1));
			m_sunIntensityScale = Clamp(settings.sunIntensityScale, 0.0, 5.0);
			m_ambientIntensityScale = Clamp(settings.ambientIntensityScale, 0.0, 3.0);
			if (settings.sunDirectionOverride)
			{
				m_sunDirectionOverride = (*settings.sunDirectionOverride % 8);
			}
			else
			{
				m_sunDirectionOverride.reset();
			}
		}

		[[nodiscard]] double getUIBodyHeight() const
		{
			return 76.0 + m_presets.size() * ui::layout::RowHeight + 8.0
				+ 38.0
				+ 42.0
				+ 26.0
				+ 98.0
				+ 8.0
				+ 30.0
				+ 8.0;
		}

		[[nodiscard]] double getHeaderHeight() const
		{
			return 76.0;
		}

		[[nodiscard]] ColorF apply() const
		{
			const Preset& p = m_presets[m_lightingPresetIndex];
			Vec3 dir = p.sunDirection;
			if (m_sunDirectionOverride)
			{
				const double y = dir.y;
				const double horizLen = std::sqrt(Max(0.0, 1.0 - y * y));
				const Vec2 xz = directionXZ(*m_sunDirectionOverride);
				const double xzLen = xz.length();
				if (xzLen > 0.0)
				{
					const Vec2 xzScaled = xz / xzLen * horizLen;
					dir = Vec3{ xzScaled.x, y, xzScaled.y };
				}
			}

			Graphics3D::SetSunDirection(dir.normalized());
			Graphics3D::SetSunColor(p.sunColor.removeSRGBCurve() * (p.sunIntensity * m_sunIntensityScale));
			Graphics3D::SetGlobalAmbientColor(p.ambientColor.removeSRGBCurve() * m_ambientIntensityScale);
			return p.backgroundColor.removeSRGBCurve();
		}

		void drawUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
		{
			const Array<String> lightingPresetNames = m_presets.map([](const Preset& p) { return p.name; });
			const double lightingSectionBodyHeight = getUIBodyHeight();
			const RectF lightingSectionRect{ uiPos, contentWidth, lightingSectionBodyHeight };
			ui::Section(lightingSectionRect);
			const Preset& lp = m_presets[m_lightingPresetIndex];
			uiFont(U"ライティング (時間帯)").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
			uiFont(U"現在: {}"_fmt(lp.name)).draw(uiPos.movedBy(8, 26), Palette::Dimgray);
			uiFont(lp.description).draw(uiPos.movedBy(8, 50), Palette::Gray);

			const RectF radioRect{
				lightingSectionRect.x + 8,
				lightingSectionRect.y + 76,
				lightingSectionRect.w - 16,
				m_presets.size() * ui::layout::RowHeight
			};
			ui::RadioList(uiFont, m_lightingPresetIndex, lightingPresetNames, radioRect, ui::layout::RowHeight);

			double cy = 76.0 + m_presets.size() * ui::layout::RowHeight + 8.0;

			const double sliderLabelW = 110.0;
			const double sliderTotalW = lightingSectionRect.w - 16;
			ui::SliderH(U"Sun 強度", m_sunIntensityScale, 0.0, 5.0,
				uiPos.movedBy(8, cy), sliderLabelW, sliderTotalW - sliderLabelW);
			cy += 38.0;

			ui::SliderH(U"Ambient 強度", m_ambientIntensityScale, 0.0, 3.0,
				uiPos.movedBy(8, cy), sliderLabelW, sliderTotalW - sliderLabelW);
			cy += 42.0;

			uiFont(U"Sun 方向 (水平方位)").draw(uiPos.movedBy(8, cy), ui::GetTheme().text);
			cy += 26.0;

			constexpr int gridLayout[3][3] = {
				{ 7, 0, 1 },
				{ 6, -1, 2 },
				{ 5, 4, 3 },
			};
			constexpr double cellW = 60.0, cellH = 30.0, gap = 4.0;
			const Vec2 gridOrigin = uiPos.movedBy(8, cy);
			for (int r = 0; r < 3; ++r)
			{
				for (int c = 0; c < 3; ++c)
				{
					const int idx = gridLayout[r][c];
					if (idx < 0) { continue; }
					const RectF btn{
						gridOrigin.x + c * (cellW + gap),
						gridOrigin.y + r * (cellH + gap),
						cellW, cellH
					};
					const bool selected = m_sunDirectionOverride && (*m_sunDirectionOverride == static_cast<size_t>(idx));
					if (ui::Button(uiFont, DirectionLabels[idx], btn))
					{
						m_sunDirectionOverride = static_cast<size_t>(idx);
					}
					if (selected)
					{
						btn.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
					}
				}
			}
			cy += 3 * cellH + 2 * gap + 8.0;

			const RectF resetBtn{ uiPos.x + 8, uiPos.y + cy, lightingSectionRect.w - 16, 30.0 };
			if (ui::Button(uiFont, U"プリセット方向に戻す", resetBtn))
			{
				m_sunDirectionOverride.reset();
			}

			uiPos.y += lightingSectionRect.h + ui::layout::SectionGap;
		}

	private:
		inline static constexpr const char32* DirectionLabels[8] = {
			U"N", U"NE", U"E", U"SE", U"S", U"SW", U"W", U"NW"
		};

		[[nodiscard]] static Vec2 directionXZ(size_t index)
		{
			constexpr Vec2 dirs[8] = {
				{  0, -1 }, {  1, -1 }, {  1,  0 }, {  1,  1 },
				{  0,  1 }, { -1,  1 }, { -1,  0 }, { -1, -1 },
			};
			return dirs[index % 8];
		}

		[[nodiscard]] static FilePath resolveTomlPath()
		{
			const Array<FilePath> candidates = {
				U"Addons/Pi3D/Resources/toml/lighting_presets.toml",
				U"../Addons/Pi3D/Resources/toml/lighting_presets.toml",
				U"3D_Pi/Addons/Pi3D/Resources/toml/lighting_presets.toml",
			};
			for (const auto& p : candidates)
			{
				if (FileSystem::Exists(p))
				{
					return p;
				}
			}
			return candidates.front();
		}

		[[nodiscard]] static Vec3 readTomlVec3(const TOMLValue& value, const Vec3& fallback)
		{
			try
			{
				if (not value.isArray())
				{
					return fallback;
				}
				Array<double> v;
				for (const auto& item : value.arrayView())
				{
					if (const auto d = item.getOpt<double>()) { v << *d; }
					else if (const auto i = item.getOpt<int32>()) { v << static_cast<double>(*i); }
					if (v.size() >= 3) { break; }
				}
				if (v.size() < 3)
				{
					return fallback;
				}
				return Vec3{ v[0], v[1], v[2] };
			}
			catch (const std::exception&)
			{
				return fallback;
			}
		}

		[[nodiscard]] static ColorF readTomlColor(const TOMLValue& value, const ColorF& fallback)
		{
			try
			{
				if (not value.isArray())
				{
					return fallback;
				}
				Array<double> v;
				for (const auto& item : value.arrayView())
				{
					if (const auto d = item.getOpt<double>()) { v << Clamp(*d, 0.0, 1.0); }
					else if (const auto i = item.getOpt<int32>()) { v << Clamp(static_cast<double>(*i) / 255.0, 0.0, 1.0); }
					if (v.size() >= 4) { break; }
				}
				if (v.size() < 3)
				{
					return fallback;
				}
				return ColorF{ v[0], v[1], v[2], (v.size() >= 4 ? v[3] : fallback.a) };
			}
			catch (const std::exception&)
			{
				return fallback;
			}
		}

		[[nodiscard]] static Array<Preset> defaultPresets()
		{
			return {
				Preset{ U"朝", U"低めの太陽、ほのかに暖色、青い空気感", Vec3{ 0.70, 0.45, 0.30 }.normalized(), ColorF{ 1.00, 0.92, 0.80 }, 1.0, ColorF{ 0.24, 0.30, 0.40 }, ColorF{ 0.55, 0.70, 0.85 } },
				Preset{ U"昼", U"高い太陽、影は短くコントラスト弱め", Vec3{ 0.30, 0.90, 0.30 }.normalized(), ColorF{ 1.00, 0.98, 0.95 }, 1.0, ColorF{ 0.32, 0.34, 0.38 }, ColorF{ 0.40, 0.60, 0.80 } },
				Preset{ U"マジックアワー", U"低い太陽、強い暖色光と青いシャドウ (ACES 推奨)", Vec3{ 0.85, 0.30, 0.50 }.normalized(), ColorF{ 1.00, 0.62, 0.38 }, 3.0, ColorF{ 0.22, 0.28, 0.42 }, ColorF{ 0.85, 0.55, 0.50 } },
				Preset{ U"夜", U"月光、暗く青いトーン、暗部を沈める", Vec3{ -0.30, 0.55, -0.50 }.normalized(), ColorF{ 0.45, 0.55, 0.85 }, 1.0, ColorF{ 0.08, 0.10, 0.16 }, ColorF{ 0.05, 0.07, 0.14 } },
			};
		}

		[[nodiscard]] static Array<Preset> loadPresetsFromToml()
		{
			Array<Preset> results;
			const TOMLReader toml{ resolveTomlPath() };
			if (not toml)
			{
				return results;
			}

			const Array<Preset> defaults = defaultPresets();
			try
			{
				for (const auto& t : toml[U"presets"].tableArrayView())
				{
					const Preset base = (results.size() < defaults.size()) ? defaults[results.size()] : defaults.front();
					Preset p;
					p.name = t[U"name"].getOpt<String>().value_or(base.name);
					p.description = t[U"description"].getOpt<String>().value_or(base.description);
					p.sunDirection = readTomlVec3(t[U"sunDirection"], base.sunDirection).normalized();
					p.sunColor = readTomlColor(t[U"sunColor"], base.sunColor);
					p.sunIntensity = t[U"sunIntensity"].getOpt<double>().value_or(base.sunIntensity);
					p.ambientColor = readTomlColor(t[U"ambientColor"], base.ambientColor);
					p.backgroundColor = readTomlColor(t[U"backgroundColor"], base.backgroundColor);
					results << p;
				}
			}
			catch (const std::exception&)
			{
			}
			return results;
		}

		Array<Preset> m_presets;
		size_t m_lightingPresetIndex = 1;
		double m_sunIntensityScale = 1.0;
		double m_ambientIntensityScale = 1.0;
		Optional<size_t> m_sunDirectionOverride;
	};
}
