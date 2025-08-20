#pragma once
#include "ClassSkill.h"
#include "EnumSkill.h"
#include "Common.h"
#include "ClassUnit.h"
#include "ClassHorizontalUnit.h"
#include "ClassBattle.h"
#include "ClassMapBattle.h"
#include "ClassAStar.h" 
#include "ClassCommonConfig.h"
#include "ClassUnitMovePlan.h"
#include "GameUIToolkit.h"
#include "ClassBuildAction.h"
#include "MapTile.h"
#include "UnitTooltip.h"
# include "280_ClassExecuteSkills.h" 

#include "AStar.h"
#include "SafeUnitManager.h"

class TestRunner; // Forward declaration for friending

/// @brief スポーン位置定数
enum class SpawnEdge : int32
{
	Left = 0,
	Right = 1,
	Top = 2,
	Bottom = 3
};

class Battle001 : public FsScene
{
	friend class TestRunner;
public:
	Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString ss);
	// Test-only constructor
	Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString ss, bool isTest);
	~Battle001() override;
	MapDetail parseMapDetail(StringView tileData, const ClassMap& classMap, CommonConfig& commonConfig);
	void initializeForTest(); // Public for test access
private:
	struct ProductionOrder {
		String spawn;
		int32 tempColBuildingTarget;
		int32 tempRowBuildingTarget;
		int32 count;
	};

	static constexpr double FOG_UPDATE_INTERVAL = 0.5;
	static constexpr double ENEMY_SPAWN_INTERVAL = 5.0;
	static constexpr int32 LIQUID_BAR_WIDTH = 64;
	static constexpr int32 LIQUID_BAR_HEIGHT = 8;
	static constexpr int32 LIQUID_BAR_HEIGHT_POS = 24;
	static constexpr double ARROW_THICKNESS = 10.0;
	static constexpr Vec2 ARROW_HEAD_SIZE = Vec2{ 40, 80 };
	static constexpr double SELECTION_THICKNESS = 3.0;
	static constexpr double BUILDING_FRAME_THICKNESS = 3.0;
	static constexpr int32 CIRCLE_FRAME_THICKNESS = 4;
	static constexpr int32 RESOURCE_CIRCLE_RADIUS = 16;
	static constexpr int32 RING_OFFSET_Y1 = 8;
	static constexpr int32 RING_OFFSET_Y2 = 16;
	static constexpr int32 EXCLAMATION_OFFSET_Y = 18;
	static constexpr double DIRECTION_ARROW_THICKNESS = 3.0;
	static constexpr Vec2 DIRECTION_ARROW_HEAD_SIZE = Vec2{ 20, 40 };
	static constexpr int32 INFO_TEXT_OFFSET_X = 20;
	static constexpr int32 INFO_TEXT_OFFSET_Y = -30;
	static constexpr int32 INFO_TEXT_PADDING = 4;
	static constexpr int32 RANDOM_MOVE_RANGE = 10;
	static constexpr int32 FORMATION_DENSE密集 = 0;
	static constexpr int32 FORMATION_HORIZONTAL横列 = 1;
	static constexpr int32 FORMATION_SQUARE正方 = 2;

	mutable std::mutex unitDataMutex;  // ユニットデータ専用ミューテックス
	/// @brief 
	SystemString ss;
	/// @brief 
	CommonConfig& m_commonConfig;
	/// @brief 
	/// @return 
	Co::Task<void> start() override;
	/// @brief 
	/// @return 
	Co::Task<void> mainLoop();
	/// @brief 
	void draw() const override;
	void initUI();
	void registerTextureAssets();
	void setupInitialUnits();
	void startAsyncTasks();
	void initFormationUI();
	void initSkillUI();
	void initBuildMenu();
	void initMinimap();
	/// @brief リソースターゲットのリストを設定します。
	/// @param classBattleManage バトル管理を行うClassBattleオブジェクト。
	/// @param resourceTargets ResourcePointTooltip::TooltipTargetの配列。設定されるリソースターゲットのリストです。
	/// @param mapTile 対象となるマップタイル。
	void SetResourceTargets(Array<Array<MapDetail>> mapData, Array<ResourcePointTooltip::TooltipTarget>& resourceTargets, MapTile mapTile);
	void UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize, MapTile& mapTile) const;
	void refreshFogOfWar(const ClassBattle& classBattleManage, Grid<Visibility>& visibilityMap, MapTile& mapTile);
	void updateBuildingHashTable(const Point& tile, const ClassBattle& classBattleManage, Grid<Visibility> visibilityMap, MapTile& mapTile);

	/// @brief 指定した経過時間後に敵ユニットをマップ上にスポーンさせます。
	/// @param classBattleManage 全ての敵ユニットのリストなど、バトル管理に関する情報を持つクラス。
	/// @param mapTile マップのサイズやタイル情報を持つクラス。
	void spawnTimedEnemy(ClassBattle& classBattleManage, MapTile mapTile);
	/// @brief ユニットを指定された位置に登録します。
	/// @param classBattleManage バトル管理用のClassBattleオブジェクト。
	/// @param mapTile ユニットを配置するマップタイル。
	/// @param unitName 登録するユニットの名前。
	/// @param col ユニットを配置する列番号。
	/// @param row ユニットを配置する行番号。
	/// @param num 登録するユニットの数。
	/// @param listU ユニットを格納するClassHorizontalUnitの配列への参照。
	/// @param enemy ユニットが敵かどうかを示すフラグ。
	void UnitRegister(ClassBattle& classBattleManage, const MapTile& mapTile, const String& unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& unit_list, bool enemy);
	/// @brief カメラの現在のビュー領域（矩形）を計算します。
	/// @param camera ビュー領域を計算するためのCamera2Dオブジェクト。
	/// @param mapTile タイルのオフセット情報を含むMapTileオブジェクト。
	/// @return カメラの中心位置、スケール、およびタイルのオフセットに基づいて計算されたRectF型のビュー領域。
	RectF getCameraView(const Camera2D& camera, const MapTile& mapTile) const;
	/// @brief カメラビューとマップタイル、可視性マップに基づいてフォグ（霧）を描画します。
	/// @param cameraView 描画範囲を指定するカメラの矩形領域。
	/// @param mapTile 描画対象となるマップタイルの情報。
	/// @param visibilityMap 各タイルの可視状態を示すグリッド。
	void drawFog(const RectF& cameraView, const MapTile& mapTile, const Grid<Visibility> visibilityMap) const;
	/// @brief タイルマップをカメラビュー内に描画します。
	/// @param cameraView 描画範囲を指定するカメラの矩形領域。
	/// @param mapTile タイルマップの情報を持つオブジェクト。
	/// @param classBattleManage バトルマップのデータを管理するクラス。
	void drawTileMap(const RectF& cameraView, const MapTile& mapTile, const ClassBattle& classBattleManage) const;
	/// @brief 指定されたファイルパスとテクスチャ設定から、テクスチャアセットデータを作成します。ロード時にアルファ値を考慮して色成分を補正し、テクスチャを生成します。
	/// @param path テクスチャファイルのパス。
	/// @param textureDesc テクスチャの設定情報。
	/// @return 作成された TextureAssetData のユニークポインタ。
	std::unique_ptr<TextureAssetData> MakeTextureAssetData1(const FilePath& path, const TextureDesc textureDesc);
	/// @brief カメラビュー内の建物を描画します。
	/// @param cameraView 描画範囲を指定するカメラの矩形領域。
	/// @param classBattleManage バトルの状態やクラス情報を管理するオブジェクト。
	/// @param mapTile 描画対象となるマップタイル。
	void drawBuildings(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const;
	/// @brief カメラビュー内のユニットを描画します。
	/// @param cameraView 描画範囲を指定する矩形領域。
	/// @param classBattleManage ユニット情報を管理するClassBattleオブジェクト。
	void drawUnits(const RectF& cameraView, const ClassBattle& classBattleManage) const;
	/// @brief リソースポイントをカメラビュー内に描画します。
	/// @param cameraView 描画範囲を指定するカメラの矩形領域。
	/// @param classBattleManage バトルの状態やマップデータを管理するクラス。
	/// @param mapTile タイル座標や描画位置の計算に使用するマップタイル情報。
	void drawResourcePoints(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const;
	/// @brief 選択範囲の矩形または矢印を描画します。
	void drawSelectionRectangleOrArrow() const;
	void drawBuildTargetHighlight(const MapTile& mapTile) const;
	void drawHUD() const;
	void drawFormationUI() const;
	void drawMinimap() const;
	Array<Point> getRangeSelectedTiles(const Point& start, const Point& end, const MapTile mapTile) const;
	bool canBuildOnTile(const Point& tile, const ClassBattle& classBattleManage, const MapTile& mapTile) const;
	void handleDenseFormation(Point end);
	void handleHorizontalFormation(Point start, Point end);
	void handleSquareFormation(Point start, Point end);
	void setUnitsSelectedInRect(const RectF& selectionRect);
	void issueMoveOrder(Point start, Point end);
	//void forEachVisibleTile(const RectF& cameraView, const MapTile& mapTile, DrawFunc drawFunc) const;
	void drawSkillUI() const;
	void drawBuildDescription() const;
	void drawBuildMenu() const;
	void drawResourcesUI() const;
	void createRenderTex();
	Color GetDominantColor(const String imageName, HashTable<String, Color>& data);
	void DrawMiniMap(const Grid<Visibility>& map, const RectF& cameraRect) const;
	void playResourceEffect();
	void updateResourceIncome();
	Co::Task<> checkCancelSelectionByUIArea();
	void handleBuildMenuSelectionA();
	void processUnitBuildMenuSelection(Unit& unit);
	void handleCarrierStoreCommand(Unit& unit);
	void handleCarrierReleaseCommand(Unit& unit);
	void handleUnitAndBuildingSelection();
	void processClickSelection();
	Optional<long long> findClickedBuildingId() const;
	Optional<long long> findClickedUnitId() const;
	void deselectAll();
	void toggleUnitSelection(long long unit_id);
	void toggleBuildingSelection(long long building_id);
	void handleSkillUISelection();
	void updateUnitHealthBars();
	void updateUnitMovements();
	void updatePlayerUnitMovements();
	void handlePlayerPathMovement(Unit& unit, ClassUnitMovePlan& plan);
	void startPlayerPathMovement(Unit& unit, ClassUnitMovePlan& plan);
	void handleCompletedPlayerPath(Unit& unit, ClassUnitMovePlan& plan);
	void updateEnemyUnitMovements();
	void handleEnemyPathMovement(Unit& unit, ClassUnitMovePlan& plan);
	void startEnemyPathMovement(Unit& unit, ClassUnitMovePlan& plan);
	void startAsyncFogCalculation();
	void calculateFogFromUnits(Grid<Visibility>& visMap, const Array<Unit>& units);
	ClassMapBattle GetClassMapBattle(ClassMap classMap, CommonConfig& commonConfig);
	//MapDetail parseMapDetail(StringView tileData, const ClassMap& classMap, CommonConfig& commonConfig);


	/// >>>ミニマップ
	/// @brief ミニマップのサイズを表す定数
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
	/// <<<ミニマップ

	/// >>> ドラッグ操作
	bool isUnitSelectionPending = false;    // ユニット選択が保留中かどうか
	Point clickStartPos;                     // クリック開始位置
	static constexpr double CLICK_THRESHOLD = 5.0;  // クリックと判定する最大移動距離
	/// <<< ドラッグ操作

	// 💡 非同期Fog計算用
	AsyncTask<void> taskFogCalculation;
	std::atomic<bool> abortFogTask{ false };
	std::atomic<bool> fogDataReady{ false };
	Grid<Visibility> nextVisibilityMap; // 次フレーム用バッファ
	mutable std::mutex fogMutex;

	/// >>> プレイヤー操作
	void handleCameraInput();
	Co::Task<void> handleRightClickUnitActions(Point start, Point end);
	Array<Array<Unit*>> GetMovableUnitGroups();
	void AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex);
	Vec2 calcLastMerge(const Array<Unit*>& units, std::function<Vec2(const Unit*)> getPos);
	void setMergePos(const Array<Unit*>& units, void (Unit::* setter)(const Vec2&), const Vec2& setPos);
	Array<Unit*> getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf);
	bool IsBuildSelectTraget = false;
	long longBuildSelectTragetId = -1;
	bool IsBuildMenuHome = false;
	/// <<< プレイヤー操作

	/// @brief 
	GameData& m_saveData;
	/// @brief 
	Camera2D camera{ Vec2{ 0, 0 },1.0,CameraControl::Wheel };
	/// @brief 
	struct stOfFont
	{
		const Font font{ FontMethod::MSDF, 48, Typeface::Bold };
		const Font fontSkill{ 12 };
		const Font fontZinkei{ FontMethod::MSDF, 12, Typeface::Bold };
		const Font fontSystem{ 30, Typeface::Bold };
		const Font emojiFontTitle = Font{ 48, Typeface::ColorEmoji };
		const Font emojiFontSection = Font{ 12, Typeface::ColorEmoji };
		const Font emojiFontInfo = Font{ 12, Typeface::ColorEmoji };
		const Font emojiFontSystem{ 30, Typeface::ColorEmoji };

		stOfFont()
		{
			font.addFallback(emojiFontTitle);
			fontSkill.addFallback(emojiFontSection);
			fontZinkei.addFallback(emojiFontInfo);
			fontSystem.addFallback(emojiFontSystem);
		}
	};
	/// @brief 
	stOfFont fontInfo;
	/// @brief 
	const int32 underBarHeight = 30;
	/// @brief 戦場の霧
	Grid<Visibility> visibilityMap;
	/// @brief 
	BattleStatus battleStatus = BattleStatus::Message;
	/// @brief 
	bool is移動指示 = false;
	/// @brief 拡張性のため、enumを使わずに配列で管理
	Array<bool> arrayBattleZinkei;
	/// @brief
	Array<bool> arrayBattleCommand;

	/// @brief カーソルの現在位置
	Point cursPos = Cursor::Pos();
	/// @brief 地図タイルを表す
	MapTile mapTile;
	/// @brief 
	AStar aStar;
	HashTable<int64, ClassUnitMovePlan> aiRootEnemy;
	HashTable<int64, ClassUnitMovePlan> aiRootMy;

	/// @brief 
	ClassBattle classBattleManage;

	void updateBuildQueue();
	void processUnitBuildQueue(Unit& unit, Array<ProductionOrder>& productionList);
	void handleUnitTooltip();
	void processBuildOnTilesWithMovement(const Array<Point>& tiles);
	void handleBuildTargetSelection();


	UnitTooltip unitTooltip;






	Stopwatch fogUpdateTimer{ StartImmediately::Yes };

	/// @brief 2つの単位間の距離を表す定数
	const double DistanceBetweenUnitWidth = 32.0;
	/// @brief 2つの単位間の距離を表す定数
	const double DistanceBetweenUnitHeight = 32.0;


	/// >>>資源
	/// @brief 
	ResourcePointTooltip resourcePointTooltip;
	Stopwatch stopwatchFinance{ StartImmediately::No };
	Stopwatch stopwatchGameTime{ StartImmediately::No };
	int32 gold = 0;
	int32 trust = 0;
	int32 food = 0;
	HashSet<Unit*> unitOf資源を狙う;
	/// <<<資源

	/// @brief A配列で、A*アルゴリズム用の建物ユニットのユニークポインタを格納
	Array<std::unique_ptr<Unit>> unitsForHsBuildingUnitForAstar;
	/// @brief  // ユニットの位置とそのユニットへのポインタを保持するht
	HashTable<Point, Array<Unit*>> hsBuildingUnitForAstar;

	/// >>> UI関連
	/// @brief 種別-アクション名,紐づくアクション 保守性を考え。
	HashTable<String, BuildAction> htBuildMenu;
	BuildAction& tempSelectComRight;
	Array<std::pair<String, BuildAction>> sortedArrayBuildMenu;

	HashTable<String, RenderTexture> htBuildMenuRenderTexture;
	RenderTexture renderTextureBuildMenuEmpty;
	RenderTexture renderTextureSkill;
	RenderTexture renderTextureSkillUP;
	RenderTexture renderTextureZinkei;
	Array<Rect> rectZinkei;
	RenderTexture renderTextureOrderSkill;
	Array<Rect> rectOrderSkill;
	RenderTexture renderTextureSelektUnit;
	Array<Rect> RectSelectUnit;
	HashTable<String, Rect> htSkill;
	Array<String> nowSelectSkill;
	bool flagDisplaySkillSetumei = false;
	String nowSelectSkillSetumei = U"";
	Rect rectSkillSetumei = { 0,0,320,320 };
	String nowSelectBuildSetumei = U"";
	Rect rectSetumei = { 0,0,320,0 };
	/// <<< UI関連

	Array<ClassExecuteSkills> m_Battle_player_skills;
	Array<ClassExecuteSkills> m_Battle_enemy_skills;
	Array<ClassExecuteSkills> m_Battle_neutral_skills;

	void SkillProcess(Array<ClassHorizontalUnit>& attacker_groups, Array<ClassHorizontalUnit>& target_groups, Array<ClassExecuteSkills>& executed_skills);
	void findAndExecuteSkillForUnit(Unit& unit, Array<ClassHorizontalUnit>& target_groups, Array<ClassExecuteSkills>& executed_skills);
	bool tryActivateSkillOnTargetGroup(Array<ClassHorizontalUnit>& target_groups, const Vec2& attacker_pos, Unit& attacker, Skill& skill, Array<ClassExecuteSkills>& executed_skills);
	bool isTargetInRange(const Unit& attacker, const Unit& target, const Skill& skill) const;
	ClassExecuteSkills createSkillExecution(Unit& attacker, const Unit& target, const Skill& skill);
	void processSkillEffects();
	void updateAndCheckCollisions(ClassExecuteSkills& executedSkill, Array<ClassHorizontalUnit>& targetUnits, Array<ClassHorizontalUnit>& friendlyUnits);
	void handleBulletCollision(ClassBullets& bullet, ClassExecuteSkills& executedSkill, Array<ClassHorizontalUnit>& targetUnits, bool& isBomb);
	void CalucDamage(Unit& itemTarget, double strTemp, ClassExecuteSkills& ces);
	bool applySkillEffectAndRegisterHit(bool& bombCheck, Array<int32>& arrayNo, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Unit& itemTarget);

	// Refactored mainLoop helpers
	void updateGameSystems();
	Co::Task<void> handlePlayerInput();
	bool wasBuildMenuClicked();
	Co::Task<void> handleRightClickInput();
	void handleFormationSelection();
	void updateAllUnits();
	void processCombat();
	void checkUnitDeaths();
};

