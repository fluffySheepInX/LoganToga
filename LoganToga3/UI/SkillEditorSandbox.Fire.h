#pragma once
# include "SkillEditorSandbox.Setup.h"

namespace LT3
{
	inline Vec2 GetSkillSandboxProjectileTargetPosition(const MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		if (projectile.targetIsAlly)
		{
			if (projectile.targetIndex > 0)
			{
				const int32 allyIndex = projectile.targetIndex - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].pos;
				}
			}
			return editor.skillSandboxAllyPos;
		}
		if (projectile.targetIndex < 0)
		{
			return editor.skillSandboxTargetPos;
		}
		if (projectile.targetIndex < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[projectile.targetIndex].pos;
		}
		return editor.skillSandboxTargetPos;
	}

	inline void SpawnSkillSandboxProjectile(MapEditorState& editor, const SkillDef& skill, int32 burstIndex, const SkillSandboxTargetRef& target)
	{
		const Vec2 toTarget = target.pos - editor.skillSandboxCasterPos;
		const Vec2 baseDir = (toTarget.lengthSq() > 1.0) ? toTarget.normalized() : Vec2{ 1.0, 0.0 };
		const double centeredIndex = static_cast<double>(burstIndex) - (Max(1, skill.burstCount) - 1) * 0.5;
		const double spreadRad = Math::ToRadians(skill.spreadDeg * centeredIndex / Max(1, skill.burstCount));
		const Vec2 dir = RotateVector(baseDir, spreadRad);
		const double distance = Max(1.0, toTarget.length());
		const double maxLife = ResolveProjectileMaxLife(skill, distance);
		const double startAngleRad = ResolveProjectileStartAngleRad(skill, dir);
		Vec2 spawnPos = ResolveProjectileSpawnPosition(skill, editor.skillSandboxCasterPos, dir);
		if (skill.projectileMotion == SkillProjectileMotion::Orbit || skill.projectileMotion == SkillProjectileMotion::Swing)
		{
			const double radius = (skill.projectileMotion == SkillProjectileMotion::Swing) ? skill.swingRadius : skill.orbitRadius;
			spawnPos = editor.skillSandboxCasterPos + Vec2{ Cos(startAngleRad), Sin(startAngleRad) } * radius;
		}
		else if (skill.projectileMotion == SkillProjectileMotion::Drop)
		{
			spawnPos = target.pos;
		}

		SkillSandboxProjectile projectile;
		projectile.position = spawnPos;
		projectile.velocity = (skill.projectileMotion == SkillProjectileMotion::Static) ? Vec2{ 0.0, 0.0 } : (dir * skill.projectileSpeed);
		projectile.startPosition = editor.skillSandboxCasterPos;
		projectile.endPosition = target.pos;
		projectile.lifeSec = maxLife;
		projectile.maxLifeSec = maxLife;
		projectile.angleRad = startAngleRad;
		projectile.baseAngleRad = startAngleRad;
		projectile.motion = skill.projectileMotion;
		projectile.targetIsAlly = target.isAlly;
		projectile.targetIndex = target.index;
		projectile.skillId = InvalidSkillDefId;
		if (skill.projectileMotion == SkillProjectileMotion::Drop)
		{
			projectile.height = (skill.projectileSpeed >= 0.0) ? Max(1.0, skill.arcHeight) : 0.0;
		}
		editor.skillSandboxProjectiles << projectile;
	}

	inline Array<SkillSandboxTargetRef> CollectSkillSandboxTargets(const MapEditorState& editor, const SkillDef& skill)
	{
		Array<SkillSandboxTargetRef> targets;
		auto appendTargetIfInBand = [&](const SkillSandboxTargetRef& target)
		{
			const double distance = editor.skillSandboxCasterPos.distanceFrom(target.pos);
			if (distance > skill.range)
			{
				return;
			}
			if (skill.rangeMin > 0.0 && distance < skill.rangeMin)
			{
				return;
			}
			targets << target;
		};

		if (skill.kind == SkillKind::Heal)
		{
			appendTargetIfInBand(SkillSandboxTargetRef{ editor.skillSandboxAllyPos, true, 0 });
			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
			{
				appendTargetIfInBand(SkillSandboxTargetRef{ editor.skillSandboxExtraAllies[i].pos, true, i + 1 });
			}
			return targets;
		}

		appendTargetIfInBand(SkillSandboxTargetRef{ editor.skillSandboxTargetPos, false, -1 });
		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
		{
			appendTargetIfInBand(SkillSandboxTargetRef{ editor.skillSandboxExtraTargets[i].pos, false, i });
		}
		return targets;
	}

	inline void SpawnSkillSandboxProjectilesForTargets(MapEditorState& editor, const SkillDef& skill, int32 burstIndex)
	{
		const Array<SkillSandboxTargetRef> targets = CollectSkillSandboxTargets(editor, skill);
		if (targets.isEmpty())
		{
			return;
		}
		if (!skill.allfunc)
		{
			SpawnSkillSandboxProjectile(editor, skill, burstIndex, targets.front());
			return;
		}

		for (const auto& target : targets)
		{
			SpawnSkillSandboxProjectile(editor, skill, burstIndex, target);
		}
	}

	inline bool CanFireSkillSandbox(const MapEditorState& editor, const SkillDef& skill)
	{
		const Array<SkillSandboxTargetRef> targets = CollectSkillSandboxTargets(editor, skill);
		if (targets.isEmpty())
		{
			return false;
		}
		if (skill.rangeMin <= 0.0)
		{
			return true;
		}
		return targets.any([&](const SkillSandboxTargetRef& target)
		{
			return editor.skillSandboxCasterPos.distanceFrom(target.pos) >= skill.rangeMin;
		});
	}

	inline void FireSkillSandbox(MapEditorState& editor, const SkillDef& skill)
	{
		if (!CanFireSkillSandbox(editor, skill))
		{
			return;
		}

		if (editor.skillSandboxCasterHp <= 0)
		{
			editor.skillSandboxCasterHp = editor.skillSandboxCasterMaxHp;
		}
		if (editor.skillSandboxTargetHp <= 0)
		{
			editor.skillSandboxTargetHp = editor.skillSandboxTargetMaxHp;
		}
		if (editor.skillSandboxAllyHp <= 0)
		{
			editor.skillSandboxAllyHp = Max(1, editor.skillSandboxAllyMaxHp / 2);
		}
		for (auto& ally : editor.skillSandboxExtraAllies)
		{
			if (ally.hp <= 0)
			{
				ally.hp = Max(1, ally.maxHp / 2);
			}
		}
		for (auto& dummy : editor.skillSandboxExtraTargets)
		{
			if (dummy.hp <= 0)
			{
				dummy.hp = dummy.maxHp;
			}
		}
		const int32 burstCount = Max(1, skill.burstCount);
		editor.skillSandboxBurstOrder.clear();
		editor.skillSandboxBurstOrder.reserve(burstCount);
		for (int32 i = 0; i < burstCount; ++i)
		{
			editor.skillSandboxBurstOrder << i;
		}
		if (skill.burstOrderMode == SkillBurstOrderMode::Random)
		{
			editor.skillSandboxBurstOrder.shuffle();
		}
		if (skill.burstFireMode == SkillBurstFireMode::Simultaneous)
		{
			for (int32 shotIndex = 0; shotIndex < burstCount; ++shotIndex)
			{
				SpawnSkillSandboxProjectilesForTargets(editor, skill, editor.skillSandboxBurstOrder[shotIndex]);
			}
			editor.skillSandboxBurstShotsLeft = 0;
			editor.skillSandboxBurstShotTimerSec = 0.0;
		}
		else
		{
			SpawnSkillSandboxProjectilesForTargets(editor, skill, editor.skillSandboxBurstOrder[0]);
			editor.skillSandboxBurstShotsLeft = burstCount - 1;
			editor.skillSandboxBurstShotTimerSec = Max(0.0, skill.burstIntervalSec);
		}
		editor.skillSandboxCooldownLeftSec = Max(0.05, skill.cooldownSec);
	}

	// Unit Mode 用: skillId を弾に刻印して発射する
	inline void SpawnSkillSandboxProjectileTagged(MapEditorState& editor, const SkillDef& skill, SkillDefId skillId, int32 burstIndex, const SkillSandboxTargetRef& target)
	{
		SpawnSkillSandboxProjectile(editor, skill, burstIndex, target);
		if (!editor.skillSandboxProjectiles.isEmpty())
		{
			editor.skillSandboxProjectiles.back().skillId = skillId;
		}
	}

	inline void SpawnSkillSandboxProjectilesForTargetsTagged(MapEditorState& editor, const SkillDef& skill, SkillDefId skillId, int32 burstIndex)
	{
		const Array<SkillSandboxTargetRef> targets = CollectSkillSandboxTargets(editor, skill);
		if (targets.isEmpty())
		{
			return;
		}
		if (!skill.allfunc)
		{
			SpawnSkillSandboxProjectileTagged(editor, skill, skillId, burstIndex, targets.front());
			return;
		}
		for (const auto& target : targets)
		{
			SpawnSkillSandboxProjectileTagged(editor, skill, skillId, burstIndex, target);
		}
	}

	// サンドボックス内でのスキル選択ルール (BattleWorld不要版)
	// 実行可否→ID昇順で最初にターゲットを捉えるスキルを返す
	inline SkillDefId ResolveSandboxExecutableSkillId(const MapEditorState& editor, const DefinitionStores& defs, const UnitCatalog& catalog, int32 unitCatalogIndex)
	{
		if (unitCatalogIndex < 0 || unitCatalogIndex >= static_cast<int32>(catalog.entries.size()))
		{
			return InvalidSkillDefId;
		}
		const UnitCatalogEntry& entry = catalog.entries[unitCatalogIndex];
		Array<SkillDefId> skillIds;
		for (const String& tag : entry.skills)
		{
			const auto it = defs.skillByTag.find(tag);
			if (it != defs.skillByTag.end())
			{
				skillIds << it->second;
			}
		}
		skillIds.sort();
		for (const SkillDefId skillId : skillIds)
		{
			if (skillId < 0 || skillId >= static_cast<SkillDefId>(defs.skills.size()))
			{
				continue;
			}
			if (CanFireSkillSandbox(editor, defs.skills[skillId]))
			{
				return skillId;
			}
		}
		return InvalidSkillDefId;
	}

	inline void FireSkillSandboxUnitMode(MapEditorState& editor, const DefinitionStores& defs, const UnitCatalog& catalog)
	{
		const SkillDefId skillId = ResolveSandboxExecutableSkillId(editor, defs, catalog, editor.skillSandboxUnitCatalogIndex);
		if (skillId == InvalidSkillDefId || skillId >= static_cast<SkillDefId>(defs.skills.size()))
		{
			editor.skillSandboxActiveSkillId = InvalidSkillDefId;
			return;
		}
		const SkillDef& skill = defs.skills[skillId];
		editor.skillSandboxActiveSkillId = skillId;

		if (editor.skillSandboxCasterHp <= 0) editor.skillSandboxCasterHp = editor.skillSandboxCasterMaxHp;
		if (editor.skillSandboxTargetHp <= 0) editor.skillSandboxTargetHp = editor.skillSandboxTargetMaxHp;
		if (editor.skillSandboxAllyHp <= 0) editor.skillSandboxAllyHp = Max(1, editor.skillSandboxAllyMaxHp / 2);
		for (auto& ally : editor.skillSandboxExtraAllies)
		{
			if (ally.hp <= 0) ally.hp = Max(1, ally.maxHp / 2);
		}
		for (auto& dummy : editor.skillSandboxExtraTargets)
		{
			if (dummy.hp <= 0) dummy.hp = dummy.maxHp;
		}

		const int32 burstCount = Max(1, skill.burstCount);
		editor.skillSandboxBurstOrder.clear();
		editor.skillSandboxBurstOrder.reserve(burstCount);
		for (int32 i = 0; i < burstCount; ++i) editor.skillSandboxBurstOrder << i;
		if (skill.burstOrderMode == SkillBurstOrderMode::Random) editor.skillSandboxBurstOrder.shuffle();

		if (skill.burstFireMode == SkillBurstFireMode::Simultaneous)
		{
			for (int32 shotIndex = 0; shotIndex < burstCount; ++shotIndex)
			{
				SpawnSkillSandboxProjectilesForTargetsTagged(editor, skill, skillId, editor.skillSandboxBurstOrder[shotIndex]);
			}
			editor.skillSandboxBurstShotsLeft = 0;
			editor.skillSandboxBurstShotTimerSec = 0.0;
		}
		else
		{
			SpawnSkillSandboxProjectilesForTargetsTagged(editor, skill, skillId, editor.skillSandboxBurstOrder[0]);
			editor.skillSandboxBurstShotsLeft = burstCount - 1;
			editor.skillSandboxBurstShotTimerSec = Max(0.0, skill.burstIntervalSec);
		}
		editor.skillSandboxCooldownLeftSec = Max(0.05, skill.cooldownSec);
	}
}
