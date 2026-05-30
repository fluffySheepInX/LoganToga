#pragma once
# include "SkillEditorDraw.Common.h"

namespace LT3
{
	inline void DrawSkillEditorUnitPanel(const MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		String hoverUnitName;
		const int32 firstUnitIndex = Max(0, static_cast<int32>(editor.skillUnitListScroll / 58.0));
		const int32 visibleUnitRows = static_cast<int32>(SkillEditorUnitViewportRect().h / 58.0) + 1;
		for (int32 visible = 0; visible < visibleUnitRows; ++visible)
		{
			const int32 unitIndex = firstUnitIndex + visible;
			if (unitIndex >= static_cast<int32>(catalog.entries.size()))
			{
				break;
			}

			const UnitCatalogEntry& entry = catalog.entries[unitIndex];
				const RectF iconRect = SkillEditorUnitIconRect(visible);
				const bool linked = HasSelectedSkill(editor, defs) && UnitHasSkill(entry, defs.skills[editor.selectedSkillIndex].tag);
				const bool sandboxSelected = (editor.selectedSkillUnitIndex == unitIndex);
				const int32 skillCount = static_cast<int32>(entry.skills.size());
				const ColorF unitBack = linked ? ColorF{ 0.10, 0.20, 0.16, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 };
				const ColorF unitFrame = sandboxSelected
					? ColorF{ 1.0, 0.84, 0.0, 0.92 }
					: (iconRect.mouseOver() ? ColorF{ 0.20, 0.72, 1.0 } : (linked ? ColorF{ 0.25, 0.9, 0.55 } : (skillCount > 0 ? ColorF{ 0.55, 0.55, 0.85, 0.60 } : ColorF{ 1, 1, 1, 0.16 })));
				iconRect.draw(unitBack).drawFrame(2, unitFrame);

			const FilePath imagePath = ResolveCatalogVisualPath(entry.kind, entry.image);
			if (!imagePath.isEmpty() && FileSystem::Exists(imagePath))
			{
				static HashTable<FilePath, Texture> unitIconCache;
				if (!unitIconCache.contains(imagePath))
				{
					unitIconCache.emplace(imagePath, Texture{ imagePath });
				}
				unitIconCache.at(imagePath).resized(42, 42).drawAt(iconRect.center());
			}
			else
			{
				uiFont(U"?").drawAt(16, iconRect.center(), Palette::Lightgray);
			}

			if (iconRect.mouseOver())
			{
				hoverUnitName = entry.name.isEmpty() ? entry.unit_id : entry.name;
			}

			if (linked)
				{
					const RectF badgeRect{ iconRect.x + iconRect.w - 18.0, iconRect.y + 4.0, 14.0, 14.0 };
					badgeRect.rounded(3.0).draw(ColorF{ 0.10, 0.34, 0.18, 0.98 }).drawFrame(1, ColorF{ 0.45, 1.0, 0.65, 0.90 });
					uiFont(U"✓").drawAt(10, badgeRect.center(), Palette::White);
				}

				if (skillCount > 1)
				{
					const RectF countRect{ iconRect.x + 2.0, iconRect.y + iconRect.h - 16.0, 20.0, 14.0 };
					countRect.rounded(3.0).draw(ColorF{ 0.12, 0.14, 0.30, 0.92 }).drawFrame(1, ColorF{ 0.55, 0.55, 1.0, 0.70 });
					uiFont(U"{}"_fmt(skillCount)).drawAt(9, countRect.center(), ColorF{ 0.78, 0.78, 1.0 });
				}

				if (sandboxSelected)
				{
					const RectF sandboxRect{ iconRect.x + 2.0, iconRect.y + 2.0, 16.0, 14.0 };
					sandboxRect.rounded(3.0).draw(ColorF{ 0.34, 0.24, 0.04, 0.96 }).drawFrame(1, ColorF{ 1.0, 0.84, 0.0, 0.92 });
					uiFont(U"U").drawAt(9, sandboxRect.center(), Palette::White);
				}
		}

		if (!hoverUnitName.isEmpty())
		{
			const Vec2 pos = Cursor::PosF() + Vec2{ 16.0, 14.0 };
			const RectF tip{ pos, 180.0, 24.0 };
			tip.draw(ColorF{ 0.02, 0.03, 0.045, 0.95 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
			uiFont(hoverUnitName).draw(11, tip.x + 8.0, tip.y + 5.0, Palette::White);
		}
	}

	inline void DrawSkillEditorSkillListPanel(const MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont)
	{
		const int32 firstIndex = Max(0, static_cast<int32>(editor.skillListScroll / 48.0));
		const int32 visibleRows = 12;
		for (int32 visible = 0; visible < visibleRows; ++visible)
		{
			const int32 skillIndex = firstIndex + visible;
			if (skillIndex >= static_cast<int32>(defs.skills.size()))
			{
				break;
			}

			const SkillDef& skill = defs.skills[skillIndex];
			const RectF row = SkillEditorSkillRowRect(visible);
			const bool selected = editor.selectedSkillIndex == skillIndex;
			row.draw(selected ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, row.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			const RectF skillIconRect{ row.x + 7.0, row.y + 6.0, 30.0, 30.0 };
			skillIconRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			DrawSkillEditorLayeredIcon(skill, skillIconRect);
			uiFont(skill.name).draw(12, row.x + 44.0, row.y + 5.0, Palette::White);
			uiFont(U"{} / {}"_fmt(skill.tag, SkillKindToTag(skill.kind))).draw(10, row.x + 44.0, row.y + 23.0, Palette::Lightgray);
			if (editor.skillRenameTargetIndex == skillIndex)
			{
				const RectF renameRect{ row.x + 42.0, row.y + 4.0, row.w - 50.0, 34.0 };
				renameRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.98 }).drawFrame(2, ColorF{ 1.0, 0.84, 0.0 });
				uiFont(editor.skillRenameEditText + U"|").draw(13, renameRect.x + 8.0, renameRect.y + 8.0, Palette::White);
				uiFont(U"Enter:確定  Esc:キャンセル").draw(10, renameRect.x + renameRect.w + 6.0, renameRect.y + 9.0, ColorF{ 1, 1, 1, 0.55 });
			}
			if (editor.skillNameEditTargetIndex == skillIndex)
			{
				const RectF renameRect{ row.x + 42.0, row.y + 4.0, row.w - 50.0, 34.0 };
				renameRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.98 }).drawFrame(2, ColorF{ 0.45, 1.0, 0.70 });
				uiFont(editor.skillNameEditText + U"|").draw(13, renameRect.x + 8.0, renameRect.y + 8.0, Palette::White);
				uiFont(U"Enter:確定  Esc:キャンセル").draw(10, renameRect.x + renameRect.w + 6.0, renameRect.y + 9.0, ColorF{ 1, 1, 1, 0.55 });
			}
		}
	}

	inline void DrawSkillEditorContextMenu(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.skillContextMenuTargetIndex)
		{
			return;
		}

		const RectF menuRect = SkillEditorContextMenuRect(editor.skillContextMenuPos);
		menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
		const RectF descriptionItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 0);
		descriptionItem.draw(descriptionItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"説明文編集").draw(13, descriptionItem.x + 8.0, descriptionItem.y + 5.0, Palette::White);
		const RectF duplicateItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 1);
		duplicateItem.draw(duplicateItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"複製").draw(13, duplicateItem.x + 8.0, duplicateItem.y + 5.0, Palette::White);
		const RectF renameItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 2);
		renameItem.draw(renameItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"タグ編集").draw(13, renameItem.x + 8.0, renameItem.y + 5.0, Palette::White);
		const RectF renameNameItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 3);
		renameNameItem.draw(renameNameItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"名前編集").draw(13, renameNameItem.x + 8.0, renameNameItem.y + 5.0, Palette::White);
	}

	inline void DrawSkillEditorUnitContextMenu(const MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.skillUnitContextMenuTargetIndex)
		{
			return;
		}

		const Vec2 menuPos = editor.skillUnitContextMenuPos;
		const RectF menuRect = SkillEditorUnitContextMenuRect(menuPos);
		menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });

		const RectF clearAllItem = SkillEditorUnitContextMenuItemRect(menuPos, 0);
		clearAllItem.draw(clearAllItem.mouseOver() ? ColorF{ 0.22, 0.12, 0.12, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"スキル全解除").draw(13, clearAllItem.x + 8.0, clearAllItem.y + 5.0, ColorF{ 1.0, 0.75, 0.70 });

		const RectF keepOnlyItem = SkillEditorUnitContextMenuItemRect(menuPos, 1);
		const bool canKeep = HasSelectedSkill(editor, defs);
		keepOnlyItem.draw(keepOnlyItem.mouseOver() && canKeep ? ColorF{ 0.12, 0.20, 0.16, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"このスキルのみ残す").draw(13, keepOnlyItem.x + 8.0, keepOnlyItem.y + 5.0, canKeep ? ColorF{ Palette::White } : ColorF{ 1, 1, 1, 0.35 });
	}
}
