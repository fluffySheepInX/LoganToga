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
# include "280_ClassExecuteSkills.h" 

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

	std::atomic<bool> changeUnitMember{ false };
	std::mutex aiRootMutex;
	std::mutex arrayMutex;

	Optional<ClassAStar*> SearchMinScore(const Array<ClassAStar*>& ls) {
		int minScore = std::numeric_limits<int>::max();
		int minCost = std::numeric_limits<int>::max();
		Optional<ClassAStar*> targetClassAStar;

		for (const auto& itemClassAStar : ls) {
			if (itemClassAStar->GetAStarStatus() != AStarStatus::Open)
			{
				continue;
			}
			int score = itemClassAStar->GetCost() + itemClassAStar->GetHCost();

			if (score < minScore || (score == minScore && itemClassAStar->GetCost() < minCost)) {
				minScore = score;
				minCost = itemClassAStar->GetCost();
				targetClassAStar = itemClassAStar;
			}
		}

		return targetClassAStar;
	}

	int32 BattleMoveAStar(Array<ClassHorizontalUnit>& target,
							Array<ClassHorizontalUnit>& enemy,
							Array<Array<MapDetail>> mapData,
							HashTable<int64, ClassUnitMovePlan>& aiRoot,
							const std::atomic<bool>& abort,
							const std::atomic<bool>& pause,
							std::atomic<bool>& changeUnitMember,
							HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
							MapTile& mapTile
	)
	{
		static size_t unitIndexEnemy = 0;
		constexpr size_t MaxUnitsPerFrame = 5;

		size_t processed = 0;
		Array<Unit*> flatList;

		// --- フラット化：FlagMoveAI が立っている敵ユニットのみ抽出
		std::scoped_lock lock(arrayMutex);
		for (auto& group : enemy)
		{
			for (auto& unit : group.ListClassUnit)
			{
				if (!unit.IsBattleEnable || unit.IsBuilding) continue;
				if (unit.moveState == moveState::MoveAI)
					flatList.push_back(&unit);
			}
		}

		const size_t total = flatList.size();
		if (total == 0)
			return 0;

		const auto isValidTarget = [](const Unit& unit) -> bool
			{
				//これどうなん？　GATEは壊せるけど無視ということか？
				if (unit.IsBuilding && (unit.mapTipObjectType == MapTipObjectType::WALL2
					|| unit.mapTipObjectType == MapTipObjectType::GATE))
					return false;

				if (!unit.IsBattleEnable)
					return false;

				return true;
			};

		for (size_t i = 0; i < total; ++i)
		{
			if (abort)
				break;

			size_t idx = (unitIndexEnemy + i) % total;
			Unit& unit = *flatList[idx];

			Optional<Size> nowIndex = mapTile.ToIndex(unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (!nowIndex)
				continue;

			//標的は次のうちのどれか
			//1.ランダムに決定
			//2.最寄りの敵
			//3.一番弱い敵
			//4.一番体力の無い敵
			//...など

			//最寄りの敵の座標を取得
			HashTable<double, Unit> dicDis;
			Vec2 posA = unit.GetNowPosiCenter();
			auto uisuif = target;
			for (const auto& ccc : uisuif) {
				for (const auto& ddd : ccc.ListClassUnit) {
					if (!isValidTarget(ddd)) continue;

					Vec2 posB = ddd.GetNowPosiCenter();
					double dist = posA.distanceFrom(posB);
					while (dicDis.contains(dist)) {
						dist += 0.0001; // 衝突回避
					}
					if (abort == true) break;
					dicDis.emplace(dist, ddd);
				}
			}

			if (dicDis.size() == 0)
				continue;

			auto minElement = dicDis.begin();
			for (auto it = dicDis.begin(); it != dicDis.end(); ++it)
			{
				if (it->first < minElement->first)
					minElement = it;
			}

			bool flagGetEscapeRange = false;
			Vec2 retreatTargetPos;
			//escape_rangeの範囲なら、撤退。その為、反対側の座標を調整したものを扱う
			if (unit.Escape_range >= 1)
			{
				Circle cCheck = Circle(unit.GetNowPosiCenter(), unit.Escape_range);
				Circle cCheck2 = Circle(minElement->second.GetNowPosiCenter(), 1);
				if (cCheck.intersects(cCheck2) == true)
				{
					//撤退
					double newDistance = 50.0;
					double angle = atan2(minElement->second.GetNowPosiCenter().y - unit.GetNowPosiCenter().y,
						minElement->second.GetNowPosiCenter().x - unit.GetNowPosiCenter().x);
					double xC, yC;
					// 反対方向に進むために角度を180度反転
					angle += Math::Pi;
					xC = unit.GetNowPosiCenter().x + newDistance * cos(angle);
					yC = unit.GetNowPosiCenter().y + newDistance * sin(angle);
					//minElement->second.nowPosiLeft = Vec2(xC, yC);

					//TODO 画面端だとタイル外となるので、調整
					retreatTargetPos = Vec2(xC, yC);
					flagGetEscapeRange = true;
				}
			}

			if ((unit.moveState == moveState::Moving || unit.moveState == moveState::MovingEnd)
				&& flagGetEscapeRange == false)
				continue;

			//最寄りの敵のマップチップを取得
			s3d::Optional<Size> nowIndexEnemy;
			nowIndexEnemy = flagGetEscapeRange
				? mapTile.ToIndex(retreatTargetPos, mapTile.columnQuads, mapTile.rowQuads)
				: mapTile.ToIndex(minElement->second.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (nowIndexEnemy.has_value() == false) continue;
			if (nowIndexEnemy.value() == nowIndex.value()) continue;

			////現在地を開く
			ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);
			Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, mapTile.N);
			Array<Point> listRoot;
			MicrosecClock mc;
			Stopwatch taskTimer;
			taskTimer.restart();
			////移動経路取得
			while (true)
			{
				if (taskTimer.s() > 3) break;
				if (abort == true) break;
				if (startAstar.has_value() == false)
				{
					listRoot.clear();
					break;
				}
				classAStarManager.OpenAround(startAstar.value(),
												mapData,
												enemy,
												target,
												mapTile.N,
												hsBuildingUnitForAstar
				);
				//Print << U"BattleMoveAStar:A*探索ノード数: {0}:{1}"_fmt(classAStarManager.GetPool().size(), mc.us());
				startAstar.value()->SetAStarStatus(AStarStatus::Closed);
				classAStarManager.RemoveClassAStar(startAstar.value());
				if (classAStarManager.GetListClassAStar().size() != 0)
					startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
				if (startAstar.has_value() == false)
					continue;

				//敵まで到達したか
				if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
				{
					startAstar.value()->GetRoot(listRoot);
					listRoot.reverse();
					classAStarManager.Clear();
					break;
				}
			}

			// 経路が取得できた場合、aiRootにセット
			if (listRoot.size() != 0)
			{
				ClassUnitMovePlan plan;
				if (flagGetEscapeRange)
				{
					// もし撤退中なら特別なターゲットIDを設定
					// 撤退中は、経路の最初の位置を最後に見た敵の位置として記録する
					plan.setRetreating(true);
					plan.setTarget(-1); // -1: 撤退中の特別なターゲットID
					//Unit iugiu = minElement->second;
					plan.setLastKnownEnemyPos(minElement->second.GetNowPosiCenter());
				}
				else
				{
					plan.setTarget(minElement->second.ID);
				}
				unit.moveState = moveState::None;
				plan.setPath(listRoot);
				{
					std::scoped_lock lock(aiRootMutex);
					aiRoot[unit.ID] = plan;
					// 経路セット時に1個除去しておく
					if (aiRoot[unit.ID].getPath().size() > 1)
						aiRoot[unit.ID].stepToNext();
				}
			}

			processed++;
			if (processed >= MaxUnitsPerFrame)
				break;
		}

		unitIndexEnemy = (unitIndexEnemy + processed) % total;
		return static_cast<int32>(processed);
	}

	int32 BattleMoveAStarMyUnitsKai(Array<ClassHorizontalUnit>& target,
							Array<ClassHorizontalUnit>& enemy,
							Array<Array<MapDetail>> mapData,
							HashTable<int64, ClassUnitMovePlan>& aiRoot,
							const std::atomic<bool>& abort,
							const std::atomic<bool>& pause,
							HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
							MapTile& mapTile
	)
	{
		const auto targetSnapshot = target;
		HashTable<int32, Unit*> htUnit;
		// フラット化して高速アクセスに備える
		Array<Unit*> flatList;
		std::scoped_lock lock(arrayMutex);
		for (auto& group : target)
		{
			for (auto& unit : group.ListClassUnit)
			{
				if (!unit.IsBattleEnable || unit.IsBuilding) continue;
				if (unit.moveState == moveState::MoveAI)
				{
					flatList.push_back(&unit);
					htUnit.emplace(unit.ID, &unit);
				}
			}
		}

		if (flatList.size() == 0) return 0;
		if (abort) return 0;

		// 共通経路(始まり合流地点→終わり合流地点)を算出
		s3d::Optional<Size> startIndex = mapTile.ToIndex(flatList[0]->getFirstMergePos(), mapTile.columnQuads, mapTile.rowQuads);
		if (startIndex.has_value() == false) return 0;
		s3d::Optional<Size> endIndex = mapTile.ToIndex(flatList[0]->getLastMergePos(), mapTile.columnQuads, mapTile.rowQuads);
		if (endIndex.has_value() == false) return 0;

		if (startIndex == endIndex)
		{
			//単一ユニットがこのケース
			s3d::Optional<Size> startIndex = mapTile.ToIndex(flatList[0]->GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (startIndex.has_value() == false) return 0;
			s3d::Optional<Size> endIndex = mapTile.ToIndex(flatList[0]->GetOrderPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (endIndex.has_value() == false) return 0;
			ClassAStarManager classAStarManager(endIndex.value().x, endIndex.value().y);
			Optional<ClassAStar*> startAstar
				= classAStarManager.OpenOne(startIndex.value().x, startIndex.value().y, 0, nullptr, mapTile.N);
			Array<Point> fullPath;

			////移動経路取得
			while (true)
			{
				if (abort == true) break;

				if (startAstar.has_value() == false)
				{
					fullPath.clear();
					break;
				}

				classAStarManager.OpenAround(startAstar.value(),
												mapData,
												enemy,
												target,
												mapTile.N,
												hsBuildingUnitForAstar
				);
				startAstar.value()->SetAStarStatus(AStarStatus::Closed);
				classAStarManager.RemoveClassAStar(startAstar.value());
				if (classAStarManager.GetListClassAStar().size() != 0)
				{
					startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
				}

				if (startAstar.has_value() == false)
					continue;

				//敵まで到達したか
				if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
				{
					startAstar.value()->GetRoot(fullPath);
					fullPath.reverse();
					break;
				}
			}
			if (fullPath.size() != 0)
			{
				ClassUnitMovePlan plan;
				plan.setPath(fullPath);
				{
					std::scoped_lock lock(aiRootMutex);
					aiRoot[flatList[0]->ID] = plan;
				}
				//debugRoot.push_back(fullPath);
				htUnit[flatList[0]->ID]->moveState = moveState::Moving;
			}
			return 0;
		}

		ClassAStarManager astarToGoal(endIndex.value().x, endIndex.value().y);
		Optional<ClassAStar*> startAstar2 = astarToGoal.OpenOne(startIndex.value().x, startIndex.value().y, 0, nullptr, mapTile.N);
		Array<Point> pathShare;
		Stopwatch taskTimer;
		taskTimer.restart();

		while (true) {
			//if (taskTimer.s() > 3) break;
			if (abort) break;
			if (!startAstar2.has_value()) { pathShare.clear(); break; }
			astarToGoal.OpenAround(startAstar2.value(), mapData, enemy, target, mapTile.N, hsBuildingUnitForAstar, true);
			startAstar2.value()->SetAStarStatus(AStarStatus::Closed);
			astarToGoal.RemoveClassAStar(startAstar2.value());
			if (astarToGoal.GetListClassAStar().size() != 0)
				startAstar2 = SearchMinScore(astarToGoal.GetListClassAStar());
			if (!startAstar2.has_value()) continue;
			//敵まで到達したか、あるいはその時点で可能性のある道を行く
			if ((startAstar2.value()->GetRow() == astarToGoal.GetEndX()
				&& startAstar2.value()->GetCol() == astarToGoal.GetEndY())
				|| taskTimer.s() > 3)
			{
				startAstar2.value()->GetRoot(pathShare);
				pathShare.reverse();
				astarToGoal.Clear();
				break;
			}
		}

		for (Unit* unit : flatList)
		{
			Array<Point> firstPath;
			{
				s3d::Optional<Size> nowIndexFirstGoal = mapTile.ToIndex(unit->getFirstMergePos(), mapTile.columnQuads, mapTile.rowQuads);
				if (nowIndexFirstGoal.has_value() == false) continue;
				s3d::Optional<Size> nowIndex = mapTile.ToIndex(unit->GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
				if (nowIndex.has_value() == false) continue;
				if (nowIndexFirstGoal != nowIndex)
				{
					ClassAStarManager classAStarManager(nowIndexFirstGoal.value().x, nowIndexFirstGoal.value().y);
					Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, mapTile.N);
					while (true)
					{
						if (abort == true) break;

						if (startAstar.has_value() == false)
						{
							firstPath.clear();
							break;
						}

						classAStarManager.OpenAround(startAstar.value(),
														mapData,
														enemy,
														target,
														mapTile.N, hsBuildingUnitForAstar
						);
						startAstar.value()->SetAStarStatus(AStarStatus::Closed);
						classAStarManager.RemoveClassAStar(startAstar.value());
						if (classAStarManager.GetListClassAStar().size() != 0)
						{
							startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
						}

						if (startAstar.has_value() == false)
							continue;

						//敵まで到達したか
						if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
						{
							startAstar.value()->GetRoot(firstPath);
							firstPath.reverse();
							break;
						}
					}
				}
			}

			Array<Point> endPath;
			{
				s3d::Optional<Size> nowIndexEndGoal = mapTile.ToIndex(unit->GetOrderPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
				if (nowIndexEndGoal.has_value() == false) continue;
				s3d::Optional<Size> nowIndex = mapTile.ToIndex(unit->getLastMergePos(), mapTile.columnQuads, mapTile.rowQuads);
				if (nowIndex.has_value() == false) continue;
				if (nowIndexEndGoal != nowIndex)
				{
					ClassAStarManager classAStarManager(nowIndexEndGoal.value().x, nowIndexEndGoal.value().y);
					Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, mapTile.N);
					while (true)
					{
						if (abort == true) break;

						if (startAstar.has_value() == false)
						{
							endPath.clear();
							break;
						}

						classAStarManager.OpenAround(startAstar.value(),
														mapData,
														enemy,
														target,
														mapTile.N, hsBuildingUnitForAstar
						);
						startAstar.value()->SetAStarStatus(AStarStatus::Closed);
						classAStarManager.RemoveClassAStar(startAstar.value());
						if (classAStarManager.GetListClassAStar().size() != 0)
						{
							startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
						}

						if (startAstar.has_value() == false)
							continue;

						//敵まで到達したか
						if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
						{
							startAstar.value()->GetRoot(endPath);
							endPath.reverse();
							break;
						}
					}
				}
			}

			auto guifd = firstPath.append(pathShare).append(endPath);
			//auto guifd = pathShare;

			if (guifd.size() != 0)
			{
				ClassUnitMovePlan plan;
				guifd.pop_front(); // 最初の位置は現在地なので削除
				plan.setPath(guifd);
				{
					std::scoped_lock lock(aiRootMutex);
					aiRoot[unit->ID] = plan;
				}
				htUnit[unit->ID]->moveState = moveState::Moving;
			}

		}
	}
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
	void initUI();
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
	void handleUnitAndBuildingSelection();
	void handleSkillUISelection();
	void updateUnitHealthBars();
	void updateUnitMovements();
	void startAsyncFogCalculation();
	void calculateFogFromUnits(Grid<Visibility>& visMap, const Array<Unit>& units);
	ClassMapBattle GetClassMapBattle(ClassMap cm, CommonConfig& commonConfig);

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
	//static constexpr double CLICK_THRESHOLD = 5.0;  // クリックと判定する最大移動距離
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
	ClassHorizontalUnit getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf);
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
	void handleUnitTooltip();
	void processBuildOnTiles(const Array<Point>& tiles);
	void processBuildOnTilesWithMovement(const Array<Point>& tiles);
	void handleBuildTargetSelection();

	struct UnitTooltip
	{
		bool isVisible = false;
		Vec2 position;
		String content;
		RenderTexture renderTexture;
		Stopwatch fadeTimer;
		String lastRenderedContent;
		stOfFont fontInfo;
		void show(const Vec2& pos, const String& text)
		{
			// 同じ内容で既に表示中の場合は位置のみ更新
			if (isVisible && content == text)
			{
				position = pos;
				return; // タイマーをリセットしない
			}

			position = pos;
			content = text;
			isVisible = true;
			fadeTimer.restart();
			updateRenderTexture();
		}

		void hide()
		{
			isVisible = false;
			// 非表示時に前回の内容をクリアして、次回必ず更新されるようにする
			lastRenderedContent.clear();
		}

		void updateRenderTexture();
		void draw() const;
	};

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

	void SkillProcess(Array<ClassHorizontalUnit>& ach, Array<ClassHorizontalUnit>& achTarget, Array<ClassExecuteSkills>& aces);
	bool SkillProcess002(Array<ClassHorizontalUnit>& aaatarget, Vec2 xA, Unit& itemUnit, Skill& itemSkill, Array<ClassExecuteSkills>& aces);
	void ColliderCheck(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, Array<ClassHorizontalUnit>& chu);
	void ColliderCheckHeal(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, Unit* itemTarget);
	void CalucDamage(Unit& itemTarget, double strTemp, ClassExecuteSkills& ces);
	bool ProcessCollid(bool& bombCheck, Array<int32>& arrayNo, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Unit& itemTarget);

};

