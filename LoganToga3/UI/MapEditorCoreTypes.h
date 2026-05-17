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

	struct MapEditorState
	{
		bool enabled = false;
		bool showDebugInfo = true;
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
		int32 buildingEditorTab = 0;
		String buildingEditorLineActionTag;
		String buildingEditorIconHorizontal;
		String buildingEditorIconDiagUpRight;
		String buildingEditorIconDiagUpLeft;
		bool buildLineIconsDirty = false;
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
	};
}
