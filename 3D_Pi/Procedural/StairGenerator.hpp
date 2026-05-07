# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "../UI/RectUI.hpp"
# include "../UI/EditorIconLayout.hpp"
# include "ProceduralTypes.hpp"
# include "ProceduralDocument.hpp"
# include "StairSerializer.hpp"

namespace procedural
{
    class StairGenerator
    {
    public:
        StairGenerator();
        [[nodiscard]] bool isEnabled() const noexcept;
        [[nodiscard]] bool isPanelOpen() const noexcept;
        bool handleCommand(app::EditorCommand command);

        [[nodiscard]] bool wantsMouseCapture() const;

        void setUIHidden(const bool hidden);

        void cancelTargetSelection();

        void update(const BasicCamera3D& camera, const bool cursorBlockedByUI);

        void draw3D(const bool showPlacementMarker) const;

        void drawUI();

    private:
        static constexpr double PanelWidth = 360.0;
        static constexpr double HeaderHeight = 42.0;

        static constexpr StringView DefaultSavePath = U"data/procedural.toml";

        [[nodiscard]] RectF getPanelRect() const;

        [[nodiscard]] RectF getCollapsedIconRect() const;

        void syncCollapsedIconRegistry() const;

        void updateCollapsedIconDrag(const RectF& dragRect);

        void expandFromCollapsedIcon();

        [[nodiscard]] double getExpandedPanelHeight() const;

        [[nodiscard]] GeneratedStair* getSelectedStair();

        [[nodiscard]] static ColorF getMaterializedColor(const GeneratedStair& stair, const int32 stepIndex);

        [[nodiscard]] uint32 makeNatureVariationSeed(uint32 serial) const;

        void regenerateNatureObjects();

        void addNatureObject(GeneratedNatureType type);

        void drawNatureObject(const GeneratedNatureObject& naturalObject) const;

        [[nodiscard]] double drawCreationSection(const RectF& generatorPanel, double topY);

        [[nodiscard]] double drawNatureSection(const RectF& generatorPanel, double topY);

        [[nodiscard]] double drawSelectionSection(const RectF& generatorPanel, double topY);

        [[nodiscard]] static double drawTransformSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair);

        [[nodiscard]] static double drawColorSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair);

        [[nodiscard]] double drawMaterialSection(const RectF& generatorPanel, double topY, GeneratedStair* selectedStair);

        void save() const;
        void load();

        FilePath m_savePath = FilePath{ DefaultSavePath };
        ProceduralDocument m_document;
        Array<GeneratedStair>& m_stairs = m_document.stairs;
        Array<GeneratedNatureObject>& m_naturalObjects = m_document.natureObjects;

        Optional<size_t> m_selectedIndex;
        Optional<Vec3> m_generatePosition;

        Vec2 m_panelPos{ ui::editor_icon::GetDockedStackPosition(3) };

        Vec2 m_dragOffset{ 0, 0 };

        Vec2 m_togglePressCursor{ 0, 0 };

        bool m_panelOpen = false;

        bool m_stairPanelOpen = true;

        bool m_naturePanelOpen = true;

        bool m_materialPanelOpen = false;

        bool m_dragging = false;

        bool m_ignoreCollapsedClickUntilRelease = false;

        bool m_waitingForPosition = false;

        bool m_uiHidden = false;

        Texture m_toggleIcon{ U"texture/proIcon.png" };

        uint32 m_nextNatureSerial = 1;

        double m_stepCount = 6.0;

        double m_height = 0.35;

        double m_width = 3.0;

        double m_depth = 0.6;

        double m_natureSeed = 1.0;

        double m_natureWetness = 0.0;
    };
}
