#pragma once
# include <Siv3D.hpp>
# include "ModContent.h"

namespace LT3
{
	inline FilePath ResolveModDefinitionPath(const ModContext& mod, StringView relativePath);

	// 選択modのアセットを優先し、継承指定時のみ既定ゲームへフォールバックします。
	inline FilePath ResolveModAssetPath(const ModContext* mod, StringView relativePath)
	{
		if (mod && !mod->rootPath.isEmpty())
		{
			return ResolveModDefinitionPath(*mod, relativePath);
		}

		for (const FilePath& defaultRoot : Array<FilePath>{ U"000_Warehouse/000_DefaultGame/", U"App/000_Warehouse/000_DefaultGame/" })
		{
			if (const Optional<FilePath> defaultPath = ResolveCanonicalModRelativePath(defaultRoot, relativePath))
			{
				return *defaultPath;
			}
		}

		return FilePath{};
	}

	// 選択modの共有定義を互換配置から解決します。
	inline FilePath ResolveModDefinitionPath(const ModContext& mod, StringView relativePath)
	{
		if (const Optional<FilePath> modPath = ResolveCanonicalModRelativePath(mod.rootPath, relativePath))
		{
			return *modPath;
		}

		if (!mod.inheritDefaultGame || (mod.id == U"000-default-game"))
		{
			return FilePath{};
		}

		for (const FilePath& defaultRoot : Array<FilePath>{ U"000_Warehouse/000_DefaultGame/", U"App/000_Warehouse/000_DefaultGame/" })
		{
			if (const Optional<FilePath> defaultPath = ResolveCanonicalModRelativePath(defaultRoot, relativePath))
			{
				return *defaultPath;
			}
		}

		return FilePath{};
	}
}
