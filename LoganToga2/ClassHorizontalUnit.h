#pragma once
#include "ClassUnit.h"

class ClassHorizontalUnit
{
public:
	ClassHorizontalUnit& operator=(const ClassHorizontalUnit& other) {
		if (this != &other) {
			FlagDisplay = other.FlagDisplay;
			FlagBuilding = other.FlagBuilding;
			ListClassUnit = other.ListClassUnit;
		}
		return *this;
	}
	ClassHorizontalUnit(const Array<Unit>& u)
	{
		ListClassUnit = u;;
	}
	ClassHorizontalUnit() = default;
	bool FlagDisplay = true;
	bool FlagBuilding = false;
	Array<Unit> ListClassUnit;
};

