#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/Loaders/SkillDefLoader.h"

namespace LT3
{
	inline HashTable<FilePath, Texture>& SkillEditorTextureCache()
	{
		static HashTable<FilePath, Texture> cache;
		return cache;
	}

	inline FilePath SkillEditorHelpIconPath()
	{
		return ResolveSystemImagePath(U"hatena.png");
	}

	inline FilePath SkillEditorWarningIconPath()
	{
		return ResolveSystemImagePath(U"bikkuri.png");
	}

	inline FilePath ResolveSkillIconPath(const String& iconName)
	{
		if (iconName.isEmpty())
		{
			return FilePath{};
		}

		const Array<FilePath> candidates = {
			ResolveBuildIconPath(iconName),
			ResolveSystemImagePath(iconName),
			ResolveUnitChipPath(iconName),
			ResolveBuildingChipPath(iconName),
		};
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}
		return FilePath{};
	}

	inline bool CopySkillEditorImageToBuildIcons(const FilePath& sourcePath, String& fileName, String& statusText)
	{
		const String extension = FileSystem::Extension(sourcePath).lowercased();
		if (!(extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"gif"))
		{
			statusText = U"Skill image must be image file: {}"_fmt(sourcePath);
			return false;
		}

		const FilePath targetDirectory = FileSystem::ParentPath(ResolveBuildIconPath(U"__lt3_directory_probe__.png"));
		FileSystem::CreateDirectories(targetDirectory);
		fileName = FileSystem::FileName(sourcePath);
		const FilePath targetPath = targetDirectory + fileName;
		if (FileSystem::FullPath(sourcePath) != FileSystem::FullPath(targetPath))
		{
			if (FileSystem::Exists(targetPath))
			{
				FileSystem::Remove(targetPath);
			}

			if (!FileSystem::Copy(sourcePath, targetPath))
			{
				statusText = U"Skill image copy failed: {}"_fmt(targetPath);
				return false;
			}
		}

		SkillEditorTextureCache().erase(ResolveBuildIconPath(fileName));
		statusText = U"Skill image set: {}"_fmt(fileName);
		return true;
	}

	inline bool UnitHasSkill(const UnitCatalogEntry& entry, const String& skillTag)
	{
		return entry.skills.any([&](const String& tag) { return tag == skillTag; });
	}

	inline Array<FilePath> SkillEditorIconPaths(const SkillDef& skill)
	{
		Array<FilePath> paths;
		const Array<String> orderedLayers = NormalizeSkillIconLayerOrder(skill.iconLayers);
		if (!orderedLayers.isEmpty())
		{
			for (const auto& icon : orderedLayers)
			{
				const FilePath path = ResolveSkillIconPath(icon);
				if (!path.isEmpty())
				{
					paths << path;
				}
			}
		}
		else if (!skill.icon.isEmpty())
		{
			const FilePath path = ResolveSkillIconPath(skill.icon);
			if (!path.isEmpty())
			{
				paths << path;
			}
		}
		return paths;
	}

	inline void DrawSkillEditorLayeredIcon(const SkillDef& skill, const RectF& iconRect)
	{
		for (const auto& iconPath : SkillEditorIconPaths(skill))
		{
			if (!FileSystem::Exists(iconPath))
			{
				continue;
			}

			auto& cache = SkillEditorTextureCache();
			if (!cache.contains(iconPath))
			{
				cache.emplace(iconPath, Texture{ iconPath });
			}

			const Texture& texture = cache.at(iconPath);
			const double fitScale = Min((iconRect.w - 4.0) / Max(1.0, static_cast<double>(texture.width())), (iconRect.h - 4.0) / Max(1.0, static_cast<double>(texture.height())));
			texture.scaled(Min(1.0, fitScale)).drawAt(iconRect.center());
		}
	}
}
