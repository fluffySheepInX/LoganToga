#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	// ユーザー設定を保存する LocalAppData ルートを返します。
	inline FilePath ResolveUserDataDirectory()
	{
		return FileSystem::GetFolderPath(SpecialFolder::LocalAppData) + U"LoganToga3/";
	}

	// ユーザー設定ファイルの書込み先を返します。
	inline FilePath ResolveUserSettingsPath(StringView fileName)
	{
		return ResolveUserDataDirectory() + fileName;
	}

	// 旧配置の設定ファイルをユーザー設定ルートへ一度だけ移行します。
	inline bool MigrateLegacyUserSettingsFile(const Array<FilePath>& legacyPaths, FilePathView destination, String& statusText)
	{
		const FilePath destinationPath{ destination };
		if (FileSystem::Exists(destinationPath))
		{
			return true;
		}

		for (const FilePath& legacyPath : legacyPaths)
		{
			if (!FileSystem::Exists(legacyPath))
			{
				continue;
			}

			FileSystem::CreateDirectories(FileSystem::ParentPath(destinationPath));
			if (!FileSystem::Copy(legacyPath, destinationPath))
			{
				statusText = U"User settings migration failed: {}"_fmt(legacyPath);
				return false;
			}

			statusText = U"User settings migrated: {}"_fmt(destinationPath);
			return true;
		}

		return true;
	}
}
