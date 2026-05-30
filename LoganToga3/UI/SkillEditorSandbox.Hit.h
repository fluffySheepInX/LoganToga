#pragma once
# include "SkillEditorSandbox.Fire.h"

namespace LT3
{
	inline void ApplySkillSandboxBomHit(MapEditorState& editor, const SkillDef& skill, const Vec2& impactPos)
	{
		editor.skillSandboxLastBomCenter = impactPos;
		editor.skillSandboxLastBomRadius = skill.bomRadius;
		editor.skillSandboxLastBomDisplaySec = (skill.bom && skill.bomRadius > 0.0) ? 0.22 : 0.0;

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
				applyAmount(editor.skillSandboxAllyHp, editor.skillSandboxAllyMaxHp);
			}
			for (auto& ally : editor.skillSandboxExtraAllies)
			{
				if (ally.pos.distanceFrom(impactPos) <= skill.bomRadius)
				{
					applyAmount(ally.hp, ally.maxHp);
				}
			}
		}
		else if (editor.skillSandboxTargetPos.distanceFrom(impactPos) <= skill.bomRadius)
		{
			applyAmount(editor.skillSandboxTargetHp, editor.skillSandboxTargetMaxHp);
		}

		for (auto& dummy : editor.skillSandboxExtraTargets)
		{
			if (skill.kind == SkillKind::Heal)
			{
				break;
			}
			if (dummy.pos.distanceFrom(impactPos) <= skill.bomRadius)
			{
				applyAmount(dummy.hp, dummy.maxHp);
			}
		}

		const int32 selfDamage = Max(0, static_cast<int32>(Math::Round(skill.damage * skill.bomSelfDamageScale + skill.selfDamageOnHit)));
		editor.skillSandboxCasterHp = Max(0, editor.skillSandboxCasterHp - selfDamage);
	}

	inline int32& ResolveSkillSandboxTargetHpRef(MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		if (projectile.targetIsAlly)
		{
			if (projectile.targetIndex > 0)
			{
				const int32 allyIndex = projectile.targetIndex - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].hp;
				}
			}
			return editor.skillSandboxAllyHp;
		}
		if (projectile.targetIndex >= 0 && projectile.targetIndex < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[projectile.targetIndex].hp;
		}
		return editor.skillSandboxTargetHp;
	}

	inline int32 ResolveSkillSandboxTargetMaxHp(const MapEditorState& editor, const SkillSandboxProjectile& projectile)
	{
		if (projectile.targetIsAlly)
		{
			if (projectile.targetIndex > 0)
			{
				const int32 allyIndex = projectile.targetIndex - 1;
				if (allyIndex < static_cast<int32>(editor.skillSandboxExtraAllies.size()))
				{
					return editor.skillSandboxExtraAllies[allyIndex].maxHp;
				}
			}
			return editor.skillSandboxAllyMaxHp;
		}
		if (projectile.targetIndex >= 0 && projectile.targetIndex < static_cast<int32>(editor.skillSandboxExtraTargets.size()))
		{
			return editor.skillSandboxExtraTargets[projectile.targetIndex].maxHp;
		}
		return editor.skillSandboxTargetMaxHp;
	}

	inline void ApplySkillSandboxHit(MapEditorState& editor, const SkillDef& skill, const SkillSandboxProjectile& projectile, const Vec2& impactPos)
	{
		if (skill.bom && skill.bomRadius > 0.0)
		{
			ApplySkillSandboxBomHit(editor, skill, impactPos);
			return;
		}

		int32& targetHp = ResolveSkillSandboxTargetHpRef(editor, projectile);
		const int32 targetMaxHp = ResolveSkillSandboxTargetMaxHp(editor, projectile);
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
	}

	inline bool IsSkillSandboxTargetAlive(int32 hp)
	{
		return hp > 0;
	}
}
