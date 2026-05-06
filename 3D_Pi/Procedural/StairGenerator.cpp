# include "../stdafx.h"
# include "StairGenerator.hpp"

namespace procedural
{
    namespace
    {
        constexpr double SectionMargin = 10.0;
        constexpr double SectionSpacing = 10.0;
        constexpr double StairHeaderHeight = 48.0;
        constexpr double StairBodyHeight = 164.0;
        constexpr double NatureHeaderHeight = 48.0;
        constexpr double NatureBodyHeight = 242.0;
        constexpr double SelectionHeight = 126.0;
        constexpr double TransformHeight = 78.0;
        constexpr double ColorHeight = 154.0;
        constexpr double MaterialHeaderHeight = 48.0;
        constexpr double MaterialBodyHeight = 126.0;

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

bool StairGenerator::wantsMouseCapture() const{
          syncCollapsedIconRegistry();
            return (not m_uiHidden) && getPanelRect().mouseOver();
        }



        void StairGenerator::setUIHidden(const bool hidden){
            m_uiHidden = hidden;
            if (m_uiHidden)
            {
                m_dragging = false;
            }
        }



        void StairGenerator::cancelTargetSelection(){
            m_waitingForPosition = false;
        }



        void StairGenerator::update(const BasicCamera3D& camera, const bool cursorBlockedByUI){
            if (m_uiHidden)
            {
                m_dragging = false;
                return;
            }

            syncCollapsedIconRegistry();

            if (not m_panelOpen)
            {
                const RectF collapsedIcon = getCollapsedIconRect();
                if (MouseL.down() && collapsedIcon.mouseOver())
                {
                    m_dragging = true;
                    m_ignoreCollapsedClickUntilRelease = false;
                    m_dragOffset = (Cursor::PosF() - collapsedIcon.pos);
                }

                if (m_dragging)
                {
                    if (MouseL.pressed())
                    {
                        if (Cursor::PosF().distanceFrom(collapsedIcon.pos + m_dragOffset) > 3.0)
                        {
                            m_ignoreCollapsedClickUntilRelease = true;
                        }
                        updateCollapsedIconDrag(collapsedIcon);
                    }
                    else
                    {
                        m_dragging = false;
                        m_ignoreCollapsedClickUntilRelease = false;
                    }
                }

                return;
            }

            const RectF panelRect = getPanelRect();
            const RectF headerRect{ panelRect.x, panelRect.y, panelRect.w, HeaderHeight };
            const RectF toggleRect{ panelRect.x + panelRect.w - 38, panelRect.y + 7, 28, 28 };

            if (MouseL.down() && headerRect.mouseOver() && (not toggleRect.mouseOver()))
            {
                m_dragging = true;
                m_dragOffset = (Cursor::PosF() - m_panelPos);
            }

            if (m_dragging)
            {
                if (MouseL.pressed())
                {
                    m_panelPos = Cursor::PosF() - m_dragOffset;
                    m_panelPos.x = Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - panelRect.w));
                    m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panelRect.h));
                }
                else
                {
                    m_dragging = false;
                }
            }

            if (m_waitingForPosition && MouseL.down() && (not cursorBlockedByUI))
            {
                const InfinitePlane gp{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };
                const Ray ray = camera.screenToRay(Cursor::PosF());
                if (const auto distance = ray.intersects(gp))
                {
                    m_generatePosition = ray.point_at(*distance);
                    m_waitingForPosition = false;
                }
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



        void StairGenerator::drawUI(){
            syncCollapsedIconRegistry();

            if (not m_panelOpen)
            {
                const RectF collapsedIcon = getCollapsedIconRect();
                collapsedIcon.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
                collapsedIcon.drawFrame(2.0, Palette::Black);
                if (m_toggleIcon)
                {
                    const double iconScale = Min(collapsedIcon.w / m_toggleIcon.width(), collapsedIcon.h / m_toggleIcon.height());
                    m_toggleIcon.scaled(iconScale).drawAt(collapsedIcon.center());
                }
                else
                {
                    ui::DefaultFont()(U"P").drawAt(collapsedIcon.center(), ui::GetTheme().text);
                }

                if ((not m_ignoreCollapsedClickUntilRelease) && collapsedIcon.leftClicked())
                {
                    expandFromCollapsedIcon();
                }
                return;
            }

            const RectF generatorPanel = getPanelRect();
            ui::Panel(generatorPanel);
            const RectF generatorHeader{ generatorPanel.x, generatorPanel.y, generatorPanel.w, HeaderHeight };
            generatorHeader.rounded(10).draw(ColorF{ 0.90, 0.93, 0.97, 0.96 });
            ui::DefaultFont()(U"3D Generator").draw(generatorHeader.x + 12, generatorHeader.y + 9, ui::GetTheme().text);
            const RectF generatorToggle{ generatorPanel.x + generatorPanel.w - 38, generatorPanel.y + 7, 28, 28 };
            if (MouseL.down() && generatorToggle.mouseOver())
            {
                m_dragging = true;
                m_ignoreCollapsedClickUntilRelease = false;
                m_togglePressCursor = Cursor::PosF();
                m_dragOffset = Cursor::PosF() - m_panelPos;
            }
            if (m_dragging && MouseL.pressed())
            {
                if (Cursor::PosF().distanceFrom(m_togglePressCursor) > 3.0)
                {
                    m_ignoreCollapsedClickUntilRelease = true;
                }
            }
            if ((not m_ignoreCollapsedClickUntilRelease) && ui::Button(ui::DefaultFont(), (m_panelOpen ? U"-" : U"+"), generatorToggle))
            {
                m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ProceduralEditor", Vec2{ generatorPanel.x, generatorPanel.y });
                m_panelOpen = (not m_panelOpen);
               syncCollapsedIconRegistry();
            }
            if (not MouseL.pressed())
            {
                m_ignoreCollapsedClickUntilRelease = false;
            }

            if (m_panelOpen)
            {
                double topY = (generatorPanel.y + 52);
                topY = drawCreationSection(generatorPanel, topY);
                topY = drawNatureSection(generatorPanel, topY);
                topY = drawSelectionSection(generatorPanel, topY);
                GeneratedStair* selectedStair = getSelectedStair();
                topY = drawTransformSection(generatorPanel, topY, selectedStair);
                topY = drawColorSection(generatorPanel, topY, selectedStair);
                drawMaterialSection(generatorPanel, topY, selectedStair);
            }
        }

RectF StairGenerator::getPanelRect() const{
         if (not m_panelOpen)
            {
                return getCollapsedIconRect();
            }

            const double panelHeight = getExpandedPanelHeight();
            const Vec2 clampedPos{
                Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - PanelWidth)),
                Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panelHeight))
            };
            return RectF{ clampedPos, PanelWidth, panelHeight };
        }



        RectF StairGenerator::getCollapsedIconRect() const{
            const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ProceduralEditor", m_panelPos);
            return RectF{ resolvedPos, ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize };
        }



        void StairGenerator::syncCollapsedIconRegistry() const{
            ui::editor_icon::RegisterCollapsedIcon(U"ProceduralEditor", (not m_panelOpen) ? Optional<RectF>{ getCollapsedIconRect() } : none);
        }



        void StairGenerator::updateCollapsedIconDrag(const RectF& dragRect){
            const Vec2 desiredPos = (Cursor::PosF() - m_dragOffset);
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ProceduralEditor", desiredPos, dragRect.size);
            syncCollapsedIconRegistry();
        }



        void StairGenerator::expandFromCollapsedIcon(){
            m_panelOpen = true;
            m_panelPos = Vec2{ 24, 112 };
            m_ignoreCollapsedClickUntilRelease = false;
            syncCollapsedIconRegistry();
        }



