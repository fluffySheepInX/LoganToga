# include "UnitEditorScene.h"

Array<String> UnitEditorScene::BuildChangeLines(const ff::UnitDefinition& loadedDefinition, const ff::UnitDefinition& editingDefinition) const
{
	Array<String> lines;

	if (loadedDefinition.label != editingDefinition.label)
	{
		lines << U"表示名: {} -> {}"_fmt(loadedDefinition.label, editingDefinition.label);
	}

	if (loadedDefinition.roleDescription != editingDefinition.roleDescription)
	{
		lines << U"説明: {} -> {}"_fmt(loadedDefinition.roleDescription, editingDefinition.roleDescription);
	}

	if (loadedDefinition.color != editingDefinition.color)
	{
		lines << U"色: {} -> {}"_fmt(FormatColor(loadedDefinition.color), FormatColor(editingDefinition.color));
	}

	if (loadedDefinition.summonCost != editingDefinition.summonCost)
	{
		lines << U"召喚コスト: {} -> {}"_fmt(loadedDefinition.summonCost, editingDefinition.summonCost);
	}

	if (loadedDefinition.maxHp != editingDefinition.maxHp)
	{
		lines << U"最大HP: {:.1f} -> {:.1f}"_fmt(loadedDefinition.maxHp, editingDefinition.maxHp);
	}

	if (loadedDefinition.attackRange != editingDefinition.attackRange)
	{
		lines << U"射程: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackRange, editingDefinition.attackRange);
	}

	if (loadedDefinition.attackInterval != editingDefinition.attackInterval)
	{
		lines << U"攻撃間隔: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackInterval, editingDefinition.attackInterval);
	}

	if (loadedDefinition.attackDamage != editingDefinition.attackDamage)
	{
		lines << U"攻撃力: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackDamage, editingDefinition.attackDamage);
	}

	return lines;
}

Array<String> UnitEditorScene::BuildChangeLines(const ff::EnemyDefinition& loadedDefinition, const ff::EnemyDefinition& editingDefinition) const
{
	Array<String> lines;

	if (loadedDefinition.label != editingDefinition.label)
	{
		lines << U"表示名: {} -> {}"_fmt(loadedDefinition.label, editingDefinition.label);
	}

	if (loadedDefinition.roleDescription != editingDefinition.roleDescription)
	{
		lines << U"説明: {} -> {}"_fmt(loadedDefinition.roleDescription, editingDefinition.roleDescription);
	}

	if (loadedDefinition.color != editingDefinition.color)
	{
		lines << U"色: {} -> {}"_fmt(FormatColor(loadedDefinition.color), FormatColor(editingDefinition.color));
	}

	if (loadedDefinition.maxHp != editingDefinition.maxHp)
	{
		lines << U"最大HP: {:.1f} -> {:.1f}"_fmt(loadedDefinition.maxHp, editingDefinition.maxHp);
	}

	if (loadedDefinition.speed != editingDefinition.speed)
	{
		lines << U"移動速度: {:.2f} -> {:.2f}"_fmt(loadedDefinition.speed, editingDefinition.speed);
	}

	if (loadedDefinition.attackRange != editingDefinition.attackRange)
	{
		lines << U"攻撃距離: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackRange, editingDefinition.attackRange);
	}

	if (loadedDefinition.attackInterval != editingDefinition.attackInterval)
	{
		lines << U"攻撃間隔: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackInterval, editingDefinition.attackInterval);
	}

	if (loadedDefinition.attackDamage != editingDefinition.attackDamage)
	{
		lines << U"攻撃力: {:.2f} -> {:.2f}"_fmt(loadedDefinition.attackDamage, editingDefinition.attackDamage);
	}

	if (loadedDefinition.rewardMultiplier != editingDefinition.rewardMultiplier)
	{
		lines << U"報酬倍率: {} -> {}"_fmt(loadedDefinition.rewardMultiplier, editingDefinition.rewardMultiplier);
	}

	return lines;
}

String UnitEditorScene::FormatColor(const ColorF& color)
{
	return U"({}, {}, {}, {})"_fmt(
		Round(color.r * 255.0),
		Round(color.g * 255.0),
		Round(color.b * 255.0),
		Round(color.a * 255.0));
}

String UnitEditorScene::FormatDefinitionSummary(const ff::UnitDefinition& definition)
{
	return U"C{} HP{:.1f} R{:.2f} I{:.2f} D{:.2f}"_fmt(
		definition.summonCost,
		definition.maxHp,
		definition.attackRange,
		definition.attackInterval,
		definition.attackDamage);
}

String UnitEditorScene::FormatDefinitionSummary(const ff::EnemyDefinition& definition)
{
	return U"HP{:.1f} SPD{:.2f} R{:.2f} I{:.2f} D{:.2f}"_fmt(
		definition.maxHp,
		definition.speed,
		definition.attackRange,
		definition.attackInterval,
		definition.attackDamage);
}

bool UnitEditorScene::AreSameDefinition(const ff::UnitDefinition& lhs, const ff::UnitDefinition& rhs)
{
	return (lhs.id == rhs.id)
		&& (lhs.stableId == rhs.stableId)
		&& (lhs.label == rhs.label)
		&& (lhs.roleDescription == rhs.roleDescription)
		&& (lhs.color == rhs.color)
		&& (lhs.summonCost == rhs.summonCost)
		&& (lhs.maxHp == rhs.maxHp)
		&& (lhs.attackRange == rhs.attackRange)
		&& (lhs.attackInterval == rhs.attackInterval)
		&& (lhs.attackDamage == rhs.attackDamage);
}

bool UnitEditorScene::AreSameDefinition(const ff::EnemyDefinition& lhs, const ff::EnemyDefinition& rhs)
{
	return (lhs.kind == rhs.kind)
		&& (lhs.stableId == rhs.stableId)
		&& (lhs.label == rhs.label)
		&& (lhs.roleDescription == rhs.roleDescription)
		&& (lhs.color == rhs.color)
		&& (lhs.maxHp == rhs.maxHp)
		&& (lhs.speed == rhs.speed)
		&& (lhs.attackRange == rhs.attackRange)
		&& (lhs.attackInterval == rhs.attackInterval)
		&& (lhs.attackDamage == rhs.attackDamage)
		&& (lhs.rewardMultiplier == rhs.rewardMultiplier);
}
