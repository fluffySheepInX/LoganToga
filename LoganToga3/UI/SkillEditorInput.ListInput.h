#pragma once
# include "SkillEditorInput.Common.h"
# include "MapEditorDescriptionEditor.h"

namespace LT3
{
		inline bool ProcessSkillEditorRenameInput(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog)
		{
			if (!editor.skillRenameTargetIndex)
			{
				if (!editor.skillNameEditTargetIndex)
				{
					return false;
				}
			}

			if (editor.skillNameEditTargetIndex)
			{
				TextInput::UpdateText(editor.skillNameEditText);
				editor.skillNameEditText.remove_if([](char32 ch)
				{
					return ch == U'\n' || ch == U'\r' || ch == U'\t' || ch < U' ';
				});
				if ((KeyControl | KeyCommand).pressed() && KeyV.down())
				{
					String clip;
					if (Clipboard::GetText(clip) && !clip.isEmpty())
					{
						clip.remove_if([](char32 ch)
						{
							return ch == U'\n' || ch == U'\r' || ch == U'\t' || ch < U' ';
						});
						editor.skillNameEditText += clip;
					}
				}
				if (KeyEnter.down())
				{
					const int32 idx = *editor.skillNameEditTargetIndex;
					RenameSkillName(editor, defs, idx, editor.skillNameEditText);
					editor.skillNameEditTargetIndex = none;
					editor.skillNameEditText.clear();
					return true;
				}
				if (KeyEscape.down())
				{
					editor.skillNameEditTargetIndex = none;
					editor.skillNameEditText.clear();
					return true;
				}
				return true;
			}

			TextInput::UpdateText(editor.skillRenameEditText);
			editor.skillRenameEditText = NormalizeSkillTagForEditor(editor.skillRenameEditText);
			if ((KeyControl | KeyCommand).pressed() && KeyV.down())
			{
				String clip;
				if (Clipboard::GetText(clip) && !clip.isEmpty())
				{
					editor.skillRenameEditText += NormalizeSkillTagForEditor(clip);
				}
			}

			if (KeyEnter.down())
			{
				const int32 idx = *editor.skillRenameTargetIndex;
				RenameSkillTag(editor, defs, catalog, idx, editor.skillRenameEditText);
				editor.skillRenameTargetIndex = none;
				editor.skillRenameEditText.clear();
				return true;
			}
			if (KeyEscape.down())
			{
				editor.skillRenameTargetIndex = none;
				editor.skillRenameEditText.clear();
				return true;
			}

			return true;
		}

