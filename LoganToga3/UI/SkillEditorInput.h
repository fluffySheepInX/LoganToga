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
		const RectF sandboxPreview = SkillEditorSandboxPreviewRect();
		if (!panel.mouseOver() && !(editor.showSkillSandboxPreview && sandboxPreview.mouseOver()))
		{
			return false;
		}

		if (editor.showSkillSandboxPreview && sandboxPreview.mouseOver())
		{
			EnsureSkillSandboxReady(editor);
			ResetSkillSandboxForSkill(editor);
			if (HasSelectedSkill(editor, defs))
			{
				SkillDef& skill = defs.skills[editor.selectedSkillIndex];
				UpdateSkillSandbox(editor, skill, Scene::DeltaTime());

				if (HandleRectButtonClick(SkillEditorSandboxButtonRect(0)))
				{
					FireSkillSandbox(editor, skill);
					return true;
				}
				if (HandleRectButtonClick(SkillEditorSandboxButtonRect(1)))
				{
					editor.skillSandboxAutoFire = !editor.skillSandboxAutoFire;
					editor.statusText = editor.skillSandboxAutoFire ? U"Skill sandbox auto fire ON" : U"Skill sandbox auto fire OFF";
					return true;
				}
				if (HandleRectButtonClick(SkillEditorSandboxButtonRect(2)))
				{
					ResetSkillSandbox(editor);
					editor.statusText = U"Skill sandbox reset";
					return true;
				}

				const RectF arena = SkillEditorSandboxArenaRect();
				if (MouseL.down() && Circle{ editor.skillSandboxTargetPos, 34.0 }.mouseOver())
				{
					editor.skillSandboxDraggingTarget = true;
					return true;
				}
				if (!MouseL.pressed())
				{
					editor.skillSandboxDraggingTarget = false;
				}
				if (editor.skillSandboxDraggingTarget)
				{
					const Vec2 mouse = Cursor::PosF();
					editor.skillSandboxTargetPos = Vec2{
						Clamp(mouse.x, arena.x + 34.0, arena.x + arena.w - 34.0),
						Clamp(mouse.y, arena.y + 34.0, arena.y + arena.h - 34.0)
					};
					return true;
				}
			}
			return true;
		}

		if (HandleRectButtonClick(SkillEditorCloseRect()))
		{
			editor.showSkillEditor = false;
			editor.statusText = U"SkillEditor OFF";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSandboxToggleRect()))
		{
			editor.showSkillSandboxPreview = !editor.showSkillSandboxPreview;
			if (editor.showSkillSandboxPreview)
			{
				EnsureSkillSandboxReady(editor);
				ResetSkillSandboxForSkill(editor);
			}
			editor.statusText = editor.showSkillSandboxPreview ? U"Skill sandbox preview ON" : U"Skill sandbox preview OFF";
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
					const String skillTag = defs.skills[editor.selectedSkillIndex].tag;
					if (UnitHasSkill(entry, skillTag))
					{
						entry.skills.remove_if([&](const String& tag)
						{
							return tag == skillTag;
						});
						editor.statusText = U"Unlinked {} -> {}"_fmt(entry.unit_id, skillTag);
					}
					else
					{
						entry.skills.clear();
						entry.skills << skillTag;
						editor.statusText = U"Linked {} -> {}"_fmt(entry.unit_id, skillTag);
					}
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
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

				skill.iconLayers = NormalizeSkillIconLayerOrder(iconLayers);
				skill.icon = skill.iconLayers.isEmpty() ? U"" : skill.iconLayers.front();
				const Array<String> warnings = ValidateSkillIconLayers(skill);
				if (warnings.isEmpty())
				{
					defs.skillIconWarningsByTag.erase(skill.tag);
				}
				else
				{
					defs.skillIconWarningsByTag[skill.tag] = warnings;
					editor.statusText = U"Skill icon warning: {}"_fmt(warnings.front());
				}
				SaveSkillEditorDefinitions(editor, defs);
			}
			return true;
		}

		for (int32 imageIndex = 0; imageIndex < 2; ++imageIndex)
		{
			if (HandleRectButtonClick(SkillEditorProjectileImageBrowseRect(imageIndex, scroll)))
			{
				const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::GIF(), FileFilter::AllFiles() };
				const Optional<FilePath> sourcePath = Dialog::OpenFile(imageFilters);
				if (sourcePath)
				{
					String fileName;
					if (!CopySkillEditorImageToBuildIcons(*sourcePath, fileName, editor.statusText))
					{
						return true;
					}

					if (imageIndex == 0)
					{
						skill.projectileImage = fileName;
					}
					else
					{
						skill.projectileDiagonalImage = fileName;
					}
					SaveSkillEditorDefinitions(editor, defs);
				}
				return true;
			}
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

		for (int32 i = 0; i < static_cast<int32>(SkillCenterLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorCenterButtonRect(i, scroll)))
			{
				skill.projectileCenter = static_cast<SkillProjectileCenter>(i);
				SaveSkillEditorDefinitions(editor, defs);
				return true;
			}
		}

		if (HandleRectButtonClick(SkillEditorToggleButtonRect(0, scroll)))
		{
			skill.projectileHoming = !skill.projectileHoming;
			SaveSkillEditorDefinitions(editor, defs);
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(1, scroll)))
		{
			skill.projectileD360 = !skill.projectileD360;
			SaveSkillEditorDefinitions(editor, defs);
			return true;
		}

		const double deltas[4] = { -10.0, -1.0, 1.0, 10.0 };
		for (int32 row = 0; row < 16; ++row)
		{
			if (IsSkillEditorValueRowLocked(skill, row))
			{
				continue;
			}
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
