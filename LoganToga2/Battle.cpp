#pragma once
#include "Battle.hpp"
#include "ClassBattle.h"
# include "GameUIToolkit.h"
#include <ranges>
#include <vector>
#include <iostream>
#include "ClassUnit.h"

/// @brief 指定した座標にあるタイルのインデックスを返します。
/// @param pos 座標
/// @param columnQuads 各列のタイルによって構成される四角形の配列
/// @param rowQuads 各行のタイルによって構成される四角形の配列
/// @return タイルのインデックス。指定した座標にタイルが無い場合は none
Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads)
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

std::mutex aiRootMutex;
std::mutex arrayMutex;

void bhjdsvjhbsd(const std::exception& ex)
{
	std::string msg = ex.what();
	Print << U"例外長: {}"_fmt(msg.size());
	Print << Unicode::Widen(msg);
	Logger << U"例外: " << Unicode::Widen(ex.what());
}

/// @brief アスターアルゴリズムで移動経路取得
/// @param target 
/// @param enemy 
/// @param mapData 
/// @param aiRoot 
/// @param debugRoot 
/// @param list 
/// @param columnQuads 
/// @param rowQuads 
/// @param N 
/// @param abort 
/// @param pause 
/// @return 
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
)
{
	static size_t unitIndexEnemy = 0;
	constexpr size_t MaxUnitsPerFrame = 5;

	size_t processed = 0;
	Array<Unit*> flatList;

	// --- フラット化：FlagMoveAI が立っている敵ユニットのみ抽出
	try
	{
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
	}
	catch (const std::exception& e)
	{
		bhjdsvjhbsd(e);
	}

	const size_t total = flatList.size();
	if (total == 0)
		return 0;

	const auto isValidTarget = [](const Unit& unit) -> bool
		{
			//これどうなん？　GATEは壊せるけど無視ということか？
			if (unit.IsBuilding && (unit.mapTipObjectType == MapTipObjectType::WALL2 || unit.mapTipObjectType == MapTipObjectType::GATE))
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

		Optional<Size> nowIndex = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads);
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
		try
		{
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
		}
		catch (const std::exception& e)
		{
			bhjdsvjhbsd(e);
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
			? ToIndex(retreatTargetPos, columnQuads, rowQuads)
			: ToIndex(minElement->second.GetNowPosiCenter(), columnQuads, rowQuads);
		if (nowIndexEnemy.has_value() == false) continue;
		if (nowIndexEnemy.value() == nowIndex.value()) continue;

		////現在地を開く
		ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);
		Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, N);
		Array<Point> listRoot;
		MicrosecClock mc;
		Stopwatch taskTimer;
		taskTimer.restart();
		////移動経路取得
		while (true)
		{
			try
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
												N,
												hsBuildingUnitForAstar
				);
				Print << U"BattleMoveAStar:A*探索ノード数: {0}:{1}"_fmt(classAStarManager.GetPool().size(), mc.us());
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
			catch (const std::exception& e)
			{
				bhjdsvjhbsd(e);
			}
		}

		// 経路が取得できた場合、aiRootにセット
		if (listRoot.size() != 0)
		{
			UnitMovePlan plan;
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
						HashTable<int64, UnitMovePlan>& aiRoot,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
						Array<Quad>& columnQuads,
						Array<Quad>& rowQuads,
						const int32 N,
						const std::atomic<bool>& abort,
						const std::atomic<bool>& pause,
						HashTable<Point, const Unit*>& hsBuildingUnitForAstar
)
{
	const auto targetSnapshot = target;
	HashTable<int32, Unit*> htUnit;
	// フラット化して高速アクセスに備える
	Array<Unit*> flatList;
	try
	{
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
	}
	catch (const std::exception& e)
	{
		bhjdsvjhbsd(e);
	}

	if (flatList.size() == 0) return 0;
	if (abort) return 0;

	// 共通経路(始まり合流地点→終わり合流地点)を算出
	s3d::Optional<Size> startIndex = ToIndex(flatList[0]->getFirstMergePos(), columnQuads, rowQuads);
	if (startIndex.has_value() == false) return 0;
	s3d::Optional<Size> endIndex = ToIndex(flatList[0]->getLastMergePos(), columnQuads, rowQuads);
	if (endIndex.has_value() == false) return 0;

	if (startIndex == endIndex)
	{
		//単一ユニットがこのケース
		s3d::Optional<Size> startIndex = ToIndex(flatList[0]->GetNowPosiCenter(), columnQuads, rowQuads);
		if (startIndex.has_value() == false) return 0;
		s3d::Optional<Size> endIndex = ToIndex(flatList[0]->GetOrderPosiCenter(), columnQuads, rowQuads);
		if (endIndex.has_value() == false) return 0;
		ClassAStarManager classAStarManager(endIndex.value().x, endIndex.value().y);
		Optional<ClassAStar*> startAstar
			= classAStarManager.OpenOne(startIndex.value().x, startIndex.value().y, 0, nullptr, N);
		Array<Point> fullPath;

		////移動経路取得
		while (true)
		{
			try
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
												N,
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
			catch (const std::exception& e)
			{
				bhjdsvjhbsd(e);
			}
		}
		if (fullPath.size() != 0)
		{
			UnitMovePlan plan;
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
	Optional<ClassAStar*> startAstar2 = astarToGoal.OpenOne(startIndex.value().x, startIndex.value().y, 0, nullptr, N);
	Array<Point> pathShare;
	Stopwatch taskTimer;
	taskTimer.restart();

	while (true) {
		//if (taskTimer.s() > 3) break;
		if (abort) break;
		if (!startAstar2.has_value()) { pathShare.clear(); break; }
		astarToGoal.OpenAround(startAstar2.value(), mapData, enemy, target, N, hsBuildingUnitForAstar, true);
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
			s3d::Optional<Size> nowIndexFirstGoal = ToIndex(unit->getFirstMergePos(), columnQuads, rowQuads);
			if (nowIndexFirstGoal.has_value() == false) continue;
			s3d::Optional<Size> nowIndex = ToIndex(unit->GetNowPosiCenter(), columnQuads, rowQuads);
			if (nowIndex.has_value() == false) continue;
			if (nowIndexFirstGoal != nowIndex)
			{
				ClassAStarManager classAStarManager(nowIndexFirstGoal.value().x, nowIndexFirstGoal.value().y);
				Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, N);
				while (true)
				{
					try
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
														N, hsBuildingUnitForAstar
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
					catch (const std::exception& e)
					{
						bhjdsvjhbsd(e);
					}
				}
			}
		}

		Array<Point> endPath;
		{
			s3d::Optional<Size> nowIndexEndGoal = ToIndex(unit->GetOrderPosiCenter(), columnQuads, rowQuads);
			if (nowIndexEndGoal.has_value() == false) continue;
			s3d::Optional<Size> nowIndex = ToIndex(unit->getLastMergePos(), columnQuads, rowQuads);
			if (nowIndex.has_value() == false) continue;
			if (nowIndexEndGoal != nowIndex)
			{
				ClassAStarManager classAStarManager(nowIndexEndGoal.value().x, nowIndexEndGoal.value().y);
				Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, N);
				while (true)
				{
					try
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
														N, hsBuildingUnitForAstar
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
					catch (const std::exception& e)
					{
						bhjdsvjhbsd(e);
					}
				}
			}
		}

		auto guifd = firstPath.append(pathShare).append(endPath);
		//auto guifd = pathShare;

		if (guifd.size() != 0)
		{
			UnitMovePlan plan;
			guifd.pop_front(); // 最初の位置は現在地なので削除
			plan.setPath(guifd);
			{
				std::scoped_lock lock(aiRootMutex);
				aiRoot[unit->ID] = plan;
			}
			debugRoot.push_back(guifd);
			htUnit[unit->ID]->moveState = moveState::Moving;
		}

	}
}

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
)
{
	const auto targetSnapshot = target;

	//経路共有方式は相いれないので後で消す
	static size_t unitIndex = 0; // ラウンドロビンインデックス
	constexpr size_t MaxUnitsPerFrame = 5; // 1フレームで処理するユニット数

	size_t processed = 0;
	Array<Unit*> flatList;

	// --- フラット化して高速アクセスに備える
	try
	{
		for (auto& group : target)
		{
			for (auto& unit : group.ListClassUnit)
			{
				if (!unit.IsBattleEnable || unit.IsBuilding) continue;
				if (unit.moveState == moveState::MoveAI)
					flatList.push_back(&unit);
			}
		}
	}
	catch (const std::exception& e)
	{
		bhjdsvjhbsd(e);
	}

	const size_t total = flatList.size();
	if (total == 0) return 0;

	for (size_t i = 0; i < total; ++i)
	{
		if (abort) break;

		size_t idx = (unitIndex + i) % total;
		Unit& unit = *flatList[idx];

		//指定のマップチップを取得
		s3d::Optional<Size> nowIndexEnemy = ToIndex(unit.orderPosiLeft, columnQuads, rowQuads);
		if (nowIndexEnemy.has_value() == false)
			continue;

		//現在のマップチップを取得
		s3d::Optional<Size> nowIndex = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads);
		if (nowIndex.has_value() == false)
			continue;

		Array<Point> fullPath;
		////現在地を開く
		ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);

		Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, N);

		////移動経路取得
		while (true)
		{
			try
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
												N, hsBuildingUnitForAstar
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
			catch (const std::exception& e)
			{
				bhjdsvjhbsd(e);
			}
		}
		if (fullPath.size() != 0)
		{
			UnitMovePlan plan;
			fullPath.pop_front(); // 最初の位置は現在地なので削除
			plan.setPath(fullPath);
			{
				std::scoped_lock lock(aiRootMutex);
				aiRoot[unit.ID] = plan;
			}
			debugRoot.push_back(fullPath);
			unit.moveState = moveState::Moving;
		}
		processed++;
		if (processed >= MaxUnitsPerFrame)
			break;
	}
	unitIndex = (unitIndex + processed) % total;
	return static_cast<int32>(processed);
}

static BuildAction& dummyMenu()
{
	static BuildAction instance;
	return instance;
}

void GetTempResource(ClassMapBattle& cmb)
{
	cmb.mapData[0][10].isResourcePoint = true;
	cmb.mapData[0][10].resourcePointType = resourceKind::Gold;
	cmb.mapData[0][10].resourcePointAmount = 11;
	cmb.mapData[0][10].resourcePointDisplayName = U"金";
	cmb.mapData[0][10].resourcePointIcon = U"point000.png";
}

Vec2 ToIso(double x, double y)
{
	return Vec2((x - y), (x + y) / 2.0);
}
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
void Battle::DrawMiniMap(const Grid<int32>& map, const RectF& cameraRect) const
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
	const Vec2 isoB = ToIso(map.width(), 0) * tileSize;
	const Vec2 isoC = ToIso(0, map.height()) * tileSize;
	const Vec2 isoD = ToIso(map.width(), map.height()) * tileSize;

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
	for (int32 y = 0; y < map.height(); ++y)
	{
		for (int32 x = 0; x < map.width(); ++x)
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
		//const Vec2 camTopLeftIso = ToIso(cameraRect.x, cameraRect.y) * tileSize;
		//const Vec2 camBottomRightIso = ToIso(cameraRect.x + cameraRect.w, cameraRect.y + cameraRect.h) * tileSize;

		//const Vec2 topLeft = camTopLeftIso * scale + offset;
		//const Vec2 bottomRight = camBottomRightIso * scale + offset;
		//const RectF cameraBox = RectF(topLeft, bottomRight - topLeft);

		//cameraBox.drawFrame(1.5, ColorF(1.0));

		const Vec2 camTopLeft = cameraRect.pos * Vec2(1.0, 1.0);
		const Vec2 camBottomRight = camTopLeft + cameraRect.size;
		RectF(camTopLeft * scale + offset, cameraRect.size * scale).drawFrame(1.5, ColorF(1.0));

	}
}
void Battle::SetResourceTargets(Array<ResourcePointTooltip::TooltipTarget>& resourceTargets)
{
	{
		const auto& mapData = classBattle.classMapBattle.value().mapData;

		const int32 mapSize = static_cast<int32>(mapData.size());

		for (int32 x = 0; x < mapSize; ++x)
		{
			for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
			{
				const auto& tile = mapData[x][y];

				if (!tile.isResourcePoint)
					continue;

				const Vec2 pos = ToTileBottomCenter(Point(x, y), N);

				const Circle area = Circle(pos.movedBy(0, -TileThickness - TileOffset.y), TextureAsset(tile.resourcePointIcon).region().w / 2);
				String desc = U"資源ポイント\n所有者: {}\n種類:{}\n量:{}"_fmt(
							(tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie) ? U"味方" : U"敵", U"金", U"5/s");

				resourceTargets << ResourcePointTooltip::TooltipTarget{ area, desc };
			}
		}
	}
}

