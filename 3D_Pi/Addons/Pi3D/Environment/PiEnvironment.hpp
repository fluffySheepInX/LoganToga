# pragma once
# include <Siv3D.hpp>
# include "PiGroundMode.hpp"
# include "PiRain.hpp"
# include "../PiSettings.hpp"
# include "../Shader/PiShaderLoader.hpp"
# include "../../../UI/RectUI.hpp"
# include "../../../UI/Layout.hpp"

namespace Pi3D
{
    class PiEnvironment
    {
    public:
        static constexpr double HeaderHeight = 86.0;

        [[nodiscard]] double getUIBodyHeight() const
        {
            return m_environmentPanelOpened ? 790.0 : 94.0;
        }

        void update(const double deltaTime)
        {
            m_rain.update(deltaTime);
        }

        void drawGround(const Mesh& groundPlane, const Texture& groundTexture) const
        {
            if (m_groundMode == PiGroundMode::None)
            {
                return;
            }

            if (m_groundMode == PiGroundMode::Gray)
            {
                groundPlane.draw(ColorF{ 0.45, 0.45, 0.45 }.removeSRGBCurve());
            }
            else if (m_groundMode == PiGroundMode::White)
            {
                groundPlane.draw(ColorF{ 0.92, 0.92, 0.92 }.removeSRGBCurve());
            }
            else if (m_groundMode == PiGroundMode::Grid)
            {
                groundPlane.draw(ColorF{ 0.22, 0.24, 0.28 }.removeSRGBCurve());

                const ScopedRenderStates3D wireframe{ BlendState::Opaque, RasterizerState::WireframeCullNone };
                groundPlane.draw(ColorF{ 0.78, 0.82, 0.88, 0.95 }.removeSRGBCurve());
            }
            else
            {
                groundPlane.draw(groundTexture);
            }
        }

        void draw3D() const
        {
            m_rain.draw3D();
        }

        [[nodiscard]] bool needsSceneDepth() const
        {
            return m_fogEnabled;
        }

        void applyFog(const Texture& source, const Texture& depthTexture) const
        {
            if (not m_fogEnabled)
            {
                source.draw();
                return;
            }

            m_fogParams = Float4{
                static_cast<float>(m_fogStartDistance),
                static_cast<float>(Max(m_fogStartDistance + 0.01, m_fogEndDistance)),
                static_cast<float>(m_fogDensity),
                0.0f };
            m_fogColorBuffer = Float4{
                static_cast<float>(m_fogColor.r),
                static_cast<float>(m_fogColor.g),
                static_cast<float>(m_fogColor.b),
                static_cast<float>(m_fogColor.a) };

            Graphics2D::SetPSConstantBuffer(1, m_fogParams);
            Graphics2D::SetPSConstantBuffer(2, m_fogColorBuffer);
            Graphics2D::SetPSTexture(1, depthTexture);

            {
                const ScopedCustomShader2D shader{ m_fogPS };
                source.draw();
            }
            Graphics2D::Flush();
            Graphics2D::SetPSTexture(1, none);
        }

        void drawUI(const Font& font, Vec2& uiPos, const double contentWidth)
        {
            const double bodyHeight = getUIBodyHeight();
            const double panelTopOffset = (ui::layout::HeaderHeight + 4.0);
            const double panelInnerPadding = 8.0;
            const double panelBottomOffset = 6.0;
            const RectF environmentSectionRect{ uiPos.x, uiPos.y + panelTopOffset, contentWidth, (bodyHeight - panelTopOffset) };
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

            if (not hoverTooltip.isEmpty())
            {
                ui::Tooltip(font, hoverTooltip, Cursor::PosF().movedBy(18, 20));
            }

            uiPos.y += bodyHeight + ui::layout::SectionGap;
        }

