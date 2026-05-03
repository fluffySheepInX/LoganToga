# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"

namespace road
{
    inline constexpr int32 MaterialTextureSize = 512;

    [[nodiscard]] inline double Saturate(const double value)
    {
        return Clamp(value, 0.0, 1.0);
    }

    [[nodiscard]] inline double Smooth01(const double value)
    {
        const double t = Saturate(value);
        return (t * t * (3.0 - (2.0 * t)));
    }

    [[nodiscard]] inline double SmoothStep(const double edge0, const double edge1, const double value)
    {
        if (Abs(edge1 - edge0) <= 0.000001)
        {
            return (value < edge0 ? 0.0 : 1.0);
        }

        return Smooth01((value - edge0) / (edge1 - edge0));
    }

    [[nodiscard]] inline double HashNoise(const int32 x, const int32 y)
    {
        uint32 n = static_cast<uint32>(x) * 1973u + static_cast<uint32>(y) * 9277u + 0x68bc21ebu;
        n = (n << 13u) ^ n;
        const uint32 hashed = (n * (n * n * 15731u + 789221u) + 1376312589u);
        return ((hashed & 0x7fffffffu) / 2147483647.0);
    }

    [[nodiscard]] inline double ValueNoise(const double x, const double y)
    {
        const int32 ix = static_cast<int32>(std::floor(x));
        const int32 iy = static_cast<int32>(std::floor(y));
        const double fx = x - ix;
        const double fy = y - iy;
        const double sx = Smooth01(fx);
        const double sy = Smooth01(fy);

        const double v00 = HashNoise(ix, iy);
        const double v10 = HashNoise(ix + 1, iy);
        const double v01 = HashNoise(ix, iy + 1);
        const double v11 = HashNoise(ix + 1, iy + 1);
        const double a = Math::Lerp(v00, v10, sx);
        const double b = Math::Lerp(v01, v11, sx);
        return Math::Lerp(a, b, sy);
    }

    [[nodiscard]] inline double Fbm(const double x, const double y, const int32 octaves)
    {
        double total = 0.0;
        double amplitude = 0.5;
        double frequency = 1.0;
        double normalization = 0.0;

        for (int32 i = 0; i < octaves; ++i)
        {
            total += (ValueNoise(x * frequency, y * frequency) * amplitude);
            normalization += amplitude;
            amplitude *= 0.5;
            frequency *= 2.0;
        }

        return ((normalization > 0.0) ? (total / normalization) : 0.0);
    }

    [[nodiscard]] inline double GaussianBand(const double x, const double center, const double sigma)
    {
        if (sigma <= 0.0)
        {
            return 0.0;
        }

        const double d = (x - center) / sigma;
        return Math::Exp(-(d * d));
    }

    [[nodiscard]] inline ColorF LerpColor(const ColorF& a, const ColorF& b, const double t)
    {
        return ColorF{
            Math::Lerp(a.r, b.r, t),
            Math::Lerp(a.g, b.g, t),
            Math::Lerp(a.b, b.b, t),
            Math::Lerp(a.a, b.a, t)
        };
    }

    inline void ClampRoadMaterialSettings(RoadMaterialSettings& settings)
    {
        settings.baseBrightness = Clamp(settings.baseBrightness, 0.4, 3.0);
        settings.baseWarmth = Clamp(settings.baseWarmth, -0.4, 1.5);
        settings.macroVariation = Clamp(settings.macroVariation, 0.0, 3.0);
        settings.detailVariation = Clamp(settings.detailVariation, 0.0, 3.0);
        settings.trackStrength = Clamp(settings.trackStrength, 0.0, 3.0);
        settings.trackWidth = Clamp(settings.trackWidth, 0.02, 0.6);
        settings.edgeMudStrength = Clamp(settings.edgeMudStrength, 0.0, 3.0);
        settings.pebbleStrength = Clamp(settings.pebbleStrength, 0.0, 3.0);
        settings.sootStrength = Clamp(settings.sootStrength, 0.0, 3.0);
        settings.shoulderWidthExpand = Clamp(settings.shoulderWidthExpand, 0.0, 8.0);
        settings.shoulderOpacity = Clamp(settings.shoulderOpacity, 0.0, 3.0);
        settings.shoulderBrightness = Clamp(settings.shoulderBrightness, 0.4, 3.0);
        settings.shoulderOuterFade = Clamp(settings.shoulderOuterFade, 0.55, 0.98);
    }

