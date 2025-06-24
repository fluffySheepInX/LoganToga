#pragma once
# include "EnumSkill.h" 

class MapDetail
{
public:
	String tip;
	/// @brief 種別、ClassUnitID、陣営
	Array<std::tuple<String, long, BattleWhichIsThePlayer>> building;
	FlagBattleMapUnit flagBattleMapUnit;

	/// @brief タイルが資源ポイントかどうか
	bool isResourcePoint = false;
	/// @brief 資源ポイントの種類
	resourceKind resourcePointType = resourceKind::None;
	/// @brief 資源ポイントの量
	int32 resourcePointAmount = -1;
	/// @brief 資源ポイントの画面表示名
	String resourcePointDisplayName = U"test-resourcePointDisplayName";
	/// @brief 資源ポイントのアイコン
	String resourcePointIcon = U"test-resourcePointIcon";
	BattleWhichIsThePlayer whichIsThePlayer = BattleWhichIsThePlayer::None;

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
