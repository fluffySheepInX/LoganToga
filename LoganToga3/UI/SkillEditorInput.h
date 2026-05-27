#pragma once
# include <Siv3D.hpp>
# include "EditorMutationHelpers.h"
# include "SkillEditorCommon.h"
# include "MapEditorDescriptionEditor.h"

namespace LT3
{
	template <class Mutator>
	inline bool MutateSelectedSkillDefinition(MapEditorState& editor, DefinitionStores& defs, Mutator&& mutator)
	{
		if (!HasSelectedSkill(editor, defs))
		{
			return false;
		}

		SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		return ApplyEditorMutation([&]()
		{
			return mutator(skill);
		}, [&]()
		{
			SaveSkillEditorDefinitions(editor, defs);
		});
	}

		inline bool CommitSelectedSkillEditorValueText(MapEditorState& editor, DefinitionStores& defs, int32 row, const String& text)
		{
			const bool committed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return TryCommitSkillEditorValueText(selected, row, text);
			});

			if (!committed)
			{
				editor.statusText = U"Invalid skill value: {}"_fmt(text);
			}

			return committed;
		}

		inline bool HandleSkillEditorValueStepperAction(MapEditorState& editor, DefinitionStores& defs, int32 row, RectNumberStepperRects rects)
	{
		switch (DetectRectNumberStepperInput(rects))
		{
		case RectNumberStepperInputAction::StartValueEdit:
			editor.skillValueEditingRow = row;
				editor.skillValueEditingText = U"{}"_fmt(GetSkillEditorValue(defs.skills[editor.selectedSkillIndex], row));
			editor.skillValueStepMenuRow = none;
			return true;
		case RectNumberStepperInputAction::CycleStep:
			CycleSkillEditorValueStep(editor, row);
			editor.statusText = U"Skill value step set to {}"_fmt(SkillEditorValueStep(editor, row));
			return true;
		case RectNumberStepperInputAction::OpenStepMenu:
			editor.skillValueStepMenuRow = row;
			editor.skillValueStepMenuPos = Cursor::PosF();
			return true;
		case RectNumberStepperInputAction::Decrement:
		{
			const double step = ApplyTemporaryStepModifier(SkillEditorValueStep(editor, row));
				return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
					ChangeSkillValue(selected, row, -step);
				return true;
			});
		}
		case RectNumberStepperInputAction::Increment:
		{
			const double step = ApplyTemporaryStepModifier(SkillEditorValueStep(editor, row));
				return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
					ChangeSkillValue(selected, row, step);
				return true;
			});
		}
		default:
			return false;
		}
	}

	inline bool ProcessSkillEditorInput(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog)
	{
		if (!editor.showSkillEditor)
		{
			return false;
		}

		EnsureSkillEditorValueSteps(editor);

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
		EnsureSkillEditorResourceCostSteps(editor, skill);
		const RectF detailViewport = SkillEditorDetailViewportRect();
		const double detailMaxScroll = Max(0.0, SkillEditorDetailContentHeight() - detailViewport.h);

		if (editor.skillValueEditingRow >= 0)
		{
			TextInput::UpdateText(editor.skillValueEditingText);
			if (KeyEscape.down())
			{
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				CommitSelectedSkillEditorValueText(editor, defs, editor.skillValueEditingRow, editor.skillValueEditingText);
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
		}

		if (editor.skillResourceCostEditingIndex >= 0)
		{
			TextInput::UpdateText(editor.skillResourceCostEditingText);
			if (KeyEscape.down())
			{
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				const int32 costIndex = editor.skillResourceCostEditingIndex;
				const String text = editor.skillResourceCostEditingText;
				const bool committed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return TryCommitSkillResourceCostAmountText(selected, costIndex, text);
				});
				if (!committed)
				{
					editor.statusText = U"Invalid resource cost: {}"_fmt(text);
				}
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
		}

		if (detailViewport.mouseOver())
		{
			editor.skillDetailScroll = Clamp(editor.skillDetailScroll - Mouse::Wheel() * 42.0, 0.0, detailMaxScroll);
		}

		const double scroll = editor.skillDetailScroll;
		if (editor.skillValueStepMenuRow)
		{
			const Array<double>& steps = SkillEditorDefaultValueSteps();
			const RectF menuRect = SkillEditorValueStepMenuRect(editor.skillValueStepMenuPos, static_cast<int32>(steps.size()));
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				if (SkillEditorValueStepMenuItemRect(editor.skillValueStepMenuPos, i).leftClicked())
				{
					SetSkillEditorValueStep(editor, *editor.skillValueStepMenuRow, steps[i]);
					editor.skillValueStepMenuRow = none;
					editor.statusText = U"Skill value step set to {}"_fmt(steps[i]);
					return true;
				}
			}

			if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.skillValueStepMenuRow = none;
				return true;
			}

			return true;
		}

		if (editor.skillResourceCostStepMenuIndex)
		{
			const Array<double>& steps = SkillEditorResourceCostStepOptions();
			const RectF menuRect = SkillEditorResourceCostStepMenuRect(editor.skillResourceCostStepMenuPos, static_cast<int32>(steps.size()));
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				if (SkillEditorResourceCostStepMenuItemRect(editor.skillResourceCostStepMenuPos, i).leftClicked())
				{
					SetSkillEditorResourceCostStep(editor, *editor.skillResourceCostStepMenuIndex, steps[i]);
					editor.skillResourceCostStepMenuIndex = none;
					editor.statusText = U"Resource cost step set to {}"_fmt(steps[i]);
					return true;
				}
			}

			if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.skillResourceCostStepMenuIndex = none;
				return true;
			}

			return true;
		}

		if (editor.skillResourceCostEditingIndex >= 0 && MouseL.down())
		{
			const int32 costIndex = editor.skillResourceCostEditingIndex;
			const RectF editingRect = SkillEditorResourceCostAmountStepperRects(costIndex, scroll).value;
			if (!editingRect.mouseOver())
			{
				const String text = editor.skillResourceCostEditingText;
				const bool committed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return TryCommitSkillResourceCostAmountText(selected, costIndex, text);
				});
				if (!committed)
				{
					editor.statusText = U"Invalid resource cost: {}"_fmt(text);
				}
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(skill.resourceCosts.size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorResourceCostTagRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					if (i < 0 || i >= static_cast<int32>(selected.resourceCosts.size()) || defs.resources.isEmpty())
					{
						return false;
					}

					int32 currentIndex = 0;
					for (int32 r = 0; r < static_cast<int32>(defs.resources.size()); ++r)
					{
						if (defs.resources[r].tag == selected.resourceCosts[i].resourceTag)
						{
							currentIndex = r;
							break;
						}
					}

					const int32 nextIndex = (currentIndex + 1) % static_cast<int32>(defs.resources.size());
					return SetFieldIfChanged(selected.resourceCosts[i].resourceTag, defs.resources[nextIndex].tag);
				});
				return true;
			}

			if (HandleRectButtonClick(SkillEditorResourceCostRemoveRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					if (i < 0 || i >= static_cast<int32>(selected.resourceCosts.size()))
					{
						return false;
					}
					selected.resourceCosts.remove_at(i);
					return true;
				});
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				editor.skillResourceCostStepMenuIndex = none;
				EnsureSkillEditorResourceCostSteps(editor, defs.skills[editor.selectedSkillIndex]);
				return true;
			}

			switch (DetectRectNumberStepperInput(SkillEditorResourceCostAmountStepperRects(i, scroll)))
			{
			case RectNumberStepperInputAction::StartValueEdit:
				editor.skillResourceCostEditingIndex = i;
				editor.skillResourceCostEditingText = U"{}"_fmt(skill.resourceCosts[i].amount);
				editor.skillResourceCostStepMenuIndex = none;
				return true;
			case RectNumberStepperInputAction::CycleStep:
				CycleSkillEditorResourceCostStep(editor, i);
				editor.statusText = U"Resource cost step set to {}"_fmt(SkillEditorResourceCostStep(editor, i));
				return true;
			case RectNumberStepperInputAction::OpenStepMenu:
				editor.skillResourceCostStepMenuIndex = i;
				editor.skillResourceCostStepMenuPos = Cursor::PosF();
				return true;
			case RectNumberStepperInputAction::Decrement:
			{
				const int32 delta = static_cast<int32>(ApplyTemporaryStepModifier(SkillEditorResourceCostStep(editor, i)));
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return ChangeSkillResourceCostAmount(selected, i, -Max(1, delta));
				});
				return true;
			}
			case RectNumberStepperInputAction::Increment:
			{
				const int32 delta = static_cast<int32>(ApplyTemporaryStepModifier(SkillEditorResourceCostStep(editor, i)));
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return ChangeSkillResourceCostAmount(selected, i, Max(1, delta));
				});
				return true;
			}
			default:
				break;
			}
		}

		if (HandleRectButtonClick(SkillEditorResourceCostAddRect(static_cast<int32>(skill.resourceCosts.size()), scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				if (defs.resources.isEmpty())
				{
					return false;
				}
				selected.resourceCosts << SkillResourceCostDef{ defs.resources.front().tag, 1 };
				return true;
			});
			EnsureSkillEditorResourceCostSteps(editor, defs.skills[editor.selectedSkillIndex]);
			return true;
		}

		if (editor.skillValueEditingRow >= 0 && MouseL.down())
		{
			const RectF editingRect = SkillEditorValueFieldRect(editor.skillValueEditingRow, scroll);
			if (!editingRect.mouseOver())
			{
				CommitSelectedSkillEditorValueText(editor, defs, editor.skillValueEditingRow, editor.skillValueEditingText);
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
		}

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

				const Array<String> normalizedIconLayers = NormalizeSkillIconLayerOrder(iconLayers);
				String skillTag;
				Array<String> warnings;
				ApplyEditorMutation([&]()
				{
					return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
					{
						skillTag = selected.tag;
						selected.iconLayers = normalizedIconLayers;
						selected.icon = selected.iconLayers.isEmpty() ? U"" : selected.iconLayers.front();
						warnings = ValidateSkillIconLayers(selected);
						return true;
					});
				}, [&]()
				{
					if (warnings.isEmpty())
					{
						defs.skillIconWarningsByTag.erase(skillTag);
					}
					else
					{
						defs.skillIconWarningsByTag[skillTag] = warnings;
						editor.statusText = U"Skill icon warning: {}"_fmt(warnings.front());
					}
				});
			}
			return true;
		}

		for (int32 imageIndex = 0; imageIndex < 2; ++imageIndex)
		{
			if (HandleRectButtonClick(SkillEditorProjectileImageClearRect(imageIndex, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					const String empty;
					if (imageIndex == 0)
					{
						return SetFieldIfChanged(selected.projectileImage, empty);
					}

					return SetFieldIfChanged(selected.projectileDiagonalImage, empty);
				});
				editor.statusText = (imageIndex == 0) ? U"Skill projectile image cleared" : U"Skill projectile diagonal image cleared";
				return true;
			}

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

					MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
					{
						if (imageIndex == 0)
						{
							return SetFieldIfChanged(selected.projectileImage, fileName);
						}

						return SetFieldIfChanged(selected.projectileDiagonalImage, fileName);
					});
				}
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorKindButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.kind, static_cast<SkillKind>(i));
				});
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorMotionButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.projectileMotion, static_cast<SkillProjectileMotion>(i));
				});
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillCenterLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorCenterButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.projectileCenter, static_cast<SkillProjectileCenter>(i));
				});
				return true;
			}
		}

		if (HandleRectButtonClick(SkillEditorToggleButtonRect(0, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.projectileHoming);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(1, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.projectileD360);
			});
			return true;
		}

		for (int32 row = 0; row < 22; ++row)
		{
			if (IsSkillEditorValueRowLocked(skill, row))
			{
				continue;
			}

			const RectNumberStepperRects rects = SkillEditorValueStepperRects(row, scroll);
			if (HandleSkillEditorValueStepperAction(editor, defs, row, rects))
			{
				return true;
			}
		}

		return true;
	}
}