Battle::Battle(GameData& saveData, CommonConfig& commonConfig, SystemString argSS)
	:FsScene(U"Battle"), m_saveData{ saveData }, m_commonConfig{ commonConfig }, N{ 64 }
	, ss{ argSS }
	, visibilityMap{ Grid<Visibility>(Size{ N, N }, Visibility::Unseen) }
	, grid{ Grid<int32>(Size{ N, N }) }
	, tempSelectComRight{ dummyMenu() }
{
	const TOMLReader tomlMap{ PATHBASE + PATH_DEFAULT_GAME + U"/016_BattleMap/map001.toml" };
	if (not tomlMap) // もし読み込みに失敗したら
		throw Error{ U"Failed to load `tomlMap`" };

	ClassMap sM;
	for (const auto& table : tomlMap[U"Map"].tableArrayView()) {
		const String name = table[U"name"].get<String>();

		{
			int32 counter = 0;
			while (true)
			{
				String aaa = U"ele{}"_fmt(counter);
				const String ele = table[aaa].get<String>();
				sM.ele.emplace(aaa, ele);
				counter++;
				if (ele == U"")
				{
					break;
				}
			}
		}
		{
			namespace views = std::views;
			const String str = table[U"data"].get<String>();
			for (const auto sv : str | views::split(U"$"_sv))
			{
				String re = ClassStaticCommonMethod::ReplaceNewLine(String(sv.begin(), sv.end()));
				if (re != U"")
				{
					sM.data.push_back(ClassStaticCommonMethod::ReplaceNewLine(re));
				}
			}
		}
	}

	classBattle.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);
	GetTempResource(classBattle.classMapBattle.value());
	Array<ResourcePointTooltip::TooltipTarget> resourceTargets;
	SetResourceTargets(resourceTargets);
	resourcePointTooltip.setTargets(resourceTargets);
	resourcePointTooltip.setTooltipEnabled(true);

	N = classBattle.classMapBattle.value().mapData.size();
	grid = Grid<int32>(Size{ N, N });
	Vec2 TileOffset{ 50, 25 };
	int32 TileThickness = 15;
	Array<Quad> columnQuads = MakeColumnQuads(N);
	Array<Quad> rowQuads = MakeRowQuads(N);
}
Battle::~Battle()
{
	abort = true;
	abortMyUnits = true;
	if (task.isValid())
		task.wait(); // 完全に終了を待つ
	if (taskMyUnits.isValid())
		taskMyUnits.wait(); // 完全に終了を待つ
}

std::unique_ptr<TextureAssetData> MakeTextureAssetData1(const FilePath& path, const TextureDesc textureDesc)
{
	// 空のテクスチャアセットデータを作成する
	std::unique_ptr<TextureAssetData> assetData = std::make_unique<TextureAssetData>();

	// ファイルパスを代入する
	assetData->path = path;

	// テクスチャの設定を代入する
	assetData->desc = textureDesc;

	// ロード時の仕事を設定する
	assetData->onLoad = [](TextureAssetData& asset, const String&)
		{
			Image image{ asset.path };
			Color* p = image.data();
			const Color* const pEnd = (p + image.num_pixels());
			while (p != pEnd)
			{
				p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
				p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
				p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
				++p;
			}
			asset.texture = Texture{ image };
			return static_cast<bool>(asset.texture);
		};

	return assetData;
}

void Battle::renB()
{
	htBuildMenuRenderTexture.clear();
	String key = U"";
	int32 counter = -1;
	Array<std::pair<String, BuildAction>> sortedArray(htBuildMenu.begin(), htBuildMenu.end());
	sortedArray.sort_by([](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	for (auto& ttt : sortedArray)
	{
		String uibg = ttt.first.split('-')[0];
		if (uibg != key)
		{
			key = uibg;
			RenderTexture rt = RenderTexture{ 328, 328 };
			{
				const ScopedRenderTarget2D target{ rt.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
				const ScopedRenderStates2D blend{ MakeBlendState() };
				Rect(328, 328).drawFrame(4, 0, ColorF{ 0.5 });
			}
			htBuildMenuRenderTexture.emplace(key.split('-')[0], rt);
			counter = 0;
		}

		//-1は無制限の為
		if (ttt.second.buildCount == 0) continue;

		htBuildMenuRenderTexture[key].clear(ColorF{ 0.5, 0.0 });
		const ScopedRenderTarget2D target{ htBuildMenuRenderTexture[key].clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		Rect rectBuildMenuHome;
		rectBuildMenuHome.x = ((counter % 6) * 64) + 4;
		rectBuildMenuHome.y = ((counter / 6) * 64) + 4;
		rectBuildMenuHome.w = 64;
		rectBuildMenuHome.h = 64;
		ttt.second.rectHantei = rectBuildMenuHome;
		TextureAsset(ttt.second.icon)
			.scaled(rectBuildMenuHome.w / rectBuildMenuHome.h)
			.draw(ttt.second.rectHantei.x, ttt.second.rectHantei.y);
		counter++;
	}

	sortedArrayBuildMenu = sortedArray;
}

/// @brief 
/// @param unitName 
/// @param col 
/// @param row 産出するユニット数
/// @param num 
/// @param listU 
/// @param enemy 
void Battle::UnitRegister(String unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& listU, bool enemy)
{
	// 事前に容量を確保して配列の再配置を防ぐ
	const size_t expectedSize = listU.size() + 1;
	if (listU.capacity() < expectedSize + 10) // 余裕をもって容量確保
	{
		listU.reserve(expectedSize + 50);
		Print << U"UnitRegister: 配列容量を拡張しました ({} -> {})"_fmt(
			listU.capacity(), expectedSize + 50);
	}

	//新しいコピーを作る
	for (auto uu : m_commonConfig.arrayUnit)
	{
		if (uu.NameTag == unitName)
		{
			uu.ID = classBattle.getIDCount();
			uu.initTilePos = Point{ col,row };
			uu.nowPosiLeft =
				ToTile(uu.initTilePos, N)
				.asPolygon()
				.centroid()
				.movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
			uu.taskTimer = Stopwatch();
			uu.taskTimer.reset();
			Vec2 temp = uu.GetNowPosiCenter()
				.movedBy(-64 / 2, (32 / 2) + 8);
			uu.bLiquidBarBattle =
				GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 8));

			if (enemy)
				uu.moveState = moveState::MoveAI;

			ClassHorizontalUnit cuu;

			for (size_t i = 0; i < num; i++)
			{
				uu.ID = classBattle.getIDCount();
				auto copy = uu;
				cuu.ListClassUnit.push_back(copy);
				if (uu.IsBuilding)
				{
					unitsForHsBuildingUnitForAstar.push_back(std::make_unique<Unit>(uu));
					hsBuildingUnitForAstar[uu.initTilePos] = unitsForHsBuildingUnitForAstar.back().get(); // これなら再配置でもアドレスは変わらない
				}
			}

			listU.push_back(cuu);
		}
	}
}

Co::Task<void> Battle::start()
{
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), MakeTextureAssetData1(filePath, TextureDesc::Mipped));
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/043_ChipImageBuild/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/041_ChipImageSkill/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);

	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleCommand.push_back(false);
	arrayBattleCommand.push_back(false);

	battleStatus = BattleStatus::Battle;

	rectZinkei.push_back(Rect{ 8,8,60,40 });
	rectZinkei.push_back(Rect{ 76,8,60,40 });
	rectZinkei.push_back(Rect{ 144,8,60,40 });
	renderTextureZinkei = RenderTexture{ 320,60 };
	renderTextureZinkei.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureZinkei.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		Rect df = Rect(320, 60);
		df.drawFrame(4, 0, ColorF{ 0.5 });

		for (auto&& [i, ttt] : Indexed(rectZinkei))
		{
			ttt.draw(Palette::Aliceblue);
			fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
		}
	}

	renderTextureBuildMenuEmpty = RenderTexture{ 328, 328 };
	renderTextureBuildMenuEmpty.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureBuildMenuHome.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };
		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}

	//建築メニューの初期化
	{
		htBuildMenu.clear();
		for (const auto uigee : m_commonConfig.htBuildMenuBaseData)
		{
			for (const auto fege : uigee.second)
			{
				String key = uigee.first + U"-" + fege.id;
				htBuildMenu.emplace(key, fege);
			}
		}
		renB();
	}

	//初期ユニット
	{
		for (auto uu : m_commonConfig.arrayUnit)
		{
			if (uu.Name == U"M14 Infantry Rifle")
			{
				uu.ID = classBattle.getIDCount();
				uu.ImageName = U"chip006.png";
				uu.initTilePos = Point{ 10, 10 };
				uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				classBattle.listOfAllUnit.push_back(cuu);
			}
		}
	}

	renderTextureSkill = RenderTexture{ 320,320 };
	renderTextureSkill.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		//skill抽出
		Array<Skill> table;
		for (auto& item : classBattle.listOfAllUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				for (auto& ski : itemUnit.arrSkill)
					table.push_back(ski);
			}
		}

		// ソート
		table.sort_by([](const Skill& a, const Skill& b)
		{
			return a.sortKey < b.sortKey;
		});
		table.erase(std::unique(table.begin(), table.end()), table.end());

		for (const auto&& [i, key] : Indexed(table))
		{
			Rect rectSkill;
			rectSkill.x = ((i % 10) * 32) + 4;
			rectSkill.y = ((i / 10) * 32) + 4;
			rectSkill.w = 32;
			rectSkill.h = 32;
			htSkill.emplace(key.nameTag, rectSkill);

			for (auto& icons : key.icon.reversed())
			{
				TextureAsset(icons.trimmed()).resized(32).draw(rectSkill.x, rectSkill.y);
			}
		}

		Rect df = Rect(320, 320);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}
	renderTextureSkillUP = RenderTexture{ 320,320 };
	renderTextureSkillUP.clear(ColorF{ 0.5, 0.0 });

	//始点設定
	viewPos = ToTileBottomCenter(classBattle.listOfAllUnit[0].ListClassUnit[0].initTilePos, N);
	camera.jumpTo(viewPos, camera.getTargetScale());
	resourcePointTooltip.setCamera(camera);

	//ユニット体力バーの設定
	for (auto& item : classBattle.listOfAllUnit)
	{
		if (!item.FlagBuilding &&
			!item.ListClassUnit.empty())
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				Vec2 temp = itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
				itemUnit.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));
			}
		}
	}
	for (auto& item : classBattle.listOfAllEnemyUnit)
	{
		if (!item.FlagBuilding &&
			!item.ListClassUnit.empty())
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				Vec2 temp = itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
				itemUnit.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));
			}
		}
	}

	//建物関係
	ClassHorizontalUnit chuSor;
	ClassHorizontalUnit chuDef;
	ClassHorizontalUnit chuNa;
	chuSor.FlagBuilding = true;
	chuDef.FlagBuilding = true;
	chuNa.FlagBuilding = true;

	{
		for (auto uu : m_commonConfig.arrayUnit)
		{
			if (uu.Name == U"M14 Infantry Rifle")
			{
				uu.ID = classBattle.getIDCount();
				uu.IsBuilding = true;
				uu.mapTipObjectType = MapTipObjectType::HOME;
				uu.NoWall2 = 0;
				uu.HPCastle = 1000;
				uu.CastleDefense = 1000;
				uu.CastleMagdef = 1000;
				uu.ImageName = U"home1.png";
				uu.rowBuilding = N / 2;
				uu.colBuilding = N / 2;
				uu.initTilePos = Point{ uu.rowBuilding, uu.colBuilding };
				uu.Move = 0.0;
				uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				chuSor.ListClassUnit.push_back(uu);
			}
		}
	}

	classBattle.listOfAllUnit.push_back(chuSor);
	//classBattle.listOfAllEnemyUnit.push_back(chuDef);

	//ミニマップ用
	for (int32 y = 0; y < grid.height(); ++y)
	{
		for (int32 x = 0; x < grid.width(); ++x)
		{
			String ttt = classBattle.classMapBattle.value().mapData[x][y].tip + U".png";
			minimapCols.emplace(Point(x, y), GetDominantColor(ttt, colData));
		}
	}

	stopwatchFinance.restart();
	stopwatchGameTime.restart();

	task = Async([this]() {
		while (!abort)
		{
			if (!pauseTask)
			{
				HashTable<Point, const Unit*> hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
				BattleMoveAStar(
					classBattle.listOfAllUnit,
					classBattle.listOfAllEnemyUnit,
					classBattle.classMapBattle.value().mapData,
					aiRootEnemy,
					debugRoot,
					debugAstar,
					columnQuads,
					rowQuads,
					N,
					abort,
					pauseTask, changeUnitMember, hsBuildingUnitForAstarSnapshot);
			}
			System::Sleep(1);
		}
	});
	//task = Async(BattleMoveAStar,
	//	std::ref(classBattle.listOfAllEnemyUnit),
	//	std::ref(classBattle.listOfAllUnit),
	//	std::ref(classBattle.classMapBattle.value().mapData),
	//	std::ref(aiRootEnemy),
	//	std::ref(debugRoot), std::ref(debugAstar),
	//	std::ref(columnQuads),
	//	std::ref(rowQuads),
	//	N,
	//	std::ref(abort), std::ref(pauseTask), std::ref(changeUnitMember));  

	// 経路探索スレッド起動
	taskMyUnits = Async([this]() {
		while (!abortMyUnits)
		{
			if (!pauseTaskMyUnits)
			{
				HashTable<Point, const Unit*> hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
				BattleMoveAStarMyUnitsKai(
					classBattle.listOfAllUnit,
					classBattle.listOfAllEnemyUnit,
					classBattle.classMapBattle.value().mapData,
					aiRootMy,
					debugRoot,
					debugAstar,
					columnQuads,
					rowQuads,
					N,
					abortMyUnits,
					pauseTaskMyUnits, hsBuildingUnitForAstarSnapshot);
			}
			System::Sleep(1); // CPU過負荷防止
		}
	});

	co_await mainLoop().pausedWhile([&]
		{
			if (KeySpace.pressed())
			{
				//TODO 消したがどうする
				//stopwatch.pause();
				//stopwatch001.pause();
				//stopwatch002.pause();
				//stopwatch003.pause();
				Rect rectPauseBack{ 0, 0, Scene::Width(), Scene::Height() };
				rectPauseBack.draw(ColorF{ 0.0, 0.0, 0.0, 0.5 });
				const String pauseText = U"Pause";

				Rect rectPause{ int32(Scene::Width() / 2 - systemFont(pauseText).region().w / 2),
								0,
								int32(systemFont(pauseText).region().w),
								int32(systemFont(pauseText).region().h) };
				rectPause.draw(Palette::Black);
				systemFont(pauseText).drawAt(rectPause.x + rectPause.w / 2, rectPause.y + rectPause.h / 2, Palette::White);

				return true;
			}
			else
			{
				//TODO 消したがどうする
				//stopwatch.resume();
				//stopwatch001.resume();
				//stopwatch002.resume();
				//stopwatch003.resume();
			}
		});; // メインループ実行
}

