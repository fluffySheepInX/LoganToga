#include "BattleRenderer.h"

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

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(config.hud.winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
