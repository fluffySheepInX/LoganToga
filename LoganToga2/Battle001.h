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
// Services & Strategy (Phase1)
#include "Services/CooldownService.h"
#include "Services/UsageService.h"
#include "Services/ProjectileHitService.h"
#include "Combat/Trajectory.h"

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
	void UnitRegister(ClassBattle& classBattleManage, const MapTile& mapTile, const String& unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& unit_list, bool enemy);

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
	static constexpr Vec2 ARROW_HEAD_SIZE = Vec2{ 40,80 };
	static constexpr double SELECTION_THICKNESS = 3.0;
	static constexpr double BUILDING_FRAME_THICKNESS = 3.0;
	static constexpr int32 CIRCLE_FRAME_THICKNESS = 4;
	static constexpr int32 RESOURCE_CIRCLE_RADIUS = 16;
	static constexpr int32 RING_OFFSET_Y1 = 8;
	static constexpr int32 RING_OFFSET_Y2 = 16;
	static constexpr int32 EXCLAMATION_OFFSET_Y = 18;
	static constexpr double DIRECTION_ARROW_THICKNESS = 3.0;
	static constexpr Vec2 DIRECTION_ARROW_HEAD_SIZE = Vec2{ 20,40 };
	static constexpr int32 INFO_TEXT_OFFSET_X = 20;
	static constexpr int32 INFO_TEXT_OFFSET_Y = -30;
	static constexpr int32 INFO_TEXT_PADDING = 4;
	static constexpr int32 RANDOM_MOVE_RANGE = 10;
	static constexpr int32 FORMATION_DENSE密集 = 0;
	static constexpr int32 FORMATION_HORIZONTAL横列 = 1;
	static constexpr int32 FORMATION_SQUARE正方 = 2;

	SystemString ss;
	CommonConfig& m_commonConfig;
	Co::Task<void> start() override;
	Co::Task<void> mainLoop();
	void draw() const override;
	void initUI();
	void registerTextureAssets();
	void setupInitialUnits();
	void startAsyncTasks();
	void initFormationUI();
	void initSkillUI();
	void initBuildMenu();
	void initMinimap();
	void SetResourceTargets(Array<Array<MapDetail>> mapData, Array<ResourcePointTooltip::TooltipTarget>& resourceTargets, MapTile mapTile);
	void UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize, MapTile& mapTile) const;
	void refreshFogOfWar(const ClassBattle& classBattleManage, Grid<Visibility>& visibilityMap, MapTile& mapTile);
	void updateBuildingHashTable(const Point& tile, const ClassBattle& classBattleManage, Grid<Visibility> visibilityMap, MapTile& mapTile);
	void spawnTimedEnemy(ClassBattle& classBattleManage, MapTile mapTile);
	void spawnTimedEnemyEni(ClassBattle& classBattleManage, MapTile mapTile);
	void spawnTimedEnemySkirmisher(ClassBattle& classBattleManage, MapTile mapTile);
	RectF getCameraView(const Camera2D& camera, const MapTile& mapTile) const;
	void drawFog(const RectF& cameraView, const MapTile& mapTile, const Grid<Visibility> visibilityMap) const;
	void drawTileMap(const RectF& cameraView, const MapTile& mapTile, const ClassBattle& classBattleManage) const;
	std::unique_ptr<TextureAssetData> MakeTextureAssetData1(const FilePath& path, const TextureDesc textureDesc);
	void drawBuildings(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const;
	void drawUnits(const RectF& cameraView, const ClassBattle& classBattleManage) const;
	void drawResourcePoints(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const;
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
	void drawSkillUI() const;
	void drawBuildDescription() const;
	void drawBuildMenu() const;
	void drawResourcesUI() const;
	void drawShadows(const RectF& cameraView, const ClassBattle& classBattleManage) const;
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
	void calculateFogFromUnits(Grid<Visibility>& visMap, const Array<const Unit*>& units);
	ClassMapBattle GetClassMapBattle(ClassMap classMap, CommonConfig& commonConfig);

	const Size miniMapSize = Size(200, 200);
	const Vec2 miniMapPosition = Scene::Size() - miniMapSize - Vec2(20, 20);
	struct MinimapCol { Color color; int32 x; int32 y; };
	HashTable<Point, ColorF> minimapCols;
	HashTable<String, Color> colData;

	bool isUnitSelectionPending = false;
	Point clickStartPos;
	static constexpr double CLICK_THRESHOLD = 5.0;

	AsyncTask<void> taskFogCalculation;
	std::atomic<bool> abortFogTask{ false };
	std::atomic<bool> fogDataReady{ false };
	Grid<Visibility> nextVisibilityMap;
	mutable std::mutex fogMutex;

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

	GameData& m_saveData;
	Camera2D camera{ Vec2{0,0 },1.0,CameraControl::Wheel };
	struct stOfFont {
		const Font font{ FontMethod::MSDF,48, Typeface::Bold };
		const Font fontSkill{ 12 };
		const Font fontZinkei{ FontMethod::MSDF,12, Typeface::Bold };
		const Font fontSystem{ 30, Typeface::Bold };
		const Font emojiFontTitle = Font{ 48, Typeface::ColorEmoji };
		const Font emojiFontSection = Font{ 12, Typeface::ColorEmoji };
		const Font emojiFontInfo = Font{ 12, Typeface::ColorEmoji };
		const Font emojiFontSystem{ 30, Typeface::ColorEmoji };
		stOfFont() {
			font.addFallback(emojiFontTitle);
			fontSkill.addFallback(emojiFontSection);
			fontZinkei.addFallback(emojiFontInfo);
			fontSystem.addFallback(emojiFontSystem);
		}
	};
	stOfFont fontInfo;
	const int32 underBarHeight = 30;
	Grid<Visibility> visibilityMap;
	BattleStatus battleStatus = BattleStatus::Message;
	bool is移動指示 = false;
	Array<bool> arrayBattleZinkei;
	Array<bool> arrayBattleCommand;
	Point cursPos = Cursor::Pos();
	MapTile mapTile;
	AStar aStar;
	HashTable<int64, ClassUnitMovePlan> aiRootEnemy;
	HashTable<int64, ClassUnitMovePlan> aiRootMy;
	ClassBattle classBattleManage;

	void updateBuildQueue();
	void processUnitBuildQueue(Unit& unit, Array<ProductionOrder>& productionList);
	void handleUnitTooltip();
	void processBuildOnTilesWithMovement(const Array<Point>& tiles);
	void handleBuildTargetSelection();

	UnitTooltip unitTooltip;
	Stopwatch fogUpdateTimer{ StartImmediately::Yes };
	const double DistanceBetweenUnitWidth = 32.0;
	const double DistanceBetweenUnitHeight = 32.0;

	ResourcePointTooltip resourcePointTooltip;
	Stopwatch stopwatchFinance{ StartImmediately::No };
	Stopwatch stopwatchGameTime{ StartImmediately::No };
	int32 gold = 0;
	int32 trust = 0;
	int32 food = 0;
	HashSet<Unit*> unitOf資源を狙う;

	Array<std::unique_ptr<Unit>> unitsForHsBuildingUnitForAstar;
	HashTable<Point, Array<Unit*>> hsBuildingUnitForAstar;

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

	void updateGameSystems();
	Co::Task<void> handlePlayerInput();
	bool wasBuildMenuClicked();
	Co::Task<void> handleRightClickInput();
	void handleFormationSelection();
	void updateAllUnits();
	void processCombat();
	void checkUnitDeaths();

	// Phase1 Services
	CooldownService cooldownService;
	UsageService usageService;
	ProjectileHitService projectileHitService;
};

