#include "stdafx.h"
#include "AStar.h"
#include "ClassUnit.h"
#include "ClassMapBattle.h"

Optional<ClassAStar*> AStar::SearchMinScore(const Array<ClassAStar*>& ls) {
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

int32 AStar::BattleMoveAStar(std::mutex& unitListMutex,
	Array<ClassHorizontalUnit>& target,
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
	std::scoped_lock lock(unitListMutex);
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

	// A*探索において、攻撃対象として有効なユニットかどうかを判定するラムダ式
	const auto isValidTarget = [](const Unit& unit) -> bool
		{
			// 特定の建物（壁や門）はターゲットにしない
			if (unit.IsBuilding && (unit.mapTipObjectType == MapTipObjectType::WALL2
				|| unit.mapTipObjectType == MapTipObjectType::GATE))
				return false;

			// 戦闘不能なユニットはターゲットにしない
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

		// 現状は「最も近い敵」をターゲットとする
		HashTable<double, Unit> dicDis;
		Vec2 posA = unit.GetNowPosiCenter();
		auto uisuif = target;
		for (const auto& ccc : uisuif) {
			for (const auto& ddd : ccc.ListClassUnit) {
				if (!isValidTarget(ddd)) continue;

				Vec2 posB = ddd.GetNowPosiCenter();
				double dist = posA.distanceFrom(posB);
				// 距離がキーとなるため、同一距離のユニットが複数いるとハッシュテーブルで上書きされてしまう。
				// それを防ぐために、キーが重複した場合は微小な値を加えてユニークにする。
				while (dicDis.contains(dist)) {
					dist += 0.0001;
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
		// ユニットが持つ退却範囲(escape_range)に入られた場合、敵から離れるように移動する
		if (unit.Escape_range >= 1)
		{
			Circle cCheck = Circle(unit.GetNowPosiCenter(), unit.Escape_range);
			Circle cCheck2 = Circle(minElement->second.GetNowPosiCenter(), 1);
			if (cCheck.intersects(cCheck2) == true)
			{
				// 敵とは反対方向の座標を計算
				double angle = atan2(minElement->second.GetNowPosiCenter().y - unit.GetNowPosiCenter().y,
					minElement->second.GetNowPosiCenter().x - unit.GetNowPosiCenter().x);
				double xC, yC;
				// 反対方向に進むために角度を180度反転
				angle += Math::Pi;
				xC = unit.GetNowPosiCenter().x + RETREAT_DISTANCE * cos(angle);
				yC = unit.GetNowPosiCenter().y + RETREAT_DISTANCE * sin(angle);

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

		Array<Point> listRoot = findPath(nowIndex.value(), nowIndexEnemy.value(), mapData, enemy, target, mapTile, hsBuildingUnitForAstar, abort, false);

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

Array<Point> AStar::findPath(const Point& start,
	const Point& end,
	Array<Array<MapDetail>>& mapData,
	Array<ClassHorizontalUnit>& enemy,
	Array<ClassHorizontalUnit>& target,
	MapTile& mapTile,
	HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
	const std::atomic<bool>& abort,
	bool isMyUnit)
{
	ClassAStarManager classAStarManager(end.x, end.y);
	Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(start.x, start.y, 0, nullptr, mapTile.N);
	Array<Point> listRoot;

	Stopwatch taskTimer;
	taskTimer.restart();

	while (true)
	{
		// 探索に時間がかかりすぎる場合、タイムアウトして探索を打ち切る
		if (taskTimer.s() > SEARCH_TIMEOUT_S) break;
		if (abort) break;

		if (!startAstar)
		{
			listRoot.clear();
			break;
		}

		classAStarManager.OpenAround(startAstar.value(),
			mapData,
			enemy,
			target,
			mapTile.N,
			hsBuildingUnitForAstar,
			isMyUnit
		);

		startAstar.value()->SetAStarStatus(AStarStatus::Closed);
		classAStarManager.RemoveClassAStar(startAstar.value());

		if (classAStarManager.GetListClassAStar().size() != 0)
		{
			startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
		}
		else
		{
			// No path found
			listRoot.clear();
			break;
		}

		if (!startAstar) continue;

		// Reached destination
		if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
		{
			startAstar.value()->GetRoot(listRoot);
			listRoot.reverse();
			classAStarManager.Clear();
			break;
		}
	}
	return listRoot;
}


int32 AStar::BattleMoveAStarMyUnitsKai(std::mutex& unitListMutex,
	Array<ClassHorizontalUnit>& target,
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
	std::scoped_lock lock(unitListMutex);
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
		Array<Point> fullPath = findPath(startIndex.value(), endIndex.value(), mapData, enemy, target, mapTile, hsBuildingUnitForAstar, abort, true);
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

	Array<Point> pathShare = findPath(startIndex.value(), endIndex.value(), mapData, enemy, target, mapTile, hsBuildingUnitForAstar, abort, true);

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
				firstPath = findPath(nowIndex.value(), nowIndexFirstGoal.value(), mapData, enemy, target, mapTile, hsBuildingUnitForAstar, abort, true);
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
				endPath = findPath(nowIndex.value(), nowIndexEndGoal.value(), mapData, enemy, target, mapTile, hsBuildingUnitForAstar, abort, true);
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
