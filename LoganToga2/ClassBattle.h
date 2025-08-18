#pragma once
#include "EnumSkill.h" 
#include "ClassHorizontalUnit.h"
#include "ClassMapBattle.h"
#include <mutex>

class ClassBattle
{
public:
	mutable std::mutex unitListMutex;

	long getIDCount() {
		long re = iDCount;
		iDCount++;
		return re;
	}
	long getBattleIDCount() {
		long re = battleIDCount;
		battleIDCount++;
		return re;
	}
	long getDeleteCESIDCount() {
		long re = deleteCESIDCount;
		deleteCESIDCount++;
		return re;
	}

	String battleSpot;
	String sortieSpot;
	String attackPower;
	String defensePower;
	Array<ClassHorizontalUnit> listOfAllUnit;
	Array<ClassHorizontalUnit> listOfAllEnemyUnit;
	BattleWhichIsThePlayer battleWhichIsThePlayer;

	HashSet<std::shared_ptr<Unit>> hsMyUnitBuilding;
	HashSet<std::shared_ptr<Unit>> hsEnemyUnitBuilding;

	//中立ユニットのグループ
	//Array<ClassHorizontalUnit> neutralUnitGroup;
	std::optional<ClassMapBattle> classMapBattle;
	//生きている建築物
	//std::vector<Rect> listBuildingAlive;
private:
	long iDCount = 0;
	long battleIDCount = 0;
	long deleteCESIDCount = 0;
};
