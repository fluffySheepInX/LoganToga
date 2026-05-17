#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleDebugOverlay.h"
# include "BattleResourceRenderer.h"
# include "QuarterView.h"
# include "MapEditor.h"

namespace LT3
{
	struct ClickDebugState
	{
		Vec2 currentScreen{ 0, 0 };
		Vec2 currentWorld{ 0, 0 };
		Optional<Point> currentCell;
		Optional<Vec2> lastLeftScreen;
		Optional<Vec2> lastLeftWorld;
		Optional<Point> lastLeftCell;
		Optional<Vec2> lastRightScreen;
		Optional<Vec2> lastRightWorld;
		Optional<Point> lastRightCell;
	};

	inline String FormatFaction(Faction faction)
	{
		switch (faction)
		{
		case Faction::Player:
			return U"Player";
		case Faction::Enemy:
			return U"Enemy";
		default:
			return U"Neutral";
		}
	}

	inline void DrawClickDebugOverlay(const ClickDebugState& debugState, const Font& uiFont)
	{
		const RectF panel{ 24, 72, 520, 176 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Click Debug").draw(44, 88, Palette::White);
		uiFont(U"Cursor screen: ({:.1f}, {:.1f})"_fmt(debugState.currentScreen.x, debugState.currentScreen.y)).draw(13, 44, 116, Palette::Lightgray);
		uiFont(U"Cursor world : ({:.1f}, {:.1f})"_fmt(debugState.currentWorld.x, debugState.currentWorld.y)).draw(13, 44, 136, Palette::Lightgray);
		uiFont(debugState.currentCell
			? U"Cursor cell  : ({}, {})"_fmt(debugState.currentCell->x, debugState.currentCell->y)
			: U"Cursor cell  : (n/a)").draw(13, 44, 156, Palette::Lightgray);

		const String lastLeft = debugState.lastLeftScreen
			? U"L screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
				debugState.lastLeftScreen->x,
				debugState.lastLeftScreen->y,
				debugState.lastLeftWorld ? debugState.lastLeftWorld->x : 0.0,
				debugState.lastLeftWorld ? debugState.lastLeftWorld->y : 0.0,
				debugState.lastLeftCell ? U"({}, {})"_fmt(debugState.lastLeftCell->x, debugState.lastLeftCell->y) : U"n/a")
			: U"L screen=(n/a) world=(n/a) cell=n/a";
		const String lastRight = debugState.lastRightScreen
			? U"R screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
				debugState.lastRightScreen->x,
				debugState.lastRightScreen->y,
				debugState.lastRightWorld ? debugState.lastRightWorld->x : 0.0,
				debugState.lastRightWorld ? debugState.lastRightWorld->y : 0.0,
				debugState.lastRightCell ? U"({}, {})"_fmt(debugState.lastRightCell->x, debugState.lastRightCell->y) : U"n/a")
			: U"R screen=(n/a) world=(n/a) cell=n/a";

