#include "../AddonSave.h"

/// @brief 
/// @param elements 
/// @param saveFileName 
void SaveFSAddon::Condition(const Array<String> elements, const String saveFileName)
{
	if (auto p = Addon::GetAddon<SaveFSAddon>(U"SaveFSAddon"))
	{
		if (FileSystem::Exists(saveFileName) == false)
		{
			INI ini = INI(saveFileName);
			for (auto& da : elements)
			{
				ini.write(U"saveData", da, U"0");
			}
			ini.save(saveFileName);
		}
	}
}

/// @brief 
/// @param key 
/// @param saveFileName 
/// @return 
String SaveFSAddon::LoadData(const String key, const String saveFileName)
{
	if (auto p = Addon::GetAddon<SaveFSAddon>(U"SaveFSAddon"))
	{
		if (FileSystem::Exists(saveFileName) == true)
		{
			INI ini = INI(saveFileName);
			return ini.get<String>(U"data", key);
		}
	}
}

/// @brief 
/// @param key 
/// @param value 
/// @param saveFileName 
void SaveFSAddon::EditData(const String key, const String value, const String saveFileName)
{
	if (auto p = Addon::GetAddon<SaveFSAddon>(U"SaveFSAddon"))
	{
		if (FileSystem::Exists(saveFileName) == true)
		{
			INI ini = INI(saveFileName);
			ini.write(U"data", key, value);
			ini.save(saveFileName);
		}
	}
}
