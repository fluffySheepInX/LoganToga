#include "BattleRendererHudInternal.h"

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
		if (!ContainsArchetype(config.playerAvailableProductionArchetypes, slot.archetype))
		{
			continue;
		}

		if (!productionText.isEmpty())
		{
			productionText += U" / ";
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = (slot.cost > 0)
			? slot.cost
			: (definition ? definition->cost : 0);
		productionText += s3d::Format(
			slot.slot,
			U": ",
			GetArchetypeLabel(slot.archetype),
			(slot.batchCount >= 2) ? U" x" + Format(slot.batchCount) : U"",
			U" @",
			GetArchetypeLabel(slot.producer),
			U" (",
			cost,
			U"G)");
	}

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

	RoundRect{ 16, 16, 480, 220, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(config.hud.controls).draw(28, 66, Palette::White);
	gameData.smallFont(productionText).draw(28, 88, Palette::White);
	gameData.smallFont(constructionText).draw(28, 110, Palette::White);
	const String formationText = U"Formation: Q="
		+ GetFormationLabel(FormationType::Line)
		+ U" / W="
		+ GetFormationLabel(FormationType::Column)
		+ U" / E="
		+ GetFormationLabel(FormationType::Square)
		+ U" / Current="
		+ GetFormationLabel(state.playerFormation);
	gameData.smallFont(formationText).draw(28, 132, Palette::White);
	gameData.smallFont(s3d::Format(U"Resource: ", playerResourceCount, U" pts / +", playerResourceIncome, U" income")).draw(28, 154, Palette::White);
	gameData.smallFont(config.hud.escapeHint).draw(28, 176, Palette::White);
	gameData.smallFont(runText).draw(28, 198, Palette::White);
	gameData.smallFont(s3d::Format(U"Gold: ", state.playerGold)).draw(28, 220, Palette::Gold);

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
