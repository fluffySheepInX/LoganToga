#pragma once
# include "SkillEditorDraw.ListPanel.h"

namespace LT3
{
	inline void DrawSkillEditorDetailPanel(MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont, const RectF& detail, const RectF& detailViewport)
	{
		if (!HasSelectedSkill(editor, defs))
		{
			uiFont(U"スキルを選択してください").draw(13, detail.x + 18.0, detail.y + 18.0, Palette::Lightgray);
			return;
		}

		const SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		const double scroll = editor.skillDetailScroll;
		const auto layout = BuildSkillEditorDetailLayoutModel(static_cast<int32>(skill.resourceCosts.size()), scroll);
		const double contentTop = layout.contentTop;
		const double contentHeight = layout.contentHeight;
		const Array<String>* iconWarnings = FindSkillIconWarnings(defs, skill.tag);
		String hoverHelpText;
		Optional<double> hoverHelpTooltipX;
		String hoverNoteText;
		Optional<double> hoverNoteTooltipX;
		String hoverToggleText;
		Optional<double> hoverToggleTooltipX;
		Array<String> hoverWarningLines;
		uiFont(skill.name).draw(16, detail.x + 12.0, contentTop + layout.titleY, Palette::White);
		uiFont(U"tag: {}"_fmt(skill.tag)).draw(11, detail.x + 12.0, contentTop + layout.tagY, Palette::Lightgray);
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
		uiFont(U"icon: {}"_fmt(skill.icon.isEmpty() ? U"<none>" : skill.icon)).draw(10, detail.x + 72.0, contentTop + layout.iconLabelY, Palette::Lightgray);
		DrawRectButton(SkillEditorIconBrowseRect(scroll), U"参照", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		uiFont(U"image").draw(10, detail.x + 12.0, contentTop + layout.projectileImageLabelY, Palette::Lightgray);
		DrawRectButton(SkillEditorProjectileImageBrowseRect(0, scroll), U"上下左右", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorProjectileImageClearRect(0, scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.projectileImage.isEmpty() ? U"未設定" : U"設定完了", 10).draw(detail.x + 162.0, contentTop + layout.projectileImageLabelY, skill.projectileImage.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		uiFont(U"diag").draw(10, detail.x + 12.0, contentTop + layout.projectileDiagonalLabelY, Palette::Lightgray);
		DrawRectButton(SkillEditorProjectileImageBrowseRect(1, scroll), U"斜め", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorProjectileImageClearRect(1, scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.projectileDiagonalImage.isEmpty() ? U"未設定" : U"設定完了", 10).draw(detail.x + 162.0, contentTop + layout.projectileDiagonalLabelY, skill.projectileDiagonalImage.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		uiFont(U"bomImg").draw(10, detail.x + 12.0, contentTop + layout.bomImageLabelY, Palette::Lightgray);
		DrawRectButton(SkillEditorBomImageBrowseRect(scroll), U"参照", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorBomImageClearRect(scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.bomImage.isEmpty() ? U"未設定" : U"設定完了", 10).draw(detail.x + 162.0, contentTop + layout.bomImageLabelY, skill.bomImage.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		uiFont(U"SE").draw(10, detail.x + 12.0, contentTop + layout.soundEffectLabelY, Palette::Lightgray);
		DrawRectButton(SkillEditorSoundEffectBrowseRect(scroll), U"参照", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorSoundEffectClearRect(scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorSoundEffectPlayRect(scroll), U"play", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		uiFont(skill.soundEffect.isEmpty() ? U"未設定" : FileSystem::BaseName(skill.soundEffect), 10)
			.draw(detail.x + 270.0, contentTop + layout.soundEffectLabelY, skill.soundEffect.isEmpty() ? Palette::Lightgray : Palette::Lightgreen);
		uiFont(U"SE vol").draw(10, detail.x + 12.0, contentTop + layout.soundEffectVolumeLabelY, Palette::Lightgray);
		DrawRectNumberStepper(SkillEditorSoundEffectVolumeStepperRects(scroll), U"{:.2f}"_fmt(skill.soundEffectVolume), U"x0.05", false, false, true, uiFont);
		if (!editor.skillSoundPreviewUnitHint.isEmpty())
		{
			uiFont(editor.skillSoundPreviewUnitHint).draw(9, detail.x + 300.0, contentTop + layout.soundEffectVolumeLabelY, ColorF{ 1, 1, 1, 0.56 });
		}
		if (iconWarnings && !iconWarnings->isEmpty())
		{
			const RectF warningRect = SkillEditorWarningIconRect(scroll);
			DrawSkillEditorInfoIcon(warningRect, SkillEditorWarningIconPath(), U"!", uiFont);
			if (warningRect.mouseOver())
			{
				hoverWarningLines = *iconWarnings;
			}
		}
		uiFont(U"Kind").draw(12, detail.x + 8.0, contentTop + layout.kindLabelY, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorKindButtonRect(i, scroll), SkillKindLabels()[i], SkillKindIndex(skill.kind) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		uiFont(U"Projectile Motion").draw(12, detail.x + 8.0, contentTop + layout.motionLabelY, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorMotionButtonRect(i, scroll), SkillMotionLabels()[i], SkillMotionIndex(skill.projectileMotion) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}

		uiFont(U"center").draw(11, detail.x + 8.0, contentTop + layout.centerLabelY, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(SkillCenterLabels().size()); ++i)
		{
			DrawRectButton(SkillEditorCenterButtonRect(i, scroll), SkillCenterLabels()[i], SkillCenterIndex(skill.projectileCenter) == i, uiFont, RectButtonStyle{ .fontSize = 10 });
		}
		uiFont(U"chain").draw(11, detail.x + 8.0, contentTop + layout.chainLabelY, Palette::Aqua);
		uiFont(U"左クリックで切替 / 右クリックで解除", 9).draw(SkillEditorNextSkillButtonRect(scroll).x + 250.0, SkillEditorNextSkillButtonRect(scroll).y + 5.0, Palette::Lightgray);
		uiFont(U"tag入力 / 候補クリックでも指定可", 9).draw(SkillEditorNextSkillInputRect(scroll).x + 250.0, SkillEditorNextSkillInputRect(scroll).y + 5.0, Palette::Lightgray);
		uiFont(U"flags").draw(11, detail.x + 8.0, contentTop + layout.flagLabelY, Palette::Aqua);
		DrawRectButton(SkillEditorToggleButtonRect(0, scroll), U"homing {}"_fmt(skill.projectileHoming ? U"on" : U"off"), skill.projectileHoming, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(1, scroll), U"d360 {}"_fmt(skill.projectileD360 ? U"on" : U"off"), skill.projectileD360, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(2, scroll), U"bom {}"_fmt(skill.bom ? U"on" : U"off"), skill.bom, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(3, scroll), U"ff {}"_fmt(skill.bomFriendlyFire ? U"on" : U"off"), skill.bomFriendlyFire, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(4, scroll), U"allfunc {}"_fmt(skill.allfunc ? U"on" : U"off"), skill.allfunc, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorNextSkillButtonRect(scroll), U"next: {}"_fmt(skill.nextSkillTag.isEmpty() ? U"<none>" : skill.nextSkillTag), !skill.nextSkillTag.isEmpty(), uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorNextSkillInputRect(scroll), editor.skillNextTagEditing ? editor.skillNextTagEditingText : (editor.skillNextTagFilterText.isEmpty() ? U"type next tag..." : editor.skillNextTagFilterText), editor.skillNextTagEditing, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorNextSkillClearRect(scroll), U"clear", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		const Array<int32> nextCandidates = BuildSkillEditorNextSkillCandidateIndices(editor, defs);
		for (int32 i = 0; i < static_cast<int32>(nextCandidates.size()); ++i)
		{
			const SkillDef& candidate = defs.skills[nextCandidates[i]];
			const RectF candidateRect = SkillEditorNextSkillCandidateRect(i, scroll);
			candidateRect.draw(candidateRect.mouseOver() ? ColorF{ 0.16, 0.24, 0.20, 0.94 } : ColorF{ 0.08, 0.10, 0.14, 0.88 })
				.drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
			uiFont(U"{}  ({})"_fmt(candidate.tag, candidate.name.isEmpty() ? candidate.tag : candidate.name))
				.draw(10, candidateRect.x + 6.0, candidateRect.y + 3.0, Palette::White);
		}
		DrawRectButton(SkillEditorToggleButtonRect(5, scroll), U"last {}"_fmt(skill.nextLast ? U"on" : U"off"), skill.nextLast, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(6, scroll), U"joint {}"_fmt(skill.jointSkill ? U"on" : U"off"), skill.jointSkill, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(7, scroll), U"target {}"_fmt(skill.sendTarget ? U"on" : U"off"), skill.sendTarget, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(SkillEditorToggleButtonRect(8, scroll), U"imgDeg {}"_fmt(skill.sendImageDegree ? U"on" : U"off"), skill.sendImageDegree, uiFont, RectButtonStyle{ .fontSize = 10 });
		for (int32 i = 0; i < 9; ++i)
		{
			const RectF toggleRect = SkillEditorToggleButtonRect(i, scroll);
			if (toggleRect.mouseOver())
			{
				hoverToggleText = SkillEditorToggleHelpText(i);
				hoverToggleTooltipX = toggleRect.x - 360.0;
				break;
			}
		}

		DrawSkillEditorValueRow(uiFont, editor, skill, 0, U"range", U"{:.1f}"_fmt(skill.range), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 1, U"rangeMin", U"{:.1f}"_fmt(skill.rangeMin), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 2, U"cool", U"{:.2f}"_fmt(skill.cooldownSec), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 3, U"dmg", U"{}"_fmt(skill.damage), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 4, U"selfDmg", U"{}"_fmt(skill.selfDamageOnHit), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 5, U"speed", U"{:.1f}"_fmt(skill.projectileSpeed), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 6, U"burst", U"{}"_fmt(skill.burstCount), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 7, U"burstMode", (skill.burstFireMode == SkillBurstFireMode::Staggered ? U"stagger" : U"simul"), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 8, U"burstOrd", (skill.burstOrderMode == SkillBurstOrderMode::Random ? U"random" : U"seq"), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 9, U"ray", (skill.rayMode == SkillRayMode::Image ? U"image" : (skill.rayMode == SkillRayMode::Line ? U"line" : U"none")), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 10, U"rayLen", U"{:.1f}"_fmt(skill.rayLength), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 11, U"rayLock", (skill.rayLockToCaster ? U"on" : U"off"), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 12, U"burstInt", U"{:.2f}"_fmt(skill.burstIntervalSec), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 13, U"spread", U"{:.1f}"_fmt(skill.spreadDeg), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 14, U"arc", U"{:.1f}"_fmt(skill.arcHeight), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 15, U"radius", U"{:.1f}"_fmt(skill.orbitRadius), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 16, U"circleV", U"{:.1f}"_fmt(skill.orbitAngularSpeedDeg), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 17, U"life", U"{:.2f}"_fmt(skill.orbitDurationSec), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 18, U"stDeg", U"{:.1f}"_fmt(skill.projectileStartDegree), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 19, U"degType", U"{}"_fmt(skill.projectileStartDegreeType), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 20, U"w", U"{:.1f}"_fmt(skill.projectileWidth), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 21, U"h", U"{:.1f}"_fmt(skill.projectileHeight), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 22, U"swingR", U"{:.1f}"_fmt(skill.swingRadius), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 23, U"swingDeg", U"{:.1f}"_fmt(skill.swingAngleDeg), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 24, U"bomR", U"{:.1f}"_fmt(skill.bomRadius), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 25, U"selfBom", U"{:.2f}"_fmt(skill.bomSelfDamageScale), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		DrawSkillEditorValueRow(uiFont, editor, skill, 26, U"swingHit", (skill.swingHitMode == SkillSwingHitMode::MultiHitOnce ? U"multi_once" : U"stop"), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX);
		const bool emphasizeBomVisualRows = (skill.bomVisual == SkillBomVisual::Image);
		DrawSkillEditorValueRow(uiFont, editor, skill, 27, U"bomVisual", SkillBomVisualLabels()[static_cast<int32>(skill.bomVisual)], scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX, emphasizeBomVisualRows);
		DrawSkillEditorValueRow(uiFont, editor, skill, 28, U"bomScale", U"{:.2f}"_fmt(skill.bomVisualScale), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX, emphasizeBomVisualRows);
		DrawSkillEditorValueRow(uiFont, editor, skill, 29, U"bomTime", U"{:.2f}"_fmt(skill.bomVisualDurationSec), scroll, hoverHelpText, hoverHelpTooltipX, hoverNoteText, hoverNoteTooltipX, emphasizeBomVisualRows);

		uiFont(U"Resource Costs").draw(12, detail.x + 8.0, contentTop + layout.resourceCostsLabelY, Palette::Aqua);
		if (skill.resourceCosts.isEmpty())
		{
			uiFont(U"<none>").draw(10, detail.x + 8.0, contentTop + layout.resourceCostsTopY + 4.0, Palette::Lightgray);
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
			DrawSkillEditorTooltip(uiFont, U"パラメータの意味", Array<String>{ hoverHelpText }, hoverHelpTooltipX);
		}
		if (!hoverNoteText.isEmpty())
		{
			DrawSkillEditorTooltip(uiFont, U"補足", Array<String>{ hoverNoteText }, hoverNoteTooltipX);
		}
		if (!hoverToggleText.isEmpty())
		{
			DrawSkillEditorTooltip(uiFont, U"flags", Array<String>{ hoverToggleText }, hoverToggleTooltipX);
		}
		if (!hoverWarningLines.isEmpty())
		{
			const double warningTooltipX = SkillEditorWarningIconRect(scroll).x - 360.0;
			DrawSkillEditorTooltip(uiFont, U"SkillIcon warning", hoverWarningLines, warningTooltipX);
		}

		const double maxScroll = Max(0.0, contentHeight - detailViewport.h);
		if (maxScroll > 0.0)
		{
			const double rate = Clamp(editor.skillDetailScroll / maxScroll, 0.0, 1.0);
			const double handleHeight = Max(32.0, detailViewport.h * detailViewport.h / contentHeight);
			RectF{ detail.x + detail.w - 7.0, detailViewport.y, 4.0, detailViewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
			RectF{ detail.x + detail.w - 7.0, detailViewport.y + (detailViewport.h - handleHeight) * rate, 4.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
		}
	}
}