        [[nodiscard]] Pi3D::EnvironmentSettings getSettings() const
        {
            Pi3D::EnvironmentSettings settings;
            settings.groundMode = groundModeToString(m_groundMode);

            const auto rain = m_rain.getSettings();
            settings.rain.enabled = rain.enabled;
            settings.rain.dropCount = rain.dropCount;
            settings.rain.fallSpeed = rain.fallSpeed;
            settings.rain.windStrength = rain.windStrength;
            settings.rain.streakLength = rain.streakLength;
            settings.rain.alpha = rain.alpha;

            settings.fog.enabled = m_fogEnabled;
            settings.fog.startDistance = m_fogStartDistance;
            settings.fog.endDistance = m_fogEndDistance;
            settings.fog.density = m_fogDensity;
            settings.fog.color = m_fogColor;
            return settings;
        }

        void applySettings(const Pi3D::EnvironmentSettings& settings)
        {
            m_groundMode = stringToGroundMode(settings.groundMode);

            PiRain::Settings rainSettings;
            rainSettings.enabled = settings.rain.enabled;
            rainSettings.dropCount = settings.rain.dropCount;
            rainSettings.fallSpeed = settings.rain.fallSpeed;
            rainSettings.windStrength = settings.rain.windStrength;
            rainSettings.streakLength = settings.rain.streakLength;
            rainSettings.alpha = settings.rain.alpha;
            m_rain.setSettings(rainSettings);

            m_fogEnabled = settings.fog.enabled;
            m_fogStartDistance = Max(0.0, settings.fog.startDistance);
            m_fogEndDistance = Max(m_fogStartDistance + 1.0, settings.fog.endDistance);
            m_fogDensity = Clamp(settings.fog.density, 0.0, 1.0);
            m_fogColor = settings.fog.color;
        }

    private:
        [[nodiscard]] static String groundModeToString(const PiGroundMode mode)
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

        [[nodiscard]] static PiGroundMode stringToGroundMode(const String& mode)
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

        [[nodiscard]] StringView getGroundModeLabel() const
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

        void cycleGroundMode()
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

        void applyFogPreset(const ColorF& color, const double startDistance, const double endDistance, const double density)
        {
            m_fogEnabled = true;
            m_fogColor = color;
            m_fogStartDistance = Max(0.0, startDistance);
            m_fogEndDistance = Max(m_fogStartDistance + 1.0, endDistance);
            m_fogDensity = Clamp(density, 0.0, 1.0);
        }

        PiGroundMode m_groundMode = PiGroundMode::Texture;
        PiRain m_rain;
        bool m_fogEnabled = false;
        bool m_environmentPanelOpened = true;
        Texture m_environmentHelpIcon{ U"texture/hatena.png" };
        Texture m_environmentOpenCloseIcon{ U"texture/kaihei.png" };
        Texture m_groundModeIcon{ U"texture/zimenTexture.png" };
        Texture m_rainToggleIcon{ U"texture/ame.png" };
        Texture m_rainAmountIcon{ U"texture/ryou.png" };
        double m_fogStartDistance = 22.0;
        double m_fogEndDistance = 90.0;
        double m_fogDensity = 0.65;
        ColorF m_fogColor{ 0.58, 0.66, 0.76, 1.0 };
        const Array<ConstantBufferBinding> m_fogPSBindings = {
            { U"PSConstants2D", 0 },
            { U"PSEffectParams", 1 },
            { U"PSFogColor", 2 },
        };
        const PixelShader m_fogPS{ HLSL{ PiShaderLoader::HLSL(U"scene_fog"), U"PS" } | GLSL{ PiShaderLoader::GLSLFragment(U"scene_fog"), m_fogPSBindings } };
        mutable ConstantBuffer<Float4> m_fogParams{ Float4{ 22.0f, 90.0f, 0.65f, 0.0f } };
        mutable ConstantBuffer<Float4> m_fogColorBuffer{ Float4{ 0.58f, 0.66f, 0.76f, 1.0f } };
    };
}