		uiFont(lastLeft).draw(13, 44, 184, Palette::Skyblue);
		uiFont(lastRight).draw(13, 44, 204, Palette::Orange);
	}

	inline void DrawUiLayoutDragHandle(const RectF& panelRect, bool active)
	{
		if (!active)
		{
			return;
		}

		const RectF handle = UiLayoutDragHandleRect(panelRect);
		handle.draw(ColorF{ 1.0, 0.84, 0.0, 0.18 }).drawFrame(2.0, ColorF{ 1.0, 0.84, 0.0, 0.90 });
	}

	inline void DrawUiLayoutTopAnchorToggle(const RectF& panelRect, bool active, bool topAnchor)
	{
		if (!active)
		{
			return;
		}

		const RectF toggleRect = UiLayoutTopAnchorToggleRect(UiLayoutDragHandleRect(panelRect));
		toggleRect.draw(topAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2.0, toggleRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.18 });

		const Vec2 center = toggleRect.center();
		Line{ center.x - 5.0, center.y - 4.0, center.x + 5.0, center.y - 4.0 }.draw(2.0, topAnchor ? Palette::White : Palette::Lightgray);
		Triangle{ Vec2{ center.x, center.y - 7.0 }, Vec2{ center.x - 4.0, center.y }, Vec2{ center.x + 4.0, center.y } }.draw(topAnchor ? Palette::White : Palette::Lightgray);
	}

	inline void DrawSelectedUnitPanel(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Font& uiFont, bool showDebugInfo)
	{
		const UnitId selected = GetSelectedUnit(world);
		if (selected == InvalidUnitId)
		{
			return;
		}

		const UnitDef& def = defs.units[world.units.defId[selected]];
		const bool showDetail = KeyControl.pressed();
		const double lineStep = 24.0;

		const RectF info = showDetail
			? BattleInfoPanelDetailRect(mapEditor)
			: BattleInfoPanelCompactRect(mapEditor);
		info.draw(ColorF{ 0.02, 0.03, 0.045, 0.78 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		DrawUiLayoutDragHandle(info, mapEditor.uiLayoutEditEnabled);
		DrawUiLayoutTopAnchorToggle(info, mapEditor.uiLayoutEditEnabled, mapEditor.uiSelectedInfoTopAnchor);

		const double nameY = info.y + 18.0;
		uiFont(def.name).draw(info.x + 18.0, nameY, Palette::White);

		const int32 maxHp = Max(1, def.hp);
		const double hpRate = Clamp(static_cast<double>(world.units.hp[selected]) / maxHp, 0.0, 1.0);
		const double hpY = nameY + 34.0;
		uiFont(U"HP:").draw(info.x + 18.0, hpY, Palette::Lightgray);

		const RectF hpBack{ info.x + 66.0, hpY + 4.0, 116.0, 14.0 };
		hpBack.draw(ColorF{ 0.08, 0.08, 0.08, 0.82 });
		ColorF hpColor{ 1.0, 0.25, 0.20 };
		if (hpRate > 0.35)
		{
			hpColor = ColorF{ 0.20, 0.80, 0.20 };
		}
		RectF{ hpBack.pos, hpBack.w * hpRate, hpBack.h }.draw(hpColor);
		hpBack.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });

		const double hpTextY = hpY + lineStep;
		uiFont(U"{}/{}  {}%"_fmt(world.units.hp[selected], def.hp, static_cast<int32>(hpRate * 100.0))).draw(info.x + 18.0, hpTextY, Palette::White);
			if (hpRate <= 0.30)
			{
				uiFont(U"⚠").draw(info.x + 188.0, hpTextY, ColorF{ 1.0, 0.70, 0.20 });
			}

		if (!showDetail)
		{
			uiFont(U"Ctrl: details").draw(info.x + 18.0, info.y + info.h - 24.0, ColorF{ 0.70, 0.80, 0.95 });
			return;
		}

		const double tagY = hpTextY + lineStep;
		const double factionY = tagY + lineStep;
		const double atkY = factionY + lineStep;
		const double spdY = atkY + lineStep;
		const double taskY = spdY + lineStep;
		uiFont(U"Tag: {}"_fmt(def.tag)).draw(info.x + 18.0, tagY, Palette::Skyblue);
		uiFont(U"Faction: {}"_fmt(FormatFaction(world.units.faction[selected]))).draw(info.x + 18.0, factionY, Palette::Skyblue);
		uiFont(U"ATK: {}"_fmt(def.attack)).draw(info.x + 18.0, atkY, Palette::Lightgray);
		uiFont(U"SPD: {:.0f}"_fmt(def.speed)).draw(info.x + 18.0, spdY, Palette::Lightgray);
		uiFont(U"Task: {}"_fmt(static_cast<int32>(world.units.task[selected]))).draw(info.x + 18.0, taskY, Palette::Lightgray);

		const Array<QueuedBuildAction>& buildQueue = GetQueuedBuildActionEntries(world, selected);
		if (!buildQueue.isEmpty() && buildQueue.front().actionId < defs.buildActions.size())
		{
			const BuildActionDef& action = defs.buildActions[buildQueue.front().actionId];
			const double rate = Clamp(world.buildQueues.progressSec[selected] / Max(0.001, action.buildTimeSec), 0.0, 1.0);
			const double buildY = taskY + lineStep;
			const double queueY = buildY + lineStep;
			uiFont(U"Build: {}"_fmt(action.name)).draw(info.x + 18.0, buildY, Palette::Gold);
			uiFont(U"Queue: {}"_fmt(buildQueue.size())).draw(info.x + 18.0, queueY, Palette::Gold);
			RectF{ info.x + 120.0, queueY + 6.0, 150.0, 12.0 }.draw(ColorF{ 0, 0, 0, 0.45 });
			RectF{ info.x + 120.0, queueY + 6.0, 150.0 * rate, 12.0 }.draw(Palette::Gold);

			const size_t previewCount = Min<size_t>(3, buildQueue.size());
			for (size_t i = 0; i < previewCount; ++i)
			{
				const BuildActionDefId queuedActionId = buildQueue[i].actionId;
				if (queuedActionId >= defs.buildActions.size())
				{
					continue;
				}

				const String prefix = (i == 0) ? U">" : U"-";
				uiFont(U"{} {}"_fmt(prefix, defs.buildActions[queuedActionId].name)).draw(16, info.x + 18.0, queueY + lineStep + static_cast<int32>(i) * 18, Palette::Lightgray);
			}
		}

		if (showDebugInfo)
		{
			DrawBattleDebugOverlay(world, defs, uiFont, info);
		}
	}

	inline void DrawResultOverlay(const BattleWorld& world, const Font& uiFont, const Font& titleFont)
	{
		if (!(world.victory || world.defeat))
		{
			return;
		}

		Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0, 0, 0, 0.58 });
		String resultText = U"DEFEAT";
		ColorF resultColor{ 1.0, 0.25, 0.20 };
		if (world.victory)
		{
			resultText = U"VICTORY";
			resultColor = ColorF{ 1.0, 0.84, 0.0 };
		}
		titleFont(resultText).drawAt(90, Vec2{ 800, 410 }, resultColor);
		uiFont(U"Press ESC or close from the Gaussian menu.").drawAt(800, 500, Palette::White);
	}

	inline RectF BattleBuildQueuePanelRect(const MapEditorState& mapEditor, int32 rows, size_t previewCount)
	{
		const RectF commandPanel = BattleCommandPanelRect(mapEditor, rows);
		const double desiredHeight = 94.0 + static_cast<double>(previewCount) * 62.0;
		const double panelHeight = Max(commandPanel.h, desiredHeight);
		return RectF{ commandPanel.x - 304.0, commandPanel.y + commandPanel.h - panelHeight, 292.0, panelHeight };
	}

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

		for (int32 visibleIndex = 0; visibleIndex < static_cast<int32>(visibleActions.size()); ++visibleIndex)
		{
			const BuildActionUiState& actionState = visibleActions[visibleIndex];
			const BuildActionDef& action = defs.buildActions[actionState.actionId];
			const RectF rect = BattleCommandIconRect(mapEditor, visibleIndex, rows);
			const bool affordable = actionState.affordable;

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
			}
			rect.drawFrame(2, frameColor);

			const bool hasIcon = DrawBuildActionIcon(action, defs, assets, rect.center().movedBy(0, -5), 60.0);

			if (!hasIcon)
			{
				uiFont(U"{}"_fmt(visibleIndex + 1)).drawAt(16, rect.center().movedBy(0, -4), Palette::White);
			}

			const ColorF costColor = affordable ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.25, 0.20 };
			uiFont(U"{}G"_fmt(action.costGold)).drawAt(12, rect.center().movedBy(0, 26), costColor);
		}

		DrawSelectedBuildQueuePanel(world, defs, mapEditor, assets, uiFont, rows);
	}
}
