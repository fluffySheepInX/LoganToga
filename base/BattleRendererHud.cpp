#include "BattleRenderer.h"

#include "BattleCommandUi.h"

namespace
{
	[[nodiscard]] String GetCommandIconGlyph(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return U"W";
		case UnitArchetype::Soldier:
			return U"S";
		case UnitArchetype::Archer:
			return U"A";
		case UnitArchetype::Barracks:
			return U"B";
		case UnitArchetype::Base:
			return U"H";
		default:
			return U"?";
		}
	}

	[[nodiscard]] ColorF GetCommandIconColor(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return ColorF{ 0.22, 0.68, 0.42 };
		case UnitArchetype::Soldier:
			return ColorF{ 0.24, 0.48, 0.88 };
		case UnitArchetype::Archer:
			return ColorF{ 0.86, 0.56, 0.22 };
		case UnitArchetype::Barracks:
			return ColorF{ 0.56, 0.42, 0.74 };
		case UnitArchetype::Base:
			return ColorF{ 0.35, 0.60, 0.66 };
		default:
			return ColorF{ 0.45, 0.48, 0.55 };
		}
	}

	void DrawCommandTooltip(const CommandPanelLayout& layout, const CommandIconLayout& icon, const GameData& gameData)
	{
		const ColorF accentColor = GetCommandIconColor(icon.command.archetype);
		const double tooltipWidth = 236.0;
		const double tooltipHeight = 86.0;
		const RectF tooltipRect{
			Clamp(icon.rect.center().x - (tooltipWidth * 0.5), 16.0, Scene::Width() - tooltipWidth - 16.0),
			Max(16.0, layout.panelRect.y - tooltipHeight - 10.0),
			tooltipWidth,
			tooltipHeight
		};

		RoundRect{ tooltipRect, 9 }.draw(ColorF{ 0.03, 0.05, 0.08, 0.96 });
		RoundRect{ tooltipRect, 9 }.drawFrame(2, 0, accentColor);
		gameData.smallFont(GetArchetypeLabel(icon.command.archetype)).draw(tooltipRect.x + 12, tooltipRect.y + 10, Palette::White);
		gameData.smallFont(s3d::Format(U"Click / ", icon.command.slot, U"   @", GetArchetypeLabel(icon.command.producer))).draw(tooltipRect.x + 12, tooltipRect.y + 30, ColorF{ 0.84, 0.88, 0.94 });
		gameData.smallFont(s3d::Format(U"Cost: ", icon.command.cost, U"G")).draw(tooltipRect.x + 12, tooltipRect.y + 50, Palette::Gold);
		gameData.smallFont(icon.command.statusText).draw(
			tooltipRect.x + 12,
			tooltipRect.y + 68,
			icon.command.isEnabled ? ColorF{ 0.50, 0.92, 0.62 } : ColorF{ 1.0, 0.52, 0.46 });
	}

	void DrawCommandIcon(const RectF& rect, const CommandIconEntry& command, const GameData& gameData, const bool isHovered, const bool isPressed)
	{
		const double alpha = command.isEnabled ? 1.0 : 0.30;
		const ColorF iconColor{ GetCommandIconColor(command.archetype).r, GetCommandIconColor(command.archetype).g, GetCommandIconColor(command.archetype).b, alpha };
		const RectF animatedRect = isPressed
			? RectF{ rect.x + 2, rect.y + 3, rect.w - 4, rect.h - 4 }
			: (isHovered ? RectF{ rect.x - 1, rect.y - 1, rect.w + 2, rect.h + 2 } : rect);
		const ColorF backgroundColor = isHovered
			? ColorF{ 0.16, 0.18, 0.24, 0.98 }
			: ColorF{ 0.10, 0.11, 0.15, 0.96 };
		const ColorF fillColor = command.isEnabled ? backgroundColor : ColorF{ 0.08, 0.09, 0.12, 0.94 };
		const ColorF textColor{ 1.0, 1.0, 1.0, alpha };
		const ColorF goldColor{ 1.0, 0.84, 0.0, alpha };
		RoundRect{ animatedRect, 10 }.draw(fillColor);
		RoundRect{ animatedRect, 10 }.drawFrame(isHovered ? 4 : 2, 0, iconColor);

		Circle{ animatedRect.x + 18, animatedRect.y + 18, 13 }.draw(iconColor);
		gameData.smallFont(Format(command.slot)).drawAt(animatedRect.x + 18, animatedRect.y + 18, textColor);

		Circle{ animatedRect.center().x, animatedRect.y + 34, 18 }.draw(iconColor);
		gameData.uiFont(GetCommandIconGlyph(command.archetype)).drawAt(animatedRect.center().x, animatedRect.y + 33, textColor);
		gameData.smallFont(GetArchetypeLabel(command.archetype)).drawAt(animatedRect.center().x, animatedRect.y + 61, textColor);
		gameData.smallFont(s3d::Format(command.cost, U"G")).drawAt(animatedRect.center().x, animatedRect.y + 79, goldColor);
	}

	void DrawCommandSection(const CommandPanelLayout& layout, const GameData& gameData)
	{
		if (layout.productionIcons.isEmpty())
		{
			return;
		}

		const Vec2 cursorScreenPos = Cursor::PosF();
		const CommandIconLayout* hoveredIcon = nullptr;

		gameData.smallFont(U"PRODUCTION").draw(layout.panelRect.x + 16, layout.panelRect.y + 26, ColorF{ 0.82, 0.86, 0.92 });

		for (const auto& icon : layout.productionIcons)
		{
			const bool isHovered = icon.rect.intersects(cursorScreenPos);
			const bool isPressed = isHovered && MouseL.pressed() && icon.command.isEnabled;
			if (isHovered)
			{
				hoveredIcon = &icon;
			}

			DrawCommandIcon(icon.rect, icon.command, gameData, isHovered, isPressed);
		}

		if (hoveredIcon)
		{
			DrawCommandTooltip(layout, *hoveredIcon, gameData);
		}
	}
}

