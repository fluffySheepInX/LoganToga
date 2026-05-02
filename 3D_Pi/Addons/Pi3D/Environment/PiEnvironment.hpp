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
        static constexpr double UIBodyHeight = 690.0;

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
            const RectF environmentSectionRect{ uiPos, contentWidth, UIBodyHeight };
            ui::Section(environmentSectionRect);
            font(U"環境").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
            font(U"雨と地面表示を調整します").draw(uiPos.movedBy(8, 28), Palette::Gray);

            const RectF rainRect{ uiPos.x + 8, uiPos.y + 58, (contentWidth - 24) * 0.5, ui::layout::AddButtonHeight };
            if (ui::Button(font, m_rain.isEnabled() ? U"雨: ON" : U"雨: OFF", rainRect))
            {
                m_rain.toggle();
            }
            if (m_rain.isEnabled())
            {
                rainRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
            }

            const RectF groundRect{ rainRect.rightX() + 8, rainRect.y, (contentWidth - 24) * 0.5, ui::layout::AddButtonHeight };
            if (ui::Button(font, U"地面: {}"_fmt(getGroundModeLabel()), groundRect))
            {
                cycleGroundMode();
            }
            if (m_groundMode != PiGroundMode::Texture)
            {
                groundRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
            }

            double dropCount = static_cast<double>(m_rain.getDropCount());
            ui::SliderH(U"雨量: {:.0f}"_fmt(dropCount), dropCount, 0.0, 1200.0,
                uiPos.movedBy(8, 108), 110.0, contentWidth - 126.0);
            m_rain.setDropCount(static_cast<int32>(std::round(dropCount)));

            double fallSpeed = m_rain.getFallSpeed();
            ui::SliderH(U"速度: {:.1f}"_fmt(fallSpeed), fallSpeed, 1.0, 60.0,
                uiPos.movedBy(8, 148), 110.0, contentWidth - 126.0);
            m_rain.setFallSpeed(fallSpeed);

            double windStrength = m_rain.getWindStrength();
            ui::SliderH(U"風: {:.2f}"_fmt(windStrength), windStrength, -0.5, 0.5,
                uiPos.movedBy(8, 188), 110.0, contentWidth - 126.0);
            m_rain.setWindStrength(windStrength);

            double streakLength = m_rain.getStreakLength();
            ui::SliderH(U"線長: {:.2f}"_fmt(streakLength), streakLength, 0.4, 4.0,
                uiPos.movedBy(8, 228), 110.0, contentWidth - 126.0);
            m_rain.setStreakLength(streakLength);

            const RectF fogRect{ uiPos.x + 8, uiPos.y + 280, contentWidth - 16, ui::layout::AddButtonHeight };
            if (ui::Button(font, m_fogEnabled ? U"フォグ: ON" : U"フォグ: OFF", fogRect))
            {
                m_fogEnabled = (not m_fogEnabled);
            }
            if (m_fogEnabled)
            {
                fogRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
            }

            ui::SliderH(U"Fog 開始: {:.1f}"_fmt(m_fogStartDistance), m_fogStartDistance, 0.0, 180.0,
                uiPos.movedBy(8, 330), 110.0, contentWidth - 126.0);
            ui::SliderH(U"Fog 終端: {:.1f}"_fmt(m_fogEndDistance), m_fogEndDistance, 1.0, 260.0,
                uiPos.movedBy(8, 370), 110.0, contentWidth - 126.0);
            ui::SliderH(U"Fog 濃度: {:.2f}"_fmt(m_fogDensity), m_fogDensity, 0.0, 1.0,
                uiPos.movedBy(8, 410), 110.0, contentWidth - 126.0);
            if (m_fogEndDistance < (m_fogStartDistance + 1.0))
            {
                m_fogEndDistance = m_fogStartDistance + 1.0;
            }

            font(U"Fog 色").draw(uiPos.movedBy(8, 450), ui::GetTheme().text);
            ui::SliderH(U"R: {:.2f}"_fmt(m_fogColor.r), m_fogColor.r, 0.0, 1.0,
                uiPos.movedBy(8, 478), 110.0, contentWidth - 126.0);
            ui::SliderH(U"G: {:.2f}"_fmt(m_fogColor.g), m_fogColor.g, 0.0, 1.0,
                uiPos.movedBy(8, 518), 110.0, contentWidth - 126.0);
            ui::SliderH(U"B: {:.2f}"_fmt(m_fogColor.b), m_fogColor.b, 0.0, 1.0,
                uiPos.movedBy(8, 558), 110.0, contentWidth - 126.0);

            const double presetButtonW = (contentWidth - 24) * 0.5;
            const RectF morningFogRect{ uiPos.x + 8, uiPos.y + 604, presetButtonW, ui::layout::AddButtonHeight };
            if (ui::Button(font, U"朝霧", morningFogRect))
            {
                applyFogPreset(ColorF{ 0.74, 0.82, 0.90, 1.0 }, 14.0, 82.0, 0.52);
            }

            const RectF sunsetFogRect{ morningFogRect.rightX() + 8, morningFogRect.y, presetButtonW, ui::layout::AddButtonHeight };
            if (ui::Button(font, U"夕焼け", sunsetFogRect))
            {
                applyFogPreset(ColorF{ 0.92, 0.58, 0.44, 1.0 }, 18.0, 100.0, 0.45);
            }

            const RectF dustFogRect{ uiPos.x + 8, uiPos.y + 642, presetButtonW, ui::layout::AddButtonHeight };
            if (ui::Button(font, U"砂埃", dustFogRect))
            {
                applyFogPreset(ColorF{ 0.72, 0.61, 0.42, 1.0 }, 10.0, 74.0, 0.62);
            }

            const RectF nightFogRect{ dustFogRect.rightX() + 8, dustFogRect.y, presetButtonW, ui::layout::AddButtonHeight };
            if (ui::Button(font, U"夜霧", nightFogRect))
            {
                applyFogPreset(ColorF{ 0.25, 0.34, 0.55, 1.0 }, 12.0, 86.0, 0.58);
            }

            uiPos.y += environmentSectionRect.h + ui::layout::SectionGap;
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
