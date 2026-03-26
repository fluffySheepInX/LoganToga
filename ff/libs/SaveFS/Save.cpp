#include "../AddonSave.h"

/// @brief 
/// @param elements 
/// @param saveFileName 
void SaveFSAddon::Condition(const Array<String> elements, const String saveFileName)
{
	if (auto p = Addon::GetAddon<SaveFSAddon>(U"SaveFSAddon"))
	{
		if (FileSystem::Exists(saveFileName))
		{
			INI ini{ saveFileName };
			for (const auto& key : elements)
			{
				if (not ini.hasSection(U"data") || (not ini.hasValue(U"data", key)))
				{
					ini.write(U"data", key, U"0");
				}
			}
			ini.save(saveFileName);
		}
		else
		{
			INI ini;
			for (const auto& key : elements)
			{
				ini.write(U"data", key, U"0");
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
			if (ini.hasSection(U"data") && ini.hasValue(U"data", key))
			{
				return ini.get<String>(U"data", key);
			}
		}
	}

	return U"";
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
			INI ini{ saveFileName };
			ini.write(U"data", key, value);
			ini.save(saveFileName);
		}
		else
		{
			INI ini;
			ini.write(U"data", key, value);
			ini.save(saveFileName);
		}
	}
}
