#pragma once
# include <Siv3D.hpp>
#include "ClassUnit.h"
#include "ClassBuildAction.h"

class CommonConfig
{
public:
	Array<Unit> arrayUnit;
	/// @brief key = 工兵(kouhei)など
	HashTable<String, Array<BuildAction>> htBuildMenuBaseData;

};

