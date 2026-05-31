#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "BattleDebugOverlay.h"
# include "MapEditor.h"
# include "BattleUiPanels.Debug.h"

namespace LT3
{
	// 選択ユニット情報パネルを描画する。
	inline void DrawSelectedUnitPanel(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Font& uiFont, bool showDebugInfo)
	{
		const UnitId selected = GetSelectedUnit(world);
		if (selected == InvalidUnitId)
		{
			return;
		}

		const Array<UnitId>& selectedUnits = GetSelectedUnits(world);
		const bool showMultiUnitList = (selectedUnits.size() >= 2);
		if (showMultiUnitList)
		{
			const double rowHeight = 28.0;
			const RectF info = BattleInfoPanelMultiRect(mapEditor, selectedUnits.size());
			info.draw(ColorF{ 0.02, 0.03, 0.045, 0.78 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
			DrawUiLayoutDragHandle(info, mapEditor.uiLayoutEditEnabled);
			DrawUiLayoutTopAnchorToggle(info, mapEditor.uiLayoutEditEnabled, mapEditor.uiSelectedInfoTopAnchor);

			uiFont(U"選択ユニット {}体"_fmt(selectedUnits.size())).draw(info.x + 18.0, info.y + 14.0, Palette::White);

			const RectF listArea{ info.x + 12.0, info.y + 42.0, info.w - 24.0, info.h - 54.0 };
			const size_t visibleCount = Min<size_t>(12, selectedUnits.size());

			for (size_t i = 0; i < visibleCount; ++i)
			{
				const UnitId unit = selectedUnits[i];
				if (!IsValidUnit(world, unit) || world.units.defId[unit] >= defs.units.size())
				{
					continue;
				}

				const UnitDef& rowDef = defs.units[world.units.defId[unit]];
				const double rowY = listArea.y + static_cast<double>(i) * rowHeight;
				const RectF rowRect{ listArea.x, rowY, listArea.w, rowHeight - 4.0 };
				const bool isEnemy = (world.units.faction[unit] == Faction::Enemy);
				rowRect.draw(isEnemy ? ColorF{ 0.18, 0.07, 0.07, 0.86 } : ColorF{ 0.07, 0.10, 0.16, 0.86 });
				rowRect.drawFrame(1.0, ColorF{ 1, 1, 1, 0.10 });

				uiFont(rowDef.name).draw(12, rowRect.x + 8.0, rowRect.y + 4.0, Palette::White);

				const int32 rowMaxHp = Max(1, rowDef.hp);
				const double hpRate = Clamp(static_cast<double>(world.units.hp[unit]) / rowMaxHp, 0.0, 1.0);
				const RectF hpBack{ rowRect.x + 120.0, rowRect.y + 7.0, rowRect.w - 168.0, 12.0 };
				hpBack.draw(ColorF{ 0.08, 0.08, 0.08, 0.82 });
				ColorF hpColor{ 1.0, 0.25, 0.20 };
				if (hpRate > 0.35)
				{
					hpColor = ColorF{ 0.20, 0.80, 0.20 };
				}
				RectF{ hpBack.pos, hpBack.w * hpRate, hpBack.h }.draw(hpColor);
				hpBack.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });
				uiFont(U"{}/{}"_fmt(world.units.hp[unit], rowDef.hp)).draw(11, rowRect.x + rowRect.w - 44.0, rowRect.y + 2.0, Palette::Lightgray);
			}

			if (visibleCount < selectedUnits.size())
			{
				uiFont(U"他 {} 体..."_fmt(selectedUnits.size() - visibleCount)).draw(info.x + 18.0, info.y + info.h - 20.0, ColorF{ 0.70, 0.80, 0.95 });
			}
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
		uiFont(U"Unit ID: {}"_fmt(def.unit_id)).draw(info.x + 18.0, tagY, Palette::Skyblue);
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
			DrawBattleDebugOverlay(world, defs, mapEditor, uiFont, info);
		}
	}
}
