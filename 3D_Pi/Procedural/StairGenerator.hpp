# pragma once
# include <Siv3D.hpp>
# include "../UI/RectUI.hpp"

namespace procedural
{
    struct GeneratedStair
    {
        Vec3 origin;
        int32 steps;
        double height;
        double width;
        double depth;
        double rotation01 = 0.0;
        ColorF color{ 0.72, 0.68, 0.60 };
        bool useDullNoise = false;
        bool useColorVariation = false;
        double dullNoiseAmount = 0.35;
        double colorVariationAmount = 0.20;
    };

    class StairGenerator
    {
    public:
        [[nodiscard]] bool wantsMouseCapture() const;



        void setUIHidden(const bool hidden);



        void cancelTargetSelection();



        void update(const BasicCamera3D& camera, const bool cursorBlockedByUI);



        void draw3D(const bool showPlacementMarker) const;



        void drawUI();



    private:
        [[nodiscard]] RectF getPanelRect() const;



        [[nodiscard]] GeneratedStair* getSelectedStair();



        [[nodiscard]] static ColorF getMaterializedColor(const GeneratedStair& stair, const int32 stepIndex);



        void drawCreationSection(const RectF& generatorPanel);



        void drawSelectionSection(const RectF& generatorPanel);



        static void drawTransformSection(const RectF& generatorPanel, GeneratedStair* selectedStair);



        static void drawColorSection(const RectF& generatorPanel, GeneratedStair* selectedStair);



        void drawMaterialSection(const RectF& generatorPanel, GeneratedStair* selectedStair);



        Array<GeneratedStair> m_stairs;

        Optional<size_t> m_selectedIndex;

        Optional<Vec3> m_generatePosition;

        Vec2 m_panelPos{ 24, 112 };

        Vec2 m_dragOffset{ 0, 0 };

        bool m_panelOpen = true;

        bool m_materialPanelOpen = false;

        bool m_dragging = false;

        bool m_waitingForPosition = false;

        bool m_uiHidden = false;

        double m_stepCount = 6.0;

        double m_height = 0.35;

        double m_width = 3.0;

        double m_depth = 0.6;
    };
}
