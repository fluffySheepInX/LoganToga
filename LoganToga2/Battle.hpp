#pragma once
#include "Common.h"
#include "ClassSkill.h" 
#include "ClassUnit.h"
#include "ClassHorizontalUnit.h"
#include "EnumSkill.h"
#include "ClassBattle.h"
#include "ClassMapBattle.h"
#include "ClassAStar.h" 
#include "ClassCommonConfig.h"
#include <mutex>

/*
	インデックスとタイルの配置の関係 (N = 4)

			(0, 0)
		(0, 1) (1, 0)
	 (0, 2) (1, 1) (2, 0)
 (0, 3) (1, 2) (2, 1) (3, 0)
	 (1, 3) (2, 2) (3, 1)
		(2, 3) (3, 2)
			(3, 3)
*/
class UnitMovePlan
{
private:
	int64 targetID = -1;
	Array<Point> path;
	Optional<Point> currentTarget;
	bool pathCompleted = false;
	bool needsRepath = false;
	bool retreating = false;
	Vec2 lastKnownEnemyPos;
public:
	Point lastPoint;
	UnitMovePlan() = default;
	void setLastKnownEnemyPos(const Vec2& pos)
	{
		lastKnownEnemyPos = pos;
	}

	Vec2 getLastKnownEnemyPos() const
	{
		return lastKnownEnemyPos;
	}
	void setTarget(int64 newTargetID)
	{
		targetID = newTargetID;
	}
	void setRetreating(bool va)
	{
		retreating = va;
	}
	bool isRetreating() const
	{
		return retreating;
	}

	int64 getTargetID() const
	{
		return targetID;
	}

	void setPath(const Array<Point>& newPath)
	{
		path = newPath;
		pathCompleted = path.isEmpty();
		currentTarget = pathCompleted ? Optional<Point>{} : Optional<Point>{ path.front() };
	}

	const Array<Point>& getPath() const
	{
		return path;
	}

	Optional<Point> getCurrentTarget() const
	{
		return currentTarget;
	}

	bool isPathCompleted() const
	{
		return pathCompleted;
	}

	void markRepathNeeded()
	{
		needsRepath = true;
	}

	bool isRepathNeeded() const
	{
		return needsRepath;
	}

	void stepToNext()
	{
		if (path.isEmpty())
		{
			pathCompleted = true;
			currentTarget.reset();
			return;
		}

		path.pop_front();

		if (path.isEmpty())
		{
			pathCompleted = true;
			currentTarget.reset();
		}
		else
		{
			currentTarget = path.front();
		}
	}
};


struct map_detail_position
{
	MapDetail classMapBattle;
	Vec2 pos;
};

enum class BuildMenu
{
	Home,
	Kouhei,
	Zirai
};

/// @brief 建築メニューの一つ一つの項目
class cRightMenu
{
public:
	/// @brief ↓↓の予約idを用いる時は、場所を紐づける用途になる
	long sortId = 0;
	/// @brief 将来、pushした後に並び替えたいことがあるかもしれない。
	/// 例えば兵優先コマンドなど←その用途ならタグ付けのほうが……
	long sortYoyakuId = 0;
	String key;
	String kindForProcess;

	Rect rect;//どれが選択されたかを判定するための矩形

	int32 count = -1;
	int32 buiSyu = 0; // 建築の種類
	Texture texture;

	double time = 1.0; // 建築にかかる時間

	/// @brief 予約と一緒のクラスで良いか疑問……
	bool isMoved = false;
	int32 rowBuilding = -1; // 予約した行
	int32 colBuilding = -1; // 予約した列

	///遠隔地に生産する用
	//int32 rowProduct = -1; // 予約した行
	//int32 colProduct = -1; // 予約した列

	String setumei = U""; // 説明文
};
class cRightMenuHomeYoyaku
{
public:
	int sortId = 0;
	String key;
	String kindForProcess;
	Texture texture;
	int32 count = -1;
	BuildMenu buildMenu = BuildMenu::Home;
};

class Battle : public FsScene
{
public:
	Battle(GameData& saveData, CommonConfig& commonConfig, SystemString ss);
	~Battle() override;
private:
	mutable std::mutex aiRootMutex;
	SystemString ss;
	Co::Task<void> start() override;

	Co::Task<void> mainLoop();

	void draw() const override;

