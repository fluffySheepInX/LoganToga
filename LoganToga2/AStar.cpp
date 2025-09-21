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

int32 AStar::BattleMoveAStar(
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
	constexpr size_t MaxUnitsPerFrame = 50;

	size_t processed = 0;
	Array<Unit*> flatList;

	// --- フラット化：FlagMoveAI が立っている敵ユニットのみ抽出
	{
		std::shared_lock lock(unitListRWMutex);
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

		Array<ClassHorizontalUnit> snapshot;
		{
			std::shared_lock lock(unitListRWMutex);
			snapshot = target; // ディープコピー or シャローコピー
		}

		for (const auto& ccc : snapshot) {
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

Unit* AStar::GetCUSafe(long ID, Array<ClassHorizontalUnit>& listOfAllUnit)
{
	std::shared_lock lock(unitListRWMutex);
	for (auto& temp : listOfAllUnit)
	{
		for (auto& temptemp : temp.ListClassUnit)
		{
			if (temptemp.ID == ID)
				return &temptemp;
		}
	}
	return nullptr;
}

int32 AStar::BattleMoveAStarMyUnitsKai(
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
	Array<int32> flatList;
	Array<ClassHorizontalUnit> snapshot;
	{
		std::shared_lock lock(unitListRWMutex);
		// 深いコピーを作成してからループ処理
		snapshot = target;
	}

	for (auto& group : snapshot)
	{
		for (auto& unit : group.ListClassUnit)
		{
			if (!unit.IsBattleEnable || unit.IsBuilding) continue;
			if (unit.moveState == moveState::MoveAI)
			{
				flatList.push_back(unit.ID);
			}
		}
	}

	if (flatList.size() == 0) return 0;
	if (abort) return 0;

	// 共通経路(始まり合流地点→終わり合流地点)を算出
	s3d::Optional<Size> startIndex = mapTile.ToIndex(GetCUSafe(flatList[0], snapshot)->getFirstMergePos(), mapTile.columnQuads, mapTile.rowQuads);
	if (startIndex.has_value() == false) return 0;
	s3d::Optional<Size> endIndex = mapTile.ToIndex(GetCUSafe(flatList[0], snapshot)->getLastMergePos(), mapTile.columnQuads, mapTile.rowQuads);
	if (endIndex.has_value() == false) return 0;

	if (startIndex == endIndex)
	{
		//単一ユニットがこのケース
		s3d::Optional<Size> startIndex = mapTile.ToIndex(GetCUSafe(flatList[0], snapshot)->GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
		if (startIndex.has_value() == false) return 0;
		s3d::Optional<Size> endIndex = mapTile.ToIndex(GetCUSafe(flatList[0], snapshot)->GetOrderPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
		if (endIndex.has_value() == false) return 0;
		Array<Point> fullPath = findPath(startIndex.value(), endIndex.value(), mapData, enemy, snapshot, mapTile, hsBuildingUnitForAstar, abort, true);
		if (fullPath.size() != 0)
		{
			ClassUnitMovePlan plan;
			plan.setPath(fullPath);
			{
				std::scoped_lock lock(aiRootMutex);
				aiRoot[GetCUSafe(flatList[0], snapshot)->ID] = plan;
			}
			//debugRoot.push_back(fullPath);
			auto huidfs = GetCUSafe(flatList[0], target);
			{
				std::shared_lock lock(unitListRWMutex);
				huidfs->moveState = moveState::Moving;
			}
			//htUnit[->ID]->moveState = moveState::Moving;
		}
		return 0;
	}

	Array<Point> pathShare = findPath(startIndex.value(), endIndex.value(), mapData, enemy, snapshot, mapTile, hsBuildingUnitForAstar, abort, true);

	for (int32 unit : flatList)
	{
		Array<Point> firstPath;
		{
			s3d::Optional<Size> nowIndexFirstGoal = mapTile.ToIndex(GetCUSafe(unit, snapshot)->getFirstMergePos(), mapTile.columnQuads, mapTile.rowQuads);
			if (nowIndexFirstGoal.has_value() == false) continue;
			s3d::Optional<Size> nowIndex = mapTile.ToIndex(GetCUSafe(unit, snapshot)->GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (nowIndex.has_value() == false) continue;
			if (nowIndexFirstGoal != nowIndex)
			{
				firstPath = findPath(nowIndex.value(), nowIndexFirstGoal.value(), mapData, enemy, snapshot, mapTile, hsBuildingUnitForAstar, abort, true);
			}
		}

		Array<Point> endPath;
		{
			s3d::Optional<Size> nowIndexEndGoal = mapTile.ToIndex(GetCUSafe(unit, snapshot)->GetOrderPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
			if (nowIndexEndGoal.has_value() == false) continue;
			s3d::Optional<Size> nowIndex = mapTile.ToIndex(GetCUSafe(unit, snapshot)->getLastMergePos(), mapTile.columnQuads, mapTile.rowQuads);
			if (nowIndex.has_value() == false) continue;
			if (nowIndexEndGoal != nowIndex)
			{
				endPath = findPath(nowIndex.value(), nowIndexEndGoal.value(), mapData, enemy, snapshot, mapTile, hsBuildingUnitForAstar, abort, true);
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
				aiRoot[GetCUSafe(unit, snapshot)->ID] = plan;
			}

			auto huidfs = GetCUSafe(unit, target);
			{
				std::shared_lock lock(unitListRWMutex);
				huidfs->moveState = moveState::Moving;
			}

			//htUnit[unit->ID]->moveState = moveState::Moving;
		}

	}
}