	/// <summary>
	/// スキルコンテキストメニュー入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorContextMenuInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (!editor.skillContextMenuTargetIndex)
		{
			return false;
		}

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
		const RectF duplicateItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 1);
		if (duplicateItem.leftClicked())
		{
			const int32 srcIdx = *editor.skillContextMenuTargetIndex;
			editor.skillContextMenuTargetIndex = none;
			return DuplicateSkillDefinition(editor, defs, srcIdx);
		}
			const RectF renameItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 2);
			if (renameItem.leftClicked())
			{
				const int32 srcIdx = *editor.skillContextMenuTargetIndex;
				editor.skillContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.skills.size()))
				{
					editor.selectedSkillIndex = srcIdx;
					editor.skillRenameTargetIndex = srcIdx;
					editor.skillRenameEditText = defs.skills[srcIdx].tag;
				}
				return true;
			}
			const RectF renameNameItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 3);
			if (renameNameItem.leftClicked())
			{
				const int32 srcIdx = *editor.skillContextMenuTargetIndex;
				editor.skillContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.skills.size()))
				{
					editor.selectedSkillIndex = srcIdx;
					editor.skillNameEditTargetIndex = srcIdx;
					editor.skillNameEditText = defs.skills[srcIdx].name;
				}
				return true;
			}
		if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			editor.skillContextMenuTargetIndex = none;
			return false;
		}
		return true;
	}

	/// <summary>
	/// スキル一覧入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorListInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (!SkillEditorListViewportRect().mouseOver())
		{
			return false;
		}

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

		return false;
	}

	/// <summary>
	/// ユニットコンテキストメニュー入力を処理します。
	/// 項目0: スキル全解除 / 項目1: このスキルのみ残す
	/// </summary>
	inline bool ProcessSkillEditorUnitContextMenuInput(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog)
	{
		if (!editor.skillUnitContextMenuTargetIndex)
		{
			return false;
		}

		const Vec2 menuPos = editor.skillUnitContextMenuPos;
		const RectF menuRect = SkillEditorUnitContextMenuRect(menuPos);

		// メニュー外クリックで閉じる
		if (MouseL.down() && !menuRect.mouseOver())
		{
			editor.skillUnitContextMenuTargetIndex = none;
			return false;
		}

		const int32 targetUnitIndex = *editor.skillUnitContextMenuTargetIndex;
		if (targetUnitIndex < 0 || targetUnitIndex >= static_cast<int32>(catalog.entries.size()))
		{
			editor.skillUnitContextMenuTargetIndex = none;
			return true;
		}

		// 項目0: 全スキル解除
		const RectF clearAllItem = SkillEditorUnitContextMenuItemRect(menuPos, 0);
		if (clearAllItem.leftClicked())
		{
			editor.skillUnitContextMenuTargetIndex = none;
			UnitCatalogEntry& entry = catalog.entries[targetUnitIndex];
			entry.skills.clear();
			editor.statusText = U"Unlinked all skills -> {}"_fmt(entry.unit_id);
			SaveUnitCatalogToml(catalog, editor.statusText);
			editor.unitCatalogDirty = true;
			return true;
		}

		// 項目1: このスキルのみ残す (選択スキルがあるときのみ有効)
		const RectF keepOnlyItem = SkillEditorUnitContextMenuItemRect(menuPos, 1);
		if (keepOnlyItem.leftClicked())
		{
			editor.skillUnitContextMenuTargetIndex = none;
			if (HasSelectedSkill(editor, defs))
			{
				UnitCatalogEntry& entry = catalog.entries[targetUnitIndex];
				const String skillTag = defs.skills[editor.selectedSkillIndex].tag;
				entry.skills.clear();
				entry.skills << skillTag;
				editor.statusText = U"Set only {} -> {}"_fmt(skillTag, entry.unit_id);
				SaveUnitCatalogToml(catalog, editor.statusText);
				editor.unitCatalogDirty = true;
			}
			return true;
		}

		return true;
	}

	/// <summary>
	/// ユニット一覧からスキル紐付け入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorUnitBindingInput(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog)
	{
		if (!SkillEditorUnitViewportRect().mouseOver())
		{
			return false;
		}

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

			if (HandleRectButtonClick(SkillEditorUnitIconRect(visible)))
			{
				editor.selectedSkillUnitIndex = unitIndex;
				if (editor.skillSandboxMode == SkillSandboxMode::Unit)
				{
					editor.skillSandboxUnitCatalogIndex = unitIndex;
					editor.skillSandboxActiveSkillId = InvalidSkillDefId;
					ResetSkillSandbox(editor);
					const UnitCatalogEntry& entry = catalog.entries[unitIndex];
					editor.statusText = U"Sandbox unit: {}"_fmt(entry.name.isEmpty() ? entry.unit_id : entry.name);
					return true;
				}

				if (!HasSelectedSkill(editor, defs))
				{
					return true;
				}

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
					entry.skills << skillTag;
					editor.statusText = U"Linked {} -> {}"_fmt(entry.unit_id, skillTag);
				}
				SaveUnitCatalogToml(catalog, editor.statusText);
				editor.unitCatalogDirty = true;
				return true;
			}

			if (SkillEditorUnitIconRect(visible).rightClicked())
			{
				editor.skillUnitContextMenuTargetIndex = unitIndex;
				editor.skillUnitContextMenuPos = Cursor::PosF();
				return true;
			}
		}

		return false;
	}
}
