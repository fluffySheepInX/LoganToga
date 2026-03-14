#pragma once

#include "Remake2Common.h"

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

[[nodiscard]] inline String ResolveTomlOverridePath(const String& path)
{
	if (path.ends_with(U".toml"))
	{
		return path.substr(0, path.size() - 5) + U"_override.toml";
	}

	return path + U"_override";
}

[[nodiscard]] inline bool HasTomlOverride(const String& path)
{
	return FileSystem::Exists(ResolveTomlOverridePath(path));
}

[[nodiscard]] inline String ResolveTomlEditorOverridePath(const String& path)
{
	if (path.ends_with(U".toml"))
	{
		return path.substr(0, path.size() - 5) + U"_editor_override.toml";
	}

	return path + U"_editor_override";
}

[[nodiscard]] inline bool HasTomlEditorOverride(const String& path)
{
	return FileSystem::Exists(ResolveTomlEditorOverridePath(path));
}
