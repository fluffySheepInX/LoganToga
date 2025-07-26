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

class MapTile
{
public:
	/// @brief マップの一辺のタイル数
	int32 N = 64;
	/// @brief タイルの一辺の長さ（ピクセル）
	Vec2 TileOffset{ 50, 25 };
	/// @brief タイルの厚み（ピクセル）
	int32 TileThickness = 15;
	/// @brief タイルの種類
	Grid<int32> grid;
	/// @brief 
	HashTable<Point, ColorF> minimapCols;
	/// @brief 
	HashTable<String, Color> colData;
	/// @brief 
	const Size miniMapSize = Size(200, 200);
	/// @brief 
	const Vec2 miniMapPosition =
		Scene::Size()
		- miniMapSize
		- Vec2(20, 20); // 右下から20pxオフセット
	/// @brief 
	Array<Quad> columnQuads;
	/// @brief 
	Array<Quad> rowQuads;
	/// @brief 
	MapTile()
	{
		columnQuads = MakeColumnQuads(N);
		rowQuads = MakeRowQuads(N);
	}

	/// @brief 指定した位置がどのタイルに属するかを調べ、そのインデックスを返します。
	/// @param pos 調べる位置（座標）。
	/// @param columnQuads 列ごとのタイル領域を表すQuadの配列。
	/// @param rowQuads 行ごとのタイル領域を表すQuadの配列。
	/// @return 位置がタイル上にある場合はそのインデックス（Point型）、タイル上にない場合はnone（Optional<Point>型）。
	Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) const
	{
		int32 x = -1, y = -1;

		// タイルの列インデックスを調べる
		for (int32 i = 0; i < columnQuads.size(); ++i)
		{
			if (columnQuads[i].intersects(pos))
			{
				x = i;
				break;
			}
		}

		// タイルの行インデックスを調べる
		for (int32 i = 0; i < rowQuads.size(); ++i)
		{
			if (rowQuads[i].intersects(pos))
			{
				y = i;
				break;
			}
		}

		// インデックスが -1 の場合、タイル上にはない
		if ((x == -1) || (y == -1))
		{
			return none;
		}

		return Point{ x, y };
	}
	/// @brief 2D座標を等角投影座標に変換します。
	/// @param x 変換する元のX座標。
	/// @param y 変換する元のY座標。
	/// @return 等角投影後の座標を表すVec2オブジェクト。
	Vec2 ToIso(double x, double y) const
	{
		return Vec2((x - y), (x + y) / 2.0);
	}
	/// @brief タイルのインデックスから、タイルの四角形を計算します。
	/// @param index タイルのインデックス
	/// @param N マップの一辺のタイル数
	/// @return タイルの四角形
	Quad ToTile(const Point& index, const int32 N) const
	{
		const Vec2 bottomCenter = ToTileBottomCenter(index, N);

		return Quad{
			bottomCenter.movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
			bottomCenter.movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
			bottomCenter.movedBy(0, -TileThickness),
			bottomCenter.movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
		};
	}
	/// @brief 指定した画像の支配的な色（最も頻度の高い色）を取得します。
	/// @param imageName 色を取得する画像のファイル名。
	/// @param data 画像名とその支配色を格納するハッシュテーブル。参照渡しで、結果がキャッシュされます。
	/// @return 画像内で最も頻度の高い色（支配色）。
	Color GetDominantColor(const String imageName, HashTable<String, Color>& data)
	{
		// dataに指定された画像名が存在する場合はその色を返す
		if (data.contains(imageName))
		{
			return data[imageName];
		}

		const Image image{ PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/" + imageName };

		// 色の頻度を記録するハッシュマップ
		HashTable<Color, int32> colorCount;

		for (const auto& pixel : image)
		{
			// 透明ピクセルを除外したい場合は以下の if を有効に
			if (pixel.a == 0) continue;

			++colorCount[pixel];
		}

		// 最も頻度の高い色を見つける
		Color dominantColor = Palette::Black;
		int32 maxCount = 0;

		for (const auto& [color, count] : colorCount)
		{
			if (count > maxCount)
			{
				dominantColor = color;
				maxCount = count;
			}
		}

		//dataに追加
		data[imageName] = dominantColor;

		return dominantColor;
	}
	/// @brief ミニマップを描画します。マップデータとカメラの表示範囲をもとに、ミニマップ上にタイルとカメラ範囲を表示します。
	/// @param map ミニマップとして描画するマップデータ（int32型のグリッド）。
	/// @param cameraRect カメラの表示範囲を示す矩形（RectF型）。
	void DrawMiniMap(const int32 N, const RectF& cameraRect) const
	{
		//何故このサイズだとちょうどいいのかよくわかっていない
		//pngファイルは100*65
		//ミニマップ描写方法と通常マップ描写方法は異なるので、無理に合わせなくて良い？
		const Vec2 tileSize = Vec2(50, 25 + 25);

		// --- ミニマップに表示する位置・サイズ
		const Vec2 miniMapTopLeft = miniMapPosition;
		const SizeF miniMapDrawArea = miniMapSize;

		// --- マップの4隅をアイソメ変換 → 範囲を取得
		const Vec2 isoA = ToIso(0, 0) * tileSize;
		const Vec2 isoB = ToIso(N, 0) * tileSize;
		const Vec2 isoC = ToIso(0, N) * tileSize;
		const Vec2 isoD = ToIso(N, N) * tileSize;

		const double minX = Min({ isoA.x, isoB.x, isoC.x, isoD.x });
		const double minY = Min({ isoA.y, isoB.y, isoC.y, isoD.y });
		const double maxX = Max({ isoA.x, isoB.x, isoC.x, isoD.x });
		const double maxY = Max({ isoA.y, isoB.y, isoC.y, isoD.y });

		const SizeF isoMapSize = Vec2(maxX - minX, maxY - minY);

		// --- ミニマップに収めるためのスケーリング
		const double scale = Min(miniMapDrawArea.x / isoMapSize.x, miniMapDrawArea.y / isoMapSize.y);

		// --- スケーリング後のマップサイズ
		const SizeF scaledMapSize = isoMapSize * scale;

		// --- 中央に表示するためのオフセット（ミニマップ内で中央揃え）
		const Vec2 offset = miniMapTopLeft + (miniMapDrawArea - scaledMapSize) / 2.0 - Vec2(minX, minY) * scale;

		// --- 背景（薄い色で下地）を描画
		RectF(miniMapTopLeft, miniMapDrawArea).draw(ColorF(0.1, 0.1, 0.1, 0.5));

		// --- タイルの描画
		for (int32 y = 0; y < N; ++y)
		{
			for (int32 x = 0; x < N; ++x)
			{
				const Vec2 iso = ToIso(x, y) * tileSize;
				const Vec2 miniPos = iso * scale + offset;
				const SizeF tileSizeScaled = tileSize * scale;

				// デフォルト色
				ColorF tileColor = Palette::White;
				if (auto it = minimapCols.find(Point(x, y)); it != minimapCols.end())
				{
					tileColor = it->second;
				}

				// 描画
				RectF(miniPos, tileSizeScaled).draw(tileColor);
			}
		}

		// --- カメラ範囲を表示（白い枠）
		{
			const Vec2 camTopLeft = cameraRect.pos * Vec2(1.0, 1.0);
			const Vec2 camBottomRight = camTopLeft + cameraRect.size;
			RectF(camTopLeft * scale + offset, cameraRect.size * scale).drawFrame(1.5, ColorF(1.0));
		}
	}
	/// @brief タイルのインデックスから、タイルの底辺中央の座標を計算します。
	/// @param index タイルのインデックス
	/// @param N マップの一辺のタイル数
	/// @return タイルの底辺中央の座標
	Vec2 ToTileBottomCenter(const Point& index, const int32 N) const
	{
		const int32 i = index.manhattanLength();
		const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
		const int32 yi = (i < (N - 1)) ? i : (N - 1);
		const int32 k = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
		const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
		const double posY = (i * TileOffset.y);
		return{ (posX + TileOffset.x * 2 * k), posY };
	}
	/// @brief 指定した列のタイルによって構成される四角形を計算します。
	/// @param x 列インデックス
	/// @param N マップの一辺のタイル数
	/// @return 指定した列のタイルによって構成される四角形
	Quad ToColumnQuad(const int32 x, const int32 N) const
	{
		return{
			ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
			ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
			ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(0, 0),
			ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
		};
	}
	/// @brief 指定した行のタイルによって構成される四角形を計算します。
	/// @param y 行インデックス
	/// @param N マップの一辺のタイル数
	/// @return 指定した行のタイルによって構成される四角形
	Quad ToRowQuad(const int32 y, const int32 N) const
	{
		return{
			ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y),
			ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
			ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
			ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(0, 0)
		};
	}
	/// @brief 各列のタイルによって構成される四角形の配列を作成します。
	/// @param N マップの一辺のタイル数
	/// @return 各列のタイルによって構成される四角形の配列
	Array<Quad> MakeColumnQuads(const int32 N) const
	{
		Array<Quad> quads;

		for (int32 x = 0; x < N; ++x)
		{
			quads << ToColumnQuad(x, N);
		}

		return quads;
	}
	/// @brief 各行のタイルによって構成される四角形の配列を作成します。
	/// @param N マップの一辺のタイル数
	/// @return 各行のタイルによって構成される四角形の配列
	Array<Quad> MakeRowQuads(const int32 N) const
	{
		Array<Quad> quads;

		for (int32 y = 0; y < N; ++y)
		{
			quads << ToRowQuad(y, N);
		}

		return quads;
	}
};

