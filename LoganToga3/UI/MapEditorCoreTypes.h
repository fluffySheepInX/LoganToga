#pragma once
# include <Siv3D.hpp>
# include "../Data/UnitCatalog.h"
# include "../Data/DefinitionStores.h"
# include "QuarterView.h"

namespace LT3
{
	inline constexpr int32 DefaultMapEditorWidth = 12;
	inline constexpr int32 DefaultMapEditorHeight = 8;
	inline constexpr int32 InvalidMapEditorAsset = -1;

	enum class DescriptionEditorTargetKind
	{
		None,
		Command,
		Unit,
		Skill,
	};

	enum class MapEditorAssetKind
	{
		Terrain,
		Object,
	};

	struct MapEditorAsset
	{
		FilePath path;
		String fileName;
		Size imageSize{ 0, 0 };
		Texture texture;
		bool isAnimatedGif = false;
		Array<Texture> animationFrames;
		Array<int32> animationFrameDelaysMillisec;
		int32 animationDurationMillisec = 0;
		MapEditorAssetKind kind = MapEditorAssetKind::Terrain;
		double decalOpacity = 1.0;
		double decalScale = 1.0;
		bool useRandomDecalOpacity = false;
		double decalOpacityMin = 1.0;
		double decalOpacityMax = 1.0;
		bool useRandomDecalScale = false;
		double decalScaleMin = 1.0;
		double decalScaleMax = 1.0;
		bool decalBlocksPassage = false;
	};

	struct MapEditorDecalPlacement
	{
		int32 assetIndex = InvalidMapEditorAsset;
		double opacity = 1.0;
		double scale = 1.0;
	};

	struct MapEditorCell
	{
		int32 terrainAsset = InvalidMapEditorAsset;
		int32 objectAsset = InvalidMapEditorAsset;
		double decalOpacity = 1.0;
		double decalScale = 1.0;
		Array<MapEditorDecalPlacement> decals;
		bool useRandomDecalOpacity = false;
		double decalOpacityMin = 1.0;
		double decalOpacityMax = 1.0;
		bool useRandomDecalScale = false;
		double decalScaleMin = 1.0;
		double decalScaleMax = 1.0;
	};

	struct ResourceNodeEditData
	{
		ResourceKind kind = ResourceKind::Gold;
		Point cell{ 0, 0 };
		int32 amount = 700;
		int32 incomePerSec = 5;
	};

	struct MapEditorPerlinStackItem
	{
		FilePath path;
		String fileName;
		int32 assetIndex = InvalidMapEditorAsset;
	};

	struct SkillSandboxProjectile
	{
		Vec2 position{ 0.0, 0.0 };
		Vec2 velocity{ 0.0, 0.0 };
		Vec2 startPosition{ 0.0, 0.0 };
		Vec2 endPosition{ 0.0, 0.0 };
		double lifeSec = 0.0;
		double ageSec = 0.0;
		double maxLifeSec = 0.0;
		double height = 0.0;
		double angleRad = 0.0;
		double baseAngleRad = 0.0;
		SkillProjectileMotion motion = SkillProjectileMotion::Direct;
	};

