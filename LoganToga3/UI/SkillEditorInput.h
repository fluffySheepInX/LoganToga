#pragma once
# include <Siv3D.hpp>
# include "SkillEditorCommon.h"
# include "MapEditorDescriptionEditor.h"

namespace LT3
{
	inline bool ProcessSkillEditorInput(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog)
	{
		if (!editor.showSkillEditor)
		{
			return false;
		}

		const RectF panel = SkillEditorPanelRect();
		if (!panel.mouseOver())
		{
			return false;
		}

		if (HandleRectButtonClick(SkillEditorCloseRect()))
		{
			editor.showSkillEditor = false;
			editor.statusText = U"SkillEditor OFF";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSaveRect()))
		{
			SaveSkillEditorDefinitions(editor, defs);
			return true;
		}

		if (editor.skillContextMenuTargetIndex)
		{
			const RectF menuRect = SkillEditorContextMenuRect(editor.skillContextMenuPos);
			const RectF descriptionItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 0);
			if (descriptionItem.leftClicked())
			{
				const int32 srcIdx = *editor.skillContextMenuTargetIndex;
				editor.skillContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.skills.size()))
				{
					editor.selectedSkillIndex = srcIdx;
					const SkillDef& skill = defs.skills[srcIdx];
					OpenDescriptionEditor(editor, DescriptionEditorTargetKind::Skill, srcIdx, U"Skill: {}"_fmt(skill.name.isEmpty() ? skill.tag : skill.name), skill.description);
				}
				return true;
			}
			else if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.skillContextMenuTargetIndex = none;
				return false;
			}
			return true;
		}

		if (SkillEditorListViewportRect().mouseOver())
		{
			editor.skillListScroll = Max(0.0, editor.skillListScroll - Mouse::Wheel() * 48.0);
			const int32 firstIndex = Max(0, static_cast<int32>(editor.skillListScroll / 48.0));
			for (int32 visible = 0; visible < 12; ++visible)
			{
				const int32 skillIndex = firstIndex + visible;
				if (skillIndex >= static_cast<int32>(defs.skills.size()))
				{
					break;
				}

				const RectF row = SkillEditorSkillRowRect(visible);
				if (row.rightClicked())
				{
					editor.selectedSkillIndex = skillIndex;
					editor.skillContextMenuTargetIndex = skillIndex;
					editor.skillContextMenuPos = Cursor::PosF();
					return true;
				}
				if (HandleRectButtonClick(row))
				{
					editor.selectedSkillIndex = skillIndex;
					return true;
				}
			}
		}

		if (SkillEditorUnitViewportRect().mouseOver())
		{
			const RectF viewport = SkillEditorUnitViewportRect();
			const double maxScroll = Max(0.0, static_cast<double>(catalog.entries.size()) * 58.0 - viewport.h);
			editor.skillUnitListScroll = Clamp(editor.skillUnitListScroll - Mouse::Wheel() * 58.0, 0.0, maxScroll);
			const int32 firstIndex = Max(0, static_cast<int32>(editor.skillUnitListScroll / 58.0));
			const int32 visibleRows = static_cast<int32>(viewport.h / 58.0) + 1;
			for (int32 visible = 0; visible < visibleRows; ++visible)
			{
				const int32 unitIndex = firstIndex + visible;
				if (unitIndex >= static_cast<int32>(catalog.entries.size()))
				{
					break;
				}

				if (HandleRectButtonClick(SkillEditorUnitIconRect(visible)) && HasSelectedSkill(editor, defs))
				{
					UnitCatalogEntry& entry = catalog.entries[unitIndex];
					editor.selectedUnitCatalogIndex = unitIndex;
					const String skillTag = defs.skills[editor.selectedSkillIndex].tag;
					entry.skills.clear();
					entry.skills << skillTag;
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
					editor.statusText = U"Linked {} -> {}"_fmt(entry.unit_id, skillTag);
					return true;
				}
			}
		}

		if (!HasSelectedSkill(editor, defs))
		{
			return true;
		}

		SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		const RectF detailViewport = SkillEditorDetailViewportRect();
		const double detailMaxScroll = Max(0.0, SkillEditorDetailContentHeight() - detailViewport.h);
		if (detailViewport.mouseOver())
		{
			editor.skillDetailScroll = Clamp(editor.skillDetailScroll - Mouse::Wheel() * 42.0, 0.0, detailMaxScroll);
		}

		const double scroll = editor.skillDetailScroll;
		if (HandleRectButtonClick(SkillEditorIconBrowseRect(scroll)))
		{
			const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::GIF(), FileFilter::AllFiles() };
			const Array<FilePath> sourcePaths = Dialog::OpenFiles(imageFilters);
			if (!sourcePaths.isEmpty())
			{
				const FilePath targetDirectory = FileSystem::ParentPath(ResolveBuildIconPath(U"__lt3_directory_probe__.png"));
				FileSystem::CreateDirectories(targetDirectory);
				Array<String> iconLayers;
				iconLayers.reserve(sourcePaths.size());

				for (const auto& sourcePath : sourcePaths)
				{
					const String extension = FileSystem::Extension(sourcePath).lowercased();
					if (!(extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"gif"))
					{
						editor.statusText = U"Skill icon must be image file: {}"_fmt(sourcePath);
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
							editor.statusText = U"Skill icon copy failed: {}"_fmt(targetPath);
							return true;
						}
					}

					iconLayers << fileName;
					BuildingEditorTextureCache().erase(ResolveBuildIconPath(fileName));
				}

				skill.iconLayers = iconLayers;
				skill.icon = skill.iconLayers.isEmpty() ? U"" : skill.iconLayers.front();
				SaveSkillEditorDefinitions(editor, defs);
			}
			return true;
		}

		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorKindButtonRect(i, scroll)))
			{
				skill.kind = static_cast<SkillKind>(i);
				SaveSkillEditorDefinitions(editor, defs);
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorMotionButtonRect(i, scroll)))
			{
				skill.projectileMotion = static_cast<SkillProjectileMotion>(i);
				SaveSkillEditorDefinitions(editor, defs);
				return true;
			}
		}

		const double deltas[4] = { -10.0, -1.0, 1.0, 10.0 };
		for (int32 row = 0; row < 10; ++row)
		{
			for (int32 button = 0; button < 4; ++button)
			{
				if (HandleRectButtonClick(SkillEditorValueButtonRect(row, button, scroll)))
				{
					ChangeSkillValue(skill, row, deltas[button]);
					SaveSkillEditorDefinitions(editor, defs);
					return true;
				}
			}
		}

		return true;
	}
}