void Battle::playResourceEffect()
{
	//CircleEffect(Vec2{ 500, 500 }, 50, ColorF{ 1.0, 1.0, 0.2 });
	//SoundAsset(U"resourceGet").playMulti();
}
void Battle::updateResourceIncome()
{
	//stopwatchFinanceが一秒経過する度に処理を行う
	if (stopwatchFinance.sF() >= 1.0)
	{
		int32 goldInc = 0;
		int32 trustInc = 0;
		int32 foodInc = 0;

		for (auto ttt : classBattle.classMapBattle.value().mapData)
		{
			for (auto jjj : ttt)
			{
				if (jjj.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				{
					//資金の増加
					switch (jjj.resourcePointType)
					{
					case resourceKind::Gold:
						goldInc += jjj.resourcePointAmount;
						break;
					case resourceKind::Trust:
						trustInc += jjj.resourcePointAmount;
						break;
					case resourceKind::Food:
						foodInc += jjj.resourcePointAmount;
						break;
					default:
						break;
					}
				}
			}
		}

		stopwatchFinance.restart();
		gold += 10 + goldInc; // 1秒ごとに10ゴールド増加
		trust += 1 + trustInc; // 1秒ごとに1権勢増加
		food += 5 + foodInc; // 1秒ごとに5食料増加

		// 資源取得エフェクトやサウンド再生（省略可）
		playResourceEffect();
	}
}
void Battle::refreshFogOfWar()
{
	//毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
	// 一度見たタイルは UnseenではなくSeenにしたい
	for (auto&& ttt : visibilityMap)
		ttt = Visibility::Unseen;

	for (auto& units : classBattle.listOfAllUnit)
		UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), N);
}
void Battle::spawnTimedEnemy()
{
	if (stopwatchGameTime.sF() >= 5)
	{
		int32 kukj = Random(1, 2);
		if (kukj / 2 == 0)
		{
			int32 iyigu = Random(0, N - 1);
			changeUnitMember = true;
			UnitRegister(U"sniperP99",
			0,
			Random(0, N - 1),
			1, classBattle.listOfAllEnemyUnit, true);

			changeUnitMember = false;
			stopwatchGameTime.restart();
		}
		else
		{
			int32 iyigu = Random(0, N - 1);
			changeUnitMember = true;
			UnitRegister(U"sniperP99",
			Random(0, N - 1),
			N - 1,
			1, classBattle.listOfAllEnemyUnit, true);
			changeUnitMember = false;
			stopwatchGameTime.restart();
		}
	}
}
void Battle::updateUnitHealthBars()
{
	constexpr Vec2 offset{ -32, +22 }; // = -64 / 2, +32 / 2 + 6

	auto updateBar = [&](Unit& unit)
		{
			double hpRatio = static_cast<double>(unit.Hp) / unit.HpMAX;
			unit.bLiquidBarBattle.update(hpRatio);
			unit.bLiquidBarBattle.ChangePoint(unit.GetNowPosiCenter() + offset);
		};

	for (auto& group : classBattle.listOfAllUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}

	for (auto& group : classBattle.listOfAllEnemyUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}
}
/// @brief 建築予約をするのが本質
void Battle::handleBuildMenuSelectionA()
{
	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };

	//全部変更可能性あるので非const
	for (auto& loau : classBattle.listOfAllUnit)
	{
		for (auto& itemUnit : loau.ListClassUnit)
		{
			if (itemUnit.IsSelect == false) continue;

			for (auto& hbm : sortedArrayBuildMenu)
			{
				Array<String> resSp = hbm.first.split('-');
				if (resSp[0] != itemUnit.classBuild) continue;

				if (hbm.second.rectHantei.leftClicked())
				{
					if (hbm.second.isMove == true)
					{
						IsBuildSelectTraget = true;
						itemUnit.tempIsBuildSelectTragetBuildAction = hbm.second;
						IsResourceSelectTraget = false;
						tempSelectComRight = hbm.second;
						itemUnit.tempSelectComRight = tempSelectComRight;
						break;
					}

					if (hbm.second.category == U"Carrier")
					{
						// 周囲3マス範囲のランダムユニット格納処理

							// 現在選択されているユニットを取得
						Unit* selectedCarrierUnit = nullptr;
						for (auto& loau : classBattle.listOfAllUnit)
						{
							for (auto& unit : loau.ListClassUnit)
							{
								if (unit.IsSelect)
								{
									selectedCarrierUnit = &unit;
									break;
								}
							}
							if (selectedCarrierUnit) break;
						}

						if (!selectedCarrierUnit) continue;

						// キャリアーコンポーネントを取得
						auto* carrierComponent = selectedCarrierUnit->getComponent<CarrierComponent>();
						if (!carrierComponent) continue;

						// 選択ユニットの現在位置のタイル座標を取得
						Optional<Point> carrierTileIndex = ToIndex(selectedCarrierUnit->GetNowPosiCenter(), columnQuads, rowQuads);
						if (!carrierTileIndex.has_value()) continue;

						// 周囲3マス範囲のユニットを検索
						Array<Unit*> nearbyUnits;
						const int32 searchRadius = 3;

						// 味方ユニットから検索
						for (auto& group : classBattle.listOfAllUnit)
						{
							for (auto& unit : group.ListClassUnit)
							{
								// 自分自身、建物、戦闘不可ユニットは除外
								if (unit.ID == selectedCarrierUnit->ID ||
									unit.IsBuilding ||
									!unit.IsBattleEnable || unit.isCarrierUnit) continue;

								// ユニットの現在位置のタイル座標を取得
								Optional<Point> unitTileIndex = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads);
								if (!unitTileIndex.has_value()) continue;

								// 距離をチェック（マンハッタン距離）
								int32 distance = carrierTileIndex->manhattanDistanceFrom(*unitTileIndex);
								if (distance <= searchRadius)
								{
									nearbyUnits.push_back(&unit);
								}
							}
						}

						// 範囲内にユニットがいない場合は処理終了
						if (nearbyUnits.isEmpty())
						{
							Print << U"周囲3マス以内にユニットが見つかりません";
							continue;
						}

						// ランダムに並び替え
						Shuffle(nearbyUnits);

						// キャリアーの容量まで格納
						int32 storedCount = 0;
						for (Unit* unit : nearbyUnits)
						{
							if (carrierComponent->store(unit))
							{
								storedCount++;
								Print << U"ユニット '{}' を格納しました"_fmt(unit->Name);

								// 容量に達したら終了
								if (carrierComponent->storedUnits.size() >= carrierComponent->capacity)
								{
									break;
								}
							}
							else
							{
								Print << U"キャリアーの容量が満杯です";
								break;
							}
						}

						if (storedCount > 0)
						{
							Print << U"合計 {} 体のユニットを格納しました"_fmt(storedCount);
						}
						else
						{
							Print << U"格納できるユニットがありませんでした";
						}
					}

					if (hbm.second.category == U"releaseAll")
					{
						// 現在選択されているユニットを取得
						Unit* selectedCarrierUnit = nullptr;
						for (auto& loau : classBattle.listOfAllUnit)
						{
							for (auto& unit : loau.ListClassUnit)
							{
								if (unit.IsSelect)
								{
									selectedCarrierUnit = &unit;
									break;
								}
							}
							if (selectedCarrierUnit) break;
						}

						if (!selectedCarrierUnit) continue;

						// キャリアーコンポーネントを取得
						auto* carrierComponent = selectedCarrierUnit->getComponent<CarrierComponent>();
						if (!carrierComponent) continue;

						// 格納されているユニットがあるかチェック
						if (carrierComponent->storedUnits.empty())
						{
							Print << U"格納されているユニットがありません";
							continue;
						}

						// 現在のキャリアーユニットの位置を取得
						Vec2 releasePosition = selectedCarrierUnit->GetNowPosiCenter();

						// 格納されているユニット数を記録（リリース前）
						int32 releasedCount = static_cast<int32>(carrierComponent->storedUnits.size());

						// 全ユニットを解放
						carrierComponent->releaseAll(releasePosition);

						Print << U"合計 {} 体のユニットを解放しました"_fmt(releasedCount);
					}

					// 設置位置の取得
					if (const auto& index = ToIndex(itemUnit.GetNowPosiCenter(), columnQuads, rowQuads))
					{
						hbm.second.rowBuildingTarget = index->y;
						hbm.second.colBuildingTarget = index->x;
						itemUnit.currentTask = UnitTask::None;
					}
					else
					{
						//現在選択ユニットはマップ外にいる……
					}

					IsBuildSelectTraget = false;

					//Battle::updateBuildQueueで作る
					if (itemUnit.taskTimer.isRunning() == false)
					{
						itemUnit.taskTimer.restart();
						itemUnit.progressTime = 0.0;
					}
					itemUnit.arrYoyakuBuild.push_back(hbm.second);
					// 回数制限の更新と再描画
					if (hbm.second.buildCount > 0)
					{
						hbm.second.buildCount--;
						//キーだけ渡して該当のrenderだけ更新するように
						//renB();
					}
				}
				else if (hbm.second.rectHantei.mouseOver())
				{
					nowSelectBuildSetumei = U"~~~Unit Or Build~~~\r\n" + hbm.second.description;
					rectSetumei = { Scene::Size().x - renderTextureBuildMenuEmpty.size().x,
						Scene::Size().y - underBarHeight - renderTextureBuildMenuEmpty.size().y,
						320, 0 };
					rectSetumei.h = fontSkill(nowSelectBuildSetumei).region().h;
					while (!fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Color(0.0, 0.0)))
					{
						rectSetumei.h += 12;
					}
					rectSetumei.y -= rectSetumei.h;
					break;
				}
				else
				{
					nowSelectBuildSetumei.clear();
				}

			}
		}
	}
}
void Battle::updateBuildQueue()
{
	using MyAnonymousClass = struct {
		String spawn;
		int32 tempColBuildingTarget;
		int32 tempRowBuildingTarget;
		int32 count;
	};
	Array<MyAnonymousClass> temo;
	for (auto& loau : classBattle.listOfAllUnit)
	{
		for (auto& itemUnit : loau.ListClassUnit)
		{
			if (itemUnit.arrYoyakuBuild.isEmpty()) continue;

			const double tempTime = itemUnit.arrYoyakuBuild.front().buildTime;
			auto tempBA = itemUnit.arrYoyakuBuild.front().result;
			//int32 kuhukgiuy = itemUnit.arrYoyakuBuild.front().
			const int32 tempRowBuildingTarget = itemUnit.arrYoyakuBuild.front().rowBuildingTarget;
			const int32 tempColBuildingTarget = itemUnit.arrYoyakuBuild.front().colBuildingTarget;
			const int32 createCount = itemUnit.arrYoyakuBuild.front().createCount;

			if (itemUnit.progressTime >= 1.0)
			{
				itemUnit.taskTimer.reset();
				itemUnit.arrYoyakuBuild.pop_front();
				if (!itemUnit.arrYoyakuBuild.isEmpty())
				{
					itemUnit.progressTime = 0.0;
					itemUnit.taskTimer.restart();
				}
				else
				{
					itemUnit.progressTime = -1.0;
				}

				if (tempBA.type == U"unit")
				{
					MyAnonymousClass yfyu;
					yfyu.spawn = tempBA.spawn;
					yfyu.tempColBuildingTarget = tempColBuildingTarget;
					yfyu.tempRowBuildingTarget = tempRowBuildingTarget;
					yfyu.count = createCount;
					temo.push_back(yfyu);
				}
			}

			// プログレス更新
			itemUnit.progressTime = Min(itemUnit.taskTimer.sF() / tempTime, 1.0);
		}
	}

	for (auto& uihbui : temo)
	{
		UnitRegister(uihbui.spawn, uihbui.tempColBuildingTarget, uihbui.tempRowBuildingTarget, uihbui.count, classBattle.listOfAllUnit, false);
	}
}
Co::Task<> Battle::checkCancelSelectionByUIArea()
{
	if (Cursor::PosF().y >= Scene::Size().y - underBarHeight)
	{
		longBuildSelectTragetId = -1;
		IsResourceSelectTraget = false;

		for (auto& target : classBattle.listOfAllUnit)
		{
			for (auto& unit : target.ListClassUnit)
			{
				unit.IsSelect = false;
			}
		}

		co_await Co::NextFrame();
	}
	co_return;
}
void Battle::updateUnitMovements()
{
	//移動処理
	{
		for (auto& item : classBattle.listOfAllUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
					continue;
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
					continue;
				if (itemUnit.IsBattleEnable == false)
					continue;
				if (itemUnit.moveState == moveState::None) continue;
				if (itemUnit.moveState == moveState::MoveAI) continue;
				if (itemUnit.moveState == moveState::FlagMoveCalc) continue;

				{
					std::scoped_lock lock(aiRootMutex);
					if (!aiRootMy.contains(itemUnit.ID)) continue;
				}
				auto& plan = aiRootMy[itemUnit.ID];

				if (plan.isPathCompleted())
				{
					if (itemUnit.moveState == moveState::MovingEnd)
					{
						itemUnit.moveState = moveState::None; // 移動完了状態から通常状態に戻す
						continue;
					}
					else if (itemUnit.moveState == moveState::Moving)
					{
						//最終移動
						itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

						// 進行方向ベクトル
						Vec2 moveDir = itemUnit.vecMove;
						// 現在位置→目標位置ベクトル
						Vec2 toTarget = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();

						// 目標を通り過ぎた or 近づいたら到達とみなす
						if (toTarget.dot(moveDir) <= 0 || toTarget.length() < 5.0)
						{
							itemUnit.nowPosiLeft = itemUnit.orderPosiLeft; // 位置をピッタリ補正して止めるのもあり
							itemUnit.FlagReachedDestination = true;
							itemUnit.moveState = moveState::MovingEnd; // 移動完了状態にする
						}
					}

					continue;
				}

				if (itemUnit.moveState == moveState::Moving)
				{
					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

					if (auto iuyb = plan.getCurrentTarget())
					{
						bool calculateResult = false;
						{
							Optional<Size> nowIndex = ToIndex(itemUnit.GetNowPosiCenter(), columnQuads, rowQuads);
							if (nowIndex.has_value())
							{
								if (plan.lastPoint.x != nowIndex->x || plan.lastPoint.y != nowIndex->y)
								{
									plan.lastPoint = nowIndex.value();
									calculateResult = true;
								}
							}
						}
						//if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) < 3.0)
						if (calculateResult)
						{
							plan.stepToNext();
							if (plan.getCurrentTarget())
							{
								// 到達チェック
								const int32 i = plan.getCurrentTarget().value().manhattanLength();
								const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
								const int32 yi = (i < (N - 1)) ? i : (N - 1);
								const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
								const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
								const double posY = (i * TileOffset.y) - TileThickness;
								const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
								Vec2 nextPos = pos;
								itemUnit.orderPosiLeft = nextPos;
								itemUnit.vecMove = (itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter()).normalized();
							}
						}
					}
					if (plan.isPathCompleted())
					{
						itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast; // 最後の位置に戻す
						Optional<Size> oor = ToIndex(itemUnit.GetOrderPosiCenter(), columnQuads, rowQuads);
						plan.lastPoint = oor.value();
						Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
						itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
					}
					continue;
				}

				// 次のマスに向けて移動準備
				if (auto iuyb = plan.getCurrentTarget())
				{
					plan.lastPoint = iuyb.value();
					plan.stepToNext();

					// そのタイルの底辺中央の座標
					const int32 i = plan.getCurrentTarget().value().manhattanLength();
					const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
					const int32 yi = (i < (N - 1)) ? i : (N - 1);
					const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
					const double posY = (i * TileOffset.y) - TileThickness;
					const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

					itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
					itemUnit.moveState = moveState::Moving; // 移動状態にする
				}
			}
		}
		for (auto& item : classBattle.listOfAllEnemyUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
					continue;
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
					continue;
				if (itemUnit.IsBattleEnable == false)
					continue;

				{
					std::scoped_lock lock(aiRootMutex);
					if (!aiRootEnemy.contains(itemUnit.ID)) continue;
				}

				auto& plan = aiRootEnemy[itemUnit.ID];

				// 1. 移動準備
				if (itemUnit.moveState == moveState::None && plan.getCurrentTarget())
				{
					const Point targetTile = plan.getCurrentTarget().value();

					// そのタイルの底辺中央の座標
					const int32 i = plan.getCurrentTarget().value().manhattanLength();
					const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
					const int32 yi = (i < (N - 1)) ? i : (N - 1);
					const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
					const double posY = (i * TileOffset.y) - TileThickness;
					const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
					itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

					itemUnit.moveState = moveState::Moving; // 移動状態にする
				}

				// 2. 移動処理
				if (itemUnit.moveState == moveState::Moving)
				{
					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

					if (plan.getCurrentTarget())
					{
						if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) <= 3.0)
						{
							plan.stepToNext();
							if (plan.getCurrentTarget())
							{
								const Point targetTile = plan.getCurrentTarget().value();
								const int32 i = plan.getCurrentTarget().value().manhattanLength();
								const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
								const int32 yi = (i < (N - 1)) ? i : (N - 1);
								const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
								const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
								const double posY = (i * TileOffset.y) - TileThickness;
								const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
								itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

								Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
								itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

								itemUnit.moveState = moveState::Moving; // 移動状態にする
							}
						}
					}

					if (plan.isPathCompleted())
					{
						itemUnit.FlagReachedDestination = true;
						itemUnit.moveState = moveState::MoveAI;
					}
				}
			}
		}
	}
}
void Battle::handleSkillUISelection()
{
	//skill選択処理
	{
		const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(0, Scene::Size().y - 320 - 30) };
		for (auto&& [i, re] : Indexed(htSkill))
		{
			if (re.second.leftClicked())
			{
				bool flgEr = false;
				for (auto it = nowSelectSkill.begin(); it != nowSelectSkill.end(); ++it)
				{
					if (it->contains(re.first))
					{
						nowSelectSkill.erase(it);
						flgEr = true;
						break;
					}
				}

				if (flgEr == false)
				{
					nowSelectSkill.push_back(re.first);
				}
			}
			if (re.second.mouseOver())
			{
				flagDisplaySkillSetumei = true;
				nowSelectSkillSetumei = U"";
				//スキル説明を書く
				for (auto& item : classBattle.listOfAllUnit)
				{
					if (!item.FlagBuilding &&
						!item.ListClassUnit.empty())
						for (auto& itemUnit : item.ListClassUnit)
						{
							for (auto& itemSkill : itemUnit.arrSkill)
							{
								if (itemSkill.nameTag == re.first)
								{
									nowSelectSkillSetumei = itemSkill.name + U"\r\n"
										+ itemSkill.help + U"\r\n"
										+ systemString.SkillAttack + U":" + Format(itemSkill.str);
									;
									break;
								}
							}
							if (nowSelectSkillSetumei != U"")
								break;
						}
					if (nowSelectSkillSetumei != U"")
						break;
				}

				nowSelectSkillSetumei = U"~~~Skill~~~\r\n" + nowSelectSkillSetumei;

				while (not fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Color(0.0, 0.0)))
				{
					rectSkillSetumei.h = rectSkillSetumei.h + 12;
				}
				rectSkillSetumei.x = re.second.pos.x + 32;
				rectSkillSetumei.y = Scene::Size().y - underBarHeight - rectSkillSetumei.h;
				break;
			}
			else
			{
				flagDisplaySkillSetumei = false;
				nowSelectSkillSetumei = U"";
			}
		}

		{
			const ScopedRenderTarget2D target{ renderTextureSkillUP.clear(ColorF{ 0.5, 0.0, 0.0, 0.0 }) };
			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };
			for (auto&& [i, re] : Indexed(htSkill))
				for (auto it = nowSelectSkill.begin(); it != nowSelectSkill.end(); ++it)
					if (it->contains(re.first))
					{
						re.second.drawFrame(2, 0, Palette::Red);
						break;
					}
		}
	}
}
void Battle::handleCameraInput()
{
	// カメラの移動処理（左クリックドラッグ）
	if (MouseL.pressed() == true)
	{
		const auto vPos = (camera.getTargetCenter() - Cursor::Delta());
		camera.jumpTo(vPos, camera.getTargetScale());
	}
	// 右クリック時のカーソル座標記録処理
	if (MouseR.pressed() == false)
	{
		if (MouseR.up() == false)
			cursPos = Cursor::Pos();
	}
	else if (MouseR.down() && IsBattleMove)
	{
		cursPos = Cursor::Pos();
	}
}
void Battle::handleUnitAndBuildingSelection()
{
	// 建築物選択チェック
	bool isSeBu = false;
	{
		// いったん押された建築物の固有IDを走査する
		// 対象以外のIsSelectを全てfalseにしたいが為
		long id = -1;
		for (auto& item : classBattle.listOfAllUnit)
		{
			if (item.FlagBuilding == true &&
				!item.ListClassUnit.empty())
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					Size tempSize = TextureAsset(itemUnit.ImageName).size();
					Quad tempQ = ToTile(Point(itemUnit.colBuilding, itemUnit.rowBuilding), N);
					const Vec2 leftCenter = (tempQ.p0 + tempQ.p3) / 2.0;
					const Vec2 rightCenter = (tempQ.p1 + tempQ.p2) / 2.0;
					const double horizontalWidth = Abs(rightCenter.x - leftCenter.x);
					const double vHe = Abs(rightCenter.y - leftCenter.y);
					double scale = tempSize.x / (horizontalWidth * 2);

					if (tempQ.scaled(scale).movedBy(0, -(vHe * scale) + TileThickness).leftClicked())
					{
						id = itemUnit.ID;
					}
				}
			}
		}

		if (id != -1)
		{
			for (auto& item : classBattle.listOfAllUnit)
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					if (id == itemUnit.ID)
					{
						itemUnit.IsSelect = !itemUnit.IsSelect;
						IsBuildMenuHome = itemUnit.IsSelect;
						//TODO どうする
						//buiSyu = itemUnit.buiSyu; // 建築の種類
						isSeBu = true;
					}
					else
					{
						itemUnit.IsSelect = false;
					}
				}
			}
		}
	}

	// ユニット選択チェック
	{
		if (isSeBu == false)
		{
			//全ユニット走査、posからrectを生成し、クリックされたかチェック
			for (auto& target : classBattle.listOfAllUnit)
			{
				for (auto& unit : target.ListClassUnit)
				{
					if ((unit.IsBuilding == false && unit.IsBattleEnable == true)
						|| (unit.IsBuilding == true && unit.IsBattleEnable == true && unit.classBuild != U""))
					{
						if (unit.GetRectNowPosi().leftClicked())
						{
							//クリックされたユニットのIDを取得
							//そのユニットのIsSelectを反転させる
							//IsBuildMenuHomeを反転させる
							//他のユニットのIsSelectはfalseにする
							for (auto& item : classBattle.listOfAllUnit)
							{
								for (auto& itemUnit : item.ListClassUnit)
								{
									if (unit.ID == itemUnit.ID)
									{

									}
									else
									{
										itemUnit.IsSelect = false;
									}
								}
							}
							unit.IsSelect = !unit.IsSelect;
							IsBuildMenuHome = unit.IsSelect;

							if (unit.IsSelect == true)
							{
								longBuildSelectTragetId = unit.ID; // 選択されたユニットのIDを保存
								IsResourceSelectTraget = true;
							}
							else
							{
								longBuildSelectTragetId = -1; // 選択されたユニットのIDをリセット
								IsResourceSelectTraget = false;
							}
						}
					}
				}
			}
		}
	}
}