	struct MapEditorState
	{
		bool enabled = false;
		bool showDebugInfo = true;
		bool showEnemyMoveMarkers = false;
		bool showBattleGrid = true;
		Array<MapEditorAsset> assets;
		Array<MapEditorCell> cells;
		int32 mapWidth = DefaultMapEditorWidth;
		int32 mapHeight = DefaultMapEditorHeight;
		int32 selectedAsset = InvalidMapEditorAsset;
		double paletteScroll = 0.0;
		int32 paletteTabIndex = 0;
		bool showResourcePanels = true;
		bool showUnitList = false;
		bool showBuildingEditor = false;
		bool showCommandEditor = false;
		bool showSkillEditor = false;
		bool showAiEditor = false;
		double commandListScroll = 0.0;
		double commandUnitListScroll = 0.0;
		double commandInspectScroll = 0.0;
		int32 selectedCommandActionIndex = -1;
		int32 commandEditorMode = 0;
		bool commandBindingsDirty = false;
		int32 buildingEditorTab = 0;
		String buildingEditorLineActionTag;
		String buildingEditorIconHorizontal;
		String buildingEditorIconDiagUpRight;
		String buildingEditorIconDiagUpLeft;
		bool buildLineIconsDirty = false;
		double aiProfileListScroll = 0.0;
		double aiProfileDetailScroll = 0.0;
		int32 selectedAiProfileIndex = 0;
		String selectedAiProfileTag = U"balanced";
		bool aiProfileSelectionInitialized = false;
		bool aiProfilesDirty = false;
		double skillListScroll = 0.0;
		double skillUnitListScroll = 0.0;
		double skillDetailScroll = 0.0;
		int32 selectedSkillIndex = 0;
		bool showSkillSandboxPreview = false;
		Vec2 skillSandboxCasterPos{ 136.0, 404.0 };
		Vec2 skillSandboxTargetPos{ 512.0, 404.0 };
		int32 skillSandboxTargetHp = 100;
		int32 skillSandboxTargetMaxHp = 100;
		double skillSandboxCooldownLeftSec = 0.0;
		bool skillSandboxAutoFire = true;
		bool skillSandboxDraggingTarget = false;
		int32 skillSandboxSkillIndex = -1;
		Array<SkillSandboxProjectile> skillSandboxProjectiles;
		bool skillDefsDirty = false;
		Optional<int32> skillContextMenuTargetIndex;
		Vec2 skillContextMenuPos{ 0.0, 0.0 };
		double unitListScroll = 0.0;
		int32 selectedUnitCatalogIndex = -1;
		bool showUnitParameterEditor = false;
		int32 unitParamEditorTab = 0;
		bool unitCatalogDirty = false;
		bool uiLayoutEditEnabled = false;
		double editorBarHiddenUntilSec = 0.0;
		int32 uiLayoutGridSize = 40;
		Vec2 uiSelectedInfoAnchor{ 24.0, 826.0 };
		Vec2 uiCommandPanelPos{ 1088.0, 668.0 };
		Vec2 uiResourcePanelPos{ 1288.0, 72.0 };
		bool uiSelectedInfoTopAnchor = false;
		bool uiCommandPanelTopAnchor = false;
		bool uiResourcePanelTopAnchor = false;
		bool uiParamEditorTopAnchor = false;
		bool uiBuildingEditorTopAnchor = false;
		bool uiLayoutDraggingSelectedInfo = false;
		bool uiLayoutDraggingCommandPanel = false;
		bool uiLayoutDraggingResourcePanel = false;
		Vec2 uiParamEditorPos{ 600.0, 92.0 };
		bool uiLayoutDraggingParamEditor = false;
		Vec2 uiBuildingEditorPos{ 600.0, 92.0 };
		bool uiLayoutDraggingBuildingEditor = false;
		Vec2 uiLayoutDragOffset{ 0.0, 0.0 };
		FilePath assetDirectory;
		FilePath savePath;
		FilePath uiLayoutPath;
		FilePath resourceNodeSavePath;
		Array<ResourceNodeEditData> resourceNodes;
		int32 selectedResourceNodeIndex = -1;
		double resourceNodeListScroll = 0.0;
		bool resourceNodeDragging = false;
		int32 resourceNodeFilterKind = -1;
		Optional<ResourceKind> resourcePlacementDragKind;
		HashTable<String, Texture> resourceIconTextures;
		bool showStarToolMenu = false;
		bool showPerlinNoisePanel = false;
		bool showFogPanel = false;
		bool fogEnabled = true;
		ColorF fogColor{ 0.02, 0.025, 0.035 };
		double fogOpacity = 0.52;
		bool fogPreviewVision = true;
		Array<MapEditorPerlinStackItem> perlinStack;
		int32 perlinMapWidth = DefaultMapEditorWidth;
		int32 perlinMapHeight = DefaultMapEditorHeight;
		Vec2 playerHomePosition{ 210.0, 450.0 };
		Vec2 enemyHomePosition{ 1390.0, 450.0 };
		bool draggingPlayerHome = false;
		bool draggingEnemyHome = false;
		Optional<Point> lastPaintCell;
		int32 lastPaintAsset = InvalidMapEditorAsset;
		Optional<Point> lastEraseCell;
		bool showDecalEditor = false;
		int32 decalEditorAssetIndex = InvalidMapEditorAsset;
		bool zOrderMode = false;
		Optional<Point> zOrderDragStartCell;
		Optional<Rect> zOrderSelectionRect;
		int32 zOrderSelectedStackIndex = 0;
		String statusText = U"Map editor ready";

		// Unit right-click context menu
		Optional<int32> unitContextMenuTargetIndex;
		Vec2 unitContextMenuPos{ 0.0, 0.0 };

		// Unit inline rename after duplicate
		Optional<int32> unitRenameTargetIndex;
		String unitRenameEditText;
		bool unitRenameIsDuplicate = false;

		// Command right-click context menu
		Optional<int32> commandContextMenuTargetIndex;
		Vec2 commandContextMenuPos{ 0.0, 0.0 };

		// Command inline rename after duplicate
		Optional<int32> commandRenameTargetIndex;
		String commandRenameEditText;
		bool commandRenameIsDuplicate = false;

		// Shared description editor
		DescriptionEditorTargetKind descriptionEditorTargetKind = DescriptionEditorTargetKind::None;
		int32 descriptionEditorTargetIndex = -1;
		String descriptionEditorTitle;
		String descriptionEditorText;
	};
}
