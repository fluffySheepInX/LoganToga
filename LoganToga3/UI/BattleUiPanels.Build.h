#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "BattleResourceRenderer.h"
# include "QuarterView.h"
# include "MapEditor.h"
# include "MapEditorUiLayout.h"

namespace LT3
{
	struct BuildActionTooltipTextSections
	{
		String bodyText;
		String redText;
	};

	// ツールチップ本文を通常文と赤字文に分割する。
	inline BuildActionTooltipTextSections SplitBuildActionTooltipText(const String& description)
	{
		BuildActionTooltipTextSections sections;
		const String normalized = description.replaced(U"\r", U"");
		constexpr StringView redOpen = U"<red>";
		constexpr StringView redClose = U"</red>";
		size_t searchPos = 0;

		while (searchPos < normalized.size())
		{
			const size_t start = normalized.indexOf(redOpen, searchPos);
			if (start == String::npos)
			{
				sections.bodyText += normalized.substr(searchPos);
				break;
			}

			sections.bodyText += normalized.substr(searchPos, start - searchPos);
			const size_t redStart = start + redOpen.size();
			const size_t end = normalized.indexOf(redClose, redStart);
			const String redSegment = (end == String::npos)
				? normalized.substr(redStart)
				: normalized.substr(redStart, end - redStart);

			if (!redSegment.isEmpty())
			{
				if (!sections.redText.isEmpty())
				{
					sections.redText += U"\n";
				}
				sections.redText += redSegment;
			}

			if (end == String::npos)
			{
				break;
			}

			searchPos = end + redClose.size();
		}

		return sections;
	}

	// 指定幅に収まるようにツールチップ文字列を折り返す。
	inline Array<String> WrapBuildActionTooltipText(const String& text, const Font& uiFont, int32 fontSize, double maxWidth)
	{
		Array<String> lines;
		const String normalized = text.replaced(U"\r", U"");
		const Array<String> paragraphs = normalized.split(U'\n');

		for (const String& paragraph : paragraphs)
		{
			if (paragraph.isEmpty())
			{
				lines << U"";
				continue;
			}

			String current;
			for (const auto ch : paragraph)
			{
				const String candidate = current + String{ ch };
				if (!current.isEmpty() && uiFont(candidate).region(fontSize).w > maxWidth)
				{
					lines << current;
					current = String{ ch };
				}
				else
				{
					current = candidate;
				}
			}

			lines << current;
		}

		while (!lines.isEmpty() && lines.back().isEmpty())
		{
			lines.pop_back();
		}

		return lines;
	}

	// 建築アクションのツールチップを描画する。
	inline void DrawBuildActionTooltip(const BuildActionDef& action, const RectF& commandPanel, const RectF& iconRect, bool affordable, bool blockedByUnique, const Font& uiFont)
	{
		(void)iconRect;
		const BuildActionTooltipTextSections sections = SplitBuildActionTooltipText(action.description);
		const double panelWidth = commandPanel.w;
		const double textWidth = Max(0.0, panelWidth - 24.0);
		const Array<String> bodyLines = WrapBuildActionTooltipText(sections.bodyText, uiFont, 12, textWidth);
		const Array<String> redLines = WrapBuildActionTooltipText(sections.redText, uiFont, 12, textWidth);
		constexpr double panelOffset = 10.0;
		constexpr double topPadding = 12.0;
		constexpr double bottomPadding = 12.0;
		constexpr double lineHeight = 18.0;
		constexpr double sectionGap = 8.0;
		constexpr double titleBlockHeight = 40.0;
		const double bodyHeight = static_cast<double>(bodyLines.size()) * lineHeight;
		const double redHeight = static_cast<double>(redLines.size()) * lineHeight;
		const double extraGap = (!bodyLines.isEmpty() && !redLines.isEmpty()) ? sectionGap : 0.0;
		const double tooltipHeight = titleBlockHeight + topPadding + bottomPadding + bodyHeight + redHeight + extraGap;

		const RectF tooltipRect{
			commandPanel.x,
			Max(8.0, commandPanel.y - tooltipHeight - panelOffset),
			panelWidth,
			tooltipHeight
		};

		ColorF accentColor{ 1.0, 0.35, 0.30 };
		if (affordable)
		{
			accentColor = ColorF{ 1.0, 0.84, 0.0 };
		}
		if (blockedByUnique)
		{
			accentColor = ColorF{ 1.0, 0.20, 0.20 };
		}

		RoundRect{ tooltipRect, 8.0 }.draw(ColorF{ 0.03, 0.05, 0.08, 0.96 });
		RoundRect{ tooltipRect, 8.0 }.drawFrame(2.0, 0.0, accentColor);
		uiFont(action.name).draw(15, tooltipRect.x + 12.0, tooltipRect.y + 10.0, Palette::White);
		const ColorF costTextColor{ affordable ? 1.0 : 1.0, affordable ? 0.84 : 0.60, affordable ? 0.0 : 0.55, 1.0 };
		uiFont(U"G{} T{} F{}"_fmt(action.costGold, action.costTrust, action.costFood)).draw(11, tooltipRect.x + 12.0, tooltipRect.y + 30.0, costTextColor);

		double lineY = tooltipRect.y + titleBlockHeight;
		for (const String& line : bodyLines)
		{
			uiFont(line).draw(12, tooltipRect.x + 12.0, lineY, ColorF{ 0.96, 0.97, 0.99 });
			lineY += lineHeight;
		}

		if (!bodyLines.isEmpty() && !redLines.isEmpty())
		{
			lineY += sectionGap;
		}

		double redY = tooltipRect.y + tooltipRect.h - bottomPadding - redHeight;
		for (const String& line : redLines)
		{
			uiFont(line).draw(12, tooltipRect.x + 12.0, redY, ColorF{ 1.0, 0.34, 0.34 });
			redY += lineHeight;
		}
	}