///// @brief 範囲選択されたタイル配列を取得
///// @param start 開始タイル座標
///// @param end 終了タイル座標
///// @return 選択範囲内のタイル座標配列
//Array<Point> Battle::getRangeSelectedTiles(const Point& start, const Point& end)
//{
//	Array<Point> selectedTiles;
//
//	// 開始点と終了点から矩形範囲を計算
//	int32 minX = Min(start.x, end.x);
//	int32 maxX = Max(start.x, end.x);
//	int32 minY = Min(start.y, end.y);
//	int32 maxY = Max(start.y, end.y);
//
//	// 矩形範囲内の全タイルを配列に追加
//	for (int32 x = minX; x <= maxX; ++x)
//	{
//		for (int32 y = minY; y <= maxY; ++y)
//		{
//			// マップ範囲内かチェック
//			if (x >= 0 && x < N && y >= 0 && y < N)
//			{
//				selectedTiles.push_back(Point(x, y));
//			}
//		}
//	}
//
//	return selectedTiles;
//}
/// @brief 範囲選択されたタイル配列を取得（線状選択版）
/// @param start 開始タイル座標
/// @param end 終了タイル座標
/// @return 選択範囲内のタイル座標配列（線状）
Array<Point> Battle::getRangeSelectedTiles(const Point& start, const Point& end) const
{
	Array<Point> selectedTiles;

	// 開始点と終了点の差分を計算
	int32 deltaX = end.x - start.x;
	int32 deltaY = end.y - start.y;

	// どちらの方向により大きく動いたかを判定
	bool isHorizontalDominant = Abs(deltaX) >= Abs(deltaY);

	if (isHorizontalDominant)
	{
		// 横方向が主軸：水平線を描画
		int32 minX = Min(start.x, end.x);
		int32 maxX = Max(start.x, end.x);
		int32 fixedY = start.y; // Y座標は開始点で固定

		for (int32 x = minX; x <= maxX; ++x)
		{
			// マップ範囲内かチェック
			if (x >= 0 && x < N && fixedY >= 0 && fixedY < N)
			{
				selectedTiles.push_back(Point(x, fixedY));
			}
		}

		Print << U"水平線選択: Y={}, X={}〜{} ({} タイル)"_fmt(
			fixedY, minX, maxX, selectedTiles.size());
	}
	else
	{
		// 縦方向が主軸：垂直線を描画
		int32 minY = Min(start.y, end.y);
		int32 maxY = Max(start.y, end.y);
		int32 fixedX = start.x; // X座標は開始点で固定

		for (int32 y = minY; y <= maxY; ++y)
		{
			// マップ範囲内かチェック
			if (fixedX >= 0 && fixedX < N && y >= 0 && y < N)
			{
				selectedTiles.push_back(Point(fixedX, y));
			}
		}

		Print << U"垂直線選択: X={}, Y={}〜{} ({} タイル)"_fmt(
			fixedX, minY, maxY, selectedTiles.size());
	}

	return selectedTiles;
}
/// @brief 選択されたタイルに対して建築処理を実行
/// @param tiles 選択されたタイル座標配列
void Battle::processBuildOnTiles(const Array<Point>& tiles)
{
	if (longBuildSelectTragetId == -1)
	{
		Print << U"建築ユニットが選択されていません";
		return;
	}

	Unit& selectedUnit = GetCU(longBuildSelectTragetId);

	// 建築可能なタイルの数をカウント
	Array<Point> validTiles;
	for (const auto& tile : tiles)
	{
		if (canBuildOnTile(tile))
		{
			validTiles.push_back(tile);
		}
	}

	if (validTiles.isEmpty())
	{
		Print << U"建築可能なタイルがありません";
		return;
	}

	// 建築可能なタイルに対して建築処理を実行
	int32 builtCount = 0;
	for (const auto& tile : validTiles)
	{
		// 建築メニューから選択された建築アクションを取得
		BuildAction buildAction = selectedUnit.tempIsBuildSelectTragetBuildAction;

		// タイル座標を建築ターゲットに設定
		buildAction.rowBuildingTarget = tile.y;
		buildAction.colBuildingTarget = tile.x;

		// 建築予約に追加
		selectedUnit.arrYoyakuBuild.push_back(buildAction);
		builtCount++;

		Print << U"タイル({}, {})に建築を予約しました"_fmt(tile.x, tile.y);
	}

	// 建築タイマーを開始（まだ動いていない場合）
	if (!selectedUnit.taskTimer.isRunning() && !selectedUnit.arrYoyakuBuild.isEmpty())
	{
		selectedUnit.taskTimer.restart();
		selectedUnit.progressTime = 0.0;
	}

	Print << U"合計 {} 箇所に建築を予約しました"_fmt(builtCount);

	// 建築選択状態を解除
	IsBuildSelectTraget = false;
	IsBuildMenuHome = false;
	selectedUnit.IsSelect = false;
	longBuildSelectTragetId = -1;
}

