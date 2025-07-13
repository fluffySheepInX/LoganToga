#pragma once
# include "ClassMapBattle.h" 
# include "ClassHorizontalUnit.h" 
#include "ClassUnit.h"

enum class AStarStatus {
	None
	,
	Open
	,
	Closed
};

inline int HeuristicMethod(int nowX, int nowY, int targetX, int targetY) {
	int x = std::abs(nowX - targetX);
	int y = std::abs(nowY - targetY);

	return (x > y) ? x : y;
}

class ClassAStar {
public:
	ClassAStar(int row, int col) : row(row), col(col) {}

	// AStarStatus のゲッターとセッター
	AStarStatus GetAStarStatus() const {
		return aStarStatus;
	}

	void SetAStarStatus(AStarStatus status) {
		aStarStatus = status;
	}

	// Row のゲッターとセッター
	int GetRow() const {
		return row;
	}

	void SetRow(int value) {
		row = value;
	}

	// Col のゲッターとセッター
	int GetCol() const {
		return col;
	}

	void SetCol(int value) {
		col = value;
	}

	// Cost のゲッターとセッター
	int GetCost() const {
		return cost;
	}

	void SetCost(int value) {
		cost = value;
	}

	// HCost のゲッターとセッター
	int GetHCost() const {
		return hCost;
	}

	void SetHCost(int value) {
		hCost = value;
	}

	// RefClassAStar のゲッターとセッター
	ClassAStar* GetRefClassAStar() const {
		return refClassAStar;
	}

	void SetRefClassAStar(ClassAStar* value) {
		refClassAStar = value;
	}

	// GetRoot メソッド
	void GetRoot(Array<Point>& target) {
		target.push_back(Point(row, col));
		if (refClassAStar != nullptr) {
			refClassAStar->GetRoot(target);
		}
	}

private:
	AStarStatus aStarStatus = AStarStatus::None;
	int row;
	int col;
	int cost = 0;
	int hCost = 0;
	ClassAStar* refClassAStar = nullptr;
};

class ClassAStarManager {
public:
	ClassAStarManager(int x, int y)
		: endX(x), endY(y), listClassAStar(Array<ClassAStar*>()) {
	}
	~ClassAStarManager() {
		for (auto& [_, ptr] : pool) {
			delete ptr;
		}
	}
	// Pool のゲッターとセッター
	const HashTable<Point, ClassAStar*>& GetPool() const {
		return pool;
	}

	void SetPool(const HashTable<Point, ClassAStar*>& value) {
		pool = value;
	}

	// ListClassAStar のゲッターとセッター
	Array<ClassAStar*>& GetListClassAStar() {
		return listClassAStar;
	}

	void SetListClassAStar(const Array<ClassAStar*>& value) {
		listClassAStar = value;
	}

	// EndX, EndY のゲッターとセッター
	int GetEndX() const {
		return endX;
	}

	void SetEndX(int value) {
		endX = value;
	}

	int GetEndY() const {
		return endY;
	}

	void SetEndY(int value) {
		endY = value;
	}

	ClassAStar* CreateClassAStar(int x, int y) {
		Point abc = Point(x, y);
		if (pool.contains(abc)) {
			// 既に存在しているのでプーリングから取得.
			return pool[abc];
		}
		ClassAStar* re = new ClassAStar(x, y);
		re->SetHCost(HeuristicMethod(x, y, GetEndX(), GetEndY()));
		pool[abc] = re;
		return re;
	}

	void RemoveClassAStar(const ClassAStar* classAStar) {
		// ポインタの配列から特定のオブジェクトへのポインタを削除
		listClassAStar.remove_if([&classAStar](const ClassAStar* item) {
			return item == classAStar;
		});
	}

	void Clear() {
		for (auto& p : pool) {
			delete p.second;
		}
		pool.clear();
		listClassAStar.clear();
	}

	Optional<ClassAStar*> OpenOne(int x, int y, int cost, ClassAStar* parent, int32 maxN, AStarStatus status = AStarStatus::Open) {
		if (x >= maxN || y >= maxN || x < 0 || y < 0)
			return none;

		if (cost > 100)  // 上限はマップサイズや用途によって調整
			return none;

		Point key(x, y);

		// ここで事前チェックしておくことで、無駄な CreateClassAStar() 呼び出しを防ぐ
		if (pool.contains(key) && pool[key]->GetAStarStatus() != AStarStatus::None)
		{
			return none;
		}

		ClassAStar* getClassAStar = CreateClassAStar(x, y);
		getClassAStar->SetAStarStatus(status);
		getClassAStar->SetCost(cost);

		if (parent != nullptr)
		{
			getClassAStar->SetRefClassAStar(parent);
		}

		listClassAStar.push_back(getClassAStar);

		return getClassAStar;
	}

	void OpenAround(ClassAStar* parent, Array<Array<MapDetail>>& mapData,
					Array<ClassHorizontalUnit>& arrayObjEnemy, Array<ClassHorizontalUnit>& arrayObjMy, int32 maxN,
					HashTable<Point, const Unit*>& hsBuildingUnitForAstar)
	{
		try
		{
			int32 x = parent->GetRow();
			int32 y = parent->GetCol();
			int32 cost = parent->GetCost() + 1;

			for (int j = -1; j <= 1; ++j)
			{
				for (int i = -1; i <= 1; ++i)
				{
					int nx = x + i;
					int ny = y + j;

					if (i == 0 && j == 0) continue; // 自分自身はスキップ
					if (nx < 0 || ny < 0 || nx >= mapData.size() || ny >= mapData[nx].size()) continue;
					if (checkObstacleAndContinue(arrayObjEnemy, nx, ny, cost, parent, maxN, hsBuildingUnitForAstar)) continue;
					if (checkObstacleAndContinue(arrayObjMy, nx, ny, cost, parent, maxN, hsBuildingUnitForAstar)) continue;
					OpenOne(nx, ny, cost, parent, maxN);
				}
			}
		}
		catch (const Error& ex)
		{
			Print << ex;
		}
	}
	// 最適化版: ユニットの位置をHashTableで事前構築し、高速に判定
	bool checkObstacleAndContinue(
		const Array<ClassHorizontalUnit>& unitArray,
		int32 mapX, int32 mapY, int32 cost,
		ClassAStar* parent, int32 maxN, HashTable<Point, const Unit*>& hsBuildingUnitForAstar)
	{
		// 指定座標に建物があるか高速判定
		Point key(mapX, mapY);
		if (hsBuildingUnitForAstar.contains(key))
		{
			const Unit* unit = hsBuildingUnitForAstar[key];
			const MapTipObjectType mapTipObjectType = unit->mapTipObjectType;
			OpenOne(mapX, mapY, cost, parent, maxN);
			return true;

			// 壊せる障害物は通行可（例：ゲート、HOME）
			if (unit->mapTipObjectType == MapTipObjectType::GATE
				|| unit->mapTipObjectType == MapTipObjectType::HOME)
			{
				if (!pool.contains(key))
				{
					OpenOne(mapX, mapY, cost, parent, maxN);
				}
				return true;
			}
			// 壁や鉄条網など通行不可
			return true;
		}
		return false;
	}
private:
	HashTable<Point, ClassAStar*> pool;
	Array<ClassAStar*> listClassAStar;
	int endX;
	int endY;
};
