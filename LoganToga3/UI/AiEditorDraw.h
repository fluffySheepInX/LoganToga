#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.h"

namespace LT3
{
	/// <summary>
	/// AI Editor の右寄せアクションボタン矩形を返します。
	/// </summary>
	inline RectF AiEditorActionButtonRect(const RectF& row, double width)
	{
		return RectF{ AiEditorActionRightX(row) - width, row.y + 4.0, width, 26.0 };
	}

	inline void DrawAiEditorValueRow(const Font& uiFont, int32 rowIndex, StringView label, StringView value, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(label).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, AiEditorRowHelpText(rowIndex), hoverHelp);
		uiFont(value).draw(12, AiEditorRowValueX(row), row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorValueButtonRect(row, 0), U"-10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 1), U"-1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 2), U"+1", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorValueButtonRect(row, 3), U"+10", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	/// <summary>
	/// AI Editor の unit priority 行を描画します。
	/// </summary>
	inline void DrawAiEditorUnitWeightRow(const MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont, int32 rowIndex, const AiUnitWeightDef& weight, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Unit Priority").draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"各ユニット種の生成重みと目標数です。weight が高いほど出やすく、desired が目標数です。", hoverHelp);

		const RectF unitButton = AiEditorUnitWeightUnitButtonRect(row);
		const RectNumberStepperRects weightStepper = AiEditorUnitWeightValueStepperRects(row);
		const RectNumberStepperRects desiredStepper = AiEditorUnitWeightDesiredStepperRects(row);
		const RectF deleteRect = AiEditorUnitWeightDeleteRect(row);
		const String unitText = ResolveAiEditorUnitDisplayName(defs, weight.unitTag);
		DrawRectButton(unitButton, unitText, editor.aiUnitWeightMenuRow && *editor.aiUnitWeightMenuRow == rowIndex && editor.aiUnitWeightMenuKind == AiEditorUnitWeightMenuKind::UnitPicker, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectNumberStepper(weightStepper, U"{:.1f}"_fmt(weight.weight), U"x{}"_fmt(editor.aiUnitWeightStep), false,
			editor.aiUnitWeightMenuRow && *editor.aiUnitWeightMenuRow == rowIndex && editor.aiUnitWeightMenuKind == AiEditorUnitWeightMenuKind::WeightStep,
			true, uiFont, RectNumberStepperStyle{ .buttonStyle = RectButtonStyle{ .fontSize = 9 }, .valueFontSize = 10 });
		DrawRectNumberStepper(desiredStepper, U"{}"_fmt(weight.desiredCount), U"x{}"_fmt(editor.aiUnitWeightDesiredStep), false,
			editor.aiUnitWeightMenuRow && *editor.aiUnitWeightMenuRow == rowIndex && editor.aiUnitWeightMenuKind == AiEditorUnitWeightMenuKind::DesiredStep,
			true, uiFont, RectNumberStepperStyle{ .buttonStyle = RectButtonStyle{ .fontSize = 9 }, .valueFontSize = 10 });
		DrawRectButton(deleteRect, U"Del", false, uiFont, RectButtonStyle{ .fontSize = 8 });
		uiFont(U"w").draw(10, weightStepper.value.x - 12.0, row.y + 9.0, Palette::Gold);
		uiFont(U"n").draw(10, desiredStepper.value.x - 12.0, row.y + 9.0, Palette::Gold);

		if (unitButton.mouseOver())
		{
			hoverHelp = ResolveAiEditorUnitDisplayName(defs, weight.unitTag);
		}
	}

	/// <summary>
	/// AI Editor の unit priority 用 step メニューを描画します。
	/// </summary>
	inline void DrawAiEditorUnitWeightStepMenu(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.aiUnitWeightMenuRow
			|| (editor.aiUnitWeightMenuKind != AiEditorUnitWeightMenuKind::WeightStep && editor.aiUnitWeightMenuKind != AiEditorUnitWeightMenuKind::DesiredStep))
		{
			return;
		}

		if (editor.aiUnitWeightMenuKind == AiEditorUnitWeightMenuKind::WeightStep)
		{
			const Array<double>& steps = AiEditorUnitWeightStepOptions();
			const RectF menuRect = AiEditorUnitWeightStepMenuRect(editor.aiUnitWeightMenuPos, static_cast<int32>(steps.size()));
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
				item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
				uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 2.0, Palette::White);
			}
			return;
		}

		const Array<int32>& steps = AiEditorUnitDesiredStepOptions();
		const RectF menuRect = AiEditorUnitWeightStepMenuRect(editor.aiUnitWeightMenuPos, static_cast<int32>(steps.size()));
		menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
		for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
		{
			const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
			item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 2.0, Palette::White);
		}
	}

	/// <summary>
	/// AI Editor の unit priority 用ユニット選択パネルを描画します。
	/// </summary>
	inline void DrawAiEditorUnitWeightUnitPicker(const MapEditorState& editor, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.aiUnitWeightMenuRow || editor.aiUnitWeightMenuKind != AiEditorUnitWeightMenuKind::UnitPicker)
		{
			return;
		}

		const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
		if (tags.isEmpty())
		{
			return;
		}

		const int32 visibleRows = Min(8, static_cast<int32>(tags.size()));
		const RectF panel = AiEditorUnitWeightPickerRect(editor.aiUnitWeightMenuPos, visibleRows);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });
		const double viewportHeight = visibleRows * 24.0;
		const double maxScroll = Max(0.0, static_cast<double>(tags.size()) * 24.0 - viewportHeight);
		const double scroll = Clamp(editor.aiUnitWeightUnitPickerScroll, 0.0, maxScroll);
		for (int32 index = 0; index < static_cast<int32>(tags.size()); ++index)
		{
			const RectF item = AiEditorUnitWeightPickerItemRect(panel, index, scroll);
			if (item.y + item.h < panel.y + 4.0 || panel.y + panel.h - 4.0 < item.y)
			{
				continue;
			}

			item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.72 })
				.drawFrame(1, item.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.10 });
			uiFont(ResolveAiEditorUnitDisplayName(defs, tags[index])).draw(10, item.x + 6.0, item.y + 3.0, Palette::White);
		}

		DrawRectVerticalScrollbar(RectF{ panel.x, panel.y + 4.0, panel.w - 10.0, viewportHeight }, static_cast<double>(tags.size()) * 24.0, scroll,
			ColorF{ 1, 1, 1, 0.08 }, ColorF{ 1.0, 0.84, 0.0, 0.70 }, 6.0, 8.0, 24.0);
	}

	inline void DrawAiEditorTargetPriorityRow(const Font& uiFont, int32 rowIndex, int32 priorityIndex, StringView target, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Target #{}"_fmt(priorityIndex + 1)).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"AI が攻撃対象として優先する種類です。< > で切替します。", hoverHelp);
		uiFont(target).draw(12, AiEditorRowValueX(row), row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorInlineButtonRect(row, 0, 3), U"<", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 1, 3), U">", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 2, 3), U"Del", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditorInitialUnitRow(const DefinitionStores& defs, const Font& uiFont, int32 rowIndex, int32 unitIndex, StringView unitTag, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Initial #{}"_fmt(unitIndex + 1)).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"AI が戦闘開始時点で使用できるユニットです。施設生産導入後はここを初期解禁リストとして扱います。", hoverHelp);
		uiFont(ResolveAiEditorUnitDisplayName(defs, unitTag)).draw(12, AiEditorRowValueX(row), row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorInlineButtonRect(row, 0, 3), U"<", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 1, 3), U">", false, uiFont, RectButtonStyle{ .fontSize = 10 });
		DrawRectButton(AiEditorInlineButtonRect(row, 2, 3), U"Del", false, uiFont, RectButtonStyle{ .fontSize = 10 });
	}

	inline void DrawAiEditorBuildPriorityRow(const Font& uiFont, int32 rowIndex, const AiBuildPriorityDef& buildPriority, double scroll, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(U"Build Priority").draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, U"建設 action ごとの重みと目標棟数です。weight が高いほど選ばれやすく、desired が目標数です。", hoverHelp);
		uiFont(U"{} w{:.1f} n{}"_fmt(buildPriority.actionTag, buildPriority.weight, buildPriority.desiredCount)).draw(11, AiEditorRowValueX(row), row.y + 8.0, Palette::Gold);
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 0, 6), U"-.5", false, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 1, 6), U"-.1", false, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 2, 6), U"+.1", false, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 3, 6), U"+.5", false, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 4, 6), U"N+", false, uiFont, RectButtonStyle{ .fontSize = 9 });
		DrawRectButton(AiEditorCompactInlineButtonRect(row, 5, 6), U"Del", false, uiFont, RectButtonStyle{ .fontSize = 9 });
	}

	inline void DrawAiEditorAddRow(const Font& uiFont, int32 rowIndex, StringView label, StringView buttonText, double scroll, StringView helpText, String& hoverHelp)
	{
		const RectF row = AiEditorValueRowRect(rowIndex, scroll);
		if (!AiEditorRowVisible(row))
		{
			return;
		}

		row.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		uiFont(label).draw(12, row.x + 10.0, row.y + 8.0, Palette::Aqua);
		DrawAiEditorHelpIcon(uiFont, row, helpText, hoverHelp);
		DrawRectButton(AiEditorActionButtonRect(row, 110.0), buttonText, false, uiFont, RectButtonStyle{ .fontSize = 10 });
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
		uiFont(U"preset: {}  initial: {}  weights: {}  builds: {}  targets: {}"_fmt(profile.presetType, profile.initialUnits.size(), profile.unitWeights.size(), profile.buildPriorities.size(), profile.targetPriority.size()))
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
		DrawAiEditorValueRow(uiFont, 8, U"Time Limit", U"{} min"_fmt(Max(1, static_cast<int32>(Round(profile.battleTimeLimitSec / 60.0)))), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 9, U"Tech Focus", U"{:.2f}"_fmt(profile.techFocus), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 10, U"Retreat HP", U"{:.2f}"_fmt(profile.retreatHpRatio), scroll, hoverHelp);
		DrawAiEditorValueRow(uiFont, 11, U"Resource Mul", U"{:.2f}"_fmt(profile.resourceMultiplier), scroll, hoverHelp);

		const RectF freeSpawnRect = AiEditorValueRowRect(12, scroll);
		if (AiEditorRowVisible(freeSpawnRect))
		{
			freeSpawnRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			uiFont(U"Free Spawn").draw(12, freeSpawnRect.x + 10.0, freeSpawnRect.y + 8.0, Palette::Aqua);
			DrawAiEditorHelpIcon(uiFont, freeSpawnRect, AiEditorRowHelpText(12), hoverHelp);
			DrawRectButton(AiEditorActionButtonRect(freeSpawnRect, 98.0), profile.freeSpawnEnabled ? U"ON" : U"OFF", profile.freeSpawnEnabled, uiFont, RectButtonStyle{ .fontSize = 11 });
		}

		const RectF contactBehaviorRect = AiEditorValueRowRect(13, scroll);
		if (AiEditorRowVisible(contactBehaviorRect))
		{
			contactBehaviorRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.72 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			uiFont(U"Contact").draw(12, contactBehaviorRect.x + 10.0, contactBehaviorRect.y + 8.0, Palette::Aqua);
			DrawAiEditorHelpIcon(uiFont, contactBehaviorRect, AiEditorRowHelpText(13), hoverHelp);
			uiFont(profile.contactBehavior).draw(12, AiEditorRowValueX(contactBehaviorRect), contactBehaviorRect.y + 8.0, Palette::Gold);
			DrawRectButton(AiEditorActionButtonRect(contactBehaviorRect, 98.0), U"Toggle", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		}

		int32 rowIndex = 14;
		DrawAiEditorAddRow(uiFont, rowIndex++, U"Initial Units", U"Add Initial", scroll, U"AI が初期状態で使用できるユニットを追加します。", hoverHelp);
		for (int32 unitIndex = 0; unitIndex < static_cast<int32>(profile.initialUnits.size()); ++unitIndex)
		{
			DrawAiEditorInitialUnitRow(defs, uiFont, rowIndex++, unitIndex, profile.initialUnits[unitIndex], scroll, hoverHelp);
		}

		DrawAiEditorAddRow(uiFont, rowIndex++, U"Unit Priorities", U"Add Unit", scroll, U"生成候補ユニットとその出現比率を追加します。", hoverHelp);
		for (const auto& weight : profile.unitWeights)
		{
			DrawAiEditorUnitWeightRow(editor, defs, uiFont, rowIndex++, weight, scroll, hoverHelp);
		}

		DrawAiEditorAddRow(uiFont, rowIndex++, U"Build Priorities", U"Add Build", scroll, U"建設 action ごとの重みと目標棟数を追加します。", hoverHelp);
		for (const auto& buildPriority : profile.buildPriorities)
		{
			DrawAiEditorBuildPriorityRow(uiFont, rowIndex++, buildPriority, scroll, hoverHelp);
		}

		DrawAiEditorAddRow(uiFont, rowIndex++, U"Target Priority", U"Add Target", scroll, U"攻撃優先対象の候補を追加します。", hoverHelp);
		for (int32 targetIndex = 0; targetIndex < static_cast<int32>(profile.targetPriority.size()); ++targetIndex)
		{
			DrawAiEditorTargetPriorityRow(uiFont, rowIndex++, targetIndex, profile.targetPriority[targetIndex], scroll, hoverHelp);
		}

		if (!hoverHelp.isEmpty())
		{
			const RectF helpRect = AiEditorHelpPopupRect(RectF{ Cursor::PosF().x - 9.0, Cursor::PosF().y - 9.0, 18.0, 18.0 });
			helpRect.draw(ColorF{ 0.02, 0.03, 0.045, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
			uiFont(hoverHelp).draw(11, helpRect.x + 10.0, helpRect.y + 10.0, Palette::Aqua);
		}

		DrawAiEditorUnitWeightStepMenu(editor, uiFont);
		DrawAiEditorUnitWeightUnitPicker(editor, defs, uiFont);
	}
}
