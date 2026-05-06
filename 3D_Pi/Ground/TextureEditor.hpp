# pragma once
# include <Siv3D.hpp>
# include "../UI/RectUI.hpp"
# include "../UI/EditorIconLayout.hpp"

struct GroundLayer
{
    String id;
    String label;
    FilePath texturePath;
    int32 categoryIndex = 0; // 0=Grass 1=Dirt 2=Plaza 3=Brick 4=Stone 5=Decal
    Vec2 position{ 0.0, 0.0 }; // world X, Z
    Vec2 size{ 10.0, 10.0 };
    double rotation = 0.0; // radians
    double yOffset = 0.014;
    ColorF tint{ 1.0 };
    double tilingScale = 4.0;
    double edgeSoftness = 0.12;
    double edgeNoiseAmount = 0.04;
    double edgeNoiseFrequency = 3.0;
    uint64 edgeNoiseSeed = 42;
    bool visible = true;
};

class TextureEditor
{
public:
    explicit TextureEditor(const FilePath& savePath = U"data/ground_layers.toml");

    [[nodiscard]] bool wantsMouseCapture() const;



    [[nodiscard]] bool wantsMouseWheelCapture() const;



    void update(const BasicCamera3D& camera);



    void draw3D();



    void drawUI();



private:
    static constexpr int32 TexRes = 256;

    static constexpr double PanelWidth = 440.0;

    static constexpr double PanelHeight = 730.0;


    FilePath m_savePath;

    Font m_font{ 18 };

    Font m_smallFont{ 12 };

    bool m_enabled = false;

 bool m_uiCollapsed = true;

    Vec2 m_panelPos{ 20, 20 };

    bool m_panelDragging = false;

    Vec2 m_panelDragOffset{ 0, 0 };

    Vec2 m_togglePressCursor{ 0, 0 };

    Texture m_toggleIcon{ U"texture/textureIcon.png" };

    String m_statusMessage = U"Ready";

    size_t m_activeTabIndex = 0;

    size_t m_selectedLayerIndex = 0;

    Array<GroundLayer> m_layers;

    Array<DynamicTexture> m_layerTextures;

    Array<bool> m_layerDirty;

    HashTable<FilePath, Image> m_sourceImageCache;

    Mesh m_overlayPlane;

    Array<FilePath> m_availableTextures;

    String m_hoverTooltip;

    bool m_autoYOffset = true;

    double m_autoYOffsetStep = 0.003;

    double m_baseYOffset = 0.002;

    bool m_placeAtClickRequested = false;

    double m_layerListScroll = 0.0;

    double m_textureListScroll = 0.0;

    Point m_lastCursorScreenPos{ 0, 0 };

    Optional<Vec2> m_lastCursorGroundPos;

    bool m_lastPlacementPanelHover = false;

    bool m_lastPlacementClickSeen = false;

    bool m_lastPlacementApplied = false;

    String m_lastPlacementReason;

    double m_placementDiagnosticsCopiedUntil = -1.0;

    bool m_ignoreCollapsedClickUntilRelease = false;


    [[nodiscard]] RectF getPanelRect() const;



    [[nodiscard]] RectF getCollapsedIconRect() const;



    void syncCollapsedIconRegistry() const;



    void updateCollapsedIconDrag(const RectF& dragRect);



    void expandFromCollapsedIcon();



    [[nodiscard]] Optional<Vec2> cursorToGround(const BasicCamera3D& camera) const;



    [[nodiscard]] bool hasSelectedLayer() const;



    [[nodiscard]] RectF getLayerListSectionRect() const;



    [[nodiscard]] RectF getTextureListRect() const;



    [[nodiscard]] double getLayerListMaxScroll() const;



    void setTooltipIfHovered(const bool hovered, const StringView text);



    void applyPlacement(const Vec2& groundPos, const StringView source);



    [[nodiscard]] String buildPlacementDiagnostics() const;



    [[nodiscard]] static String categoryName(const int32 index);



    void scanAvailableTextures();



    [[nodiscard]] const Image& getOrLoadSourceImage(const FilePath& path);



    [[nodiscard]] Image buildLayerImage(const GroundLayer& layer);



    void ensureAllLayerTextures();



    void markLayerDirty(const size_t index);



    void addLayer(GroundLayer layer);



    void removeLayer(const size_t index);



    void clearAllLayers();



    void undoLastLayer();



    void duplicateSelectedLayer();



    void moveLayerUp(const size_t index);



    void moveLayerDown(const size_t index);



    void cycleTexture(GroundLayer& layer, const int32 direction);



    // ---- UI Tabs ----

    void drawLayersTab(const RectF& panel);



    void drawPropertiesTab(const RectF& panel);



    void drawEdgeTab(const RectF& panel);



    // ---- Persistence ----

    void save() const;



    void load();



    void loadDefaults();

};
