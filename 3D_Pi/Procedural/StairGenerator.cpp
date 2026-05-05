# include "../stdafx.h"
# include "StairGenerator.hpp"

namespace procedural
{

bool StairGenerator::wantsMouseCapture() const{
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

            const RectF panelRect = getPanelRect();
            const RectF headerRect{ panelRect.x, panelRect.y, panelRect.w, 42 };
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
                    m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - headerRect.h));
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

            if (showPlacementMarker && m_generatePosition)
            {
                Cylinder{ *m_generatePosition + Vec3{ 0, 0.02, 0 }, 0.28, 0.04 }.draw(ColorF{ 0.25, 0.55, 0.95 }.removeSRGBCurve());
            }
        }



        void StairGenerator::drawUI(){
            const RectF generatorPanel = getPanelRect();
            ui::Panel(generatorPanel);
            const RectF generatorHeader{ generatorPanel.x, generatorPanel.y, generatorPanel.w, 42 };
            generatorHeader.rounded(10).draw(ColorF{ 0.90, 0.93, 0.97, 0.96 });
            ui::DefaultFont()(U"3D Generator").draw(generatorHeader.x + 12, generatorHeader.y + 9, ui::GetTheme().text);
            const RectF generatorToggle{ generatorPanel.x + generatorPanel.w - 38, generatorPanel.y + 7, 28, 28 };
            if (ui::Button(ui::DefaultFont(), (m_panelOpen ? U"-" : U"+"), generatorToggle))
            {
                m_panelOpen = (not m_panelOpen);
            }

            if (m_panelOpen)
            {
                drawCreationSection(generatorPanel);
                drawSelectionSection(generatorPanel);
                GeneratedStair* selectedStair = getSelectedStair();
                drawTransformSection(generatorPanel, selectedStair);
                drawColorSection(generatorPanel, selectedStair);
                drawMaterialSection(generatorPanel, selectedStair);
            }
        }

RectF StairGenerator::getPanelRect() const{
            return RectF{ m_panelPos, 360, (m_panelOpen ? (m_materialPanelOpen ? 732 : 596) : 42) };
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



        void StairGenerator::drawCreationSection(const RectF& generatorPanel){
            const RectF stairSection{ generatorPanel.x + 10, generatorPanel.y + 52, generatorPanel.w - 20, 212 };
            ui::Section(stairSection);
            ui::DefaultFont()(U"Stairs").draw(stairSection.x + 12, stairSection.y + 8, ui::GetTheme().text);
            ui::SliderH(U"Steps", m_stepCount, 1.0, 20.0, Vec2{ stairSection.x + 12, stairSection.y + 42 }, 84, 184);
            ui::SliderH(U"Height", m_height, 0.1, 2.0, Vec2{ stairSection.x + 12, stairSection.y + 78 }, 84, 184);
            ui::SliderH(U"Width", m_width, 0.5, 10.0, Vec2{ stairSection.x + 12, stairSection.y + 114 }, 84, 184);

            const RectF pickButton{ stairSection.x + 12, stairSection.y + 154, 122, 32 };
            if (ui::Button(ui::DefaultFont(), m_waitingForPosition ? U"Click target" : U"Set target", pickButton))
            {
                m_waitingForPosition = true;
            }

            const RectF generateButton{ stairSection.x + 144, stairSection.y + 154, 122, 32 };
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
            ui::DefaultFont()(targetText).draw(stairSection.x + 12, stairSection.y + 188, ui::GetTheme().textMuted);
        }



        void StairGenerator::drawSelectionSection(const RectF& generatorPanel){
            const RectF selectSection{ generatorPanel.x + 10, generatorPanel.y + 274, generatorPanel.w - 20, 126 };
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
        }



        void StairGenerator::drawTransformSection(const RectF& generatorPanel, GeneratedStair* selectedStair){
            const RectF transformSection{ generatorPanel.x + 10, generatorPanel.y + 410, generatorPanel.w - 20, 78 };
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
        }



        void StairGenerator::drawColorSection(const RectF& generatorPanel, GeneratedStair* selectedStair){
            const RectF colorSection{ generatorPanel.x + 10, generatorPanel.y + 498, generatorPanel.w - 20, 154 };
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
        }



        void StairGenerator::drawMaterialSection(const RectF& generatorPanel, GeneratedStair* selectedStair){
            const RectF materialHeader{ generatorPanel.x + 10, generatorPanel.y + 662, generatorPanel.w - 20, 48 };
            ui::Section(materialHeader);
            ui::DefaultFont()(U"Material").draw(materialHeader.x + 12, materialHeader.y + 11, ui::GetTheme().text);
            const RectF materialToggle{ materialHeader.x + materialHeader.w - 40, materialHeader.y + 10, 28, 28 };
            if (ui::Button(ui::DefaultFont(), (m_materialPanelOpen ? U"-" : U"+"), materialToggle))
            {
                m_materialPanelOpen = (not m_materialPanelOpen);
            }

            if (m_materialPanelOpen)
            {
                const RectF materialSection{ generatorPanel.x + 10, generatorPanel.y + 720, generatorPanel.w - 20, 126 };
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
        }
}