/// @brief 指定タイルに建築可能かチェック
/// @param tile タイル座標
/// @return 建築可能ならtrue
bool Battle::canBuildOnTile(const Point& tile) const
{
	// マップ範囲外チェック
	if (tile.x < 0 || tile.x >= N || tile.y < 0 || tile.y >= N)
	{
		return false;
	}

	// 建物が既に存在するかチェック
	for (const auto& group : classBattle.listOfAllUnit)
	{
		if (group.FlagBuilding)
		{
			for (const auto& unit : group.ListClassUnit)
			{
				if (unit.initTilePos == tile && unit.IsBattleEnable)
				{
					return false; // 既に建物が存在
				}
			}
		}
	}

	// 敵の建物もチェック
	for (const auto& group : classBattle.listOfAllEnemyUnit)
	{
		if (group.FlagBuilding)
		{
			for (const auto& unit : group.ListClassUnit)
			{
				if (unit.initTilePos == tile && unit.IsBattleEnable)
				{
					return false; // 敵の建物が存在
				}
			}
		}
	}

	// ユニットが存在するかチェック
	for (const auto& group : classBattle.listOfAllUnit)
	{
		for (const auto& unit : group.ListClassUnit)
		{
			if (!unit.IsBuilding && unit.IsBattleEnable)
			{
				Optional<Point> unitTilePos = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads);
				if (unitTilePos && *unitTilePos == tile)
				{
					return false; // ユニットが存在
				}
			}
		}
	}

	// 地形チェック（水タイルなど建築不可地形があれば追加）
	// TODO: mapDataから地形情報を取得して建築可否を判定

	return true; // 建築可能
}

/// @brief 建物をタイル上に作成
/// @param tile タイル座標
/// @param buildAction 建築アクション
void Battle::createBuildingOnTile(const Point& tile, const BuildAction& buildAction)
{
	// 建物ユニットの作成
	for (auto& baseUnit : m_commonConfig.arrayUnit)
	{
		if (baseUnit.NameTag == buildAction.result.spawn)
		{
			Unit newBuilding = baseUnit;
			newBuilding.ID = classBattle.getIDCount();
			newBuilding.IsBuilding = true;
			newBuilding.initTilePos = tile;
			newBuilding.colBuilding = tile.x;
			newBuilding.rowBuilding = tile.y;
			newBuilding.nowPosiLeft = ToTile(tile, N).asPolygon().centroid()
				.movedBy(-(newBuilding.yokoUnit / 2), -(newBuilding.TakasaUnit / 2));

			// 建築物グループに追加
			ClassHorizontalUnit buildingGroup;
			buildingGroup.FlagBuilding = true;
			buildingGroup.ListClassUnit.push_back(newBuilding);
			classBattle.listOfAllUnit.push_back(buildingGroup);

			// A*用ハッシュテーブルに追加
			unitsForHsBuildingUnitForAstar.push_back(std::make_unique<Unit>(newBuilding));
			hsBuildingUnitForAstar[tile] = unitsForHsBuildingUnitForAstar.back().get();

			break;
		}
	}
}
/// @brief 建築ハッシュテーブルの更新
/// @param tile 建築されたタイル座標
void Battle::updateBuildingHashTable(const Point& tile)
{
	// ミニマップの色更新
	minimapCols[tile] = ColorF{ 0.5, 0.3, 0.1 }; // 建物の色

	// 視界の更新（建物により視界が変わる場合）
	refreshFogOfWar();
}
/// @brief 建築完了エフェクトの再生
/// @param tile 建築されたタイル座標
void Battle::playBuildCompleteEffect(const Point& tile)
{
	Vec2 worldPos = ToTileBottomCenter(tile, N);

	// パーティクルエフェクト（実装されている場合）
	// Effect::Add<BuildCompleteEffect>(worldPos);

	// サウンドエフェクト（実装されている場合）
	// SoundAsset(U"buildComplete").playOneShot();

	Print << U"建築完了エフェクトを再生: タイル({}, {})"_fmt(tile.x, tile.y);
}
/// @brief 指定タイルで建築を実行（実際の建築処理）
/// @param tile タイル座標
/// @param itemUnit 建築を行うユニット
void Battle::executeBuildOnTile(Unit& itemUnit)
{
	// 現在のタスクの建築を実行
	BuildAction& currentBuildAction = itemUnit.arrYoyakuBuild.front();

	// 建築位置の設定
	Point tile(currentBuildAction.colBuildingTarget, currentBuildAction.rowBuildingTarget);

	if (!canBuildOnTile(tile))
	{
		Print << U"タイル({}, {})は建築できません"_fmt(tile.x, tile.y);
		return;
	}

	if (itemUnit.arrYoyakuBuild.isEmpty())
	{
		Print << U"建築キューが空です";
		return;
	}

	// 建築結果の処理
	if (currentBuildAction.result.type == U"unit")
	{
		const auto logFor = currentBuildAction;

		// ユニットの生成
		UnitRegister(currentBuildAction.result.spawn,
				   tile.x, tile.y,
				   currentBuildAction.createCount,
				   classBattle.listOfAllUnit, false);

		Print << U"タイル({}, {})にユニット '{}' を {} 体生成しました"_fmt(
			tile.x, tile.y, logFor.result.spawn, logFor.createCount);
	}
	//else if (currentBuildAction.result.type == U"building")
	//{
	//	// 建物の配置
	//	createBuildingOnTile(tile, currentBuildAction);

	//	Print << U"タイル({}, {})に建物 '{}' を建設しました"_fmt(
	//		tile.x, tile.y, currentBuildAction.result.spawn);
	//}
	//else if (currentBuildAction.result.type == U"wall")
	//{
	//	// 壁の建設
	//	createWallOnTile(tile, currentBuildAction);

	//	Print << U"タイル({}, {})に壁を建設しました"_fmt(tile.x, tile.y);
	//}

	// 資源の消費
	//if (currentBuildAction.cost.gold > 0)
	//{
	//	gold = Max(0, gold - currentBuildAction.cost.gold);
	//}
	//if (currentBuildAction.cost.food > 0)
	//{
	//	food = Max(0, food - currentBuildAction.cost.food);
	//}

	// 建築ハッシュテーブルの更新（A*用）
	updateBuildingHashTable(tile);

	// 建築完了エフェクト
	playBuildCompleteEffect(tile);
}

/// @brief 範囲選択に対応した建築処理（移動→建築のシーケンス）
/// @param tiles 選択されたタイル座標配列
void Battle::processBuildOnTilesWithMovement(const Array<Point>& tiles)
{
	if (longBuildSelectTragetId == -1)
	{
		Print << U"建築ユニットが選択されていません";
		return;
	}

	Unit& selectedUnit = GetCU(longBuildSelectTragetId);

	// 建築可能なタイルのみを抽出
	Array<Point> validTiles;
	for (const auto& tile : tiles)
	{
		if (canBuildOnTile(tile))
		{
			validTiles.push_back(tile);
		}
	}

	if (validTiles.isEmpty())
	{
		Print << U"建築可能なタイルがありません";
		return;
	}

	// 現在位置からの距離でソート（最適な移動順序）
	Optional<Point> currentTilePos = ToIndex(selectedUnit.GetNowPosiCenter(), columnQuads, rowQuads);
	if (currentTilePos)
	{
		validTiles.sort_by([&](const Point& a, const Point& b) {
			return currentTilePos->manhattanDistanceFrom(a) < currentTilePos->manhattanDistanceFrom(b);
		});
	}

	// 各タイルに対して移動→建築のタスクを作成
	for (const auto& tile : validTiles)
	{
		BuildAction buildAction = selectedUnit.tempIsBuildSelectTragetBuildAction;
		buildAction.rowBuildingTarget = tile.y;
		buildAction.colBuildingTarget = tile.x;

		// 移動が必要な建築として特別なタスクタイプを設定
		selectedUnit.requiresMovement = true;

		selectedUnit.arrYoyakuBuild.push_back(buildAction);
		Print << U"タイル({}, {})への移動→建築を予約しました"_fmt(tile.x, tile.y);
	}

	// 最初の建築タスクを開始
	if (!selectedUnit.arrYoyakuBuild.isEmpty())
	{
		selectedUnit.currentTask = UnitTask::MovingToBuild;

		// 最初のタイルへの移動を開始
		Point firstTile = Point(selectedUnit.arrYoyakuBuild.front().colBuildingTarget,
							   selectedUnit.arrYoyakuBuild.front().rowBuildingTarget);

		Vec2 targetPos = ToTileBottomCenter(firstTile, N);
		selectedUnit.orderPosiLeft = targetPos.movedBy(-(selectedUnit.yokoUnit / 2), -(selectedUnit.TakasaUnit / 2));
		selectedUnit.orderPosiLeftLast = targetPos;
		selectedUnit.vecMove = (selectedUnit.orderPosiLeft - selectedUnit.nowPosiLeft).normalized();
		selectedUnit.moveState = moveState::MoveAI;

		if (!selectedUnit.taskTimer.isRunning())
		{
			selectedUnit.taskTimer.restart();
			selectedUnit.progressTime = 0.0;
		}
	}

	Print << U"合計 {} 箇所への移動→建築を予約しました"_fmt(validTiles.size());

	// 建築選択状態を解除
	IsBuildSelectTraget = false;
	IsBuildMenuHome = false;
	selectedUnit.IsSelect = false;
	longBuildSelectTragetId = -1;
}
/// @brief 移動完了後の建築処理（改良版）
/// @param unit 建築ユニット
void Battle::afterMovedPushToBuildMenuAdvanced(Unit& itemUnit)
{
	if (itemUnit.classBuild == U"") return;
	if (itemUnit.arrYoyakuBuild.isEmpty()) return;

	// ユニットIDを事前に保存（参照破壊対策）
	const int64 unitId = itemUnit.ID;
	const size_t buildQueueSizeBefore = itemUnit.arrYoyakuBuild.size();

	Print << U"建築前: ユニットID {} の建築キューサイズ: {}"_fmt(unitId, buildQueueSizeBefore);

	// 建築実行
	executeBuildOnTile(itemUnit);

	// IDから再度ユニット参照を安全に取得
	Unit* currentUnit = GetCUSafe(unitId);
	if (!currentUnit)
	{
		Print << U"警告: ユニットID {} が見つかりません。建築処理を中断します。"_fmt(unitId);
		return;
	}

	// 4. 建築キューの状態を確認・ログ出力
	const size_t buildQueueSizeAfter = currentUnit->arrYoyakuBuild.size();
	Print << U"建築後: ユニットID {} の建築キューサイズ: {} -> {}"_fmt(
		unitId, buildQueueSizeBefore, buildQueueSizeAfter);

	// 5. 建築キューが意図せず空になっていないかチェック
	if (buildQueueSizeAfter == 0 && buildQueueSizeBefore > 1)
	{
		Print << U"警告: 建築キューが予想外に空になりました。参照破壊の可能性があります。";
		Print << U"ユニットID: {}, 元のサイズ: {}, 現在のサイズ: {}"_fmt(
			unitId, buildQueueSizeBefore, buildQueueSizeAfter);

		// 緊急事態なので、建築処理を中断
		currentUnit->currentTask = UnitTask::None;
		currentUnit->moveState = moveState::None;
		currentUnit->taskTimer.reset();
		currentUnit->progressTime = -1.0;
		IsBuildSelectTraget = false;
		return;
	}

	// 6. 正常な場合のみ建築タスクを削除
	if (!currentUnit->arrYoyakuBuild.isEmpty())
	{
		currentUnit->arrYoyakuBuild.pop_front();
		Print << U"建築タスクを削除しました。残りタスク数: {}"_fmt(currentUnit->arrYoyakuBuild.size());
	}
	else
	{
		Print << U"警告: 削除すべき建築タスクが見つかりません";
	}

	// 次の建築タスクがあるかチェック
	if (!currentUnit->arrYoyakuBuild.isEmpty())
	{
		// 次のタイルへの移動を開始
		BuildAction& nextBuildAction = currentUnit->arrYoyakuBuild.front();
		Point nextTile(nextBuildAction.colBuildingTarget, nextBuildAction.rowBuildingTarget);

		Vec2 nextTargetPos = ToTileBottomCenter(nextTile, N);
		currentUnit->orderPosiLeft = nextTargetPos.movedBy(-(currentUnit->yokoUnit / 2), -(currentUnit->TakasaUnit / 2));
		currentUnit->orderPosiLeftLast = nextTargetPos;
		currentUnit->vecMove = (currentUnit->orderPosiLeft - currentUnit->nowPosiLeft).normalized();
		currentUnit->moveState = moveState::MoveAI;
		currentUnit->currentTask = UnitTask::MovingToBuild;

		Print << U"次のタイル({}, {})への移動を開始します"_fmt(nextTile.x, nextTile.y);
	}
	else
	{
		// すべての建築タスクが完了
		currentUnit->currentTask = UnitTask::None;
		currentUnit->moveState = moveState::None;
		currentUnit->taskTimer.reset();
		currentUnit->progressTime = -1.0;

		Print << U"すべての建築タスクが完了しました";
	}

	IsBuildSelectTraget = false;
}

