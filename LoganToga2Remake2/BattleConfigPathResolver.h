#pragma once

#include "BattleConfigTypes.h"

[[nodiscard]] inline String ResolveBattleConfigSourcePath(const String& rootPath, const String& relativePath)
{
	const String directory = FileSystem::ParentPath(rootPath);
	if (directory.isEmpty())
	{
		return relativePath;
	}

	if (directory.ends_with(U"/") || directory.ends_with(U"\\"))
	{
		return directory + relativePath;
	}

	return directory + U"/" + relativePath;
}