void BattleRenderer::drawHud(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	int32 playerResourceIncome = 0;
	int32 playerResourceCount = 0;
	for (const auto& resourcePoint : state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Player)
		{
			playerResourceIncome += resourcePoint.incomeAmount;
			++playerResourceCount;
		}
	}

	String productionText;
	for (const auto& slot : config.playerProductionSlots)
	{
		if (!productionText.isEmpty())
		{
			productionText += U" / ";
		}

		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		productionText += s3d::Format(slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" @", GetArchetypeLabel(slot.producer), U" (", cost, U"G)");
	}

	String constructionText = U"Build: none";
	for (const auto& slot : config.playerConstructionSlots)
	{
		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		constructionText = s3d::Format(U"Build: ", slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" (", cost, U"G)");
		break;
	}

	String queueText = U"Queue: idle";
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (unit && unit->isAlive && (unit->owner == Owner::Player))
		{
			if (!building.isConstructed)
			{
				queueText = s3d::Format(U"Build: ", GetArchetypeLabel(unit->archetype), U" (", building.constructionRemaining, U"s)");
				break;
			}

			if (!building.productionQueue.isEmpty())
			{
				const auto& currentItem = building.productionQueue.front();
				queueText = s3d::Format(U"Queue: ", GetArchetypeLabel(currentItem.archetype), U" @", GetArchetypeLabel(unit->archetype), U" x", building.productionQueue.size(), U" (", currentItem.remainingTime, U"s)");
				break;
			}
		}
	}

	RoundRect{ 16, 16, 480, 216, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(config.hud.controls).draw(28, 66, Palette::White);
	gameData.smallFont(productionText).draw(28, 88, Palette::White);
	gameData.smallFont(constructionText).draw(28, 110, Palette::White);
	gameData.smallFont(queueText).draw(28, 132, Palette::White);
	const String formationText = U"Formation: Q="
		+ GetFormationLabel(FormationType::Line)
		+ U" / W="
		+ GetFormationLabel(FormationType::Column)
		+ U" / E="
		+ GetFormationLabel(FormationType::Square)
		+ U" / Current="
		+ GetFormationLabel(state.playerFormation);
	gameData.smallFont(formationText).draw(28, 154, Palette::White);
	gameData.smallFont(s3d::Format(U"Resource: ", playerResourceCount, U" pts / +", playerResourceIncome, U" income")).draw(28, 176, Palette::White);
	gameData.smallFont(config.hud.escapeHint).draw(28, 198, Palette::White);
	gameData.smallFont(s3d::Format(U"Gold: ", state.playerGold)).draw(28, 220, Palette::Gold);

	if (const auto layout = BuildCommandPanelLayout(state, config))
	{
		RoundRect{ layout->panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		RoundRect{ layout->panelRect, 12 }.drawFrame(2, 0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.uiFont(layout->title).draw(layout->panelRect.x + 16, layout->panelRect.y + 8, Palette::White);
		DrawCommandSection(*layout, gameData);
	}

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(config.hud.winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
