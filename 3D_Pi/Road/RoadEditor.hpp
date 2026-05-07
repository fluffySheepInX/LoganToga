# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "../UI/RectUI.hpp"
# include "../UI/EditorIconLayout.hpp"
# include "RoadTypes.hpp"
# include "RoadMaterialBuilder.hpp"
# include "RoadMeshBuilder.hpp"
# include "RoadSerializer.hpp"
# include "RoadGeometry.hpp"
# include "RoadPlacementTypes.hpp"
# include "RoadScatterRules.hpp"
# include "RoadSceneSnapshot.hpp"
# include "RoadGhostViewState.hpp"
# include "RoadEditSession.hpp"
# include "RoadPresetSerializer.hpp"
# include "RoadDocument.hpp"

class RoadEditor
{
public:
    explicit RoadEditor(const FilePath& texturePath, const FilePath& savePath = U"data/roads.toml",
        const FilePath& presetsDir = U"data/road_presets/");

    [[nodiscard]] bool isEnabled() const noexcept;
    [[nodiscard]] bool isPanelOpen() const noexcept;
    bool handleCommand(app::EditorCommand command);

    [[nodiscard]] bool wantsMouseCapture() const;

    void update(const BasicCamera3D& camera);

    void draw3D() const;

    void drawUI();

private:
    static constexpr double GuideYOffset = 0.05;

    static constexpr double PointSpacing = 0.5;

    static constexpr double MinSnapDistance = 0.3;

    static constexpr double MaxSnapDistance = 3.0;


    FilePath m_textureSourcePath;

    Texture m_roadTexture;

    Texture m_roadShoulderTexture;

    FilePath m_savePath;

    FilePath m_assetCatalogPath = U"Road/road_placement_assets.toml";

    FilePath m_presetsDir = U"data/road_presets/";

    Array<RoadSceneSnapshot> m_presets;

    String m_presetNameInput = U"Preset1";

    size_t m_selectedPresetIndex = 0;

    RoadGhostViewState m_ghost;

    RoadEditSession m_session;

    Font m_font{ 18 };

    bool m_enabled = false;

    bool m_uiCollapsed = true;

    Vec2 m_panelPos{ 20, 20 };

    bool m_panelDragging = false;

    Vec2 m_panelDragOffset{ 0, 0 };

    Vec2 m_togglePressCursor{ 0, 0 };

    Texture m_toggleIcon{ U"texture/roadIcon.png" };

    String m_statusMessage = U"Ready";

    double m_snapDistance = 1.0;

    RoadDocument m_document;
    Array<RoadPath>& m_roads = m_document.roads;
    RoadMaterialSettings& m_materialSettings = m_document.material;

    Array<Optional<Mesh>> m_roadMeshes;

    Array<Optional<Mesh>> m_roadShoulderMeshes;

    Array<Mesh> m_connectionPatchMeshes;

    Optional<RoadPath> m_editingRoad;

    Optional<Mesh> m_editingMesh;

    Optional<Vec3> m_hoverPoint;

    Optional<Vec3> m_snapPoint;

    size_t m_activeTabIndex = 0;

    bool m_materialDirty = true;

    Array<road::IntersectionCluster> m_intersectionClusters;

    Array<road::PlacementAsset> m_placementAssets = road::DefaultPlacementAssets();

    HashTable<String, Model> m_assetModelCache;

    HashTable<String, Texture> m_assetTextureCache;

    road::PlacementSettings m_placementSettings;

    String m_scatterDebugSummary;

    Array<road::PlacedScatterItem> m_scatterItems;

    Optional<size_t> m_hoverScatterItemIndex;

    String m_hoverTooltip;

    bool m_ignoreCollapsedClickUntilRelease = false;


    [[nodiscard]] RectF getPanelRect() const;



    [[nodiscard]] RectF getCollapsedIconRect() const;



    void syncCollapsedIconRegistry() const;



    void updateCollapsedIconDrag(const RectF& dragRect);
 



    void expandFromCollapsedIcon();
 



    [[nodiscard]] bool isCursorOnUI() const;



    void resetMaterialSettings();
 



    void refreshRoadMaterialTextureIfDirty();
 



    void setTooltipIfHovered(const bool hovered, const StringView text);
 



    bool drawAdjustableMaterialSlider(const StringView label, double& value, double& maxValue, const double minValue,
        const double maxLimit, const double maxStep, const Vec2& pos, const StringView tooltip,
        const double labelWidth = 136.0, const double sliderWidth = 126.0, const bool rebuildMesh = false);
 



    bool drawMaterialSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth = 136.0, const double sliderWidth = 126.0);
 



    bool drawMaterialMeshSlider(const StringView label, double& value, const double min, const double max, const Vec2& pos,
        const double labelWidth = 136.0, const double sliderWidth = 126.0);
 



    void drawEditTab(const RectF& panel);
 



    void drawMaterialTab(const RectF& panel);
 



    void drawScatterTab(const RectF& panel);
 



    [[nodiscard]] const road::PlacementAsset* findActivePlacementAsset() const;



    [[nodiscard]] Optional<size_t> findHoveredScatterItemIndex() const;



    void updateScatterInteraction();
 



    void drawScatterItems3D() const;



    void drawScatterHoverGuide3D() const;



    [[nodiscard]] Optional<Vec3> cursorToGround(const BasicCamera3D& camera) const;



    [[nodiscard]] Vec3 getCurrentInputPoint() const;



    [[nodiscard]] Optional<Vec3> findSnapPoint(const Optional<Vec3>& point) const;



    void appendPoint(const Vec3& point);
 



    void confirmEditingRoad();
 



    void cancelEditingRoad();
 



    void clearAllPlacedData();
 



    void saveCurrentAsPreset(const String& name);
 



    void startTraceSession();
 



    void restoreSession();
 



    void commitSession();
 



    void toggleGhostVisible();
 



    void undo();
 



    void rebuildAllMeshes();
 



    void rebuildEditingMesh();
 



    [[nodiscard]] Optional<road::RoadBoundaryContext> evaluateBoundaryContext(const Vec3& point) const;



    void drawPathGuide(const Optional<RoadPath>& road, const ColorF& lineColor, const ColorF& pointColor) const;



    void drawGhostRoads3D() const;



    void save() const;



    void load();
 



    void loadPlacementAssets();

};