void Battle::handleBuildTargetSelection()
{
	if (!IsBuildSelectTraget)
		return;

	if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
	{
		if (ToTile(*index, N).leftClicked() && longBuildSelectTragetId != -1)
		{
			IsBuildSelectTraget = false;
			IsBuildMenuHome = false;

			Unit& cu = GetCU(longBuildSelectTragetId);
			cu.currentTask = UnitTask::MovingToBuild;
			cu.orderPosiLeft = Cursor::PosF().movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
			cu.orderPosiLeftLast = Cursor::PosF();
			cu.vecMove = (cu.orderPosiLeft - cu.nowPosiLeft).normalized();
			cu.moveState = moveState::MoveAI;
			cu.IsSelect = false;

			IsBattleMove = false;
			abortMyUnits = false;
		}
		// 右クリックドラッグによる範囲選択
		if (MouseR.up() && longBuildSelectTragetId != -1) {
			Point startTile, endTile;

			if (auto startIndex = ToIndex(cursPos, columnQuads, rowQuads)) {
				startTile = *startIndex;
				endTile = *index;

				Array<Point> selectedTiles = getRangeSelectedTiles(startTile, endTile);
				// 新しい移動→建築処理を呼び出し
				processBuildOnTilesWithMovement(selectedTiles);
			}
		}
	}
}
Co::Task<void> Battle::co_handleResourcePointSelection()
{
	//資源ポイントの選択
	if (const auto index = ToIndex(cursPos, columnQuads, rowQuads))
	{
		if (ToTile(*index, N).leftClicked() && IsResourceSelectTraget)//ダブルクリックが良いかも　画面ドラッグを考慮し
		{
			//[index->x]は試して駄目だったので[index->y]に
			//そもそも0番目で良いだろう　どこも横は同じサイズである
			int32 xxx = classBattle.classMapBattle.value().mapData.size();
			int32 yyy = classBattle.classMapBattle.value().mapData[0].size();
			int32 indexX = index->x;
			int32 indexY = index->y;
			if (index->x < 0 || index->y < 0 || index->x >= xxx || index->y >= yyy)
			{
				IsResourceSelectTraget = false;
				co_await Co::NextFrame(); // 範囲外アクセスを防ぐ
			}

			//TODO 閉じるボタン押下時にここでエラー発生　原因・修正はともかく把握しておくこと
			//ユニット選択後に閉じるボタン押下→問題無し
			if (classBattle.classMapBattle.value().mapData[index->x][index->y].isResourcePoint)
			{
				IsBuildSelectTraget = false;
				IsBuildMenuHome = false;
				Unit& cu = GetCU(longBuildSelectTragetId);
				cu.rowResourceTarget = index.value().y;
				cu.colResourceTarget = index.value().x;

				// 移動先の座標算出
				Vec2 nor = ToTileBottomCenter(*index, N);
				// 移動先が有効かどうかチェックは実質上で済んでいる
				cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
				cu.orderPosiLeft = nor.movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
				cu.orderPosiLeftLast = nor;
				cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
				cu.moveState = moveState::MoveAI;
				cu.IsSelect = false;
				cu.currentTask = UnitTask::MovingToResource;
				IsBattleMove = false;
				abortMyUnits = false;

				//taskMyUnits = Async(BattleMoveAStarMyUnits,
				//					std::ref(classBattle.listOfAllUnit),
				//					std::ref(classBattle.listOfAllEnemyUnit),
				//					std::ref(classBattle.classMapBattle.value().mapData),
				//					std::ref(aiRootMy),
				//					std::ref(debugRoot),
				//					std::ref(debugAstar),
				//					std::ref(columnQuads),
				//					std::ref(rowQuads),
				//					N,
				//					std::ref(abortMyUnits),
				//					std::ref(pauseTaskMyUnits)
				//);
			}
		}
		if (ToTile(*index, N).mouseOver())
		{
			int32 xxx = classBattle.classMapBattle.value().mapData.size();
			int32 yyy = classBattle.classMapBattle.value().mapData[0].size();
			int32 indexX = index->x;
			int32 indexY = index->y;
			if (index->x < 0 || index->y < 0 || index->x >= xxx || index->y >= yyy)
			{
				IsResourceSelectTraget = false;
				co_return;
			}
			if (classBattle.classMapBattle.value().mapData[index->x][index->y].isResourcePoint)
				Cursor::RequestStyle(CursorStyle::Hand);
		}
	}
}
Co::Task<void> Battle::processBattlePhase()
{
	co_await checkCancelSelectionByUIArea();

	{
		const auto t = camera.createTransformer();

		//// プレイヤー入力処理全般
		//handleUnitAndBuildingSelection();
		//handleCameraInput();
		//handleRightClickMovementOrSelection();
		//handleBuildTargetSelection();
		//handleResourcePointSelection();
	}

	updateUnitMovements();
	updateUnitHealthBars();
	handleSkillUISelection();
	handleBuildMenuSelectionA();

	co_return;
}
Co::Task<void> Battle::handleRightClickUnitActions(Point start, Point end)
{
	if (IsBattleMove == true)
	{
		if (arrayBattleZinkei[0] == true)
		{
			for (auto& target : classBattle.listOfAllUnit)
				for (auto& unit : target.ListClassUnit)
				{
					Unit& cuu = GetCU(unit.ID);
					cuu.orderPosiLeft = end.movedBy(Random(-10, 10), Random(-10, 10));
					cuu.orderPosiLeftLast = cuu.orderPosiLeft;
					cuu.moveState = moveState::MoveAI;
				}
		}
		else if (arrayBattleZinkei[1] == true)
		{
			ClassHorizontalUnit liZenei;
			liZenei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::F);
			ClassHorizontalUnit liKouei;
			liKouei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::B);
			ClassHorizontalUnit liKihei;
			liKihei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::M);
			Array<ClassHorizontalUnit> lisClassHorizontalUnitLoop;
			lisClassHorizontalUnitLoop.push_back(liZenei);
			lisClassHorizontalUnitLoop.push_back(liKouei);
			lisClassHorizontalUnitLoop.push_back(liKihei);

			for (auto&& [i, loopLisClassHorizontalUnit] : IndexedRef(lisClassHorizontalUnitLoop))
			{
				Array<Unit*> target;
				for (auto& unit : loopLisClassHorizontalUnit.ListClassUnit)
					if (unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable == true)
						target.push_back(&unit);
				if (target.size() == 0) continue;

				AssignUnitsInFormation(target, start, end, i);

				if (target.size() == 1) continue;
				auto pos = calcLastMerge(target, [](const Unit* u) { return u->GetOrderPosiCenter(); });
				auto pos2 = calcLastMerge(target, [](const Unit* u) { return u->GetNowPosiCenter(); });
				setMergePos(target, &Unit::setFirstMergePos, pos2);
				setMergePos(target, &Unit::setLastMergePos, pos);
			}
		}
		else
		{
			//正方
			Array<Unit*> target;
			auto groups = GetMovableUnitGroups();
			for (auto&& [i, group] : Indexed(groups))
			{
				AssignUnitsInFormation(group, start, end, i);
				target.append(group);
			}
			if (target.size() > 1)
			{
				auto pos = calcLastMerge(target, [](const Unit* u) { return u->GetOrderPosiCenter(); });
				auto pos2 = calcLastMerge(target, [](const Unit* u) { return u->GetNowPosiCenter(); });
				setMergePos(target, &Unit::setFirstMergePos, pos2);
				setMergePos(target, &Unit::setLastMergePos, pos);
			}
		}

		IsBattleMove = false;

		abortMyUnits = false;
	}
	else
	{
		////範囲選択

		// 範囲選択の矩形を生成（start, endの大小関係を吸収）
		const RectF selectionRect = RectF::FromPoints(start, end);
		for (auto& target : classBattle.listOfAllUnit)
		{
			for (auto& unit : target.ListClassUnit)
			{
				if (unit.IsBuilding) continue;

				const Vec2 gnpc = unit.GetNowPosiCenter();
				const bool inRect = selectionRect.intersects(gnpc);

				if (inRect)
				{
					GetCU(unit.ID).moveState = moveState::FlagMoveCalc;
					IsBattleMove = true;
				}
			}
		}
	}

	co_return;
}
/// @brief 建築予約をするのが本質
/// @param unit 
void Battle::afterMovedPushToBuildMenu(Unit& itemUnit)
{
	if (itemUnit.classBuild == U"") return;//そもそも建設出来ないはず

	const auto& index = ToIndex(itemUnit.GetNowPosiCenter(), columnQuads, rowQuads);
	itemUnit.tempIsBuildSelectTragetBuildAction.rowBuildingTarget = index->y;
	itemUnit.tempIsBuildSelectTragetBuildAction.colBuildingTarget = index->x;
	itemUnit.IsSelect = false;
	itemUnit.moveState = moveState::None;
	itemUnit.currentTask = UnitTask::None;

	IsBuildSelectTraget = false;

	//Battle::updateBuildQueueで作る
	if (itemUnit.taskTimer.isRunning() == false)
	{
		itemUnit.progressTime = 0.0;
		itemUnit.taskTimer.restart();
	}
	itemUnit.arrYoyakuBuild.push_back(itemUnit.tempIsBuildSelectTragetBuildAction);
	// 回数制限の更新と再描画
	if (itemUnit.tempIsBuildSelectTragetBuildAction.buildCount > 0)
	{
		htBuildMenu[itemUnit.classBuild + U"-" + itemUnit.tempIsBuildSelectTragetBuildAction.id].buildCount--;
		//キーだけ渡して該当のrenderだけ更新するように
		//renB();
	}
}
void Battle::addResource(Unit& unit)
{
	classBattle.classMapBattle.value().mapData[unit.colResourceTarget][unit.rowResourceTarget]
		.whichIsThePlayer = BattleWhichIsThePlayer::Sortie;
	classBattle.classMapBattle.value().mapData[unit.colResourceTarget][unit.rowResourceTarget]
		.resourcePointAmount += 7;
	unit.taskTimer.reset();
	unit.currentTask = UnitTask::None;
	Array<ResourcePointTooltip::TooltipTarget> resourceTargets;
	SetResourceTargets(resourceTargets);
	resourcePointTooltip.setTargets(resourceTargets);
}
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
Co::Task<void> Battle::mainLoop()
{
	const auto _tooltip = resourcePointTooltip.playScoped();

	while (true)
	{
		if (shouldExit == false)
			co_return;

		camera.update();
		resourcePointTooltip.setCamera(camera);

		spawnTimedEnemy();
		updateResourceIncome();
		// ここを変更
		if (fogUpdateTimer.sF() >= 0.5)
		{
			refreshFogOfWar();
			fogUpdateTimer.restart();
		}
		////後でbattle内に移動(ポーズ処理を考慮
		updateBuildQueue();
		try {
			// 移動処理 - 安全なインデックスベースループ
			for (size_t groupIndex = 0; groupIndex < classBattle.listOfAllUnit.size(); ++groupIndex)
			{
				auto& item = classBattle.listOfAllUnit[groupIndex];

				// 各フレームでサイズを再取得（動的変更に対応）
				for (size_t unitIndex = 0; unitIndex < item.ListClassUnit.size(); ++unitIndex)
				{
					auto& unit = item.ListClassUnit[unitIndex];
					// 安全な参照チェック
					auto safeUnit = SafeUnitManager::GetSafeUnit(unit.ID);
					if (!safeUnit) continue;

					switch (safeUnit->currentTask)
					{
					case UnitTask::MovingToBuild:
					{
						std::scoped_lock lock(aiRootMutex);
						if (aiRootMy[safeUnit->ID].getPath().isEmpty())
						{
							safeUnit->currentTask = UnitTask::None;
							//afterMovedPushToBuildMenu(unit);
											// 移動完了→建築実行→次のタスクチェック
							afterMovedPushToBuildMenuAdvanced(*safeUnit);
						}
					}
					break;
					case UnitTask::MovingToResource:
					{
						std::scoped_lock lock(aiRootMutex);
						if (aiRootMy[safeUnit->ID].getPath().isEmpty())
						{
							safeUnit->currentTask = UnitTask::WorkingOnResource;
							safeUnit->taskTimer.restart();
						}
					}
					break;

					case UnitTask::WorkingOnResource:
						if (safeUnit->taskTimer.sF() >= safeUnit->waitTimeResource)
						{
							addResource(*safeUnit);
						}
						break;

					default:
						break;
					}

				}
			}
		}
		catch (const std::exception& e) {
		}
		catch (...) {
		}

		switch (battleStatus)
		{
		case BattleStatus::Battle:
		{
			co_await checkCancelSelectionByUIArea();

			//カメラ移動 || 部隊を選択状態にする。もしくは既に選択状態なら移動させる
			{
				const auto t = camera.createTransformer();

				handleUnitAndBuildingSelection();
				handleCameraInput();

				if (MouseR.up())
				{
					Point start = cursPos;
					Point end = Cursor::Pos();

					//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
					co_await handleRightClickUnitActions(start, end);
				}

				//建物の建築場所を選択する処理
				//ここに存在するのは、MouseL.pressedで建築メニュー押下時に続けて処理されてしまう為
				handleBuildTargetSelection();
				co_await co_handleResourcePointSelection();
			}

			//陣形処理
			{
				const Transformer2D transformer{ Mat3x2::Identity(),
					Mat3x2::Translate(0,Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight) };

				for (auto&& [j, ttt] : Indexed(rectZinkei))
				{
					if (ttt.leftClicked())
					{
						arrayBattleZinkei.clear();
						for (size_t k = 0; k < rectZinkei.size(); k++)
						{
							arrayBattleZinkei.push_back(false);
						}
						arrayBattleZinkei[j] = true;

						renderTextureZinkei.clear(ColorF{ 0.5, 0.0 });
						{
							const ScopedRenderTarget2D target{ renderTextureZinkei.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
							const ScopedRenderStates2D blend{ MakeBlendState() };

							Rect df = Rect(320, 60);
							df.drawFrame(4, 0, ColorF{ 0.5 });

							for (auto&& [i, ttt] : Indexed(rectZinkei))
							{
								ttt.draw(Palette::Aliceblue);
								if (arrayBattleZinkei[i] == true)
									ttt.drawFrame(4, 0, Palette::Red);
								fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
							}
						}

					}
				}
			}


			updateUnitMovements();
			updateUnitHealthBars();
			handleSkillUISelection();
			handleBuildMenuSelectionA();
		}
		break;
		case BattleStatus::Message:
			break;
		case BattleStatus::Event:
			break;
		default:
			break;
		}

		// 非同期タスクが完了したら
		if (task.isReady()) {}
		if (taskMyUnits.isReady()) {}

		co_await Co::NextFrame();
	}
}

RectF Battle::getCameraView() const
{
	int32 testPadding = -TileOffset.x;
	return RectF{
		camera.getCenter() - (Scene::Size() / 2.0) / camera.getScale(),
		Scene::Size() / camera.getScale()
	}.stretched(-testPadding);
}
void Battle::drawTileMap(const RectF& cameraView) const
{
	for (int32 i = 0; i < (N * 2 - 1); ++i)
	{
		int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
		int32 yi = (i < (N - 1)) ? i : (N - 1);

		for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			Vec2 pos = ToTileBottomCenter(index, N);
			if (!cameraView.intersects(pos))
				continue;

			const auto& tile = classBattle.classMapBattle.value().mapData[index.x][index.y];
			TextureAsset(tile.tip + U".png").draw(Arg::bottomCenter = pos);
		}
	}
}
void Battle::drawFog(const RectF& cameraView) const
{
	for (int32 i = 0; i < (N * 2 - 1); ++i)
	{
		int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
		int32 yi = (i < (N - 1)) ? i : (N - 1);

		for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			switch (visibilityMap[index])
			{
			case Visibility::Unseen:
				ToTile(index, N).draw(ColorF{ 0.0, 0.6 });
				break;
			case Visibility::Visible:
				break;
			}
		}
	}
}
void Battle::drawBuildings(const RectF& cameraView) const
{
	Array<Unit> buildings;
	for (const auto& group : { classBattle.listOfAllUnit, classBattle.listOfAllEnemyUnit })
	{
		for (const auto& item : group)
		{
			if (item.FlagBuilding)
			{
				for (const auto& u : item.ListClassUnit)
					buildings.push_back(u);
			}
		}
	}

	for (const auto& u : buildings)
	{
		if (!u.IsBattleEnable)
			continue;
		Vec2 pos = ToTileBottomCenter(Point(u.colBuilding, u.rowBuilding), N);
		if (!cameraView.intersects(pos))
			continue;

		TextureAsset(u.ImageName).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness));
		if (u.IsSelect)
			RectF(Arg::bottomCenter = pos.movedBy(0, -TileThickness), TextureAsset(u.ImageName).size()).drawFrame(3.0, Palette::Red);
	}
}
void Battle::drawUnits(const RectF& cameraView) const
{
	auto drawGroup = [&](const Array<ClassHorizontalUnit>& group, const String& ringA, const String& ringB)
		{
			for (const auto& item : group)
			{
				if (item.FlagBuilding || item.ListClassUnit.empty())
					continue;

				for (const auto& u : item.ListClassUnit)
				{
					if (!u.IsBattleEnable) continue;

					const Vec2 center = u.GetNowPosiCenter();

					// 画面外なら描画しない
					if (!cameraView.intersects(center))
						continue;

					if (!u.IsBuilding)
						TextureAsset(ringA).drawAt(center.movedBy(0, 8));

					TextureAsset(u.ImageName).draw(Arg::center = center);

					if (u.IsSelect)
						TextureAsset(u.ImageName).draw(Arg::center = center).drawFrame(3.0, Palette::Red);
					if (!u.IsBuilding)
						TextureAsset(ringB).drawAt(center.movedBy(0, 16));

					if (!u.IsBuilding)
					{
						u.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}
					else
					{
						u.bLiquidBarBattle.draw(ColorF{ 0.5, 0.1, 1.0 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}

					// --- ここから追加 ---
					// moveStateがFlagMoveCalcなら「！」を表示
					if (u.moveState == moveState::FlagMoveCalc)
					{
						const Vec2 exclamationPos = center.movedBy(0, -u.TakasaUnit / 2 - 18);
						font(U"！").drawAt(exclamationPos, Palette::Orange);
						// 例: FontAsset(U"SystemFont_Exclamation")は事前に登録しておく
					}
					else if (u.moveState == moveState::MoveAI)
					{
						const Vec2 exclamationPos = center.movedBy(0, -u.TakasaUnit / 2 - 18);
						font(U"！").drawAt(exclamationPos, Palette::Red);
					}
					// --- ここまで追加 ---
				}
			}
		};

	drawGroup(classBattle.listOfAllUnit, U"ringA.png", U"ringB.png");
	drawGroup(classBattle.listOfAllEnemyUnit, U"ringA_E.png", U"ringB_E.png");
}
void Battle::drawHealthBars() const
{
	auto drawBars = [](const Array<ClassHorizontalUnit>& group)
		{
			for (const auto& item : group)
			{
				if (item.FlagBuilding || item.ListClassUnit.empty())
					continue;

				for (const auto& u : item.ListClassUnit)
				{
					if (u.IsBattleEnable)
						u.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
				}
			}
		};
	drawBars(classBattle.listOfAllUnit);
	drawBars(classBattle.listOfAllEnemyUnit);
}
void Battle::drawSelectionRectangleOrArrow() const
{
	if (!MouseR.pressed())
		return;

	if (!IsBattleMove)
	{
		const double thickness = 3.0;
		double offset = Scene::DeltaTime() * 10;
		const Rect rect{ cursPos, Cursor::Pos() - cursPos };
		rect.top().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.right().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.bottom().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.left().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
	}
	else
	{
		Line{ cursPos, Cursor::Pos() }.drawArrow(10, Vec2{ 40, 80 }, Palette::Orange);
	}
}
void Battle::drawSkillUI() const
{
	const int32 baseY = Scene::Size().y - renderTextureSkill.height() - underBarHeight;

	renderTextureSkill.draw(0, baseY);
	renderTextureSkillUP.draw(0, baseY);

	if (!nowSelectSkillSetumei.isEmpty())
	{
		rectSkillSetumei.draw(Palette::Black);
		fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Palette::White);
	}
}
void Battle::drawBuildMenu() const
{
	if (!IsBuildMenuHome)
	{
		renderTextureBuildMenuEmpty.draw(Scene::Size().x - 328, Scene::Size().y - 328 - underBarHeight);
		return;
	}

	const int32 baseX = Scene::Size().x - 328;
	const int32 baseY = Scene::Size().y - 328 - underBarHeight;
	const Rect bbb = Rect(baseX - 64 - 6, baseY, 70, 328).drawFrame(4, 0, Palette::Black);

	String targetClassBuild = U"";
	for (auto& item : classBattle.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.IsSelect == false) continue;
			targetClassBuild = itemUnit.classBuild;
		}
	}
	for (auto&& [i, re] : Indexed(htBuildMenuRenderTexture))
	{
		if (re.first != targetClassBuild) continue;
		re.second.draw(baseX, baseY);
	}

	for (auto& item : classBattle.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.IsSelect == false) continue;
			for (auto&& [i, uybuy] : Indexed(itemUnit.arrYoyakuBuild))
			{
				if (i == 0)
				{
					TextureAsset(uybuy.icon).resized(64).draw(baseX - 64, baseY + 4);
					double progressRatio = Saturate(itemUnit.taskTimer.sF() / uybuy.buildTime); // 0.0 ～ 1.0 に制限
					double gaugeRatio = Max(progressRatio, 0.1);
					double gaugeHeight = 64 * gaugeRatio;

					RectF{ baseX - 64, baseY + 4, 64, gaugeHeight }
					.draw(ColorF{ 0.0, 0.5 });
				}
				else
				{
					TextureAsset(uybuy.icon).resized(32).draw(baseX - 32, baseY + 32 + (i * 32) + 4);
				}
			}
		}
	}
}
void Battle::drawResourcesUI() const
{
	const String goldText = U"Gold:{0}"_fmt(gold);
	const String trustText = U"Trust:{0}"_fmt(trust);
	const String foodText = U"Food:{0}"_fmt(food);

	int32 baseX = 0;
	int32 baseY = 0;

	if (longBuildSelectTragetId != -1)
	{
		// ビルド選択中 → 右下寄せに変更
		baseX = Scene::Size().x - 328 - int32(systemFont(goldText).region().w) - 64 - 6;
		baseY = Scene::Size().y - 328 - 30;
	}

	const Array<String> texts = { goldText, trustText, foodText };

	for (size_t i = 0; i < texts.size(); ++i)
	{
		const String& text = texts[i];
		const auto region = systemFont(text).region();
		Rect rect{ baseX, baseY + static_cast<int32>(i * region.h), static_cast<int32>(region.w), static_cast<int32>(region.h) };

		rect.draw(Palette::Black);
		systemFont(text).drawAt(rect.center(), Palette::White);
	}
}
////void Battle::drawBuildTargetHighlight() const
////{
////	if (IsBuildSelectTraget)
////	{
////		if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
////		{
////			// マウスカーソルがあるタイルを強調表示する
////			ToTile(*index, N).draw(ColorF{ 1.0, 0.2 });
////		}
////	}
////}
//void Battle::drawBuildTargetHighlight() const
//{
//	if (IsBuildSelectTraget)
//	{
//		if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
//		{
//			// 右クリック範囲選択中の処理
//			if (MouseR.pressed() && longBuildSelectTragetId != -1)
//			{
//				// 開始点の取得
//				if (auto startIndex = ToIndex(cursPos, columnQuads, rowQuads))
//				{
//					Point startTile = *startIndex;
//					Point endTile = *index;
//
//					// 範囲選択されるタイル一覧を取得
//					Array<Point> previewTiles;
//
//					// 開始点と終了点から矩形範囲を計算
//					int32 minX = Min(startTile.x, endTile.x);
//					int32 maxX = Max(startTile.x, endTile.x);
//					int32 minY = Min(startTile.y, endTile.y);
//					int32 maxY = Max(startTile.y, endTile.y);
//
//					// 矩形範囲内の全タイルを配列に追加
//					for (int32 x = minX; x <= maxX; ++x)
//					{
//						for (int32 y = minY; y <= maxY; ++y)
//						{
//							// マップ範囲内かチェック
//							if (x >= 0 && x < N && y >= 0 && y < N)
//							{
//								previewTiles.push_back(Point(x, y));
//							}
//						}
//					}
//
//					// 範囲選択プレビューの描画
//					for (const auto& tile : previewTiles)
//					{
//						// 建築可能かどうかで色を変える
//						ColorF tileColor = canBuildOnTile(tile) ?
//							ColorF{ 0.0, 1.0, 0.2, 0.4 } :  // 緑半透明：建築可能
//							ColorF{ 1.0, 0.2, 0.2, 0.4 };   // 赤半透明：建築不可
//
//						ToTile(tile, N).draw(tileColor);
//					}
//
//					// 範囲選択の枠線を描画
//					{
//						const auto t = camera.createTransformer();
//						const double thickness = 2.0;
//						const double offset = Scene::DeltaTime() * 8;
//
//						// 開始タイルと終了タイルの座標を取得
//						Vec2 startPos = ToTileBottomCenter(startTile, N);
//						Vec2 endPos = ToTileBottomCenter(endTile, N);
//
//						// 矩形の四隅を計算
//						const Rect selectionRect = Rect::FromPoints(startPos.asPoint(), endPos.asPoint());
//
//						// 点線で枠を描画
//						selectionRect.top().draw(LineStyle::SquareDot(offset), thickness, ColorF{ 1.0, 1.0, 0.0, 0.8 });
//						selectionRect.right().draw(LineStyle::SquareDot(offset), thickness, ColorF{ 1.0, 1.0, 0.0, 0.8 });
//						selectionRect.bottom().draw(LineStyle::SquareDot(offset), thickness, ColorF{ 1.0, 1.0, 0.0, 0.8 });
//						selectionRect.left().draw(LineStyle::SquareDot(offset), thickness, ColorF{ 1.0, 1.0, 0.0, 0.8 });
//					}
//
//					// 選択予定数の表示
//					{
//						int32 validCount = 0;
//						int32 totalCount = previewTiles.size();
//
//						for (const auto& tile : previewTiles)
//						{
//							if (canBuildOnTile(tile)) validCount++;
//						}
//
//						// カーソル近くに情報表示
//						const String infoText = U"選択: {}/{} タイル"_fmt(validCount, totalCount);
//						const Vec2 textPos = Cursor::PosF().movedBy(20, -30);
//
//						// 背景を描画
//						const auto textRegion = font(infoText).region();
//						const RectF bgRect = RectF(textPos, textRegion.size).stretched(4);
//						bgRect.draw(ColorF{ 0.0, 0.0, 0.0, 0.7 });
//
//						// テキストを描画
//						font(infoText).draw(textPos, Palette::White);
//					}
//				}
//			}
//			else
//			{
//				// 通常の単一タイルハイライト
//				ToTile(*index, N).draw(ColorF{ 1.0, 1.0, 0.2, 0.3 });
//			}
//		}
//	}
//}
void Battle::drawBuildTargetHighlight() const
{
	if (IsBuildSelectTraget)
	{
		if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
		{
			// 右クリック範囲選択中の処理
			if (MouseR.pressed() && longBuildSelectTragetId != -1)
			{
				// 開始点の取得
				if (auto startIndex = ToIndex(cursPos, columnQuads, rowQuads))
				{
					Point startTile = *startIndex;
					Point endTile = *index;

					// **線状選択のプレビュー**
					Array<Point> previewTiles = getRangeSelectedTiles(startTile, endTile);

					// 選択方向の表示
					int32 deltaX = endTile.x - startTile.x;
					int32 deltaY = endTile.y - startTile.y;
					bool isHorizontal = Abs(deltaX) >= Abs(deltaY);

					String directionText = isHorizontal ? U"水平線" : U"垂直線";
					ColorF lineColor = isHorizontal ?
						ColorF{ 0.0, 0.8, 1.0, 0.6 } :  // 水平：シアン
						ColorF{ 1.0, 0.8, 0.0, 0.6 };   // 垂直：オレンジ

					// 線状選択プレビューの描画
					for (const auto& tile : previewTiles)
					{
						// 建築可能かどうかで色を調整
						ColorF tileColor = canBuildOnTile(tile) ?
							lineColor :  // 元の方向色
							ColorF{ 1.0, 0.2, 0.2, 0.4 };   // 赤半透明：建築不可

						ToTile(tile, N).draw(tileColor);
					}

					// 方向線の描画（開始点→終了点）
					{
						const double thickness = 3.0;

						Vec2 startPos = ToTileBottomCenter(startTile, N);
						Vec2 endPos = ToTileBottomCenter(endTile, N);

						// 方向を示すアロー
						Line{ startPos, endPos }.drawArrow(
							thickness,
							Vec2{ 20, 40 },
							ColorF{ 1.0, 1.0, 0.0, 0.9 }
						);
					}

					// 選択情報の表示
					{
						int32 validCount = 0;
						int32 totalCount = previewTiles.size();

						for (const auto& tile : previewTiles)
						{
							if (canBuildOnTile(tile)) validCount++;
						}

						// カーソル近くに情報表示
						const String infoText = U"{}: {}/{} タイル"_fmt(
							directionText, validCount, totalCount);
						const Vec2 textPos = Cursor::PosF().movedBy(20, -30);

						// 背景を描画
						const auto textRegion = font(infoText).region();
						const RectF bgRect = RectF(textPos, textRegion.size).stretched(4);
						bgRect.draw(ColorF{ 0.0, 0.0, 0.0, 0.8 });

						// テキストを描画
						fontSkill(infoText).draw(textPos, Palette::White);
					}
				}
			}
			else
			{
				// 通常の単一タイルハイライト
				ToTile(*index, N).draw(ColorF{ 1.0, 1.0, 0.2, 0.3 });
			}
		}
	}
}
void Battle::drawBuildDescription() const
{
	if (nowSelectBuildSetumei != U"")
	{
		rectSetumei.draw(Palette::Black);
		fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Palette::White);
	}
}
void Battle::drawResourcePoints(const RectF& cameraView) const
{
	if (!classBattle.classMapBattle)
		return;

	const auto& mapData = classBattle.classMapBattle.value().mapData;

	const int32 mapSize = static_cast<int32>(mapData.size());

	for (int32 x = 0; x < mapSize; ++x)
	{
		for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
		{
			const auto& tile = mapData[x][y];

			if (!tile.isResourcePoint)
				continue;

			const Vec2 pos = ToTileBottomCenter(Point(x, y), N);

			if (!cameraView.intersects(pos))
				continue;

			// アイコンの描画
			TextureAsset(tile.resourcePointIcon).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness));

			// 所有者に応じて円枠の色を変える
			const ColorF circleColor = (tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				? ColorF{ Palette::Aqua }
			: ColorF(Palette::Red);

			Circle(pos.movedBy(0, -TileThickness - TileOffset.y), 16).drawFrame(4, 0, circleColor);
		}
	}
}
void Battle::draw() const
{
	FsScene::draw();

	{
		// 2D カメラによる座標変換を適用する
		const auto tr = camera.createTransformer();
		// 乗算済みアルファ用のブレンドステートを適用する
		const ScopedRenderStates2D blend{ BlendState::Premultiplied };
		const RectF cameraView = getCameraView();

		drawTileMap(cameraView);
		drawFog(cameraView);
		drawBuildings(cameraView);
		drawUnits(cameraView);
		drawResourcePoints(cameraView);
		resourcePointTooltip.draw();
		drawSelectionRectangleOrArrow();
		drawBuildTargetHighlight();
	}

	renderTextureZinkei.draw(0, Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight);
	drawSkillUI();
	drawBuildDescription();
	drawBuildMenu();
	drawResourcesUI();

	if (longBuildSelectTragetId == -1)
		DrawMiniMap(grid, camera.getRegion());
}

