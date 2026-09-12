#pragma once
# include <Siv3D.hpp>
# include <filesystem>
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

		if (!ModPolicy::CanInheritDefaultGame(mod.inheritDefaultGame, ToModPolicyTextView(mod.id)))
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

	// Map Editor 標準アセットディレクトリを Mod 優先・既定ゲーム継承で解決します。
	inline FilePath ResolveModMapEditorAssetDirectory(const ModContext* mod)
	{
		constexpr StringView RelativeDirectory = U"015_BattleMapCellImage";
		if (mod && !mod->rootPath.isEmpty())
		{
			if (const Optional<FilePath> modDirectory = ResolveCanonicalModRelativePath(mod->rootPath, RelativeDirectory))
			{
				if (FileSystem::IsDirectory(*modDirectory))
				{
					return *modDirectory + U"/";
				}
			}
			if (!ModPolicy::CanInheritDefaultGame(mod->inheritDefaultGame, ToModPolicyTextView(mod->id)))
			{
				return FilePath{};
			}
		}

		for (const FilePath& defaultRoot : Array<FilePath>{ U"000_Warehouse/000_DefaultGame/", U"App/000_Warehouse/000_DefaultGame/" })
		{
			if (const Optional<FilePath> defaultDirectory = ResolveCanonicalModRelativePath(defaultRoot, RelativeDirectory))
			{
				if (FileSystem::IsDirectory(*defaultDirectory))
				{
					return *defaultDirectory + U"/";
				}
			}
		}

		return FilePath{};
	}

	// canonical な実体パスを指定 Mod root からの安全な相対パスに変換します。
	inline Optional<FilePath> MakeCanonicalModRelativePath(const ModContext& mod, const FilePath& path)
	{
		if (mod.rootPath.isEmpty() || path.isEmpty())
		{
			return none;
		}

		std::error_code error;
		const std::filesystem::path canonicalRoot = std::filesystem::canonical(std::filesystem::path{ mod.rootPath.toWstr() }, error);
		const std::filesystem::path canonicalPath = std::filesystem::canonical(std::filesystem::path{ path.toWstr() }, error);
		if (error || !IsStrictlyWithinCanonicalRoot(canonicalRoot, canonicalPath))
		{
			return none;
		}

		const std::filesystem::path relativePath = canonicalPath.lexically_relative(canonicalRoot);
		if (relativePath.empty() || !IsLexicallySafeModRelativePath(Unicode::FromWstring(relativePath.generic_wstring())))
		{
			return none;
		}

		return Unicode::FromWstring(relativePath.generic_wstring());
	}

	// 解決済みアセットパスを選択 Mod、または既定ゲーム root からの安全な相対パスへ変換します。
	inline Optional<FilePath> MakeCanonicalAssetRelativePath(const ModContext* mod, const FilePath& path)
	{
		if (mod && !mod->rootPath.isEmpty())
		{
			if (const Optional<FilePath> relativePath = MakeCanonicalModRelativePath(*mod, path))
			{
				return relativePath;
			}
			if (!ModPolicy::CanInheritDefaultGame(mod->inheritDefaultGame, ToModPolicyTextView(mod->id)))
			{
				return none;
			}
		}

		for (const FilePath& defaultRoot : Array<FilePath>{ U"000_Warehouse/000_DefaultGame/", U"App/000_Warehouse/000_DefaultGame/" })
		{
			ModContext defaultGame;
			defaultGame.rootPath = defaultRoot;
			if (const Optional<FilePath> relativePath = MakeCanonicalModRelativePath(defaultGame, path))
			{
				return relativePath;
			}
		}

		return none;
	}
}
