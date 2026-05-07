# include "../stdafx.h"
# include "StairGenerator.hpp"

namespace procedural
{
    namespace
    {
        [[nodiscard]] Vec3 SafeNormalize(const Vec3& value, const Vec3& fallback)
        {
            if (value.lengthSq() <= 0.000001)
            {
                return fallback;
            }

            return value.normalized();
        }

        [[nodiscard]] Quaternion MakeUpVectorRotation(const Vec3& direction)
        {
            const Vec3 up = Vec3{ 0, 1, 0 };
            const Vec3 forward = SafeNormalize(direction, up);
            const double dot = Clamp(up.dot(forward), -1.0, 1.0);

            if (Abs(dot - 1.0) <= 0.000001)
            {
                return Quaternion::Identity();
            }

            if (Abs(dot + 1.0) <= 0.000001)
            {
                return Quaternion::RotateX(Math::Pi);
            }

            const Vec3 axis = SafeNormalize(up.cross(forward), Vec3{ 1, 0, 0 });
            return Quaternion{ axis, Math::Acos(dot) };
        }

        [[nodiscard]] double Noise01(const PerlinNoise& noise, const double x)
        {
            return (0.5 + noise.noise1D(x) * 0.5);
        }

        [[nodiscard]] double Hash01(const double x, const double y, const double seed)
        {
            const double value = Math::Sin((x * 12.9898) + (y * 78.233) + (seed * 37.719)) * 43758.5453;
            return value - Math::Floor(value);
        }

        [[nodiscard]] ColorF ApplyWetness(const ColorF& baseColor, const double wetness, const double lightAmount)
        {
            const double w = Clamp(wetness, 0.0, 1.0);
            ColorF result = baseColor;
            result.r = Math::Lerp(result.r, result.r * 0.62 + lightAmount, w);
            result.g = Math::Lerp(result.g, result.g * 0.68 + lightAmount, w);
            result.b = Math::Lerp(result.b, result.b * 0.72 + lightAmount, w);
            return ColorF{ Clamp(result.r, 0.0, 1.0), Clamp(result.g, 0.0, 1.0), Clamp(result.b, 0.0, 1.0), result.a };
        }

        [[nodiscard]] ColorF ShadeMaterial(const ColorF& baseColor, const Vec3& normal, const double wetness, const double rim = 0.0)
        {
            const Vec3 lightDir = SafeNormalize(Vec3{ -0.42, 0.86, -0.28 }, Vec3{ 0, 1, 0 });
            const double diffuse = 0.45 + 0.55 * Max(0.0, SafeNormalize(normal, Vec3{ 0, 1, 0 }).dot(lightDir));
            const double wetSpecular = Clamp(wetness, 0.0, 1.0) * (0.10 + rim * 0.20);
            ColorF lit = ApplyWetness(baseColor, wetness, wetSpecular);
            lit.r = lit.r * diffuse + wetSpecular;
            lit.g = lit.g * diffuse + wetSpecular;
            lit.b = lit.b * diffuse + wetSpecular;
            return ColorF{ Clamp(lit.r, 0.0, 1.0), Clamp(lit.g, 0.0, 1.0), Clamp(lit.b, 0.0, 1.0), lit.a }.removeSRGBCurve();
        }

        void DrawDropShadow(const Vec3& origin, const double radius, const double opacity)
        {
            const Transformer3D shadowTransform{
                Mat4x4::Identity()
                    .scaled(Float3{ static_cast<float>(radius), 0.018f, static_cast<float>(radius * 0.62) })
                    .translated(origin + Vec3{ 0.18, 0.012, 0.22 })
            };
            const double shadowTone = Clamp(0.10 + opacity * 0.20, 0.0, 0.35);
            Sphere{ Vec3{ 0, 0, 0 }, 1.0 }.draw(ColorF{ shadowTone, shadowTone, shadowTone }.removeSRGBCurve());
        }

        void DrawStairBlobShadow(const GeneratedStair& stair)
        {
            const double stairLength = stair.depth * stair.steps;
            const double maxHeight = stair.height * stair.steps;
            const Vec3 localCenter{ 0, 0.0, stairLength * 0.45 };
            const double angle = (stair.rotation01 * Math::TwoPi);
            const double c = Math::Cos(angle);
            const double s = Math::Sin(angle);
            const Vec3 rotatedCenter{
                (localCenter.x * c) - (localCenter.z * s),
                localCenter.y,
                (localCenter.x * s) + (localCenter.z * c)
            };
            const Vec3 shadowOrigin = stair.origin + rotatedCenter;

            const double baseRadius = Max(stair.width * 0.46, stairLength * 0.22);
            const double stretch = Clamp(1.0 + maxHeight * 0.08, 1.0, 1.5);
            DrawDropShadow(shadowOrigin, baseRadius * stretch, 0.24);
        }
    }

