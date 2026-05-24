#pragma once
# include "MapEditorCommandEditorInputCommon.h"

namespace LT3
{
	inline bool ChangeSelectedCommandImageFromDialog(MapEditorState& editor, DefinitionStores& defs, int32 actionIndex)
	{
		if (actionIndex < 0 || static_cast<int32>(defs.buildActions.size()) <= actionIndex)
		{
			return false;
		}

		const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::GIF(), FileFilter::AllFiles() };
		const Array<FilePath> sourcePaths = Dialog::OpenFiles(imageFilters);
		if (sourcePaths.isEmpty())
		{
			return false;
		}

		BuildActionDef& action = defs.buildActions[actionIndex];
		const FilePath targetDirectory = FileSystem::ParentPath(ResolveBuildIconPath(U"__lt3_directory_probe__.png"));
		FileSystem::CreateDirectories(targetDirectory);
		Array<String> iconLayers;
		iconLayers.reserve(sourcePaths.size());

		for (const auto& sourcePath : sourcePaths)
		{
			if (!IsCommandSelectableImageFile(sourcePath))
			{
				editor.statusText = U"Command image must be png or gif: {}"_fmt(sourcePath);
				return true;
			}

			const String fileName = FileSystem::FileName(sourcePath);
			const FilePath targetPath = targetDirectory + fileName;
			if (FileSystem::FullPath(sourcePath) != FileSystem::FullPath(targetPath))
			{
				if (FileSystem::Exists(targetPath))
				{
					FileSystem::Remove(targetPath);
				}

				if (!FileSystem::Copy(sourcePath, targetPath))
				{
					editor.statusText = U"Command image copy failed: {}"_fmt(targetPath);
					return true;
				}
			}

			iconLayers << fileName;
			BuildingEditorTextureCache().erase(ResolveBuildIconPath(fileName));
		}

		action.iconLayers = NormalizeCommandIconLayers(iconLayers);
		action.icon = action.iconLayers.isEmpty() ? U"" : action.iconLayers.front();
		editor.commandBindingsDirty = true;
		editor.statusText = U"Command image layers changed: {} ({} files)"_fmt(action.id, action.iconLayers.size());
		return true;
	}
}
