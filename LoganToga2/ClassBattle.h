#pragma once
#include "EnumSkill.h" 
#include "ClassHorizontalUnit.h"
#include "ClassMapBattle.h"

class ClassBattle
{
public:
	long getIDCount() {
		long re = iDCount;
		iDCount++;
		return re;
	}

	String battleSpot;
	String sortieSpot;
	String attackPower;
	String defensePower;
	Array<ClassHorizontalUnit> listOfAllUnit;
	Array<ClassHorizontalUnit> listOfAllEnemyUnit;
	BattleWhichIsThePlayer battleWhichIsThePlayer;

	HashSet<Unit*> hsMyUnitBuilding;
	HashSet<Unit*> hsEnemyUnitBuilding;

	//中立ユニットのグループ
	//Array<ClassHorizontalUnit> neutralUnitGroup;
	std::optional<ClassMapBattle> classMapBattle;
	//生きている建築物
	//std::vector<Rect> listBuildingAlive;
private:
	long iDCount = 0;
};