//ヘルパーメソッド

void Battle::UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize) const
{
	for (const auto& unit : units)
	{
		for (int dy = -unit.visionRadius; dy <= unit.visionRadius; ++dy)
		{
			for (int dx = -unit.visionRadius; dx <= unit.visionRadius; ++dx)
			{
				Vec2 pos = unit.GetNowPosiCenter();
				if (const auto index = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads))
				{
					Point p = index.value() + Point{ dx, dy };
					if (InRange(p.x, 0, mapSize - 1) && InRange(p.y, 0, mapSize - 1))
						if (p.manhattanDistanceFrom(index.value()) <= unit.visionRadius)
							vis[p] = Visibility::Visible;
				}
			}
		}
	}
}
/// @brief タイルのインデックスから、タイルの底辺中央の座標を計算します。
/// @param index タイルのインデックス
/// @param N マップの一辺のタイル数
/// @return タイルの底辺中央の座標
Vec2 Battle::ToTileBottomCenter(const Point& index, const int32 N) const
{
	const int32 i = index.manhattanLength();
	const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	const int32 yi = (i < (N - 1)) ? i : (N - 1);
	const int32 k = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
	const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	const double posY = (i * TileOffset.y);
	return{ (posX + TileOffset.x * 2 * k), posY };
}
Vec2 Battle::ToTileBottomCenterTTT(const Point& index, const int32 N) const
{
	// index は (x, y) グリッド上のタイル位置
	const double posX = (index.x - index.y) * TileOffset.x;
	const double posY = (index.x + index.y) * TileOffset.y;

	return Vec2(posX, posY);
}
/// @brief タイルのインデックスから、タイルの四角形を計算します。
/// @param index タイルのインデックス
/// @param N マップの一辺のタイル数
/// @return タイルの四角形
Quad Battle::ToTile(const Point& index, const int32 N) const
{
	const Vec2 bottomCenter = ToTileBottomCenter(index, N);

	return Quad{
		bottomCenter.movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		bottomCenter.movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		bottomCenter.movedBy(0, -TileThickness),
		bottomCenter.movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
	};
}
/// @brief 指定した列のタイルによって構成される四角形を計算します。
/// @param x 列インデックス
/// @param N マップの一辺のタイル数
/// @return 指定した列のタイルによって構成される四角形
Quad Battle::ToColumnQuad(const int32 x, const int32 N) const
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
Quad Battle::ToRowQuad(const int32 y, const int32 N) const
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
Array<Quad> Battle::MakeColumnQuads(const int32 N) const
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
Array<Quad> Battle::MakeRowQuads(const int32 N) const
{
	Array<Quad> quads;

	for (int32 y = 0; y < N; ++y)
	{
		quads << ToRowQuad(y, N);
	}

	return quads;
}