	// 建築キューパネルの矩形を返す。
	inline RectF BattleBuildQueuePanelRect(const MapEditorState& mapEditor, int32 rows, size_t previewCount)
	{
		const RectF commandPanel = BattleCommandPanelRect(mapEditor, rows);
		const double desiredHeight = 94.0 + static_cast<double>(previewCount) * 62.0;
		const double panelHeight = Max(commandPanel.h, desiredHeight);
		return RectF{ commandPanel.x - 304.0, commandPanel.y + commandPanel.h - panelHeight, 292.0, panelHeight };
	}

	// 選択ユニットの建築キューを描画する。
	inline void DrawSelectedBuildQueuePanel(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const BattleRenderAssets& assets, const Font& uiFont, int32 commandRows)
	{
		const UnitId selected = GetSelectedUnit(world);
		const Array<QueuedBuildAction>& queue = GetQueuedBuildActionEntries(world, selected);
		if (queue.isEmpty())
		{
			return;
		}

		const size_t previewCount = Min<size_t>(4, queue.size());
		const RectF panel = BattleBuildQueuePanelRect(mapEditor, commandRows, previewCount);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });
		uiFont(U"キュー").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);
		uiFont(U"{}件"_fmt(queue.size())).draw(14, panel.x + panel.w - 56.0, panel.y + 12.0, Palette::Gold);

		for (size_t i = 0; i < previewCount; ++i)
		{
			const BuildActionDefId actionId = queue[i].actionId;
			if (actionId >= defs.buildActions.size())
			{
				continue;
			}

			const BuildActionDef& action = defs.buildActions[actionId];
			const RectF slot{ panel.x + 16.0, panel.y + 42.0 + static_cast<double>(i) * 62.0, 56.0, 56.0 };
			slot.draw(i == 0 ? ColorF{ 0.14, 0.12, 0.06, 0.96 } : ColorF{ 0.08, 0.08, 0.10, 0.92 });
			slot.drawFrame(2.0, i == 0 ? ColorF{ 1.0, 0.84, 0.0, 0.90 } : ColorF{ 1, 1, 1, 0.18 });

			if (!DrawBuildActionIcon(action, defs, assets, slot.center().movedBy(0, -3), 42.0))
			{
				uiFont(U"{}"_fmt(i + 1)).drawAt(16, slot.center().movedBy(0, -3), Palette::White);
			}

			uiFont(action.name).draw(13, panel.x + 82.0, slot.y + 8.0, i == 0 ? Palette::Gold : Palette::Lightgray);
		}

		const BuildActionDefId currentActionId = queue.front().actionId;
		if (currentActionId < defs.buildActions.size())
		{
			const BuildActionDef& action = defs.buildActions[currentActionId];
			const double rate = Clamp(world.buildQueues.progressSec[selected] / Max(0.001, action.buildTimeSec), 0.0, 1.0);
			const RectF progressBack{ panel.x + 16.0, panel.y + panel.h - 24.0, panel.w - 32.0, 10.0 };
			progressBack.draw(ColorF{ 0, 0, 0, 0.48 });
			RectF{ progressBack.pos, progressBack.w * rate, progressBack.h }.draw(Palette::Gold);
			progressBack.drawFrame(1.0, ColorF{ 1, 1, 1, 0.14 });
			uiFont(action.name).draw(13, panel.x + 16.0, panel.y + panel.h - 48.0, Palette::Lightgray);
		}
	}

	// 建築コマンドバーを描画する。
	inline void DrawQuarterCommandBar(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const BattleRenderAssets& assets, const Font& uiFont)
	{
		const Array<BuildActionUiState> visibleActions = CollectVisibleBuildActionsForSelectedUnit(world, defs);

		if (visibleActions.isEmpty())
		{
			return;
		}

		const int32 rows = (static_cast<int32>(visibleActions.size()) + 2) / 3;
		const RectF panel = BattleCommandPanelRect(mapEditor, rows);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });
		DrawUiLayoutDragHandle(panel, mapEditor.uiLayoutEditEnabled);
		DrawUiLayoutTopAnchorToggle(panel, mapEditor.uiLayoutEditEnabled, mapEditor.uiCommandPanelTopAnchor);

		const BuildActionDef* hoveredAction = nullptr;
		RectF hoveredRect;
		bool hoveredAffordable = false;
		bool hoveredBlockedByUnique = false;

		for (int32 visibleIndex = 0; visibleIndex < static_cast<int32>(visibleActions.size()); ++visibleIndex)
		{
			const BuildActionUiState& actionState = visibleActions[visibleIndex];
			const BuildActionDef& action = defs.buildActions[actionState.actionId];
			const RectF rect = BattleCommandIconRect(mapEditor, visibleIndex, rows);
			const bool affordable = actionState.affordable;
			const bool blockedByUnique = actionState.blockedByUnique;

			ColorF backColor{ 0.08, 0.08, 0.10, 0.92 };
			if (affordable)
			{
				backColor = ColorF{ 0.12, 0.20, 0.16, 0.96 };
			}
			rect.draw(backColor);

			ColorF frameColor{ 1, 1, 1, 0.18 };
			if (rect.mouseOver())
			{
				frameColor = ColorF{ 1.0, 0.84, 0.0 };
				hoveredAction = &action;
				hoveredRect = rect;
				hoveredAffordable = affordable;
				hoveredBlockedByUnique = blockedByUnique;
			}
			rect.drawFrame(2, frameColor);

			const bool hasIcon = DrawBuildActionIcon(action, defs, assets, rect.center().movedBy(0, -5), 60.0);

			if (!hasIcon)
			{
				uiFont(U"{}"_fmt(visibleIndex + 1)).drawAt(16, rect.center().movedBy(0, -4), Palette::White);
			}

			if (blockedByUnique)
			{
				Line{ rect.tl().movedBy(8.0, 8.0), rect.br().movedBy(-8.0, -8.0) }.draw(4.0, ColorF{ 1.0, 0.20, 0.20, 0.92 });
				Line{ rect.tr().movedBy(-8.0, 8.0), rect.bl().movedBy(8.0, -8.0) }.draw(4.0, ColorF{ 1.0, 0.20, 0.20, 0.92 });
				uiFont(U"UNIQ").drawAt(10, rect.center().movedBy(0, -30), Palette::White);
			}

			const ColorF costColor = affordable ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.25, 0.20 };
			uiFont(U"G{} T{} F{}"_fmt(action.costGold, action.costTrust, action.costFood)).drawAt(10, rect.center().movedBy(0, 26), costColor);
		}

		DrawSelectedBuildQueuePanel(world, defs, mapEditor, assets, uiFont, rows);
		if (hoveredAction)
		{
			DrawBuildActionTooltip(*hoveredAction, panel, hoveredRect, hoveredAffordable, hoveredBlockedByUnique, uiFont);
		}
	}
}
