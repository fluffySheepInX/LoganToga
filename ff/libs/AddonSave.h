#pragma once
#include <Siv3D.hpp>

class SaveFSAddon : public IAddon
{
public:
	/// @brief 
	static void Condition(const Array<String> elements, const String saveFileName = U"data.ini");
	static String LoadData(const String key, const String saveFileName = U"data.ini");
	static void EditData(const String key, const String value, const String saveFileName = U"data.ini");
};
