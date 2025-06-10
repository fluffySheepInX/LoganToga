#pragma once
# include "EnumSkill.h" 

class MapDetail
{
public:
	String tip;
	/// @brief 種別、ClassUnitID、陣営
	Array<std::tuple<String, long, BattleWhichIsThePlayer>> building;
	FlagBattleMapUnit flagBattleMapUnit;
	String unit;
	String houkou;
	String zinkei;
	String posSpecial;
	bool kougekiButaiNoIti = false;
	bool boueiButaiNoIti = false;
	//s3d::Optional<s3d::Path> mapPath;
};

class ClassMapBattle
{
public:
	//// コピーコンストラクタ
	//ClassMapBattle(const ClassMapBattle& other)
	//	: name(other.name), mapData(other.mapData), frontPosi(other.frontPosi), defensePosi(other.defensePosi), neutralPosi(other.neutralPosi) {
	//}
	//ClassMapBattle() = default;
	//// ムーブ代入演算子
	//ClassMapBattle& operator=(const ClassMapBattle& other) {
	//	if (this != &other) {
	//		name = other.name;
	//		mapData = other.mapData;
	//		frontPosi = other.frontPosi;
	//		defensePosi = other.defensePosi;
	//		neutralPosi = other.neutralPosi;
	//	}
	//	return *this;
	//}
	//ClassMapBattle& operator=(ClassMapBattle&& other) {
	//	if (this != &other) {
	//		name = other.name;
	//		mapData = other.mapData;
	//		frontPosi = other.frontPosi;
	//		defensePosi = other.defensePosi;
	//		neutralPosi = other.neutralPosi;
	//	}
	//	return *this;
	//}

	String name;
	//std::string tagName;
	Array<Array<MapDetail>> mapData;
	Optional<Point> frontPosi;
	Optional<Point> defensePosi;
	Optional<Point> neutralPosi;
};