double StairGenerator::getExpandedPanelHeight() const{
            double height = 52.0 + SelectionHeight + TransformHeight + ColorHeight + MaterialHeaderHeight + (SectionSpacing * 4.0) + SectionMargin;

            height += StairHeaderHeight + SectionSpacing;
            if (m_stairPanelOpen)
            {
                height += StairBodyHeight + SectionSpacing;
            }

            height += NatureHeaderHeight + SectionSpacing;
            if (m_naturePanelOpen)
            {
                height += NatureBodyHeight + SectionSpacing;
            }

            if (m_materialPanelOpen)
            {
                height += MaterialBodyHeight + SectionSpacing;
            }

            return height;
        }



uint32 StairGenerator::makeNatureVariationSeed(const uint32 serial) const{
            uint32 seed = static_cast<uint32>(Math::Round(Clamp(m_natureSeed, 0.0, 9999.0)));
            seed ^= (serial * 0x9E3779B9u);
            seed ^= (seed << 13u);
            seed ^= (seed >> 17u);
            seed ^= (seed << 5u);
            return seed;
        }



void StairGenerator::regenerateNatureObjects(){
            for (auto& naturalObject : m_naturalObjects)
            {
                naturalObject.variationSeed = makeNatureVariationSeed(naturalObject.serial);
            }
        }



