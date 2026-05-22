#pragma once
# include <Siv3D.hpp>
# include "SkillEditorCommon.h"
# include "BattleUnitRendererAssetOps.h"

namespace LT3
{
	inline void DrawSkillEditorValueRow(const Font& uiFont, int32 row, StringView label, StringView value, double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		const RectF viewport = SkillEditorDetailViewportRect();
		const double y = detail.y + 290.0 + row * 38.0 - scroll;
		if (y + 28.0 < viewport.y || viewport.y + viewport.h < y)
		{
			return;
		}
		uiFont(label).draw(11, detail.x + 8.0, y + 4.0, Palette::Aqua);
		uiFont(value).draw(12, detail.x + 64.0, y + 4.0, Palette::Gold);
		DrawRectButton(SkillEditorValueButtonRect(row, 0, scroll), U"-10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorValueButtonRect(row, 1, scroll), U"-1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorValueButtonRect(row, 2, scroll), U"+1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorValueButtonRect(row, 3, scroll), U"+10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawSkillEditor(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showSkillEditor)
		{
			return;
		}

		const RectF panel = SkillEditorPanelRect();
		const RectF list = SkillEditorListViewportRect();
		const RectF detail = SkillEditorDetailRect();
		const RectF unitViewport = SkillEditorUnitViewportRect();
		const RectF detailViewport = SkillEditorDetailViewportRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Skill Editor").draw(16, panel.x + 18.0, panel.y + 14.0, Palette::White);
		DrawRectButton(SkillEditorCloseRect(), U"×", false, uiFont);
		DrawRectButton(SkillEditorSaveRect(), U"Save TOML", false, uiFont, RectButtonStyle{ .fontSize = 12 });

		unitViewport.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		list.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detail.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detailViewport.draw(ColorF{ 0, 0, 0, 0.08 });
		uiFont(U"Units").draw(11, unitViewport.x + 10.0, unitViewport.y - 18.0, Palette::Aqua);
		uiFont(U"Skills").draw(11, list.x + 8.0, list.y - 18.0, Palette::Aqua);

		String hoverUnitName;
		const int32 firstUnitIndex = Max(0, static_cast<int32>(editor.skillUnitListScroll / 58.0));
		const int32 visibleUnitRows = static_cast<int32>(unitViewport.h / 58.0) + 1;
		for (int32 visible = 0; visible < visibleUnitRows; ++visible)
		{
			const int32 unitIndex = firstUnitIndex + visible;
			if (unitIndex >= static_cast<int32>(catalog.entries.size()))
			{
				break;
			}

			const UnitCatalogEntry& entry = catalog.entries[unitIndex];
			const RectF iconRect = SkillEditorUnitIconRect(visible);
			const bool selectedUnit = editor.selectedUnitCatalogIndex == unitIndex;
			const bool linked = HasSelectedSkill(editor, defs) && UnitHasSkill(entry, defs.skills[editor.selectedSkillIndex].tag);
			const ColorF unitBack = selectedUnit ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : (linked ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 });
			const ColorF unitFrame = iconRect.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : (selectedUnit ? ColorF{ 1.0, 0.84, 0.0 } : (linked ? ColorF{ 0.25, 0.9, 0.55 } : ColorF{ 1, 1, 1, 0.16 }));
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
		}

		if (!hoverUnitName.isEmpty())
		{
			const Vec2 pos = Cursor::PosF() + Vec2{ 16.0, 14.0 };
			const RectF tip{ pos, 180.0, 24.0 };
			tip.draw(ColorF{ 0.02, 0.03, 0.045, 0.95 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
			uiFont(hoverUnitName).draw(11, tip.x + 8.0, tip.y + 5.0, Palette::White);
		}

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
		}

		if (editor.skillContextMenuTargetIndex)
		{
			const RectF menuRect = SkillEditorContextMenuRect(editor.skillContextMenuPos);
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			const RectF descriptionItem = SkillEditorContextMenuItemRect(editor.skillContextMenuPos, 0);
			descriptionItem.draw(descriptionItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"説明文編集").draw(13, descriptionItem.x + 8.0, descriptionItem.y + 5.0, Palette::White);
		}

		if (!HasSelectedSkill(editor, defs))
		{
			uiFont(U"スキルを選択してください").draw(13, detail.x + 18.0, detail.y + 18.0, Palette::Lightgray);
			return;
		}

		const SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		const double scroll = editor.skillDetailScroll;
		const double contentTop = detail.y - scroll;
		uiFont(skill.name).draw(16, detail.x + 12.0, contentTop + 12.0, Palette::White);
		uiFont(U"tag: {}"_fmt(skill.tag)).draw(11, detail.x + 12.0, contentTop + 36.0, Palette::Lightgray);
		const RectF iconRect = SkillEditorIconPreviewRect(scroll);
		iconRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		if (!SkillEditorIconPaths(skill).isEmpty())
		{
			DrawSkillEditorLayeredIcon(skill, iconRect.stretched(-4.0));
		}
		else
		{
			uiFont(U"icon").drawAt(10, iconRect.center(), Palette::Gray);
		}
		uiFont(U"icon: {}"_fmt(skill.icon.isEmpty() ? U"<none>" : skill.icon)).draw(10, detail.x + 72.0, contentTop + 58.0, Palette::Lightgray);
		DrawRectButton(SkillEditorIconBrowseRect(scroll), U"参照", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		uiFont(U"Kind").draw(12, detail.x + 8.0, contentTop + 130.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorKindButtonRect(i, scroll), SkillKindLabels()[i], SkillKindIndex(skill.kind) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		uiFont(U"Projectile Motion").draw(12, detail.x + 8.0, contentTop + 208.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorMotionButtonRect(i, scroll), SkillMotionLabels()[i], SkillMotionIndex(skill.projectileMotion) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		DrawSkillEditorValueRow(uiFont, 0, U"range", U"{:.1f}"_fmt(skill.range), scroll);
		DrawSkillEditorValueRow(uiFont, 1, U"cool", U"{:.2f}"_fmt(skill.cooldownSec), scroll);
		DrawSkillEditorValueRow(uiFont, 2, U"dmg", U"{}"_fmt(skill.damage), scroll);
		DrawSkillEditorValueRow(uiFont, 3, U"speed", U"{:.1f}"_fmt(skill.projectileSpeed), scroll);
		DrawSkillEditorValueRow(uiFont, 4, U"burst", U"{}"_fmt(skill.burstCount), scroll);
		DrawSkillEditorValueRow(uiFont, 5, U"spread", U"{:.1f}"_fmt(skill.spreadDeg), scroll);
		DrawSkillEditorValueRow(uiFont, 6, U"arc", U"{:.1f}"_fmt(skill.arcHeight), scroll);
		DrawSkillEditorValueRow(uiFont, 7, U"orbitR", U"{:.1f}"_fmt(skill.orbitRadius), scroll);
		DrawSkillEditorValueRow(uiFont, 8, U"orbitV", U"{:.1f}"_fmt(skill.orbitAngularSpeedDeg), scroll);
		DrawSkillEditorValueRow(uiFont, 9, U"orbitT", U"{:.2f}"_fmt(skill.orbitDurationSec), scroll);

		const double maxScroll = Max(0.0, SkillEditorDetailContentHeight() - detailViewport.h);
		if (maxScroll > 0.0)
		{
			const double rate = Clamp(editor.skillDetailScroll / maxScroll, 0.0, 1.0);
			const double handleHeight = Max(32.0, detailViewport.h * detailViewport.h / SkillEditorDetailContentHeight());
			RectF{ detail.x + detail.w - 7.0, detailViewport.y, 4.0, detailViewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
			RectF{ detail.x + detail.w - 7.0, detailViewport.y + (detailViewport.h - handleHeight) * rate, 4.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
		}
	}
}
