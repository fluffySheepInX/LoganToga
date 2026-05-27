#pragma once
# include <Siv3D.hpp>
# include "SkillEditorCommon.h"
# include "BattleUnitRendererAssetOps.h"
# include "BattleProjectileRendererOps.h"

namespace LT3
{
	inline void DrawSkillEditorInfoIcon(const RectF& rect, const FilePath& iconPath, StringView fallbackText, const Font& uiFont)
	{
		rect.draw(ColorF{ 0.05, 0.06, 0.08, 0.84 }).drawFrame(1, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		if (!iconPath.isEmpty() && FileSystem::Exists(iconPath))
		{
			auto& cache = BuildingEditorTextureCache();
			if (!cache.contains(iconPath))
			{
				cache.emplace(iconPath, Texture{ iconPath });
			}

			cache.at(iconPath).resized(static_cast<int32>(rect.w - 2.0), static_cast<int32>(rect.h - 2.0)).drawAt(rect.center());
		}
		else
		{
			uiFont(fallbackText).drawAt(12, rect.center(), Palette::White);
		}
	}

	inline void DrawSkillEditorTooltip(const Font& uiFont, StringView title, const Array<String>& lines)
	{
		if (lines.isEmpty())
		{
			return;
		}

		const Vec2 pos = Cursor::PosF() + Vec2{ 16.0, 14.0 };
		const double width = 360.0;
		const double height = 34.0 + lines.size() * 18.0;
		const RectF tip{ pos, width, height };
		tip.draw(ColorF{ 0.02, 0.03, 0.045, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.22 });
		uiFont(title).draw(12, tip.x + 8.0, tip.y + 7.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(lines.size()); ++i)
		{
			uiFont(lines[i]).draw(10, tip.x + 8.0, tip.y + 28.0 + i * 18.0, Palette::White);
		}
	}

	inline void DrawSkillEditorValueRow(const Font& uiFont, const MapEditorState& editor, const SkillDef& skill, int32 row, StringView label, StringView value, double scroll, String& hoverHelpText, String& hoverNoteText)
	{
		const RectF detail = SkillEditorDetailRect();
		const RectF viewport = SkillEditorDetailViewportRect();
		const double y = detail.y + 358.0 + row * 38.0 - scroll;
		if (y + 28.0 < viewport.y || viewport.y + viewport.h < y)
		{
			return;
		}
		const bool locked = IsSkillEditorValueRowLocked(skill, row);
		uiFont(label).draw(11, detail.x + 8.0, y + 4.0, locked ? ColorF{ 0.62, 0.66, 0.72 } : ColorF{ Palette::Aqua });
		const String shownValue = (editor.skillValueEditingRow == row) ? editor.skillValueEditingText : String{ value };
		const String stepText = U"x{}"_fmt(SkillEditorValueStep(editor, row));
		DrawRectNumberStepper(SkillEditorValueStepperRects(row, scroll), shownValue, stepText, editor.skillValueEditingRow == row, editor.skillValueStepMenuRow && *editor.skillValueStepMenuRow == row, !locked, uiFont);
		const RectF helpRect = SkillEditorValueHelpIconRect(row, scroll);
		DrawSkillEditorInfoIcon(helpRect, SkillEditorHelpIconPath(), U"?", uiFont);
		if (helpRect.mouseOver() && row < static_cast<int32>(SkillEditorValueHelpTexts().size()))
		{
			hoverHelpText = SkillEditorValueHelpTexts()[row];
		}
		const bool showNoteIcon = locked || (row == 20 && skill.projectileMotion == SkillProjectileMotion::Swing);
		if (showNoteIcon)
		{
			const RectF noteRect = SkillEditorValueNoteIconRect(row, scroll);
			DrawSkillEditorInfoIcon(noteRect, SkillEditorHelpIconPath(), U"?", uiFont);
			if (noteRect.mouseOver())
			{
				hoverNoteText = SkillEditorValueRowLockNote(skill, row);
			}
		}
	}

	inline void DrawSkillEditorSandboxPreview(const MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showSkillSandboxPreview)
		{
			return;
		}

		const RectF preview = SkillEditorSandboxPreviewRect();
		preview.draw(ColorF{ 0.015, 0.022, 0.032, 0.96 }).drawFrame(2, ColorF{ 0.25, 0.70, 1.0, 0.55 });
		uiFont(U"Skill Sandbox Preview").draw(18, preview.x + 18.0, preview.y + 14.0, Palette::White);

		const bool hasSkill = HasSelectedSkill(editor, defs);
		const String skillName = hasSkill ? defs.skills[editor.selectedSkillIndex].name : U"<none>";
		uiFont(U"selected: {}"_fmt(skillName.isEmpty() && hasSkill ? defs.skills[editor.selectedSkillIndex].tag : skillName))
			.draw(12, preview.x + 20.0, preview.y + 44.0, Palette::Lightgray);
		DrawRectButton(SkillEditorSandboxButtonRect(0), U"Play", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(SkillEditorSandboxButtonRect(1), editor.skillSandboxAutoFire ? U"Auto ON" : U"Auto OFF", editor.skillSandboxAutoFire, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(SkillEditorSandboxButtonRect(2), U"Reset", false, uiFont, RectButtonStyle{ .fontSize = 11 });

		const RectF arena = SkillEditorSandboxArenaRect();
		arena.draw(ColorF{ 0.04, 0.05, 0.065, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		Line{ arena.x + 28.0, editor.skillSandboxCasterPos.y, arena.x + arena.w - 28.0, editor.skillSandboxCasterPos.y }.draw(1.5, ColorF{ 0.35, 0.55, 0.80, 0.30 });
		Circle{ editor.skillSandboxCasterPos, 24.0 }.draw(ColorF{ 0.20, 0.55, 1.0, 0.85 }).drawFrame(2, ColorF{ 0.0, 1.0, 1.0, 0.75 });
		Circle{ editor.skillSandboxTargetPos, 30.0 }.draw(ColorF{ 0.85, 0.18, 0.13, 0.85 }).drawFrame(2, editor.skillSandboxDraggingTarget ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.45, 0.35, 0.75 });
		uiFont(U"Caster").drawAt(12, editor.skillSandboxCasterPos + Vec2{ 0.0, 44.0 }, Palette::Lightgray);
		uiFont(U"Sandbag").drawAt(12, editor.skillSandboxTargetPos + Vec2{ 0.0, 50.0 }, Palette::Lightgray);

		const double hpRate = editor.skillSandboxTargetMaxHp > 0 ? Clamp(static_cast<double>(editor.skillSandboxTargetHp) / editor.skillSandboxTargetMaxHp, 0.0, 1.0) : 0.0;
		const RectF hpBack{ Arg::center = editor.skillSandboxTargetPos + Vec2{ 0.0, -46.0 }, 88.0, 8.0 };
		hpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
		RectF{ hpBack.pos, hpBack.w * hpRate, hpBack.h }.draw(ColorF{ 0.25, 0.95, 0.25 });
		uiFont(U"HP {}/{}"_fmt(editor.skillSandboxTargetHp, editor.skillSandboxTargetMaxHp)).drawAt(10, hpBack.center().movedBy(0.0, -14.0), Palette::White);

		if (!hasSkill)
		{
			uiFont(U"スキルを選択してください").drawAt(14, arena.center(), ColorF{ 1, 1, 1, 0.70 });
			return;
		}

		static BattleRenderAssets sandboxAssets;
		const SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		for (const auto& projectile : editor.skillSandboxProjectiles)
		{
			const Vec2 drawPos = projectile.position + Vec2{ 0.0, -projectile.height };
			const Optional<Vec2> rayTailPos = skill.rayLockToCaster ? Optional<Vec2>{ editor.skillSandboxCasterPos } : none;
			DrawSkillRay(sandboxAssets, skill, drawPos, projectile.angleRad, false, rayTailPos);
			if (const Optional<Vec2> tipScreen = ResolveSwingEndProjectileTipScreenInPlane(skill, projectile.position, projectile.angleRad))
			{
				if (DrawSwingEndProjectileTexture(sandboxAssets, skill, drawPos, *tipScreen))
				{
					continue;
				}
			}
			if (!DrawProjectileTexture(sandboxAssets, skill, drawPos, projectile.angleRad, false))
			{
				if (projectile.motion == SkillProjectileMotion::Parabola && projectile.height > 1.0)
				{
					Circle{ projectile.position, 3.5 }.draw(ColorF{ 0, 0, 0, 0.25 });
					Line{ projectile.position, drawPos }.draw(1.0, ColorF{ skill.color, 0.28 });
				}
				if (projectile.motion == SkillProjectileMotion::Orbit)
				{
					Circle{ drawPos, 6 }.drawFrame(2.0, skill.color);
				}
				else
				{
					Circle{ drawPos, 4 }.draw(skill.color);
				}
			}
		}

		uiFont(U"cool {:.2f}s  shots {}  {}"_fmt(
			editor.skillSandboxCooldownLeftSec,
			editor.skillSandboxProjectiles.size(),
			skill.burstFireMode == SkillBurstFireMode::Staggered ? U"stagger" : U"simul"))
			.draw(11, arena.x + 12.0, arena.y + 10.0, ColorF{ 1, 1, 1, 0.62 });
		uiFont(U"order {}"_fmt(skill.burstOrderMode == SkillBurstOrderMode::Random ? U"random" : U"seq"))
			.draw(11, arena.x + 12.0, arena.y + 26.0, ColorF{ 1, 1, 1, 0.52 });
		uiFont(U"Sandbagはドラッグで移動できます").draw(11, arena.x + 12.0, arena.y + arena.h - 24.0, ColorF{ 1, 1, 1, 0.56 });
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
		DrawSkillEditorSandboxPreview(editor, defs, uiFont);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Skill Editor").draw(16, panel.x + 18.0, panel.y + 14.0, Palette::White);
		DrawRectButton(SkillEditorSandboxToggleRect(), editor.showSkillSandboxPreview ? U"Preview ON" : U"Preview OFF", editor.showSkillSandboxPreview, uiFont, RectButtonStyle{ .fontSize = 12 });
		DrawRectButton(SkillEditorCloseRect(), U"×", false, uiFont);
		DrawRectButton(SkillEditorSaveRect(), U"Save TOML", false, uiFont, RectButtonStyle{ .fontSize = 12 });

		unitViewport.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		list.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detail.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detailViewport.draw(ColorF{ 0, 0, 0, 0.08 });
		uiFont(U"Units").draw(11, unitViewport.x + 10.0, unitViewport.y - 18.0, Palette::Aqua);
		uiFont(U"Skills").draw(11, list.x + 8.0, list.y - 18.0, Palette::Aqua);
		uiFont(U"クリック: 付与/解除").draw(10, unitViewport.x + 2.0, unitViewport.y + unitViewport.h + 8.0, ColorF{ 1, 1, 1, 0.60 });
		uiFont(U"緑チェック: 所持中").draw(10, unitViewport.x + 2.0, unitViewport.y + unitViewport.h + 24.0, ColorF{ 0.40, 1.0, 0.65, 0.85 });

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
			const bool linked = HasSelectedSkill(editor, defs) && UnitHasSkill(entry, defs.skills[editor.selectedSkillIndex].tag);
			const ColorF unitBack = linked ? ColorF{ 0.10, 0.20, 0.16, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 };
			const ColorF unitFrame = iconRect.mouseOver() ? ColorF{ 0.20, 0.72, 1.0 } : (linked ? ColorF{ 0.25, 0.9, 0.55 } : ColorF{ 1, 1, 1, 0.16 });
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
		const Array<String>* iconWarnings = FindSkillIconWarnings(defs, skill.tag);
		String hoverHelpText;
		String hoverNoteText;
		Array<String> hoverWarningLines;
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
		uiFont(U"image").draw(10, detail.x + 12.0, contentTop + 108.0, Palette::Lightgray);
		DrawRectButton(SkillEditorProjectileImageBrowseRect(0, scroll), U"上下左右", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorProjectileImageClearRect(0, scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.projectileImage.isEmpty() ? U"未設定" : U"設定完了", 10).draw(detail.x + 162.0, contentTop + 108.0, skill.projectileImage.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		uiFont(U"diag").draw(10, detail.x + 12.0, contentTop + 138.0, Palette::Lightgray);
		DrawRectButton(SkillEditorProjectileImageBrowseRect(1, scroll), U"斜め", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorProjectileImageClearRect(1, scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.projectileDiagonalImage.isEmpty() ? U"未設定" : U"設定完了", 10).draw(detail.x + 162.0, contentTop + 138.0, skill.projectileDiagonalImage.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		if (iconWarnings && !iconWarnings->isEmpty())
		{
			const RectF warningRect = SkillEditorWarningIconRect(scroll);
			DrawSkillEditorInfoIcon(warningRect, SkillEditorWarningIconPath(), U"!", uiFont);
			if (warningRect.mouseOver())
			{
				hoverWarningLines = *iconWarnings;
			}
		}
		uiFont(U"Kind").draw(12, detail.x + 8.0, contentTop + 170.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorKindButtonRect(i, scroll), SkillKindLabels()[i], SkillKindIndex(skill.kind) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		uiFont(U"Projectile Motion").draw(12, detail.x + 8.0, contentTop + 208.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorMotionButtonRect(i, scroll), SkillMotionLabels()[i], SkillMotionIndex(skill.projectileMotion) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		uiFont(U"center").draw(11, detail.x + 8.0, contentTop + 288.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillCenterLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorCenterButtonRect(i, scroll), SkillCenterLabels()[i], SkillCenterIndex(skill.projectileCenter) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}
		uiFont(U"flags").draw(11, detail.x + 8.0, contentTop + 322.0, Palette::Aqua);
		DrawRectButton(SkillEditorToggleButtonRect(0, scroll), U"homing {}"_fmt(skill.projectileHoming ? U"on" : U"off"), skill.projectileHoming, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(1, scroll), U"d360 {}"_fmt(skill.projectileD360 ? U"on" : U"off"), skill.projectileD360, uiFont, RectButtonStyle{ .fontSize = 10 });

		DrawSkillEditorValueRow(uiFont, editor, skill, 0, U"range", U"{:.1f}"_fmt(skill.range), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 1, U"cool", U"{:.2f}"_fmt(skill.cooldownSec), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 2, U"dmg", U"{}"_fmt(skill.damage), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 3, U"speed", U"{:.1f}"_fmt(skill.projectileSpeed), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 4, U"burst", U"{}"_fmt(skill.burstCount), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 5, U"burstMode", (skill.burstFireMode == SkillBurstFireMode::Staggered ? U"stagger" : U"simul"), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 6, U"burstOrd", (skill.burstOrderMode == SkillBurstOrderMode::Random ? U"random" : U"seq"), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 7, U"ray", (skill.rayMode == SkillRayMode::Image ? U"image" : (skill.rayMode == SkillRayMode::Line ? U"line" : U"none")), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 8, U"rayLen", U"{:.1f}"_fmt(skill.rayLength), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 9, U"rayLock", (skill.rayLockToCaster ? U"on" : U"off"), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 10, U"burstInt", U"{:.2f}"_fmt(skill.burstIntervalSec), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 11, U"spread", U"{:.1f}"_fmt(skill.spreadDeg), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 12, U"arc", U"{:.1f}"_fmt(skill.arcHeight), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 13, U"radius", U"{:.1f}"_fmt(skill.orbitRadius), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 14, U"circleV", U"{:.1f}"_fmt(skill.orbitAngularSpeedDeg), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 15, U"life", U"{:.2f}"_fmt(skill.orbitDurationSec), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 16, U"stDeg", U"{:.1f}"_fmt(skill.projectileStartDegree), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 17, U"degType", U"{}"_fmt(skill.projectileStartDegreeType), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 18, U"w", U"{:.1f}"_fmt(skill.projectileWidth), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 19, U"h", U"{:.1f}"_fmt(skill.projectileHeight), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 20, U"swingR", U"{:.1f}"_fmt(skill.swingRadius), scroll, hoverHelpText, hoverNoteText);
		DrawSkillEditorValueRow(uiFont, editor, skill, 21, U"swingDeg", U"{:.1f}"_fmt(skill.swingAngleDeg), scroll, hoverHelpText, hoverNoteText);

		uiFont(U"Resource Costs").draw(12, detail.x + 8.0, contentTop + 1198.0, Palette::Aqua);
		if (skill.resourceCosts.isEmpty())
		{
			uiFont(U"<none>").draw(10, detail.x + 8.0, contentTop + 1226.0, Palette::Lightgray);
		}
		for (int32 i = 0; i < static_cast<int32>(skill.resourceCosts.size()); ++i)
		{
			const SkillResourceCostDef& cost = skill.resourceCosts[i];
			String resourceLabel = cost.resourceTag;
			const String lowerTag = cost.resourceTag.lowercased();
			if (defs.resourceByTag.contains(lowerTag))
			{
				const ResourceDefId resourceId = defs.resourceByTag.at(lowerTag);
				if (resourceId < defs.resources.size())
				{
					resourceLabel = defs.resources[resourceId].name;
				}
			}
			DrawRectButton(SkillEditorResourceCostTagRect(i, scroll), resourceLabel, false, uiFont, RectButtonStyle{ .fontSize = 10 });
			const String amountText = (editor.skillResourceCostEditingIndex == i)
				? editor.skillResourceCostEditingText
				: U"{}"_fmt(cost.amount);
			DrawRectNumberStepper(
				SkillEditorResourceCostAmountStepperRects(i, scroll),
				amountText,
				U"x{}"_fmt(SkillEditorResourceCostStep(editor, i)),
				editor.skillResourceCostEditingIndex == i,
				editor.skillResourceCostStepMenuIndex && *editor.skillResourceCostStepMenuIndex == i,
				true,
				uiFont);
			DrawRectButton(SkillEditorResourceCostRemoveRect(i, scroll), U"del", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		}
		DrawRectButton(SkillEditorResourceCostAddRect(static_cast<int32>(skill.resourceCosts.size()), scroll), U"+ cost", false, uiFont, RectButtonStyle{ .fontSize = 10 });

		if (editor.skillValueStepMenuRow)
		{
			const Array<double>& steps = SkillEditorDefaultValueSteps();
			const RectF menuRect = SkillEditorValueStepMenuRect(editor.skillValueStepMenuPos, static_cast<int32>(steps.size()));
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				const RectF item = SkillEditorValueStepMenuItemRect(editor.skillValueStepMenuPos, i);
				item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
				uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 3.0, Palette::White);
			}
		}

		if (editor.skillResourceCostStepMenuIndex)
		{
			const Array<double>& steps = SkillEditorResourceCostStepOptions();
			const RectF menuRect = SkillEditorResourceCostStepMenuRect(editor.skillResourceCostStepMenuPos, static_cast<int32>(steps.size()));
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				const RectF item = SkillEditorResourceCostStepMenuItemRect(editor.skillResourceCostStepMenuPos, i);
				item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
				uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 3.0, Palette::White);
			}
		}

		if (!hoverHelpText.isEmpty())
		{
			DrawSkillEditorTooltip(uiFont, U"パラメータの意味", Array<String>{ hoverHelpText });
		}
		if (!hoverNoteText.isEmpty())
		{
			DrawSkillEditorTooltip(uiFont, U"補足", Array<String>{ hoverNoteText });
		}
		if (!hoverWarningLines.isEmpty())
		{
			DrawSkillEditorTooltip(uiFont, U"SkillIcon warning", hoverWarningLines);
		}

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
