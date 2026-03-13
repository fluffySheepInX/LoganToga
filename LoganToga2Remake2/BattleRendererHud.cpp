#include "BattleRendererHudInternal.h"

namespace
{
	Array<String> BuildWrappedHudLines(const Array<String>& entries, const Font& font, const double maxWidth, const String& prefix)
	{
		Array<String> lines;
		String currentLine = prefix;

		for (size_t index = 0; index < entries.size(); ++index)
		{
			const String segment = ((currentLine == prefix) ? U"" : U" / ") + entries[index];
			const String candidate = currentLine + segment;
			if ((currentLine != prefix) && (font(candidate).region().w > maxWidth))
			{
				lines << currentLine;
				currentLine = prefix + entries[index];
				continue;
			}

			currentLine = candidate;
		}

		if (!currentLine.isEmpty())
		{
			lines << currentLine;
		}

		return lines;
	}

	ColorF GetFormationAccentColor(const FormationType formation)
	{
		switch (formation)
		{
		case FormationType::Line:
			return ColorF{ 0.24, 0.68, 0.96 };
		case FormationType::Column:
			return ColorF{ 0.44, 0.84, 0.54 };
		case FormationType::Square:
			return ColorF{ 0.96, 0.72, 0.30 };
		default:
			return ColorF{ 0.72, 0.76, 0.84 };
		}
	}

	void DrawFormationPanel(const BattleState& state, const GameData& gameData)
	{
		const FormationPanelLayout layout = BuildFormationPanelLayout(state);
		const Vec2 cursorScreenPos = Cursor::PosF();

		RoundRect{ layout.panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		RoundRect{ layout.panelRect, 12 }.drawFrame(2, 0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.smallFont(layout.title).draw(layout.panelRect.x + 16, layout.panelRect.y + 12, ColorF{ 0.82, 0.86, 0.92 });

		for (const auto& button : layout.buttons)
		{
			const bool isHovered = button.rect.intersects(cursorScreenPos);
			const bool isPressed = isHovered && MouseL.pressed();
			const ColorF accentColor = GetFormationAccentColor(button.button.formation);
			const RectF animatedRect = isPressed
				? RectF{ button.rect.x + 2, button.rect.y + 3, button.rect.w - 4, button.rect.h - 4 }
				: (isHovered ? RectF{ button.rect.x - 1, button.rect.y - 1, button.rect.w + 2, button.rect.h + 2 } : button.rect);
			const ColorF fillColor = button.button.isActive
				? ColorF{ accentColor.r * 0.30 + 0.12, accentColor.g * 0.30 + 0.12, accentColor.b * 0.30 + 0.12, 0.98 }
				: (isHovered ? ColorF{ 0.16, 0.18, 0.24, 0.98 } : ColorF{ 0.10, 0.11, 0.15, 0.96 });
			const double borderThickness = button.button.isActive ? 4.0 : (isHovered ? 3.0 : 2.0);

			RoundRect{ animatedRect, 10 }.draw(fillColor);
			RoundRect{ animatedRect, 10 }.drawFrame(borderThickness, 0, accentColor);
			RectF{ animatedRect.x + 10, animatedRect.bottomY() - 10, animatedRect.w - 20, 4 }
				.draw(button.button.isActive ? accentColor : ColorF{ accentColor.r, accentColor.g, accentColor.b, 0.45 });
			gameData.smallFont(button.button.label).drawAt(animatedRect.center().movedBy(0, -2), Palette::White);
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

	Array<String> productionEntries;
	for (const auto& slot : config.playerProductionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableProductionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = (slot.cost > 0)
			? slot.cost
			: (definition ? definition->cost : 0);
		productionEntries << s3d::Format(
			slot.slot,
			U": ",
			GetArchetypeLabel(slot.archetype),
			(slot.batchCount >= 2) ? U" x" + Format(slot.batchCount) : U"",
			U" (",
			cost,
			U"G)");
	}
	const Array<String> productionLines = BuildWrappedHudLines(productionEntries, gameData.smallFont, 440.0, U"Production: ");
	const double productionBaseY = 88.0;
	const double lineStep = 22.0;
	const double detailBaseY = productionBaseY + (lineStep * productionLines.size());
	const double hudHeight = Max(220.0, detailBaseY + 110.0 - 16.0);

	String constructionText = U"Build: none";
	for (const auto& slot : config.playerConstructionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		constructionText = s3d::Format(U"Build: ", slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" (", cost, U"G)");
		break;
	}
	const String runText = gameData.runState.isActive
		? s3d::Format(U"Run: battle ", gameData.runState.currentBattleIndex + 1, U"/", gameData.runState.totalBattles)
		: U"Run: inactive";
	const auto commandLayout = BuildCommandPanelLayout(state, config);
	const auto queueTarget = BattleRendererHudInternal::FindQueueDisplayTarget(state);

	RoundRect{ 16, 16, 480, hudHeight, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(config.hud.controls).draw(28, 66, Palette::White);
	for (size_t index = 0; index < productionLines.size(); ++index)
	{
		gameData.smallFont(productionLines[index]).draw(28, productionBaseY + (lineStep * index), Palette::White);
	}
	gameData.smallFont(constructionText).draw(28, detailBaseY, Palette::White);
	gameData.smallFont(s3d::Format(U"Resource: ", playerResourceCount, U" pts / +", playerResourceIncome, U" income")).draw(28, detailBaseY + 22.0, Palette::White);
	gameData.smallFont(config.hud.escapeHint).draw(28, detailBaseY + 44.0, Palette::White);
	gameData.smallFont(runText).draw(28, detailBaseY + 66.0, Palette::White);
	gameData.smallFont(s3d::Format(U"Gold: ", state.playerGold)).draw(28, detailBaseY + 88.0, Palette::Gold);
	DrawFormationPanel(state, gameData);

	if (queueTarget)
	{
		const RectF queuePanelRect = commandLayout
			? RectF{ Max(16.0, commandLayout->panelRect.x - 242.0 - 12.0), commandLayout->panelRect.y, 242.0, 212.0 }
			: RectF{ Scene::Width() - 242.0 - 16.0, Scene::Height() - 212.0 - 16.0, 242.0, 212.0 };
		BattleRendererHudInternal::DrawQueuePanel(queuePanelRect, *queueTarget, gameData);
	}

	if (commandLayout)
	{
		RoundRect{ commandLayout->panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		RoundRect{ commandLayout->panelRect, 12 }.drawFrame(2, 0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.uiFont(commandLayout->title).draw(commandLayout->panelRect.x + 16, commandLayout->panelRect.y + 8, Palette::White);
		BattleRendererHudInternal::DrawCommandSection(*commandLayout, gameData);
	}

	if (!state.statusMessage.isEmpty())
	{
		const RectF messageRect{ Arg::center(Scene::CenterF().movedBy(0, -220)), 420, 42 };
		RoundRect{ messageRect, 10 }.draw(ColorF{ 0.10, 0.05, 0.05, 0.90 });
		RoundRect{ messageRect, 10 }.drawFrame(2, 0, ColorF{ 0.96, 0.42, 0.36, 0.95 });
		gameData.smallFont(state.statusMessage).drawAt(messageRect.center(), Palette::White);
	}

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		String winHint = config.hud.winHint;
		if (gameData.runState.isActive)
		{
			if ((*state.winner == Owner::Player) && ((gameData.runState.currentBattleIndex + 1) < gameData.runState.totalBattles))
			{
				winHint = U"Enter: choose reward / R: new run";
			}
			else
			{
				winHint = U"Enter: title / R: new run";
			}
		}
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
