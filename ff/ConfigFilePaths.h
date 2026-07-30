# pragma once
# include <Siv3D.hpp>

namespace ff
{
	struct ConfigFilePaths
	{
		String runtimeRelativePath;
		String projectRelativePath;
		String userRelativePath;
	};

	// 同梱設定ファイルの実行時・開発時の候補から利用可能なパスを取得します。
	[[nodiscard]] inline String ResolveBundledConfigPath(const ConfigFilePaths& paths)
	{
		if (FileSystem::Exists(paths.runtimeRelativePath))
		{
			return paths.runtimeRelativePath;
		}

		if (FileSystem::Exists(paths.projectRelativePath))
		{
			return paths.projectRelativePath;
		}

		return paths.runtimeRelativePath;
	}

	// ユーザー設定を優先し、存在しない場合は同梱設定のパスを取得します。
	[[nodiscard]] inline String ResolveConfigPath(const ConfigFilePaths& paths)
	{
		if (FileSystem::Exists(paths.userRelativePath))
		{
			return paths.userRelativePath;
		}

		return ResolveBundledConfigPath(paths);
	}
}
