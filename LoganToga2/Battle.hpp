#pragma once
#include "Common.h"
#include "ClassSkill.h" 
#include "ClassUnit.h"
#include "ClassHorizontalUnit.h"
#include "EnumSkill.h"
#include "ClassBattle.h"
#include "ClassMapBattle.h"
#include "ClassAStar.h" 

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


enum class BuildMenu
{
	Home,
	Kouhei,
	Zirai
};
struct cBuildPrepareKindAndCount {
	int32 sortId = 0;
	String kind;
	int32 count;
};
class cBuildPrepare
{
public:
	HashTable<String, cBuildPrepareKindAndCount> htCountAndSyu;
	int32 buiSyu = 0; // 建築の種類
};

class cRightMenu
{
public:
	int sortId = 0;
	String key;
	String nameOfUnitOrBuildingOrSkill;
	Rect rect;
	int32 count = -1;
	int32 buiSyu = 0; // 建築の種類
	Texture texture;
};
class cRightMenuHomeYoyaku
{
public:
	int sortId = 0;
	String key;
	String nameOfUnitOrBuildingOrSkill;
	Texture texture;
	int32 count = -1;
	BuildMenu buildMenu = BuildMenu::Home;
};

class Battle : public FsScene
{
public:
	Battle(GameData& saveData);
private:
	Co::Task<void> start() override;

	Co::Task<void> mainLoop();

	void draw() const override;

	void renB(RenderTexture&, cBuildPrepare&, Array<cRightMenu>&);

	GameData& m_saveData;
	Camera2D camera{ Vec2{ 0, 0 },1.0,CameraControl::Wheel };
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	bool PauseFlag = false;
	Array<Texture> textures;
	// マップの一辺のタイル数
	int32 N;
	// タイルの一辺の長さ（ピクセル）
	Vec2 TileOffset{ 48, 24 };
	// タイルの厚み（ピクセル）
	int32 TileThickness = 17;
	// 各列の四角形
	Array<Quad> columnQuads = MakeColumnQuads(N);
	// 各行の四角形
	Array<Quad> rowQuads = MakeRowQuads(N);
	// タイルの種類
	Grid<int32> grid;
	/// @brief 戦場の霧
	Grid<Visibility> visibilityMap;

	int32 DistanceBetweenUnit = 0;
	int32 DistanceBetweenUnitTate = 0;

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


	Unit& GetCU(long ID);
	Array<bool> arrayBattleZinkei;
	Array<bool> arrayBattleCommand;

	AsyncTask<int32> task;
	AsyncTask<int32> taskMyUnits;

	std::atomic<bool> abort{ false };
	std::atomic<bool> abortMyUnits{ false };
	std::atomic<bool> pauseTask{ false };
	std::atomic<bool> pauseTaskMyUnits{ false };
	HashTable<int64, Array<Point>> aiRoot;
	Array<Array<Point>> debugRoot;
	Array<ClassAStar*> debugAstar;

	RenderTexture renderTextureSkill;
	RenderTexture renderTextureSkillUP;
	HashTable<String, Rect> htSkill;
	Array<String> nowSelectSkill;
	bool flagDisplaySkillSetumei = false;
	String nowSelectSkillSetumei = U"";
	Rect rectSkillSetumei = { 0,0,320,320 };

	RenderTexture renderTextureZinkei;
	Array<Rect> rectZinkei;

	RenderTexture renderTextureOrderSkill;
	Array<Rect> rectOrderSkill;

	RenderTexture renderTextureSelektUnit;
	Array<Rect> RectSelectUnit;

	RenderTexture renderTextureBuildMenuEmpty;
	RenderTexture renderTextureBuildMenuHome;
	RenderTexture renderTextureBuildMenuKouhei;
	//Array<Rect> rectBuildMenuHome;

	/// @brief home
	cBuildPrepare cbp;
	/// @brief 工兵
	cBuildPrepare cbpKouhei;
	Array<cRightMenu> htBuildMenuHome;
	Array<cRightMenu> htBuildMenuKouhei;
	bool IsBuildMenuHome = false;
	int32 buiSyu = 0; // 建築の種類

	long longBuildMenuHomeYoyakuIdCount = 0;
	//Array<Array<cRightMenu>>将来的にはこうして拡張性を持たせる
	Array<cRightMenu> arrBuildMenuHomeYoyaku;
	Array<cRightMenu> arrBuildMenuHouheiYoyaku;
	Stopwatch stopwatch{ StartImmediately::No };
	const double durationSec = 5.0;
	//これArrayにしなきゃ駄目（並列して建築できないから）
	double t = -1.0;
	Array<double> arrT = { -1.0,-1.0 };
	Stopwatch stopwatch001{ StartImmediately::No };

};
