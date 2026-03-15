#include "BalanceEditScene.h"

bool BalanceEditScene::saveEditorOverrides() const
{
	FileSystem::CreateDirectories(U"config");
	return saveCoreEditorOverride() && saveUnitsEditorOverride() && saveCardsEditorOverride();
}

bool BalanceEditScene::clearEditorOverrides() const
{
	bool success = true;
	if (FileSystem::Exists(getCoreEditorOverridePath()))
	{
		success = success && FileSystem::Remove(getCoreEditorOverridePath());
	}
	if (FileSystem::Exists(getUnitsEditorOverridePath()))
	{
		success = success && FileSystem::Remove(getUnitsEditorOverridePath());
	}
	if (FileSystem::Exists(getCardsEditorOverridePath()))
	{
		success = success && FileSystem::Remove(getCardsEditorOverridePath());
	}
	return success;
}

bool BalanceEditScene::clearAllOverrides() const
{
	bool success = true;
	success = removeFileIfExists(getCoreOverridePath()) && success;
	success = removeFileIfExists(getUnitsOverridePath()) && success;
	success = removeFileIfExists(getCardsOverridePath()) && success;
	success = removeFileIfExists(getCoreEditorOverridePath()) && success;
	success = removeFileIfExists(getUnitsEditorOverridePath()) && success;
	success = removeFileIfExists(getCardsEditorOverridePath()) && success;
	return success;
}

bool BalanceEditScene::writeTextFile(const String& path, const String& content)
{
	TextWriter writer{ path };
	if (!writer)
	{
		return false;
	}

	writer.write(content);
	return true;
}

bool BalanceEditScene::removeFileIfExists(const String& path)
{
	return !FileSystem::Exists(path) || FileSystem::Remove(path);
}

bool BalanceEditScene::saveCoreEditorOverride() const
{
	String content;
	content += U"[economy]\n";
	appendTomlLine(content, U"player_gold", Format(m_editConfig.playerGold));
	appendTomlLine(content, U"enemy_gold", Format(m_editConfig.enemyGold));
	content += U"\n[income]\n";
	appendTomlLine(content, U"interval", Format(m_editConfig.income.interval));
	appendTomlLine(content, U"player_amount", Format(m_editConfig.income.playerAmount));
	appendTomlLine(content, U"enemy_amount", Format(m_editConfig.income.enemyAmount));
	content += U"\n[enemy_spawn]\n";
	appendTomlLine(content, U"interval", Format(m_editConfig.enemySpawn.interval));
	appendTomlLine(content, U"advanced_probability", Format(m_editConfig.enemySpawn.advancedProbability));
	appendTomlLine(content, U"random_y_offset", Format(m_editConfig.enemySpawn.randomYOffset));
	content += U"\n[enemy_ai]\n";
	appendTomlLine(content, U"assault_unit_threshold", Format(m_editConfig.enemyAI.assaultUnitThreshold));
	appendTomlLine(content, U"staging_assault_min_units", Format(m_editConfig.enemyAI.stagingAssaultMinUnits));
	return writeTextFile(getCoreEditorOverridePath(), content);
}

bool BalanceEditScene::saveUnitsEditorOverride() const
{
	String content;
	for (const auto& unit : m_editConfig.unitDefinitions)
	{
		content += U"[[units]]\n";
		appendTomlLine(content, U"archetype", quoteTomlString(toUnitArchetypeTomlString(unit.archetype)));
		appendTomlLine(content, U"hp", Format(unit.hp));
		appendTomlLine(content, U"attack_power", Format(unit.attackPower));
		appendTomlLine(content, U"cost", Format(unit.cost));
		appendTomlLine(content, U"production_time", Format(unit.productionTime));
		appendTomlLine(content, U"move_speed", Format(unit.moveSpeed));
		appendTomlLine(content, U"attack_range", Format(unit.attackRange));
		appendTomlLine(content, U"attack_cooldown", Format(unit.attackCooldown));
		appendTomlLine(content, U"aggro_range", Format(unit.aggroRange));
		content += U"\n";
	}

	for (const auto& slot : m_editConfig.playerProductionSlots)
	{
		content += U"[[player_production]]\n";
		appendTomlLine(content, U"slot", Format(slot.slot));
		appendTomlLine(content, U"producer", quoteTomlString(toUnitArchetypeTomlString(slot.producer)));
		appendTomlLine(content, U"archetype", quoteTomlString(toUnitArchetypeTomlString(slot.archetype)));
		appendTomlLine(content, U"cost", Format(slot.cost));
		appendTomlLine(content, U"batch_count", Format(slot.batchCount));
		content += U"\n";
	}

	return writeTextFile(getUnitsEditorOverridePath(), content);
}

bool BalanceEditScene::saveCardsEditorOverride() const
{
	String content;
	for (const auto& card : m_editCards)
	{
		content += U"[[cards]]\n";
		appendTomlLine(content, U"id", quoteTomlString(card.id));
		appendTomlLine(content, U"value", Format(card.value));
		appendTomlLine(content, U"rarity", quoteTomlString(toRewardCardRarityTomlString(card.rarity)));
		appendTomlLine(content, U"repeatable", card.repeatable ? U"true" : U"false");
		content += U"\n";
	}

	return writeTextFile(getCardsEditorOverridePath(), content);
}

String BalanceEditScene::getCoreEditorOverridePath()
{
	return ResolveTomlEditorOverridePath(U"config/battle_core.toml");
}

String BalanceEditScene::getCoreOverridePath()
{
	return ResolveTomlOverridePath(U"config/battle_core.toml");
}

String BalanceEditScene::getUnitsEditorOverridePath()
{
	return ResolveTomlEditorOverridePath(U"config/battle_units.toml");
}

String BalanceEditScene::getUnitsOverridePath()
{
	return ResolveTomlOverridePath(U"config/battle_units.toml");
}

String BalanceEditScene::getCardsEditorOverridePath()
{
	return ResolveTomlEditorOverridePath(U"config/cards.toml");
}

String BalanceEditScene::getCardsOverridePath()
{
	return ResolveTomlOverridePath(U"config/cards.toml");
}