class AStar
{
public:
	/// @brief 
	AsyncTask<void> taskAStarEnemy;
	/// @brief 
	AsyncTask<void> taskAStarMyUnits;
	/// @brief 
	std::atomic<bool> abortAStarEnemy{ false };
	/// @brief 
	std::atomic<bool> abortAStarMyUnits{ false };
	/// @brief 
	std::atomic<bool> pauseAStarTaskEnemy{ false };
	/// @brief
	std::atomic<bool> pauseAStarTaskMyUnits{ false };

};

class SafeUnitManager {
private:
	static inline HashTable<int64, std::weak_ptr<Unit>> unitRegistry;
	static inline std::mutex registryMutex;

public:
	static std::shared_ptr<Unit> GetSafeUnit(int64 unitID) {
		std::scoped_lock lock(registryMutex);

		if (auto it = unitRegistry.find(unitID); it != unitRegistry.end()) {
			if (auto unit = it->second.lock()) {
				return unit;  // 有効な参照
			}
			else {
				unitRegistry.erase(it);
				return nullptr;
			}
		}
		return nullptr;
	}

	static void RegisterUnit(std::shared_ptr<Unit> unit) {
		std::scoped_lock lock(registryMutex);
		unitRegistry[unit->ID] = unit;
	}
};

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
public:
	Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString ss);
	~Battle001() override;
