#pragma once
# include "SkillEditorSandbox.Fire.h"

namespace LT3
{
	struct SkillSandboxHitTargetInfo
	{
		SkillSandboxTargetRef target;
		Vec2 position{ 0.0, 0.0 };
	};

	/// <summary>
	/// サンドボックス対象の現在 HP を参照します。
	/// </summary>
	inline int32 ResolveSkillSandboxTargetHpValue(const MapEditorState& editor, const SkillSandboxTargetRef& target)
	{
		if (target.isAlly)
		{
			if (target.index > 0)
			{
				const int32 allyIndex = target.index - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].hp;
				}
			}
			return editor.skillSandboxAllyHp;
		}
		if (target.index >= 0 && target.index < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[target.index].hp;
		}
		return editor.skillSandboxTargetHp;
	}

	inline Vec2 ResolveSkillSandboxTargetRefPosition(const MapEditorState& editor, const SkillSandboxTargetRef& target)
	{
		if (target.isAlly)
		{
			if (target.index > 0)
			{
				const int32 allyIndex = target.index - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].pos;
				}
			}
			return editor.skillSandboxAllyPos;
		}
		if (target.index >= 0 && target.index < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[target.index].pos;
		}
		return editor.skillSandboxTargetPos;
	}

	inline bool IsSkillSandboxTargetRefAlive(const MapEditorState& editor, const SkillSandboxTargetRef& target)
	{
		return ResolveSkillSandboxTargetHpValue(editor, target) > 0;
	}

	/// <summary>
	/// swing multi_hit_once 用に対象を一意キーへ変換します。
	/// </summary>
	inline int32 MakeSkillSandboxSwingHitTargetKey(const SkillSandboxTargetRef& target)
	{
		return target.isAlly ? -(target.index + 2) : (target.index + 1);
	}

	inline SkillSandboxTargetRef MakeSkillSandboxTargetRef(const MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		return SkillSandboxTargetRef{ GetSkillSandboxProjectileTargetPosition(editor, projectile), projectile.targetIsAlly, projectile.targetIndex };
	}

	/// <summary>
	/// swing projectile が対象へ再ヒット可能かを判定します。
	/// </summary>
	inline bool CanSkillSandboxSwingHitTarget(const SkillSandboxProjectile& projectile, const SkillSandboxTargetRef& target, const SkillDef& skill)
	{
		if (skill.projectileMotion != SkillProjectileMotion::Swing)
		{
			return true;
		}

		if (skill.swingHitMode == SkillSwingHitMode::MultiHitOnce)
		{
			return !projectile.swingHitTargetIndices.contains(MakeSkillSandboxSwingHitTargetKey(target));
		}

		return true;
	}

	/// <summary>
	/// swing multi_hit_once で今回接触している対象一覧を収集します。
	/// </summary>
	inline Array<SkillSandboxTargetRef> CollectSkillSandboxSwingHitTargets(const MapEditorState& editor, const SkillSandboxProjectile& projectile, const SkillDef& skill)
	{
		Array<SkillSandboxTargetRef> targets;
		if (skill.projectileMotion != SkillProjectileMotion::Swing)
		{
			return targets;
		}

		auto tryAppendTarget = [&](const SkillSandboxTargetRef& target)
		{
			if (!CanSkillSandboxSwingHitTarget(projectile, target, skill))
			{
				return;
			}
			if (!target.isAlly && !IsSkillSandboxTargetRefAlive(editor, target))
			{
				return;
			}

			const Vec2 targetPos = ResolveSkillSandboxTargetRefPosition(editor, target);
			const double hitRadius = IsSkillSandboxTargetRefAlive(editor, target) ? 34.0 : 6.0;
			if (IsSwingEndProjectileHit(skill, projectile.position, projectile.angleRad, targetPos, hitRadius)
				|| (projectile.position.distanceFrom(targetPos) <= hitRadius))
			{
				targets << SkillSandboxTargetRef{ targetPos, target.isAlly, target.index };
			}
		};

		if (DoesSkillTargetAllies(skill))
		{
			tryAppendTarget(SkillSandboxTargetRef{ editor.skillSandboxAllyPos, true, 0 });
			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
			{
				tryAppendTarget(SkillSandboxTargetRef{ editor.skillSandboxExtraAllies[i].pos, true, i + 1 });
			}
		}
		else
		{
			tryAppendTarget(SkillSandboxTargetRef{ editor.skillSandboxTargetPos, false, -1 });
			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
			{
				tryAppendTarget(SkillSandboxTargetRef{ editor.skillSandboxExtraTargets[i].pos, false, i });
			}
		}

		return targets;
	}

	/// <summary>
	/// next 連鎖のため、着弾点から見て条件に合う対象を選びます。
	/// </summary>
	inline Optional<SkillSandboxTargetRef> ResolveSkillSandboxNextTarget(const MapEditorState& editor, const SkillDef& skill, const Optional<SkillSandboxTargetRef>& preferredTarget, const Vec2& pivotPos)
	{
		Array<SkillSandboxTargetRef> targets;
		auto appendTarget = [&](const SkillSandboxTargetRef& target)
		{
			targets << target;
		};

		if (skill.kind == SkillKind::Heal)
		{
			appendTarget(SkillSandboxTargetRef{ editor.skillSandboxAllyPos, true, 0 });
			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
			{
				appendTarget(SkillSandboxTargetRef{ editor.skillSandboxExtraAllies[i].pos, true, i + 1 });
			}
		}
		else
		{
			appendTarget(SkillSandboxTargetRef{ editor.skillSandboxTargetPos, false, -1 });
			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
			{
				appendTarget(SkillSandboxTargetRef{ editor.skillSandboxExtraTargets[i].pos, false, i });
			}
		}

		if (targets.isEmpty())
		{
			return none;
		}

		const bool targetAllies = DoesSkillTargetAllies(skill);
		if (preferredTarget && preferredTarget->isAlly == targetAllies && IsSkillSandboxTargetRefAlive(editor, *preferredTarget))
		{
			return preferredTarget;
		}

		Optional<SkillSandboxTargetRef> bestTarget;
		double bestDistanceSq = Math::Inf;
		for (const auto& target : targets)
		{
			if (target.isAlly != targetAllies)
			{
				continue;
			}
			if (!targetAllies && ResolveSkillSandboxTargetHpValue(editor, target) <= 0)
			{
				continue;
			}

			const double distanceSq = pivotPos.distanceFromSq(target.pos);
			if (!bestTarget || distanceSq < bestDistanceSq)
			{
				bestTarget = target;
				bestDistanceSq = distanceSq;
			}
		}
		return bestTarget;
	}

	/// <summary>
	/// 命中時または寿命切れ時に next 連鎖を発生させます。
	/// </summary>
	inline void SpawnSkillSandboxNextFromProjectile(MapEditorState& editor, const DefinitionStores& defs, const SkillSandboxProjectile& projectile, const Vec2& pivotPos, bool fromLast, const Array<SkillSandboxHitTargetInfo>& hitTargets = {})
	{
		const SkillDefId sourceSkillId = projectile.skillId;
		if (sourceSkillId == InvalidSkillDefId || sourceSkillId >= static_cast<SkillDefId>(defs.skills.size()))
		{
			return;
		}

		const SkillDef& sourceSkill = defs.skills[sourceSkillId];
		if (sourceSkill.nextSkill == InvalidSkillDefId || sourceSkill.nextSkill >= static_cast<SkillDefId>(defs.skills.size()))
		{
			return;
		}
		if (fromLast && !sourceSkill.nextLast)
		{
			return;
		}
		if (projectile.chainDepth >= MaxSkillNextChainDepth)
		{
			return;
		}

		const SkillDef& nextSkill = defs.skills[sourceSkill.nextSkill];
		Array<SkillSandboxHitTargetInfo> spawnSources = hitTargets;
		if (spawnSources.isEmpty())
		{
			spawnSources << SkillSandboxHitTargetInfo{ MakeSkillSandboxTargetRef(editor, projectile), pivotPos };
		}

		for (const auto& source : spawnSources)
		{
			const Optional<SkillSandboxTargetRef> nextTarget = ResolveSkillSandboxNextTarget(editor, nextSkill, source.target, source.position);
			if (!nextTarget)
			{
				continue;
			}

			SkillNextSpawnContext nextContext;
			nextContext.chainDepth = projectile.chainDepth + 1;
			if (sourceSkill.sendTarget)
			{
				nextContext.targetPosition = IsSkillSandboxTargetRefAlive(editor, *nextTarget)
					? ResolveSkillSandboxTargetRefPosition(editor, *nextTarget)
					: projectile.endPosition;
			}
			else
			{
				nextContext.targetPosition = projectile.firedTargetPosition;
			}
			if (sourceSkill.sendImageDegree)
			{
				nextContext.imageAngleRad = projectile.angleRad;
			}

			SpawnSkillSandboxProjectilesForTargetsTagged(editor, nextSkill, sourceSkill.nextSkill, 0, *nextTarget, nextContext, source.position);
		}
	}

	inline Array<SkillSandboxHitTargetInfo> ApplySkillSandboxBomHit(MapEditorState& editor, const SkillDef& skill, const Vec2& impactPos)
	{
		Array<SkillSandboxHitTargetInfo> hitTargets;
		editor.skillSandboxLastBomCenter = impactPos;
		editor.skillSandboxLastBomRadius = skill.bomRadius;
		editor.skillSandboxLastBomDisplaySec = (skill.bom && skill.bomRadius > 0.0) ? Max(0.05, skill.bomVisualDurationSec) : 0.0;
		editor.skillSandboxLastBomVisualDurationSec = Max(0.05, skill.bomVisualDurationSec);
		editor.skillSandboxLastBomVisualScale = Max(0.1, skill.bomVisualScale);
		editor.skillSandboxLastBomVisual = skill.bomVisual;
		editor.skillSandboxLastBomKind = skill.kind;
		editor.skillSandboxLastBomFriendlyFire = skill.bomFriendlyFire;
		editor.skillSandboxLastBomImage = skill.bomImage;

		auto applyAmount = [&](int32& hp, int32 maxHp)
		{
			if (maxHp <= 0)
			{
				return;
			}

			int32 sandboxAmount = static_cast<int32>(Math::Round(skill.damage));
			if (sandboxAmount < 1)
			{
				sandboxAmount = 1;
			}
			if (skill.kind == SkillKind::Heal)
			{
				hp = Min(maxHp, hp + sandboxAmount);
				return;
			}
			if (hp <= 0)
			{
				return;
			}
			hp = Max(0, hp - sandboxAmount);
		};

		if (skill.kind == SkillKind::Heal)
		{
			if (editor.skillSandboxAllyPos.distanceFrom(impactPos) <= skill.bomRadius)
			{
				hitTargets << SkillSandboxHitTargetInfo{ SkillSandboxTargetRef{ editor.skillSandboxAllyPos, true, 0 }, editor.skillSandboxAllyPos };
				applyAmount(editor.skillSandboxAllyHp, editor.skillSandboxAllyMaxHp);
			}

			for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
			{
				auto& ally = editor.skillSandboxExtraAllies[i];
				if (ally.pos.distanceFrom(impactPos) <= skill.bomRadius)
				{
					hitTargets << SkillSandboxHitTargetInfo{ SkillSandboxTargetRef{ ally.pos, true, i + 1 }, ally.pos };
					applyAmount(ally.hp, ally.maxHp);
				}
			}
		}
		else if (editor.skillSandboxTargetPos.distanceFrom(impactPos) <= skill.bomRadius)
		{
			hitTargets << SkillSandboxHitTargetInfo{ SkillSandboxTargetRef{ editor.skillSandboxTargetPos, false, -1 }, editor.skillSandboxTargetPos };
			applyAmount(editor.skillSandboxTargetHp, editor.skillSandboxTargetMaxHp);
		}

		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
		{
			auto& dummy = editor.skillSandboxExtraTargets[i];
			if (skill.kind == SkillKind::Heal)
			{
				break;
			}
			if (dummy.pos.distanceFrom(impactPos) <= skill.bomRadius)
			{
				hitTargets << SkillSandboxHitTargetInfo{ SkillSandboxTargetRef{ dummy.pos, false, i }, dummy.pos };
				applyAmount(dummy.hp, dummy.maxHp);
			}
		}

		const int32 selfDamage = Max(0, static_cast<int32>(Math::Round(skill.damage * skill.bomSelfDamageScale + skill.selfDamageOnHit)));
		editor.skillSandboxCasterHp = Max(0, editor.skillSandboxCasterHp - selfDamage);
		return hitTargets;
	}

	inline int32& ResolveSkillSandboxTargetHpRef(MapEditorState& editor, const SkillSandboxTargetRef& target)
	{
		if (target.isAlly)
		{
			if (target.index > 0)
			{
				const int32 allyIndex = target.index - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].hp;
				}
			}
			return editor.skillSandboxAllyHp;
		}
		if (target.index >= 0 && target.index < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[target.index].hp;
		}
		return editor.skillSandboxTargetHp;
	}

	inline int32& ResolveSkillSandboxTargetHpRef(MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		return ResolveSkillSandboxTargetHpRef(editor, MakeSkillSandboxTargetRef(editor, projectile));
	}

	inline int32 ResolveSkillSandboxTargetMaxHp(const MapEditorState& editor, const SkillSandboxTargetRef& target)
	{
		if (target.isAlly)
		{
			if (target.index > 0)
			{
				const int32 allyIndex = target.index - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].maxHp;
				}
			}
			return editor.skillSandboxAllyMaxHp;
		}
		if (target.index >= 0 && target.index < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[target.index].maxHp;
		}
		return editor.skillSandboxTargetMaxHp;
	}

	inline int32 ResolveSkillSandboxTargetMaxHp(const MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		return ResolveSkillSandboxTargetMaxHp(editor, MakeSkillSandboxTargetRef(editor, projectile));
	}

	/// <summary>
	/// 指定対象へ sandbox 上のダメージ/回復を適用します。
	/// </summary>
	inline Array<SkillSandboxHitTargetInfo> ApplySkillSandboxHitToTarget(MapEditorState& editor, const SkillDef& skill, const SkillSandboxTargetRef& target, const Vec2& impactPos)
	{
		Array<SkillSandboxHitTargetInfo> hitTargets;
		int32& targetHp = ResolveSkillSandboxTargetHpRef(editor, target);
		const int32 targetMaxHp = ResolveSkillSandboxTargetMaxHp(editor, target);
		hitTargets << SkillSandboxHitTargetInfo{ SkillSandboxTargetRef{ ResolveSkillSandboxTargetRefPosition(editor, target), target.isAlly, target.index }, impactPos };
		int32 sandboxAmount = static_cast<int32>(Math::Round(skill.damage));
		if (sandboxAmount < 1)
		{
			sandboxAmount = 1;
		}
		if (skill.kind == SkillKind::Heal)
		{
			targetHp = Min(targetMaxHp, targetHp + sandboxAmount);
		}
		else
		{
			targetHp -= sandboxAmount;
			if (targetHp < 0)
			{
				targetHp = 0;
			}
		}

		const int32 selfDamage = Max(0, static_cast<int32>(Math::Round(skill.selfDamageOnHit)));
		editor.skillSandboxCasterHp -= selfDamage;
		if (editor.skillSandboxCasterHp < 0)
		{
			editor.skillSandboxCasterHp = 0;
		}
		return hitTargets;
	}

	inline Array<SkillSandboxHitTargetInfo> ApplySkillSandboxHit(MapEditorState& editor, const SkillDef& skill, const SkillSandboxProjectile& projectile, const Vec2& impactPos)
	{
		if (skill.bom && skill.bomRadius > 0.0)
		{
			return ApplySkillSandboxBomHit(editor, skill, impactPos);
		}

		return ApplySkillSandboxHitToTarget(editor, skill, MakeSkillSandboxTargetRef(editor, projectile), impactPos);
	}

	inline bool IsSkillSandboxTargetAlive(int32 hp)
	{
		return hp > 0;
	}
}
