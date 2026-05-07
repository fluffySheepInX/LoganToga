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
                    ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapsedIcon);
                }
                else
                {
                    ui::DefaultFont()(U"P").drawAt(collapsedIcon.center(), ui::GetTheme().text);
                }

                if (collapsedIcon.leftClicked())
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
            const RectF generatorToggle{ generatorPanel.x + generatorPanel.w - 74, generatorPanel.y + 10, 64, 64 };
            generatorToggle.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            generatorToggle.drawFrame(2.0, Palette::Black);
            if (m_toggleIcon)
            {
                ui::editor_icon::DrawToggleIcon(m_toggleIcon, generatorToggle);
            }
            else
            {
                ui::DefaultFont()(U"P").drawAt(generatorToggle.center(), ui::GetTheme().text);
            }
            if (generatorToggle.leftClicked())
            {
                const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                    generatorToggle, SizeF{ ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize });
                m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"ProceduralEditor", desiredCollapsedPos);
                m_panelOpen = (not m_panelOpen);
               syncCollapsedIconRegistry();
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
            const RectF collapsedIcon = getCollapsedIconRect();
            m_panelOpen = true;
            m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(collapsedIcon, SizeF{ PanelWidth, getExpandedPanelHeight() });
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