/// @brief 画像を読み込み、アルファ乗算済みのテクスチャを作成します。
/// @param path 画像ファイルのパス
/// @return アルファ乗算済みのテクスチャ
/// @remark 境界付近の品質を向上させるため、アルファ乗算済みのテクスチャを作成します。
/// @remark 描画時は `BlendState::Premultiplied` を指定してください。
[[nodiscard]]
Texture Battle::LoadPremultipliedTexture(FilePathView path)
{
	Image image{ path };
	Color* p = image.data();
	const Color* const pEnd = (p + image.num_pixels());
	while (p != pEnd)
	{
		p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
		p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
		p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
		++p;
	}
	return Texture{ image };
}

/// @brief 
/// @param ID 
/// @return 
Unit& Battle::GetCU(long ID)
{
	for (auto& temp : classBattle.listOfAllUnit)
		for (auto& temptemp : temp.ListClassUnit)
			if (temptemp.ID == ID)
				return temptemp;
}
/// @brief IDからユニットを安全に取得（Optional版）
/// @param ID ユニットID
/// @return ユニットのポインタ（見つからない場合はnullptr）
Unit* Battle::GetCUSafe(long ID)
{
	for (auto& temp : classBattle.listOfAllUnit)
	{
		for (auto& temptemp : temp.ListClassUnit)
		{
			if (temptemp.ID == ID)
				return &temptemp;
		}
	}
	return nullptr;
}
/// @brief 全ユニットから「移動可能なユニット」だけを抽出して部隊ごとにまとめる
/// @return 
Array<Array<Unit*>> Battle::GetMovableUnitGroups()
{
	Array<Array<Unit*>> groups;

	for (auto& target : classBattle.listOfAllUnit)
	{
		Array<Unit*> group;

		for (auto& unit : target.ListClassUnit)
		{
			if (unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable)
				group.push_back(&unit);
		}

		if (!group.isEmpty())
			groups.push_back(group);
	}

	return groups;
}
ClassHorizontalUnit Battle::getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf)
{
	ClassHorizontalUnit result;

	for (auto& target : source)
		for (auto& unit : target.ListClassUnit)
		{
			if (unit.Formation == bf && unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable == true)
				result.ListClassUnit.push_back(unit);
		}

	return result;
}

/// @brief 指定されたユニットの部隊に対して、指定された開始位置と終了位置に沿ってユニットを配置します。
/// @param units 
/// @param start 
/// @param end 
/// @param rowIndex 
void Battle::AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex)
{
	const int32 count = units.size();
	const int32 centerOffset = (count - 1) / 2;

	double angleForward = (start == end) ? 0.0 : Math::Atan2(end.y - start.y, end.x - start.x);
	double anglePerpendicular = Math::Pi / 2 - angleForward;

	for (auto&& [i, unit] : Indexed(units))
	{
		// 変数名はcopilot君
		double cosPerpendicular = Math::Cos(anglePerpendicular);
		double sinPerpendicular = Math::Sin(anglePerpendicular);

		double distance_between_units_cos = Math::Round(DistanceBetweenUnit * cosPerpendicular);
		int32 unit_spacing_offset_factor = (i - centerOffset);
		int32 unit_spacing_offset_x = unit_spacing_offset_factor * distance_between_units_cos;
		int32 unit_spacing_offset_y = unit_spacing_offset_factor * Math::Round(DistanceBetweenUnit * sinPerpendicular);

		double rowOffsetX = rowIndex * DistanceBetweenUnitTate * Math::Cos(angleForward);
		double rowOffsetY = rowIndex * DistanceBetweenUnitTate * Math::Sin(angleForward);

		double x = end.x + unit_spacing_offset_x - rowOffsetX;
		double y = end.y - unit_spacing_offset_y - rowOffsetY;

		Unit& cu = GetCU(unit->ID);
		cu.orderPosiLeft = Vec2(Floor(end.x), Floor(end.y)).movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
		cu.orderPosiLeftLast = cu.orderPosiLeft = Vec2(Floor(x), Floor(y)).movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
		cu.moveState = moveState::MoveAI;
	}
}

Vec2 Battle::calcLastMerge(const Array<Unit*>& units, std::function<Vec2(const Unit*)> getPos)
{
	Vec2 sum = Vec2::Zero();
	int count = 0;
	for (const auto* u : units) {
		sum += getPos(u);
		++count;
	}
	return (count > 0) ? (sum / count) : Vec2::Zero();
}
void Battle::setMergePos(const Array<Unit*>& units, void (Unit::* setter)(const Vec2&), const Vec2& setPos)
{
	for (auto* u : units) {
		(u->*setter)(setPos);
	}
}