        void StairGenerator::draw3D(const bool showPlacementMarker) const{
            for (size_t stairIndex = 0; stairIndex < m_stairs.size(); ++stairIndex)
            {
                const auto& stair = m_stairs[stairIndex];
                const bool selected = (m_selectedIndex && (*m_selectedIndex == stairIndex));
                DrawStairBlobShadow(stair);
                const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
                const Transformer3D transform{
                    Mat4x4::Identity()
                        .rotated(Quaternion::RotateY(static_cast<float>(stair.rotation01 * Math::TwoPi)))
                        .translated(stair.origin)
                };

                for (int32 i = 0; i < stair.steps; ++i)
                {
                    const double blockHeight = stair.height * (i + 1);
                    const Vec3 center{ 0, stair.origin.y + (blockHeight * 0.5), (stair.depth * i) + (stair.depth * 0.5) };
                    ColorF blockColor = getMaterializedColor(stair, i);
                    if (selected)
                    {
                        blockColor = ColorF{ Min(blockColor.r + 0.08, 1.0), Min(blockColor.g + 0.08, 1.0), Min(blockColor.b + 0.12, 1.0) };
                    }
                    Box{ center, stair.width, blockHeight, stair.depth }.draw(blockColor);
                }
            }

            for (const auto& naturalObject : m_naturalObjects)
            {
                drawNatureObject(naturalObject);
            }

            if (showPlacementMarker && m_generatePosition)
            {
                Cylinder{ *m_generatePosition + Vec3{ 0, 0.02, 0 }, 0.28, 0.04 }.draw(ColorF{ 0.25, 0.55, 0.95 }.removeSRGBCurve());
            }
        }



void StairGenerator::drawNatureObject(const GeneratedNatureObject& naturalObject) const{
            const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
            PerlinNoise noise{ naturalObject.variationSeed };
            const double wetness = Clamp(m_natureWetness, 0.0, 1.0);

            if (naturalObject.type == GeneratedNatureType::Tree)
            {
                const double trunkHeightNoise = Noise01(noise, 0.13);
                const double trunkRadiusNoise = Noise01(noise, 1.37);
                const double tiltXNoise = noise.noise1D(2.11);
                const double tiltZNoise = noise.noise1D(2.79);
                const double trunkHeight = 0.95 + trunkHeightNoise * 0.7;
                const double trunkRadius = 0.12 + trunkRadiusNoise * 0.08;
                const Vec3 trunkDirection = SafeNormalize(Vec3{ tiltXNoise * 0.3, 1.0, tiltZNoise * 0.3 }, Vec3{ 0, 1, 0 });
                DrawDropShadow(naturalObject.origin, 0.52 + trunkRadius * 2.4, 0.18 + wetness * 0.10);
                const Vec3 trunkCenter = naturalObject.origin + trunkDirection * (trunkHeight * 0.5);
                const Quaternion trunkRotation = MakeUpVectorRotation(trunkDirection);
                for (int32 barkIndex = 0; barkIndex < 9; ++barkIndex)
                {
                    const double barkT = (barkIndex + 0.5) / 9.0;
                    const double barkY = (barkT - 0.5) * trunkHeight;
                    const double barkNoise = Noise01(noise, 4.0 + barkIndex * 0.71);
                    const ColorF barkColor = ShadeMaterial(
                        ColorF{ 0.32 + barkNoise * 0.15, 0.18 + barkNoise * 0.07, 0.07 + barkNoise * 0.05 },
                        trunkDirection + Vec3{ noise.noise1D(7.0 + barkIndex) * 0.28, 0.0, noise.noise1D(9.0 + barkIndex) * 0.28 },
                        wetness,
                        barkNoise * 0.25);
                    const Transformer3D trunkTransform{
                        Mat4x4::Identity()
                            .rotated(trunkRotation)
                            .translated(trunkCenter + trunkDirection * barkY)
                    };
                    Cylinder{ Vec3{ 0, 0, 0 }, trunkRadius * (0.92 + barkNoise * 0.14), trunkHeight / 8.0 }.draw(barkColor);
                }

                const Vec3 canopyBase = naturalObject.origin + trunkDirection * trunkHeight;
                for (int32 leafIndex = 0; leafIndex < 4; ++leafIndex)
                {
                    const double offsetX = noise.noise1D(10.0 + leafIndex * 1.73) * 0.26;
                    const double offsetY = noise.noise1D(20.0 + leafIndex * 1.91) * 0.18;
                    const double offsetZ = noise.noise1D(30.0 + leafIndex * 2.07) * 0.26;
                    const double leafRadius = 0.24 + Noise01(noise, 40.0 + leafIndex * 1.37) * 0.20;
                    const double leafWarp = noise.noise1D(50.0 + leafIndex * 1.11) * 0.16;
                    const Vec3 leafCenter = canopyBase + Vec3{ offsetX, 0.18 + offsetY, offsetZ };
                    for (int32 gradientIndex = 0; gradientIndex < 3; ++gradientIndex)
                    {
                        const double gradientT = gradientIndex / 2.0;
                        const double brightness = 0.72 + gradientT * 0.34;
                        const Vec3 normal = SafeNormalize(Vec3{ offsetX * 0.8, 0.45 + gradientT, offsetZ * 0.8 }, Vec3{ 0, 1, 0 });
                        const ColorF leafColor = ShadeMaterial(
                            ColorF{ 0.13 * brightness, 0.48 * brightness, 0.18 * brightness },
                            normal,
                            wetness,
                            gradientT * 0.35);
                        const Transformer3D leafTransform{
                            Mat4x4::Identity()
                                .scaled(Float3{ 1.0f + static_cast<float>(leafWarp), 0.42f - static_cast<float>(leafWarp * 0.08), 1.0f + static_cast<float>(leafWarp * 0.3) })
                                .translated(leafCenter + Vec3{ 0, (gradientT - 0.5) * leafRadius * 0.72, 0 })
                        };
                        Sphere{ Vec3{ 0, 0, 0 }, leafRadius * (0.98 - gradientT * 0.10) }.draw(leafColor);
                    }
                }
                return;
            }

            const double stemHeight = 0.26 + Noise01(noise, 60.0) * 0.28;
            const double stemRadius = 0.06 + Noise01(noise, 61.0) * 0.04;
            const double capRadius = 0.18 + Noise01(noise, 62.0) * 0.18;
            const double capScaleY = 0.38 + Noise01(noise, 63.0) * 0.42;
            const Vec3 capOffset{ noise.noise1D(64.0) * 0.08, 0.0, noise.noise1D(65.0) * 0.08 };
            DrawDropShadow(naturalObject.origin, 0.28 + capRadius * 1.6, 0.14 + wetness * 0.08);
            const ColorF stemColor = ShadeMaterial(ColorF{ 0.96, 0.94, 0.86 }, Vec3{ 0.2, 0.9, -0.1 }, wetness, 0.1);
            Cylinder{ naturalObject.origin + Vec3{ 0, stemHeight * 0.5, 0 }, stemRadius, stemHeight }.draw(stemColor);
            {
                const Transformer3D capTransform{
                    Mat4x4::Identity()
                        .scaled(Float3{ 1.0f, static_cast<float>(capScaleY), 1.0f })
                        .translated(naturalObject.origin + Vec3{ 0, stemHeight, 0 } + capOffset)
                };
                Sphere{ Vec3{ 0, 0, 0 }, capRadius }.draw(ShadeMaterial(ColorF{ 0.78, 0.10, 0.09 }, Vec3{ 0, 1, 0 }, wetness, 0.2));
            }
            for (int32 dotIndex = 0; dotIndex < 10; ++dotIndex)
            {
                const double angle = Hash01(dotIndex, naturalObject.variationSeed, 2.0) * Math::TwoPi;
                const double radiusT = Math::Sqrt(Hash01(dotIndex, naturalObject.variationSeed, 3.0)) * 0.78;
                const Vec3 dotOffset{ Math::Cos(angle) * capRadius * radiusT, capRadius * capScaleY * (0.50 + 0.20 * (1.0 - radiusT)), Math::Sin(angle) * capRadius * radiusT };
                const double dotRadius = capRadius * (0.055 + Hash01(dotIndex, naturalObject.variationSeed, 4.0) * 0.035);
                Sphere{ naturalObject.origin + Vec3{ 0, stemHeight, 0 } + capOffset + dotOffset, dotRadius }
                    .draw(ShadeMaterial(ColorF{ 0.98, 0.92, 0.78 }, SafeNormalize(dotOffset + Vec3{ 0, capRadius, 0 }, Vec3{ 0, 1, 0 }), wetness, 0.1));
            }
            {
                const Transformer3D rimTransform{
                    Mat4x4::Identity()
                        .scaled(Float3{ 1.0f, 0.045f, 1.0f })
                        .translated(naturalObject.origin + Vec3{ 0, stemHeight + capRadius * capScaleY * 0.08, 0 } + capOffset)
                };
                const double rimBoost = 0.70 + wetness * 0.20;
                Sphere{ Vec3{ 0, 0, 0 }, capRadius * 1.03 }.draw(ColorF{ rimBoost, 0.24 + wetness * 0.10, 0.18 + wetness * 0.08 }.removeSRGBCurve());
            }
        }
}