    [[nodiscard]] inline Image CreateRoadMaterialTexture(const FilePath& texturePath, RoadMaterialSettings settings)
    {
        static_cast<void>(texturePath);
        ClampRoadMaterialSettings(settings);

        Image image{ Size{ MaterialTextureSize, MaterialTextureSize }, Color{ 0, 0, 0, 255 } };
        constexpr double RoadEdgeFadeBegin = 0.82;
        constexpr double RoadEdgeFadeEnd = 1.0;
        const double warmth = settings.baseWarmth;
        const double brightness = settings.baseBrightness;
        const ColorF dryBase{ (0.46 + warmth * 0.10) * brightness, (0.42 + warmth * 0.05) * brightness, (0.36 - warmth * 0.06) * brightness };
        const ColorF dryHighlight{ (0.56 + warmth * 0.12) * brightness, (0.52 + warmth * 0.06) * brightness, (0.44 - warmth * 0.08) * brightness };
        const ColorF mudColor{ 0.27, 0.24, 0.20 };
        const ColorF trackColor{ 0.22, 0.20, 0.18 };
        const ColorF pebbleColor{ 0.70, 0.66, 0.57 };
        const ColorF sootColor{ 0.12, 0.11, 0.11 };

        for (int32 y = 0; y < MaterialTextureSize; ++y)
        {
            for (int32 x = 0; x < MaterialTextureSize; ++x)
            {
                const double u = (x / static_cast<double>(MaterialTextureSize - 1));
                const double v = (y / static_cast<double>(MaterialTextureSize - 1));
                const double centeredU = Abs((u - 0.5) * 2.0);

                const double macroNoise = Fbm((u * 2.8) + 9.7, (v * 3.4) + 1.5, 4);
                const double detailNoise = Fbm((u * 14.0) + 3.2, (v * 18.0) + 5.8, 3);
                const double longitudinalNoise = Fbm(0.8, (v * 10.0) + 12.0, 4);
                const double edgeMask = SmoothStep(0.56, 1.0, centeredU);
                const double centerWear = 1.0 - SmoothStep(0.0, 0.72, centeredU);
                const double edgeFade = (1.0 - SmoothStep(RoadEdgeFadeBegin, RoadEdgeFadeEnd, centeredU));
                const double mudMask = Saturate(edgeMask * (0.45 + (0.55 * Fbm((u * 8.0) + 20.0, (v * 7.0) + 14.0, 4))));
                const double trackMask = Saturate((GaussianBand(u, 0.34, settings.trackWidth) + GaussianBand(u, 0.66, settings.trackWidth)) * (0.65 + (0.35 * longitudinalNoise)));
                const double stoneMask = SmoothStep(0.84, 0.97, Fbm((u * 42.0) + 2.0, (v * 54.0) + 7.0, 2)) * (0.35 + (0.65 * edgeMask));
                const double sootMask = SmoothStep(0.90, 0.985, Fbm((u * 16.0) + 31.0, (v * 11.0) + 17.0, 3)) * (0.35 + (0.65 * centerWear));

                ColorF color = LerpColor(dryBase, dryHighlight, (0.5 + ((macroNoise - 0.5) * settings.macroVariation * 1.6)));
                color *= (1.0 + ((detailNoise - 0.5) * settings.detailVariation * 0.35));
                color = LerpColor(color, mudColor, mudMask * settings.edgeMudStrength);
                color = LerpColor(color, trackColor, trackMask * settings.trackStrength);
                color = LerpColor(color, pebbleColor, stoneMask * settings.pebbleStrength);
                color = LerpColor(color, sootColor, sootMask * settings.sootStrength);
                image[y][x] = ColorF{ Saturate(color.r), Saturate(color.g), Saturate(color.b), edgeFade }.toColor();
            }
        }

        return image;
    }

    [[nodiscard]] inline Image CreateRoadShoulderBlendTexture(const FilePath& texturePath, RoadMaterialSettings settings)
    {
        static_cast<void>(texturePath);
        ClampRoadMaterialSettings(settings);

        Image image{ Size{ MaterialTextureSize, MaterialTextureSize }, Color{ 0, 0, 0, 0 } };
        const double shoulderBrightness = settings.shoulderBrightness;
        const ColorF groundBlend{ 0.50, 0.45, 0.39 };
        const ColorF shoulderBase{ 0.38 * shoulderBrightness, 0.34 * shoulderBrightness, 0.28 * shoulderBrightness };
        const ColorF shoulderPebble{ 0.58 * shoulderBrightness, 0.53 * shoulderBrightness, 0.44 * shoulderBrightness };
        const ColorF shoulderGrassTint{ 0.32 * shoulderBrightness, 0.37 * shoulderBrightness, 0.24 * shoulderBrightness };

        for (int32 y = 0; y < MaterialTextureSize; ++y)
        {
            for (int32 x = 0; x < MaterialTextureSize; ++x)
            {
                const double u = (x / static_cast<double>(MaterialTextureSize - 1));
                const double v = (y / static_cast<double>(MaterialTextureSize - 1));
                const double centeredU = Abs((u - 0.5) * 2.0);
                const double macroNoise = Fbm((u * 4.2) + 11.3, (v * 3.0) + 2.1, 4);
                const double detailNoise = Fbm((u * 18.0) + 4.0, (v * 22.0) + 7.0, 3);
                const double pebbleNoise = Fbm((u * 38.0) + 17.0, (v * 41.0) + 29.0, 2);
                const double grassNoise = Fbm((u * 9.0) + 33.0, (v * 8.0) + 14.0, 3);

                const double feather = SmoothStep(0.55, 1.0, centeredU);
                const double outerFade = (1.0 - SmoothStep(settings.shoulderOuterFade, 1.0, centeredU));
                const double outerColorFade = (1.0 - outerFade);
                const double alphaNoise = (0.72 + ((macroNoise - 0.5) * 0.28));
                const double alpha = Saturate(outerFade * (1.0 - (feather * 0.28)) * alphaNoise * settings.shoulderOpacity);

                ColorF color = LerpColor(shoulderBase, shoulderPebble, SmoothStep(0.76, 0.96, pebbleNoise) * (0.25 + settings.pebbleStrength * 0.45));
                color = LerpColor(color, shoulderGrassTint, SmoothStep(0.62, 0.92, grassNoise) * 0.18);
                color *= (0.92 + ((detailNoise - 0.5) * 0.18));
                color = LerpColor(color, groundBlend, SmoothStep(0.62, 1.0, centeredU) * (0.60 + (macroNoise * 0.18)));
                if (settings.shoulderUseColorFade)
                {
                    const double fadeStrength = Saturate((outerColorFade * 0.82) + (SmoothStep(0.70, 1.0, centeredU) * 0.18));
                    color = LerpColor(color, groundBlend, fadeStrength);
                }
                image[y][x] = ColorF{ Saturate(color.r), Saturate(color.g), Saturate(color.b), alpha }.toColor();
            }
        }

        return image;
    }
}