	void renB();
	Array<Array<Unit*>> GetMovableUnitGroups();
	void AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex);

	void SetResourceTargets(Array<ResourcePointTooltip::TooltipTarget>& resourceTargets);

	CommonConfig& m_commonConfig;
	GameData& m_saveData;
	Camera2D camera{ Vec2{ 0, 0 },1.0,CameraControl::Wheel };
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };
	const Font fontSkill{ FontMethod::MSDF, 12, Typeface::Bold };
	const Font fontZinkei{ FontMethod::MSDF, 12, Typeface::Bold };
	const int32 underBarHeight = 30;

	bool PauseFlag = false;
	Array<Texture> textures;
	// マップの一辺のタイル数
	int32 N;
	// タイルの一辺の長さ（ピクセル）
	//Vec2 TileOffset{ 48, 24 };
	Vec2 TileOffset{ 50, 25 };
	// タイルの厚み（ピクセル）
	int32 TileThickness = 15;
	// 各列の四角形
	Array<Quad> columnQuads = MakeColumnQuads(N);
	// 各行の四角形
	Array<Quad> rowQuads = MakeRowQuads(N);
	// タイルの種類
	Grid<int32> grid;
	/// @brief 戦場の霧
	Grid<Visibility> visibilityMap;

	const double DistanceBetweenUnit = 32.0;
	const double DistanceBetweenUnitTate = 32.0;
	const Font systemFont{ 30, Typeface::Bold };

	ClassBattle classBattle;
	BattleStatus battleStatus = BattleStatus::Message;
	bool IsBattleMove = false;
	Vec2 viewPos;
	Point cursPos = Cursor::Pos();

	void UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize) const;
	Vec2 ToTileBottomCenter(const Point& index, const int32 N) const;
	Quad ToTile(const Point& index, const int32 N) const;
	Quad ToColumnQuad(const int32 x, const int32 N) const;
	Quad ToRowQuad(const int32 y, const int32 N) const;
	Array<Quad> MakeColumnQuads(const int32 N) const;
	Array<Quad> MakeRowQuads(const int32 N) const;
	//Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads);
	Texture LoadPremultipliedTexture(FilePathView path);

	Vec2 ToTileBottomCenterTTT(const Point& index, const int32 N) const;

	Unit& GetCU(long ID);
	Array<bool> arrayBattleZinkei;
	Array<bool> arrayBattleCommand;

	AsyncTask<void> task;
	AsyncTask<void> taskMyUnits;

	std::atomic<bool> abort{ false };
	std::atomic<bool> abortMyUnits{ false };
	std::atomic<bool> pauseTask{ false };
	std::atomic<bool> pauseTaskMyUnits{ false };
	std::atomic<bool> changeUnitMember{ false };
	HashTable<int64, UnitMovePlan> aiRootEnemy;
	HashTable<int64, UnitMovePlan> aiRootMy;
	Array<Array<Point>> debugRoot;
	Array<ClassAStar*> debugAstar;

	RenderTexture renderTextureSkill;
	RenderTexture renderTextureSkillUP;
	HashTable<String, Rect> htSkill;
	Array<String> nowSelectSkill;
	bool flagDisplaySkillSetumei = false;
	String nowSelectSkillSetumei = U"";
	Rect rectSkillSetumei = { 0,0,320,320 };

	String nowSelectBuildSetumei = U"";
	Rect rectSetumei = { 0,0,320,0 };

	RenderTexture renderTextureZinkei;
	Array<Rect> rectZinkei;

	RenderTexture renderTextureOrderSkill;
	Array<Rect> rectOrderSkill;

	RenderTexture renderTextureSelektUnit;
	Array<Rect> RectSelectUnit;

	RenderTexture renderTextureBuildMenuEmpty;
	RenderTexture renderTextureBuildMenuHome;
	RenderTexture renderTextureBuildMenuThunderwalker;
	RenderTexture renderTextureBuildMenuKouhei;
	RenderTexture renderTextureBuildMenuKeisouHoheiT;

	/// @brief 種別-アクション名,紐づくアクション 保守性を考え。
	HashTable<String, BuildAction> htBuildMenu;
	BuildAction& tempSelectComRight;
	Array<std::pair<String, BuildAction>> sortedArrayBuildMenu;

	HashTable<String, RenderTexture> htBuildMenuRenderTexture;


	// memo
	//0=参謀本部,1=Thunderwalker(大将),2=軽装歩兵詰所,3=騎兵詰所
