#pragma once
struct BuildAction {
	String id;
	String name;
	String description;
	String icon;
	/// @brief -1は無制限
	int32 buildCount = -1;
	int32 createCount = 1;
	HashTable<String, int32> cost;
	double buildTime;
	String category;
	Array<String> arrRequires;
	struct {
		String type;
		String spawn;
	} result;
	Rect rectHantei;
	bool isMove = false;
	/// @brief 建てる場所
	int32 rowBuildingTarget = -1;
	int32 colBuildingTarget = -1;
};

