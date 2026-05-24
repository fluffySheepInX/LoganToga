#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.h"

namespace LT3
{
	inline void DrawAiEditorValueRow(const Font& uiFont, int32 rowIndex, StringView label, StringView value, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		const RectF detail = AiEditorDetailRect();
		if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(label).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, AiEditorRowHelpText(rowIndex), hoverHelp);
		uiFont(value).draw(12, row.x + 210.0, row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorValueButtonRect(row, 0), U"-10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 1), U"-1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 2), U"+1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 3), U"+10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditorUnitWeightRow(const Font& uiFont, int32 rowIndex, const AiUnitWeightDef& weight, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		const RectF detail = AiEditorDetailRect();
		if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Unit Weight").draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"各ユニット種をどれだけ優先的に生成するかの重みです。", hoverHelp);
		uiFont(U"{}  {:.2f}"_fmt(weight.unitTag, weight.weight)).draw(12, row.x + 130.0, row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorInlineButtonRect(row, 0, 5), U"-0.5", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 1, 5), U"-0.1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 2, 5), U"+0.1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 3, 5), U"+0.5", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 4, 5), U"Del", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditorTargetPriorityRow(const Font& uiFont, int32 rowIndex, int32 priorityIndex, StringView target, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		const RectF detail = AiEditorDetailRect();
		if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Target #{}"_fmt(priorityIndex + 1)).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"AI が攻撃対象として優先する種類です。< > で切替します。", hoverHelp);
		uiFont(target).draw(12, row.x + 130.0, row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorInlineButtonRect(row, 0, 3), U"<", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 1, 3), U">", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 2, 3), U"Del", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditorAddRow(const Font& uiFont, int32 rowIndex, StringView label, StringView buttonText, double scroll, StringView helpText, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		const RectF detail = AiEditorDetailRect();
		if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(label).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, helpText, hoverHelp);
		DrawRectButton(RectF{ row.x + row.w - 124.0, row.y + 4.0, 110.0, 26.0 }, buttonText, false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditor(MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showAiEditor)
		{
			return;
		}

		const RectF panel = AiEditorPanelRect();
		const RectF list = AiEditorListViewportRect();
		const RectF detail = AiEditorDetailRect();
		SyncAiEditorSelectionFromTag(editor, defs);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"AI Editor").draw(16, panel.x + 18.0, panel.y + 14.0, Palette::White);
		DrawRectButton(AiEditorApplyRect(), U"Apply", false, uiFont, RectButtonStyle{ .fontSize = 12 });
		DrawRectButton(AiEditorSaveRect(), U"Save TOML", false, uiFont, RectButtonStyle{ .fontSize = 12 });
		DrawRectButton(AiEditorCloseRect(), U"Close", false, uiFont, RectButtonStyle{ .fontSize = 12 });

		list.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detail.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Profiles").draw(12, list.x + 8.0, list.y - 20.0, Palette::Aqua);
		uiFont(U"Parameters").draw(12, detail.x + 8.0, detail.y - 20.0, Palette::Aqua);

		const int32 firstIndex = Max(0, static_cast<int32>(editor.aiProfileListScroll / 58.0));
		const int32 visibleRows = static_cast<int32>(list.h / 58.0) + 1;
		for (int32 visible = 0; visible < visibleRows; ++visible)
		{
			const int32 profileIndex = firstIndex + visible;
			if (profileIndex >= static_cast<int32>(defs.aiProfiles.size()))
			{
				break;
			}

			const AiProfileDef& profile = defs.aiProfiles[profileIndex];
			const RectF row = AiEditorProfileRowRect(list, visible, 0.0);
			const bool selected = editor.selectedAiProfileIndex == profileIndex;
			row.draw(selected ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, row.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(profile.name.isEmpty() ? profile.tag : profile.name).draw(13, row.x + 10.0, row.y + 6.0, Palette::White);
			uiFont(U"{} / {}"_fmt(profile.tag, profile.presetType)).draw(10, row.x + 10.0, row.y + 28.0, Palette::Lightgray);
		}

		if (defs.aiProfiles.isEmpty())
		{
			uiFont(U"AI profiles are empty").drawAt(14, detail.center(), Palette::Lightgray);
			return;
		}

		const AiProfileDef& profile = defs.aiProfiles[Clamp(editor.selectedAiProfileIndex, 0, static_cast<int32>(defs.aiProfiles.size()) - 1)];
		uiFont(profile.name.isEmpty() ? profile.tag : profile.name).draw(18, detail.x + 18.0, detail.y + 14.0, Palette::White);
		uiFont(profile.description).draw(11, detail.x + 18.0, detail.y + 44.0, ColorF{ 1, 1, 1, 0.70 });
		uiFont(U"preset: {}  weights: {}  targets: {}"_fmt(profile.presetType, profile.unitWeights.size(), profile.targetPriority.size()))
			.draw(11, detail.x + 18.0, detail.y + 72.0, Palette::Lightgray);
		const bool applied = (profile.tag.lowercased() == editor.selectedAiProfileTag.lowercased());
		uiFont(applied
			? U"Applied to next battle"
			: U"Selected only. Press Apply to use in next battle")
			.draw(11, detail.x + 18.0, detail.y + 92.0, applied ? Palette::Aqua : Palette::Orange);

		String hoverHelp;
		const double scroll = editor.aiProfileDetailScroll;
		DrawAiEditorValueRow(uiFont, 0, U"Opening Delay", U"{:.1f}s"_fmt(profile.openingDelaySec), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 1, U"Spawn Interval", U"{:.1f}s"_fmt(profile.spawnIntervalSec), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 2, U"Wave Interval", U"{:.1f}s"_fmt(profile.attackWaveIntervalSec), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 3, U"Aggression", U"{:.2f}"_fmt(profile.aggression), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 4, U"Economy Focus", U"{:.2f}"_fmt(profile.economyFocus), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 5, U"Defense Focus", U"{:.2f}"_fmt(profile.defenseFocus), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 6, U"Attack Group", U"{}"_fmt(profile.attackGroupSize), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 7, U"Max Army", U"{}"_fmt(profile.maxArmySize), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 8, U"Tech Focus", U"{:.2f}"_fmt(profile.techFocus), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 9, U"Retreat HP", U"{:.2f}"_fmt(profile.retreatHpRatio), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 10, U"Resource Mul", U"{:.2f}"_fmt(profile.resourceMultiplier), scroll, hoverHelp);

		const RectF freeSpawnRect = AiEditorValueRowRect(11, scroll);
		if (detail.y <= freeSpawnRect.y + freeSpawnRect.h && freeSpawnRect.y <= detail.y + detail.h)
		{
			freeSpawnRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			uiFont(U"Free Spawn").draw(12, freeSpawnRect.x + 10.0, freeSpawnRect.y + 8.0, Palette::Aqua);
			DrawAiEditorHelpIcon(uiFont, freeSpawnRect, AiEditorRowHelpText(11), hoverHelp);
			DrawRectButton(RectF{ freeSpawnRect.x + freeSpawnRect.w - 112.0, freeSpawnRect.y + 4.0, 98.0, 26.0 }, profile.freeSpawnEnabled ? U"ON" : U"OFF", profile.freeSpawnEnabled, uiFont, RectButtonStyle{ .fontSize = 11 });
		}

		const RectF contactBehaviorRect = AiEditorValueRowRect(12, scroll);
		if (detail.y <= contactBehaviorRect.y + contactBehaviorRect.h && contactBehaviorRect.y <= detail.y + detail.h)
		{
			contactBehaviorRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			uiFont(U"Contact").draw(12, contactBehaviorRect.x + 10.0, contactBehaviorRect.y + 8.0, Palette::Aqua);
			DrawAiEditorHelpIcon(uiFont, contactBehaviorRect, AiEditorRowHelpText(12), hoverHelp);
			uiFont(profile.contactBehavior).draw(12, contactBehaviorRect.x + 130.0, contactBehaviorRect.y + 8.0, Palette::Gold);
			DrawRectButton(RectF{ contactBehaviorRect.x + contactBehaviorRect.w - 112.0, contactBehaviorRect.y + 4.0, 98.0, 26.0 }, U"Toggle", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		}

		int32 rowIndex = 13;
		DrawAiEditorAddRow(uiFont, rowIndex++, U"Unit Weights", U"Add Unit", scroll, U"生成候補ユニットとその出現比率を追加します。", hoverHelp);
		for (const auto& weight : profile.unitWeights)
		{
			DrawAiEditorUnitWeightRow(uiFont, rowIndex++, weight, scroll, hoverHelp);
		}

		DrawAiEditorAddRow(uiFont, rowIndex++, U"Target Priority", U"Add Target", scroll, U"攻撃優先対象の候補を追加します。", hoverHelp);
		for (int32 targetIndex = 0; targetIndex < static_cast<int32>(profile.targetPriority.size()); ++targetIndex)
		{
			DrawAiEditorTargetPriorityRow(uiFont, rowIndex++, targetIndex, profile.targetPriority[targetIndex], scroll, hoverHelp);
		}

		if (!hoverHelp.isEmpty())
		{
			const RectF helpRect{ detail.x + 16.0, detail.y + detail.h - 52.0, detail.w - 32.0, 36.0 };
			helpRect.draw(ColorF{ 0.02, 0.03, 0.045, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
			uiFont(hoverHelp).draw(11, helpRect.x + 10.0, helpRect.y + 10.0, Palette::Aqua);
		}
	}
}
