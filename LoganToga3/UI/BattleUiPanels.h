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
		inline constexpr int32 BattleSkillPanelColumnCount = 5;

		inline RectF BattleSkillPanelRect()
		{
			return RectF{ 18.0, 612.0, 356.0, 270.0 };
		}

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

		inline Array<BattleSkillFilterKind> CollectBattleSkillFilterKinds()
		{
			return { BattleSkillFilterKind::All, BattleSkillFilterKind::Heal, BattleSkillFilterKind::ResourceCost };
		}

		inline RectF BattleSkillFilterButtonRect(const RectF& panel, int32 index)
		{
			return RectF{ panel.x + 12.0 + index * 108.0, panel.y + 28.0, 100.0, 26.0 };
		}

		inline RectF BattleSkillIconRect(const RectF& panel, int32 index)
		{
			constexpr double iconSize = 52.0;
			constexpr double gapX = 10.0;
			constexpr double gapY = 10.0;
			const int32 column = index % BattleSkillPanelColumnCount;
			const int32 row = index / BattleSkillPanelColumnCount;
			return RectF{ panel.x + 14.0 + column * (iconSize + gapX), panel.y + 72.0 + row * (iconSize + gapY), iconSize, iconSize };
		}

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

		inline bool IsCursorOnBattleSkillUi(const BattleWorld& world, const DefinitionStores& defs)
		{
			const UnitId selected = GetSelectedUnit(world);
			if (!IsValidUnit(world, selected) || world.units.faction[selected] != Faction::Player)
			{
				return false;
			}

			return BattleSkillPanelRect().mouseOver();
		}

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

		struct BuildActionTooltipTextSections
		{
			String bodyText;
			String redText;
		};

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

	inline String FormatBattleTimerText(double totalSec)
	{
		const int32 remainingSec = Max(0, static_cast<int32>(Ceil(totalSec)));
		const int32 minutes = remainingSec / 60;
		const int32 seconds = remainingSec % 60;
		return U"{:02}:{:02}"_fmt(minutes, seconds);
	}

	inline void DrawBattleTimerOverlay(const BattleWorld& world, const Font& uiFont)
	{
		if (world.aiRuntime.battleTimeLimitSec <= 0.0)
		{
			return;
		}

		const double remainingSec = Max(0.0, world.aiRuntime.battleTimeLimitSec - world.elapsedSec);
		const bool urgent = remainingSec <= 60.0;
		ColorF timerColor{ 1.0, 1.0, 1.0 };
		ColorF frameColor{ 1.0, 0.84, 0.0, 0.60 };
		if (urgent)
		{
			timerColor = ColorF{ 1.0, 0.35, 0.30 };
			frameColor = ColorF{ 1.0, 0.30, 0.26, 0.92 };
		}
		const RectF panel{ 636.0, 18.0, 328.0, 54.0 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(2.0, 0.0, frameColor);
		uiFont(U"Time Limit").drawAt(13, panel.center().movedBy(0.0, -12.0), ColorF{ 0.0, 1.0, 1.0 });
		uiFont(FormatBattleTimerText(remainingSec)).drawAt(26, panel.center().movedBy(0.0, 10.0), timerColor);
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
