#pragma once
# include "230_ClassUnit.h" 

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

	bool FlagDisplay = true;
	bool FlagBuilding = false;
	Array<ClassUnit> ListClassUnit;
};
