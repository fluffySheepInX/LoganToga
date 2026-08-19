#pragma once
# include <Siv3D.hpp>
# include "ModContent.h"

namespace LT3
{
	// 選択modの共有定義を互換配置から解決します。
	inline FilePath ResolveModDefinitionPath(const ModContext& mod, StringView relativePath)
	{
		if (!IsSafeModRelativePath(relativePath))
		{
			return FilePath{};
		}

		const FilePath modPath = mod.rootPath + relativePath;
		if (FileSystem::Exists(modPath))
		{
			return modPath;
		}

		if (!mod.inheritDefaultGame || (mod.id == U"000-default-game"))
		{
			return modPath;
		}

		for (const FilePath& defaultRoot : Array<FilePath>{ U"000_Warehouse/000_DefaultGame/", U"App/000_Warehouse/000_DefaultGame/" })
		{
			const FilePath defaultPath = defaultRoot + relativePath;
			if (FileSystem::Exists(defaultPath))
			{
				return defaultPath;
			}
		}

		return modPath;
	}
}