//,4=重装歩兵詰所,5=工兵詰所,6=砲兵詰所,7=弓兵詰所,8=魔法墨詰所,9=騎士団詰所

	bool IsBuildMenuHome = false;

	/// @brief 単一選択とドラッグ選択が出来るようにしたいので、いつかenumにする
	bool IsBuildSelectTraget = false;
	long longBuildSelectTragetId = -1;
	/// @brief 移動後に建築するムーヴが発動しているか
	bool isMovedYoyaku = false;
	/// @brief 移動後に何を建築するか特定する為
	int32 cRightMenuCount = 0;
	/// @brief 選択された建築メニューのID
	int32 cRightMenuTargetCount = -1;
	/// @brief 移動後に資源ポイントを征服するユニットはどれか
	bool isGetResource = false;
	bool IsResourceSelectTraget = false;
	int32 rowResourceTarget = -1; // 予約した行
	int32 colResourceTarget = -1; // 予約した列

	long longBuildMenuHomeYoyakuIdCount = 0;

	//
	struct BuildMenuCategory
	{
		Array<cRightMenu> reservations;
		Stopwatch timer{ StartImmediately::No };
		double progressTime = -1.0;
	};
	// 建物種別ごとのリスト（buiSyu == index）
	Array<BuildMenuCategory> buildMenus = Array<BuildMenuCategory>(6);

	//

	//資源

	Stopwatch stopwatchFinance{ StartImmediately::No };
	Stopwatch stopwatchGameTime{ StartImmediately::No };
	/// @brief 金
	int32 gold = 0;

	/// @brief 信頼
	int32 trust = 0;

	/// @brief 物資
	int32 food = 0;

	/// @brief ミニマップ
	const Size miniMapSize = Size(200, 200);
	const Vec2 miniMapPosition = Scene::Size() - miniMapSize - Vec2(20, 20); // 右下から20pxオフセット
	struct MinimapCol
	{
		Color color;
		int32 x;
		int32 y;
	};
	HashTable<Point, ColorF> minimapCols;
	HashTable<String, Color> colData;
	void DrawMiniMap(const Grid<int32>& map, const RectF& cameraRect) const;
	void UnitRegister(String unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& listU, bool enemy);


	RectF getCameraView() const;
	void drawTileMap(const RectF& cameraView) const;
	void drawFog(const RectF& cameraView) const;
	void drawBuildings(const RectF& cameraView) const;
	void drawUnits(const RectF& cameraView) const;
	void drawHealthBars() const;
	void drawSelectionRectangleOrArrow() const;
	void drawSkillUI() const;
	void drawBuildMenu() const;
	void drawResourcesUI() const;
	void drawBuildTargetHighlight() const;
	void drawBuildDescription() const;
	void drawResourcePoints(const RectF& cameraView) const;

	ResourcePointTooltip resourcePointTooltip;

	void updateResourceIncome();
	void playResourceEffect();
	void refreshFogOfWar();
	void spawnTimedEnemy();
	void updateUnitHealthBars();
	void handleBuildMenuSelectionA();
	void updateBuildQueue();
	Co::Task<> checkCancelSelectionByUIArea();
	void updateUnitMovements();
	void handleSkillUISelection();
	Co::Task<void> processBattlePhase();
	void handleCameraInput();
	void handleUnitAndBuildingSelection();
	void handleBuildTargetSelection();
	Co::Task<void> co_handleResourcePointSelection();
	Co::Task<void> handleRightClickUnitActions(Point start, Point end);
	ClassHorizontalUnit getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf);
	void afterMovedPushToBuildMenu(Unit& unit);
	void addResource(Unit& unit);
	Vec2 calcLastMerge(const Array<Unit*>& units, std::function<Vec2(const Unit*)> getPos);
	void setMergePos(const Array<Unit*>& units, void (Unit::* setter)(const Vec2&), const Vec2& setPos);
	Stopwatch fogUpdateTimer{ StartImmediately::Yes };
	HashTable<Point, const Unit*> hsBuildingUnitForAstar;
	Array<std::unique_ptr<Unit>> unitsForHsBuildingUnitForAstar;

	void processBuildOnTilesWithMovement(const Array<Point>& tiles);
	void afterMovedPushToBuildMenuAdvanced(Unit& itemUnit);
	Array<Point> getRangeSelectedTiles(const Point& start, const Point& end) const;
	void processBuildOnTiles(const Array<Point>& tiles);
	bool canBuildOnTile(const Point& tile) const;
	void executeBuildOnTile(Unit& itemUnit);
	void createBuildingOnTile(const Point& tile, const BuildAction& buildAction);
	void updateBuildingHashTable(const Point& tile);
	void playBuildCompleteEffect(const Point& tile);
	Unit* GetCUSafe(long ID);

	int32 BattleMoveAStar(Array<ClassHorizontalUnit>& target,
		Array<ClassHorizontalUnit>& enemy,
		Array<Array<MapDetail>> mapData,
		HashTable<int64, UnitMovePlan>& aiRoot,
		Array<Array<Point>>& debugRoot,
		Array<ClassAStar*>& list,
		Array<Quad>& columnQuads,
		Array<Quad>& rowQuads,
		const int32 N,
		const std::atomic<bool>& abort,
		const std::atomic<bool>& pause,
		std::atomic<bool>& changeUnitMember,
		HashTable<Point, const Unit*>& hsBuildingUnitForAstar
	);

	int32 BattleMoveAStarMyUnitsKai(Array<ClassHorizontalUnit>& target,
		Array<ClassHorizontalUnit>& enemy,
		Array<Array<MapDetail>> mapData,
		HashTable<int64, UnitMovePlan>& aiRoot,
		Array<Array<Point>>& debugRoot,
		Array<ClassAStar*>& list,
		Array<Quad>& columnQuads,
		Array<Quad>& rowQuads,
		const int32 N,
		const std::atomic<bool>& abort,
		const std::atomic<bool>& pause,
		HashTable<Point, const Unit*>& hsBuildingUnitForAstar
	);

	int32 BattleMoveAStarMyUnits(Array<ClassHorizontalUnit>& target,
		Array<ClassHorizontalUnit>& enemy,
		Array<Array<MapDetail>> mapData,
		HashTable<int64, UnitMovePlan>& aiRoot,
		Array<Array<Point>>& debugRoot,
		Array<ClassAStar*>& list,
		Array<Quad>& columnQuads,
		Array<Quad>& rowQuads,
		const int32 N,
		const std::atomic<bool>& abort,
		const std::atomic<bool>& pause,
		HashTable<Point, const Unit*>& hsBuildingUnitForAstar
	);
};
