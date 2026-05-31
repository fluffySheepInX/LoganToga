#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleResourceRenderer.h"
# include "QuarterView.h"

namespace LT3
{
	inline constexpr int32 BattleSkillPanelColumnCount = 5;

	// スキルパネル全体の表示領域を返す。
	inline RectF BattleSkillPanelRect()
	{
		return RectF{ 18.0, 612.0, 356.0, 270.0 };
	}

	// フィルター種別の表示ラベルを返す。
	inline String FormatBattleSkillFilterLabel(BattleSkillFilterKind filter)
	{
		switch (filter)
		{
		case BattleSkillFilterKind::Heal:
			return U"回復";
		case BattleSkillFilterKind::ResourceCost:
			return U"リソース消費型";
		default:
			return U"ALL";
		}
	}

	// 表示するスキルフィルター一覧を返す。
	inline Array<BattleSkillFilterKind> CollectBattleSkillFilterKinds()
	{
		return { BattleSkillFilterKind::All, BattleSkillFilterKind::Heal, BattleSkillFilterKind::ResourceCost };
	}

	// フィルターボタンの矩形を計算する。
	inline RectF BattleSkillFilterButtonRect(const RectF& panel, int32 index)
	{
		return RectF{ panel.x + 12.0 + index * 108.0, panel.y + 28.0, 100.0, 26.0 };
	}

	// スキルアイコンの表示矩形を計算する。
	inline RectF BattleSkillIconRect(const RectF& panel, int32 index)
	{
		constexpr double iconSize = 52.0;
		constexpr double gapX = 10.0;
		constexpr double gapY = 10.0;
		const int32 column = index % BattleSkillPanelColumnCount;
		const int32 row = index / BattleSkillPanelColumnCount;
		return RectF{ panel.x + 14.0 + column * (iconSize + gapX), panel.y + 72.0 + row * (iconSize + gapY), iconSize, iconSize };
	}

	// スキルアイコンを描画し、何か描けたかを返す。
	inline bool DrawBattleSkillIcon(const SkillDef& skill, const BattleRenderAssets& assets, const Vec2& center, double size)
	{
		Array<FilePath> iconPaths;
		for (const auto& icon : skill.iconLayers)
		{
			if (!icon.isEmpty())
			{
				iconPaths << ResolveBuildIconPath(icon);
			}
		}
		if (iconPaths.isEmpty() && !skill.icon.isEmpty())
		{
			iconPaths << ResolveBuildIconPath(skill.icon);
		}

		bool drewAny = false;
		for (const auto& iconPath : iconPaths)
		{
			if (iconPath.isEmpty() || !FileSystem::Exists(iconPath))
			{
				continue;
			}

			if (!assets.iconTextureCache.contains(iconPath))
			{
				assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
			}
			assets.iconTextureCache.at(iconPath).resized(size, size).drawAt(center);
			drewAny = true;
		}

		return drewAny;
	}

	// 現在のフィルターで可視なスキル一覧を収集する。
	inline Array<SkillDefId> CollectVisibleBattleSkills(const BattleWorld& world, const DefinitionStores& defs)
	{
		Array<SkillDefId> visibleSkills;
		const UnitId selected = GetSelectedUnit(world);
		if (!IsValidUnit(world, selected) || world.units.defId[selected] >= defs.units.size())
		{
			return visibleSkills;
		}

		const UnitDef& unitDef = defs.units[world.units.defId[selected]];
		for (const SkillDefId skillId : ResolveUnitSkillIds(unitDef, defs))
		{
			if (skillId == InvalidSkillDefId || skillId >= defs.skills.size())
			{
				continue;
			}

			if (DoesSkillMatchBattleFilter(defs.skills[skillId], world.selection.skillFilter))
			{
				visibleSkills << skillId;
			}
		}

		return visibleSkills;
	}

	// カーソルがスキルUI上にあるかを判定する。
	inline bool IsCursorOnBattleSkillUi(const BattleWorld& world, const DefinitionStores& defs)
	{
		const UnitId selected = GetSelectedUnit(world);
		if (!IsValidUnit(world, selected) || world.units.faction[selected] != Faction::Player)
		{
			return false;
		}

		return BattleSkillPanelRect().mouseOver();
	}