void StairGenerator::addNatureObject(const GeneratedNatureType type){
            if (not m_generatePosition)
            {
                return;
            }

            GeneratedNatureObject naturalObject;
            naturalObject.origin = *m_generatePosition;
            naturalObject.type = type;
            naturalObject.serial = m_nextNatureSerial++;
            naturalObject.variationSeed = makeNatureVariationSeed(naturalObject.serial);
            m_naturalObjects << naturalObject;
        }



GeneratedStair* StairGenerator::getSelectedStair(){
            if (m_selectedIndex && (*m_selectedIndex < m_stairs.size()))
            {
                return &m_stairs[*m_selectedIndex];
            }

            return nullptr;
        }



ColorF StairGenerator::getMaterializedColor(const GeneratedStair& stair, const int32 stepIndex){
            ColorF result = stair.color;
            const auto fract = [](const double value) { return value - Math::Floor(value); };
            const double noise = fract(Math::Sin((stair.origin.x * 12.9898) + (stair.origin.z * 78.233) + (stepIndex * 37.719)) * 43758.5453);

            if (stair.useDullNoise)
            {
                const double amount = Clamp(stair.dullNoiseAmount, 0.0, 1.0);
                const double gray = (result.r + result.g + result.b) / 3.0;
                const double saturationMix = amount * (0.35 + noise * 0.35);
                const double darken = 1.0 - amount * (0.08 + noise * 0.22);
                result.r = Math::Lerp(result.r, gray, saturationMix) * darken;
                result.g = Math::Lerp(result.g, gray, saturationMix) * darken;
                result.b = Math::Lerp(result.b, gray, saturationMix) * darken;
            }

            if (stair.useColorVariation)
            {
                const double amount = Clamp(stair.colorVariationAmount, 0.0, 1.0);
                const double rNoise = fract(noise * 17.13);
                const double gNoise = fract(noise * 29.71);
                const double bNoise = fract(noise * 43.37);
                result.r *= 1.0 + (rNoise - 0.5) * amount;
                result.g *= 1.0 + (gNoise - 0.5) * amount;
                result.b *= 1.0 + (bNoise - 0.5) * amount;
            }

            result.r = Clamp(result.r, 0.0, 1.0);
            result.g = Clamp(result.g, 0.0, 1.0);
            result.b = Clamp(result.b, 0.0, 1.0);
            return result.removeSRGBCurve();
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



        double StairGenerator::drawCreationSection(const RectF& generatorPanel, double topY){
            const RectF stairHeader{ generatorPanel.x + SectionMargin, topY, generatorPanel.w - (SectionMargin * 2.0), StairHeaderHeight };
            ui::Section(stairHeader);
            ui::DefaultFont()(U"Stairs").draw(stairHeader.x + 12, stairHeader.y + 11, ui::GetTheme().text);
            const RectF stairToggle{ stairHeader.x + stairHeader.w - 40, stairHeader.y + 10, 28, 28 };
            if (ui::Button(ui::DefaultFont(), (m_stairPanelOpen ? U"-" : U"+"), stairToggle))
            {
                m_stairPanelOpen = (not m_stairPanelOpen);
            }

            topY += (StairHeaderHeight + SectionSpacing);

            if (m_stairPanelOpen)
            {
                const RectF stairSection{ generatorPanel.x + SectionMargin, topY, generatorPanel.w - (SectionMargin * 2.0), StairBodyHeight };
                ui::Section(stairSection);
                ui::SliderH(U"Steps", m_stepCount, 1.0, 20.0, Vec2{ stairSection.x + 12, stairSection.y + 14 }, 84, 184);
                ui::SliderH(U"Height", m_height, 0.1, 2.0, Vec2{ stairSection.x + 12, stairSection.y + 50 }, 84, 184);
                ui::SliderH(U"Width", m_width, 0.5, 10.0, Vec2{ stairSection.x + 12, stairSection.y + 86 }, 84, 184);

                const RectF pickButton{ stairSection.x + 12, stairSection.y + 126, 122, 32 };
                if (ui::Button(ui::DefaultFont(), m_waitingForPosition ? U"Click target" : U"Set target", pickButton))
                {
                    m_waitingForPosition = true;
                }

                const RectF generateButton{ stairSection.x + 144, stairSection.y + 126, 122, 32 };
                if (ui::Button(ui::DefaultFont(), U"Generate", generateButton) && m_generatePosition)
                {
                    m_stairs << GeneratedStair{
                        *m_generatePosition,
                        static_cast<int32>(Math::Round(m_stepCount)),
                        m_height,
                        m_width,
                        m_depth
                    };
                    m_selectedIndex = (m_stairs.size() - 1);
                }

                const String targetText = m_generatePosition ? U"Target: set" : U"Target: none";
                ui::DefaultFont()(targetText).draw(stairSection.x + 12, stairSection.y + 132, ui::GetTheme().textMuted);
                topY += (StairBodyHeight + SectionSpacing);
            }

            return topY;
        }



        double StairGenerator::drawNatureSection(const RectF& generatorPanel, double topY){
            const RectF natureHeader{ generatorPanel.x + SectionMargin, topY, generatorPanel.w - (SectionMargin * 2.0), NatureHeaderHeight };
            ui::Section(natureHeader);
            ui::DefaultFont()(U"Nature").draw(natureHeader.x + 12, natureHeader.y + 11, ui::GetTheme().text);
            const RectF natureToggle{ natureHeader.x + natureHeader.w - 40, natureHeader.y + 10, 28, 28 };
            if (ui::Button(ui::DefaultFont(), (m_naturePanelOpen ? U"-" : U"+"), natureToggle))
            {
                m_naturePanelOpen = (not m_naturePanelOpen);
            }

            topY += (NatureHeaderHeight + SectionSpacing);

            if (m_naturePanelOpen)
            {
                const RectF natureSection{ generatorPanel.x + SectionMargin, topY, generatorPanel.w - (SectionMargin * 2.0), NatureBodyHeight };
                ui::Section(natureSection);

                const RectF treeButton{ natureSection.x + 12, natureSection.y + 14, 112, 32 };
                if (ui::Button(ui::DefaultFont(), U"木を生成", treeButton))
                {
                    addNatureObject(GeneratedNatureType::Tree);
                }

                const RectF mushroomButton{ natureSection.x + 132, natureSection.y + 14, 132, 32 };
                if (ui::Button(ui::DefaultFont(), U"きのこを生成", mushroomButton))
                {
                    addNatureObject(GeneratedNatureType::Mushroom);
                }

                const RectF pickButton{ natureSection.x + 12, natureSection.y + 58, 122, 32 };
                if (ui::Button(ui::DefaultFont(), m_waitingForPosition ? U"Click target" : U"場所選択", pickButton))
                {
                    m_waitingForPosition = true;
                }

                ui::SliderH(U"ランダムシード", m_natureSeed, 0.0, 9999.0, Vec2{ natureSection.x + 12, natureSection.y + 102 }, 120, 174);
                ui::SliderH(U"湿り具合", m_natureWetness, 0.0, 1.0, Vec2{ natureSection.x + 12, natureSection.y + 138 }, 120, 174);

                const RectF regenerateButton{ natureSection.x + 12, natureSection.y + 182, 122, 32 };
                if (ui::Button(ui::DefaultFont(), U"再生成", regenerateButton))
                {
                    regenerateNatureObjects();
                }

                const String targetText = m_generatePosition ? U"Target: set" : U"Target: none";
                ui::DefaultFont()(targetText).draw(natureSection.x + 144, natureSection.y + 188, ui::GetTheme().textMuted);
                topY += (NatureBodyHeight + SectionSpacing);
            }

            return topY;
        }



        double StairGenerator::drawSelectionSection(const RectF& generatorPanel, double topY){
            const RectF selectSection{ generatorPanel.x + 10, topY, generatorPanel.w - 20, SelectionHeight };
            ui::Section(selectSection);
            ui::DefaultFont()(U"Selected Object").draw(selectSection.x + 12, selectSection.y + 8, ui::GetTheme().text);
            const RectF prevStairButton{ selectSection.x + 12, selectSection.y + 42, 58, 30 };
            const RectF nextStairButton{ selectSection.x + 78, selectSection.y + 42, 58, 30 };
            const RectF clearStairButton{ selectSection.x + 144, selectSection.y + 42, 94, 30 };
            if (ui::Button(ui::DefaultFont(), U"Prev", prevStairButton) && (not m_stairs.isEmpty()))
            {
                const size_t current = m_selectedIndex.value_or(0);
                m_selectedIndex = (current == 0) ? (m_stairs.size() - 1) : (current - 1);
            }
            if (ui::Button(ui::DefaultFont(), U"Next", nextStairButton) && (not m_stairs.isEmpty()))
            {
                const size_t current = m_selectedIndex.value_or(m_stairs.size() - 1);
                m_selectedIndex = (current + 1) % m_stairs.size();
            }
            if (ui::Button(ui::DefaultFont(), U"Deselect", clearStairButton))
            {
                m_selectedIndex.reset();
            }

            if (m_selectedIndex && (*m_selectedIndex < m_stairs.size()))
            {
                ui::DefaultFont()(U"Object: Stair {} / {}"_fmt((*m_selectedIndex + 1), m_stairs.size()))
                    .draw(selectSection.x + 12, selectSection.y + 84, ui::GetTheme().textMuted);
            }
            else
            {
                ui::DefaultFont()(U"Object: none").draw(selectSection.x + 12, selectSection.y + 84, ui::GetTheme().textMuted);
            }

            return (topY + SelectionHeight + SectionSpacing);
        }



        double StairGenerator::drawTransformSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair){
            const RectF transformSection{ generatorPanel.x + 10, topY, generatorPanel.w - 20, TransformHeight };
            ui::Section(transformSection);
            ui::DefaultFont()(U"Orientation").draw(transformSection.x + 12, transformSection.y + 8, ui::GetTheme().text);
            if (selectedStair)
            {
                ui::SliderH(U"Rotate", selectedStair->rotation01, 0.0, 1.0, Vec2{ transformSection.x + 12, transformSection.y + 42 }, 84, 184);
            }
            else
            {
                ui::DefaultFont()(U"Select a generated stair first.").draw(transformSection.x + 12, transformSection.y + 42, ui::GetTheme().textMuted);
            }

            return (topY + TransformHeight + SectionSpacing);
        }



        double StairGenerator::drawColorSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair){
            const RectF colorSection{ generatorPanel.x + 10, topY, generatorPanel.w - 20, ColorHeight };
            ui::Section(colorSection);
            ui::DefaultFont()(U"Color").draw(colorSection.x + 12, colorSection.y + 8, ui::GetTheme().text);
            if (selectedStair)
            {
                ui::SliderH(U"R", selectedStair->color.r, 0.0, 1.0, Vec2{ colorSection.x + 12, colorSection.y + 42 }, 84, 184);
                ui::SliderH(U"G", selectedStair->color.g, 0.0, 1.0, Vec2{ colorSection.x + 12, colorSection.y + 78 }, 84, 184);
                ui::SliderH(U"B", selectedStair->color.b, 0.0, 1.0, Vec2{ colorSection.x + 12, colorSection.y + 114 }, 84, 184);
                RectF{ colorSection.x + colorSection.w - 44, colorSection.y + 10, 28, 28 }.rounded(6).draw(selectedStair->color);
                RectF{ colorSection.x + colorSection.w - 44, colorSection.y + 10, 28, 28 }.rounded(6).drawFrame(1.0, ui::GetTheme().panelBorder);
            }
            else
            {
                ui::DefaultFont()(U"Select a generated stair first.").draw(colorSection.x + 12, colorSection.y + 42, ui::GetTheme().textMuted);
            }

            return (topY + ColorHeight + SectionSpacing);
        }



        double StairGenerator::drawMaterialSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair){
            const RectF materialHeader{ generatorPanel.x + 10, topY, generatorPanel.w - 20, MaterialHeaderHeight };
            ui::Section(materialHeader);
            ui::DefaultFont()(U"Material").draw(materialHeader.x + 12, materialHeader.y + 11, ui::GetTheme().text);
            const RectF materialToggle{ materialHeader.x + materialHeader.w - 40, materialHeader.y + 10, 28, 28 };
            if (ui::Button(ui::DefaultFont(), (m_materialPanelOpen ? U"-" : U"+"), materialToggle))
            {
                m_materialPanelOpen = (not m_materialPanelOpen);
            }

            if (m_materialPanelOpen)
            {
                const RectF materialSection{ generatorPanel.x + 10, topY + MaterialHeaderHeight + SectionSpacing, generatorPanel.w - 20, MaterialBodyHeight };
                ui::Section(materialSection);
                if (selectedStair)
                {
                    const RectF dullButton{ materialSection.x + 12, materialSection.y + 12, 112, 30 };
                    const RectF variationButton{ materialSection.x + 132, materialSection.y + 12, 112, 30 };
                    if (ui::Button(ui::DefaultFont(), selectedStair->useDullNoise ? U"Dull: ON" : U"Dull: OFF", dullButton))
                    {
                        selectedStair->useDullNoise = (not selectedStair->useDullNoise);
                    }
                    if (ui::Button(ui::DefaultFont(), selectedStair->useColorVariation ? U"Var: ON" : U"Var: OFF", variationButton))
                    {
                        selectedStair->useColorVariation = (not selectedStair->useColorVariation);
                    }
                    ui::SliderH(U"Dull Amt", selectedStair->dullNoiseAmount, 0.0, 1.0, Vec2{ materialSection.x + 12, materialSection.y + 52 }, 84, 184);
                    ui::SliderH(U"Var Amt", selectedStair->colorVariationAmount, 0.0, 1.0, Vec2{ materialSection.x + 12, materialSection.y + 88 }, 84, 184);
                }
                else
                {
                    ui::DefaultFont()(U"Select a generated stair first.").draw(materialSection.x + 12, materialSection.y + 14, ui::GetTheme().textMuted);
                }
            }

            return topY;
        }
}