private:
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
	/// @brief リソースターゲットのリストを設定します。
	/// @param classBattleManage バトル管理を行うClassBattleオブジェクト。
	/// @param resourceTargets ResourcePointTooltip::TooltipTargetの配列。設定されるリソースターゲットのリストです。
	/// @param mapTile 対象となるマップタイル。
	void SetResourceTargets(ClassBattle classBattleManage, Array<ResourcePointTooltip::TooltipTarget>& resourceTargets, MapTile mapTile);
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
	void UnitRegister(ClassBattle& classBattleManage, MapTile mapTile, String unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& listU, bool enemy);
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
	Array<Point> getRangeSelectedTiles(const Point& start, const Point& end, const MapTile mapTile) const;
	bool canBuildOnTile(const Point& tile, const ClassBattle& classBattleManage, const MapTile& mapTile) const;
	void handleDenseFormation(Point end);
	void handleHorizontalFormation(Point start, Point end);
	void handleSquareFormation(Point start, Point end);
	void handleUnitSelection(const RectF& selectionRect);
	//void forEachVisibleTile(const RectF& cameraView, const MapTile& mapTile, DrawFunc drawFunc) const;

	/// >>> プレイヤー操作
	void handleCameraInput();
	Co::Task<void> handleRightClickUnitActions(Point start, Point end);
	Array<Array<Unit*>> GetMovableUnitGroups();
	void AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex);
	Vec2 calcLastMerge(const Array<Unit*>& units, std::function<Vec2(const Unit*)> getPos);
	void setMergePos(const Array<Unit*>& units, void (Unit::* setter)(const Vec2&), const Vec2& setPos);
	ClassHorizontalUnit getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf);
	bool IsBuildSelectTraget = false;
	long longBuildSelectTragetId = -1;
	/// <<< プレイヤー操作

	/// @brief TODO:後で消す
	/// @param cmb 
	void GetTempResource(ClassMapBattle& cmb);


	/// @brief 
	GameData& m_saveData;
	/// @brief 
	Camera2D camera{ Vec2{ 0, 0 },1.0,CameraControl::Wheel };
	/// @brief 
	struct stOfFont
	{
		const Font font{ FontMethod::MSDF, 48, Typeface::Bold };
		const Font fontSkill{ FontMethod::MSDF, 12, Typeface::Bold };
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
	/// @brief 
	ClassBattle classBattleManage;


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
	/// <<<資源

	/// @brief A配列で、A*アルゴリズム用の建物ユニットのユニークポインタを格納
	Array<std::unique_ptr<Unit>> unitsForHsBuildingUnitForAstar;
	/// @brief  // ユニットの位置とそのユニットへのポインタを保持するht
	HashTable<Point, Array<Unit*>> hsBuildingUnitForAstar;

};

