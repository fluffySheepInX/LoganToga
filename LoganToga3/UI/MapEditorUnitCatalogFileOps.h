#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "BuildingEditorCommon.h"

namespace LT3
{
	inline void ChangeSelectedUnitVisualScale(MapEditorState& editor, UnitCatalog& catalog, double delta)
	{
		if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		entry.visualScale = Math::Round(Clamp(entry.visualScale + delta, 0.25, 3.0) * 100.0) / 100.0;
		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
	}

	inline void ChangeSelectedUnitVisionRadius(MapEditorState& editor, UnitCatalog& catalog, int32 delta)
	{
		if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		entry.visionRadius = Clamp(entry.visionRadius + delta, 0, 40);
		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
	}

	inline void ChangeSelectedUnitMove(MapEditorState& editor, UnitCatalog& catalog, int32 delta)
	{
		if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		entry.move = Clamp(entry.move + delta, 0, 2000);
		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
	}

	inline FilePath ResolveCatalogVisualDirectory(const String& kind)
	{
		const FilePath existingPath = (kind.lowercased() == U"building")
			? ResolveBuildingChipPath(U"__lt3_directory_probe__.png")
			: ResolveUnitChipPath(U"__lt3_directory_probe__.png");
		return FileSystem::ParentPath(existingPath);
	}

	inline bool IsUnitCatalogSelectableImageFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"png" || extension == U"gif";
	}

	inline bool ChangeSelectedUnitImageFromDialog(MapEditorState& editor, UnitCatalog& catalog)
	{
		if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return false;
		}

		const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::GIF(), FileFilter::AllFiles() };
		const Optional<FilePath> sourcePath = Dialog::OpenFile(imageFilters);
		if (!sourcePath)
		{
			return false;
		}

		if (!IsUnitCatalogSelectableImageFile(*sourcePath))
		{
			editor.statusText = U"Unit image must be png or gif: {}"_fmt(*sourcePath);
			return true;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		const FilePath targetDirectory = ResolveCatalogVisualDirectory(entry.kind);
		FileSystem::CreateDirectories(targetDirectory);

		const String fileName = FileSystem::FileName(*sourcePath);
		const FilePath targetPath = targetDirectory + fileName;
		if (FileSystem::FullPath(*sourcePath) != FileSystem::FullPath(targetPath))
		{
			if (FileSystem::Exists(targetPath))
			{
				FileSystem::Remove(targetPath);
			}

			if (!FileSystem::Copy(*sourcePath, targetPath))
			{
				editor.statusText = U"Unit image copy failed: {}"_fmt(targetPath);
				return true;
			}
		}

		entry.image = fileName;
		BuildingEditorTextureCache().erase(targetPath);
		BuildingEditorTextureCache().erase(ResolveCatalogVisualPath(entry.kind, fileName));
		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
		editor.statusText = U"Unit image changed: {} -> {}"_fmt(entry.tag, fileName);
		return true;
	}
}
