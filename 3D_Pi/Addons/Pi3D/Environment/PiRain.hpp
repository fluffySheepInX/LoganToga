# pragma once
# include <Siv3D.hpp>

namespace Pi3D
{
    class PiRain
    {
    public:
        struct Settings
        {
            bool enabled = false;
            int32 dropCount = 420;
            double fallSpeed = 24.0;
            double windStrength = 0.12;
            double streakLength = 1.8;
            double alpha = 0.55;
        };

        PiRain(const int32 dropCount = 420)
        {
            setDropCount(dropCount);
        }

        void setEnabled(const bool enabled)
        {
            m_enabled = enabled;
        }

        [[nodiscard]] bool isEnabled() const
        {
            return m_enabled;
        }

        void toggle()
        {
            m_enabled = (not m_enabled);
        }

        void setDropCount(const int32 dropCount)
        {
            const int32 clamped = Max(0, dropCount);
            m_drops.clear();
            m_drops.reserve(clamped);
            for (int32 i = 0; i < clamped; ++i)
            {
                m_drops << Vec3{
                    Random(-70.0, 70.0),
                    Random(2.0, 38.0),
                    Random(-70.0, 70.0)
                };
            }
            m_settings.dropCount = clamped;
        }

        [[nodiscard]] int32 getDropCount() const
        {
            return m_settings.dropCount;
        }

        void setFallSpeed(const double fallSpeed)
        {
            m_settings.fallSpeed = Max(0.0, fallSpeed);
        }

        [[nodiscard]] double getFallSpeed() const
        {
            return m_settings.fallSpeed;
        }

        void setWindStrength(const double windStrength)
        {
            m_settings.windStrength = Clamp(windStrength, -1.0, 1.0);
        }

        [[nodiscard]] double getWindStrength() const
        {
            return m_settings.windStrength;
        }

        void setStreakLength(const double streakLength)
        {
            m_settings.streakLength = Clamp(streakLength, 0.2, 6.0);
        }

        [[nodiscard]] double getStreakLength() const
        {
            return m_settings.streakLength;
        }

        void setAlpha(const double alpha)
        {
            m_settings.alpha = Clamp(alpha, 0.05, 1.0);
        }

        [[nodiscard]] double getAlpha() const
        {
            return m_settings.alpha;
        }

        void setSettings(const Settings& settings)
        {
            setEnabled(settings.enabled);
            setDropCount(settings.dropCount);
            setFallSpeed(settings.fallSpeed);
            setWindStrength(settings.windStrength);
            setStreakLength(settings.streakLength);
            setAlpha(settings.alpha);
        }

        [[nodiscard]] Settings getSettings() const
        {
            Settings settings = m_settings;
            settings.enabled = m_enabled;
            return settings;
        }

        void update(const double deltaTime)
        {
            if (not m_enabled)
            {
                return;
            }

            const double fallDistance = deltaTime * m_settings.fallSpeed;
            for (Vec3& drop : m_drops)
            {
                drop.y -= fallDistance;
                drop.x -= fallDistance * m_settings.windStrength;
                if (drop.y < 0.2)
                {
                    drop.x = Random(-70.0, 70.0);
                    drop.y = Random(24.0, 38.0);
                    drop.z = Random(-70.0, 70.0);
                }
            }
        }

        void draw3D() const
        {
            if (not m_enabled)
            {
                return;
            }

            const ScopedRenderStates3D renderStates{ BlendState::Additive, RasterizerState::SolidCullNone };
            for (const Vec3& drop : m_drops)
            {
                Line3D{ drop, drop + Vec3{ -m_settings.windStrength, -m_settings.streakLength, 0.0 } }.draw(ColorF{ 0.58, 0.70, 0.90, m_settings.alpha }.removeSRGBCurve());
            }
        }

    private:
        bool m_enabled = false;
        Settings m_settings;
        Array<Vec3> m_drops;
    };
}
