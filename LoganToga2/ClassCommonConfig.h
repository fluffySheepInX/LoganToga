#pragma once
# include <Siv3D.hpp>
#include "ClassUnit.h"
#include "ClassBuildAction.h"
#include "EnumSkill.h"

class ClassResource
{
public:
	String resourceName;
	String resourceIcon;          // リソースのアイコンファイル名
	resourceKind resourceType;          // リソースの種類（例：金、木材、石材など）
	int32 resourceAmount = 0;     // リソースの量
};

class Unit;

class CommonConfig
{
public:
	Array<Unit> arrayInfoUnit;
	/// @brief key = 工兵(kouhei)など
	HashTable<String, Array<BuildAction>> htBuildMenuBaseData;
	HashTable<String, ClassResource> htResourceData;
};