	// 選択ユニット用のスキルパネルを描画する。
	inline void DrawBattleSkillPanel(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
	{
		const UnitId selected = GetSelectedUnit(world);
		if (!IsValidUnit(world, selected) || world.units.faction[selected] != Faction::Player)
		{
			return;
		}

		const Array<SkillDefId> visibleSkills = CollectVisibleBattleSkills(world, defs);
		const RectF panel = BattleSkillPanelRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.88 }).drawFrame(1.0, 0.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });
		uiFont(U"Skillの種類でフィルター").draw(12, panel.x + 12.0, panel.y + 8.0, Palette::White);

		const Array<BattleSkillFilterKind> filters = CollectBattleSkillFilterKinds();
		for (int32 i = 0; i < static_cast<int32>(filters.size()); ++i)
		{
			const BattleSkillFilterKind filter = filters[i];
			const RectF filterRect = BattleSkillFilterButtonRect(panel, i);
			const bool active = (world.selection.skillFilter == filter);
			filterRect.draw(active ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2.0, 0.0, filterRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 1.0, 1.0, 0.16 });
			uiFont(FormatBattleSkillFilterLabel(filter)).drawAt(12, filterRect.center(), active ? Palette::White : Palette::Lightgray);
		}

		for (int32 i = 0; i < static_cast<int32>(visibleSkills.size()); ++i)
		{
			const SkillDefId skillId = visibleSkills[i];
			const SkillDef& skill = defs.skills[skillId];
			const RectF iconRect = BattleSkillIconRect(panel, i);
			const bool selectedSkill = (world.selection.selectedSkill == skillId);
			const bool affordable = CanAffordSkillResourceCosts(world, defs, Faction::Player, skill);
			iconRect.draw(selectedSkill ? ColorF{ 0.20, 0.18, 0.08, 0.98 } : ColorF{ 0.08, 0.08, 0.10, 0.94 })
				.drawFrame(2.0, 0.0, selectedSkill ? ColorF{ 1.0, 0.84, 0.0, 0.92 } : (affordable ? ColorF{ 1.0, 1.0, 1.0, 0.18 } : ColorF{ 1.0, 0.28, 0.28, 0.75 }));

			if (!DrawBattleSkillIcon(skill, assets, iconRect.center().movedBy(0.0, -2.0), 34.0))
			{
				uiFont(U"S{}"_fmt(skillId)).drawAt(12, iconRect.center().movedBy(0.0, -2.0), Palette::White);
			}

			if (DoesSkillTargetAllies(skill))
			{
				uiFont(U"H").draw(11, iconRect.x + 4.0, iconRect.y + 2.0, ColorF{ 0.45, 1.0, 0.55 });
			}
			else if (DoesSkillConsumeResources(skill))
			{
				uiFont(U"R").draw(11, iconRect.x + 4.0, iconRect.y + 2.0, ColorF{ 0.95, 0.82, 0.35 });
			}

			if (iconRect.mouseOver())
			{
				const RectF tip{ panel.x, Max(12.0, panel.y - 94.0), panel.w, 86.0 };
				tip.draw(ColorF{ 0.03, 0.05, 0.08, 0.96 }).drawFrame(2.0, 0.0, affordable ? ColorF{ 1.0, 0.84, 0.0, 0.75 } : ColorF{ 1.0, 0.28, 0.28, 0.78 });
				uiFont(skill.name.isEmpty() ? U"Skill {}"_fmt(skillId) : skill.name).draw(14, tip.x + 10.0, tip.y + 8.0, Palette::White);
				uiFont(U"CD {:.1f}s  Range {:.0f}"_fmt(skill.cooldownSec, skill.range)).draw(11, tip.x + 10.0, tip.y + 30.0, Palette::Lightgray);
				if (!skill.resourceCosts.isEmpty())
				{
					String costText;
					for (const auto& cost : skill.resourceCosts)
					{
						if (!costText.isEmpty())
						{
							costText += U" / ";
						}
						costText += U"{} {}"_fmt(cost.resourceTag, cost.amount);
					}
					uiFont(costText).draw(11, tip.x + 10.0, tip.y + 50.0, affordable ? ColorF{ 0.0, 1.0, 1.0 } : ColorF{ 1.0, 0.36, 0.36 });
				}
			}
		}

		if (visibleSkills.isEmpty())
		{
			uiFont(U"該当Skillなし").draw(panel.x + 14.0, panel.y + 82.0, Palette::Lightgray);
		}
	}
}
