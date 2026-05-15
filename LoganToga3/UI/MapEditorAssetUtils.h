#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"

namespace LT3
{
	inline FilePath ResolveMapEditorAssetDirectory()
	{
		const FilePath fromApp = U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
		if (FileSystem::IsDirectory(fromApp))
		{
			return fromApp;
		}

		const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
		if (FileSystem::IsDirectory(fromRepo))
		{
			return fromRepo;
		}

		return fromApp;
	}

	inline bool IsMapEditorImageFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"webp";
	}

	inline String TomlEscape(StringView text)
	{
		String result;
		for (const char32 ch : text)
		{
			if (ch == U'\\')
			{
				result += U"\\\\";
			}
			else if (ch == U'\"')
			{
				result += U"\\\"";
			}
			else
			{
				result += ch;
			}
		}
		return result;
	}

	inline int32 FindMapEditorAssetIndexByFileName(const MapEditorState& editor, StringView fileName)
	{
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			if (editor.assets[i].fileName == fileName)
			{
				return i;
			}
		}

		return InvalidMapEditorAsset;
	}
}
