#pragma once
# include "SkillEditorSandbox.Hit.h"

namespace LT3
{
		inline bool ProcessSkillSandboxProjectileHit(MapEditorState& editor, const DefinitionStores& defs, SkillSandboxProjectile& projectile, const SkillDef& skill)
		{
			Array<SkillSandboxHitTargetInfo> hitResults;
			bool hit = false;
			if (skill.projectileMotion == SkillProjectileMotion::Swing && skill.swingHitMode == SkillSwingHitMode::MultiHitOnce)
			{
				const Array<SkillSandboxTargetRef> swingTargets = CollectSkillSandboxSwingHitTargets(editor, projectile, skill);
				for (const auto& swingTarget : swingTargets)
				{
					projectile.swingHitTargetIndices << MakeSkillSandboxSwingHitTargetKey(swingTarget);
					const Array<SkillSandboxHitTargetInfo> targetHitResults = ApplySkillSandboxHitToTarget(editor, skill, swingTarget, ResolveSkillSandboxTargetRefPosition(editor, swingTarget));
					for (const auto& hitResult : targetHitResults)
					{
						hitResults << hitResult;
					}
				}
				hit = !swingTargets.isEmpty();
			}
			else
			{
				const SkillSandboxTargetRef projectileTarget = MakeSkillSandboxTargetRef(editor, projectile);
				const Vec2 targetPos = projectile.endPosition;
				const double hitRadius = IsSkillSandboxTargetRefAlive(editor, projectileTarget) ? 34.0 : 6.0;
				hit = CanSkillSandboxSwingHitTarget(projectile, projectileTarget, skill)
					&& (IsSwingEndProjectileHit(skill, projectile.position, projectile.angleRad, targetPos, hitRadius)
						|| (projectile.position.distanceFrom(targetPos) <= hitRadius));
				if (hit)
				{
					if (skill.projectileMotion == SkillProjectileMotion::Swing)
					{
						projectile.swingHitTargetIndices << MakeSkillSandboxSwingHitTargetKey(projectileTarget);
					}
					hitResults = ApplySkillSandboxHit(editor, skill, projectile, targetPos);
				}
			}

			if (hit)
			{
				SpawnSkillSandboxNextFromProjectile(editor, defs, projectile, projectile.position, false, hitResults);
				return !(skill.projectileMotion == SkillProjectileMotion::Swing && skill.swingHitMode == SkillSwingHitMode::MultiHitOnce);
			}

			return false;
		}

		/// <summary>
		/// Skill Mode のサンドボックスを更新し、next 連鎖も反映します。
		/// </summary>
		inline void UpdateSkillSandbox(MapEditorState& editor, const DefinitionStores& defs, SkillDefId skillId, double dt)
	{
			if (skillId == InvalidSkillDefId || skillId >= static_cast<SkillDefId>(defs.skills.size()))
			{
				return;
			}

			const SkillDef& rootSkill = defs.skills[skillId];
		editor.skillSandboxCooldownLeftSec = Max(0.0, editor.skillSandboxCooldownLeftSec - dt);
		editor.skillSandboxLastBomDisplaySec = Max(0.0, editor.skillSandboxLastBomDisplaySec - dt);
		if (editor.skillSandboxBurstShotsLeft > 0)
		{
			editor.skillSandboxBurstShotTimerSec -= dt;
			while (editor.skillSandboxBurstShotsLeft > 0 && editor.skillSandboxBurstShotTimerSec <= 0.0)
			{
					const int32 burstCount = Max(1, rootSkill.burstCount);
				const int32 shotIndex = burstCount - editor.skillSandboxBurstShotsLeft;
				const int32 burstIndex = (shotIndex < static_cast<int32>(editor.skillSandboxBurstOrder.size())) ? editor.skillSandboxBurstOrder[shotIndex] : shotIndex;
					SpawnSkillSandboxProjectilesForTargetsTagged(editor, rootSkill, skillId, burstIndex);
				--editor.skillSandboxBurstShotsLeft;
					editor.skillSandboxBurstShotTimerSec += Max(0.0, rootSkill.burstIntervalSec);
					if (rootSkill.burstIntervalSec <= 0.0)
				{
					editor.skillSandboxBurstShotTimerSec = 0.0;
				}
			}
		}
		if (editor.skillSandboxAutoFire && editor.skillSandboxCooldownLeftSec <= 0.0)
		{
				FireSkillSandbox(editor, defs, skillId);
		}

		for (size_t i = 0; i < editor.skillSandboxProjectiles.size();)
		{
			SkillSandboxProjectile& projectile = editor.skillSandboxProjectiles[i];
				const SkillDefId pid = projectile.skillId;
				if (pid == InvalidSkillDefId || pid >= static_cast<SkillDefId>(defs.skills.size()))
				{
					editor.skillSandboxProjectiles.remove_at(i);
					continue;
				}
				const SkillDef& skill = defs.skills[pid];
			projectile.lifeSec -= dt;
			projectile.ageSec += dt;
			const SkillSandboxTargetRef projectileTarget = MakeSkillSandboxTargetRef(editor, projectile);
			const bool targetAlive = IsSkillSandboxTargetRefAlive(editor, projectileTarget);
			const bool usesFixedImpactPoint = (projectile.motion == SkillProjectileMotion::Arc)
				|| (projectile.motion == SkillProjectileMotion::Parabola)
				|| (projectile.motion == SkillProjectileMotion::Drop);
			if (!usesFixedImpactPoint && targetAlive)
			{
				projectile.endPosition = ResolveSkillSandboxTargetRefPosition(editor, projectileTarget);
			}
			if (projectile.lifeSec <= 0.0)
			{
					SpawnSkillSandboxNextFromProjectile(editor, defs, projectile, projectile.position, true);
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}

			if (projectile.motion == SkillProjectileMotion::Orbit)
			{
				projectile.angleRad += Math::ToRadians(skill.orbitAngularSpeedDeg) * dt;
				projectile.position = editor.skillSandboxCasterPos + Vec2{ Cos(projectile.angleRad), Sin(projectile.angleRad) } * skill.orbitRadius;
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Swing)
			{
				const double sign = (skill.swingAngleDeg < 0.0) ? -1.0 : 1.0;
				const double progressDeg = Min(Abs(skill.swingAngleDeg), projectile.ageSec * Abs(skill.projectileSpeed));
				projectile.angleRad = projectile.baseAngleRad + Math::ToRadians(progressDeg * sign);
				projectile.position = editor.skillSandboxCasterPos + Vec2{ Cos(projectile.angleRad), Sin(projectile.angleRad) } * skill.swingRadius;
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Arc)
			{
				const double t = Clamp(projectile.ageSec / Max(0.05, projectile.maxLifeSec), 0.0, 1.0);
				const Vec2 linePos = projectile.startPosition.lerp(projectile.endPosition, t);
				const Vec2 delta = projectile.endPosition - projectile.startPosition;
				const Vec2 normal = (delta.lengthSq() > 1.0) ? Vec2{ -delta.y, delta.x }.normalized() : Vec2{ 0.0, -1.0 };
				projectile.position = linePos + normal * (Sin(t * Math::Pi) * skill.arcHeight);
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Parabola)
			{
				const double t = Clamp(projectile.ageSec / Max(0.05, projectile.maxLifeSec), 0.0, 1.0);
				projectile.position = projectile.startPosition.lerp(projectile.endPosition, t);
				projectile.height = Sin(t * Math::Pi) * skill.arcHeight;
			}
			else if (projectile.motion == SkillProjectileMotion::Drop)
			{
				projectile.position = projectile.endPosition;
				projectile.height = (skill.projectileSpeed >= 0.0)
					? Max(0.0, Max(1.0, skill.arcHeight) - Abs(skill.projectileSpeed) * projectile.ageSec)
					: Abs(skill.projectileSpeed) * projectile.ageSec;
			}
			else if (projectile.motion == SkillProjectileMotion::Static)
			{
				projectile.height = 0.0;
			}
			else
			{
				if (skill.projectileHoming)
				{
					const Vec2 toTarget = projectile.endPosition - projectile.position;
					if (toTarget.length() > 1.0)
					{
						projectile.velocity = projectile.velocity.lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
						projectile.angleRad = Math::Atan2(projectile.velocity.y, projectile.velocity.x);
					}
				}
				projectile.position += projectile.velocity * dt;
				projectile.height = 0.0;
			}

			if (ProcessSkillSandboxProjectileHit(editor, defs, projectile, skill))
			{
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}
			++i;
		}
	}

	// Unit Mode 用: 各弾が持つ skillId でスキル定義を引く
	inline void UpdateSkillSandboxUnitMode(MapEditorState& editor, const DefinitionStores& defs, double dt)
	{
		editor.skillSandboxCooldownLeftSec = Max(0.0, editor.skillSandboxCooldownLeftSec - dt);
		editor.skillSandboxLastBomDisplaySec = Max(0.0, editor.skillSandboxLastBomDisplaySec - dt);

		// バースト継続: activeSkillId で定義を取得
		if (editor.skillSandboxBurstShotsLeft > 0 && editor.skillSandboxActiveSkillId != InvalidSkillDefId
			&& editor.skillSandboxActiveSkillId < static_cast<SkillDefId>(defs.skills.size()))
		{
			const SkillDef& burstSkill = defs.skills[editor.skillSandboxActiveSkillId];
			editor.skillSandboxBurstShotTimerSec -= dt;
			while (editor.skillSandboxBurstShotsLeft > 0 && editor.skillSandboxBurstShotTimerSec <= 0.0)
			{
				const int32 burstCount = Max(1, burstSkill.burstCount);
				const int32 shotIndex = burstCount - editor.skillSandboxBurstShotsLeft;
				const int32 burstIndex = (shotIndex < static_cast<int32>(editor.skillSandboxBurstOrder.size())) ? editor.skillSandboxBurstOrder[shotIndex] : shotIndex;
				SpawnSkillSandboxProjectilesForTargetsTagged(editor, burstSkill, editor.skillSandboxActiveSkillId, burstIndex);
				--editor.skillSandboxBurstShotsLeft;
				editor.skillSandboxBurstShotTimerSec += Max(0.0, burstSkill.burstIntervalSec);
				if (burstSkill.burstIntervalSec <= 0.0) editor.skillSandboxBurstShotTimerSec = 0.0;
			}
		}

		for (size_t i = 0; i < editor.skillSandboxProjectiles.size();)
		{
			SkillSandboxProjectile& projectile = editor.skillSandboxProjectiles[i];
			const SkillDefId pid = projectile.skillId;
			if (pid == InvalidSkillDefId || pid >= static_cast<SkillDefId>(defs.skills.size()))
			{
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}
			const SkillDef& skill = defs.skills[pid];

			projectile.lifeSec -= dt;
			projectile.ageSec += dt;
			const SkillSandboxTargetRef projectileTarget = MakeSkillSandboxTargetRef(editor, projectile);
			const bool targetAlive = IsSkillSandboxTargetRefAlive(editor, projectileTarget);
			const bool usesFixedImpactPoint = (projectile.motion == SkillProjectileMotion::Arc)
				|| (projectile.motion == SkillProjectileMotion::Parabola)
				|| (projectile.motion == SkillProjectileMotion::Drop);
			if (!usesFixedImpactPoint && targetAlive)
			{
				projectile.endPosition = ResolveSkillSandboxTargetRefPosition(editor, projectileTarget);
			}
			if (projectile.lifeSec <= 0.0)
			{
					SpawnSkillSandboxNextFromProjectile(editor, defs, projectile, projectile.position, true);
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}

			if (projectile.motion == SkillProjectileMotion::Orbit)
			{
				projectile.angleRad += Math::ToRadians(skill.orbitAngularSpeedDeg) * dt;
				projectile.position = editor.skillSandboxCasterPos + Vec2{ Cos(projectile.angleRad), Sin(projectile.angleRad) } * skill.orbitRadius;
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Swing)
			{
				const double sign = (skill.swingAngleDeg < 0.0) ? -1.0 : 1.0;
				const double progressDeg = Min(Abs(skill.swingAngleDeg), projectile.ageSec * Abs(skill.projectileSpeed));
				projectile.angleRad = projectile.baseAngleRad + Math::ToRadians(progressDeg * sign);
				projectile.position = editor.skillSandboxCasterPos + Vec2{ Cos(projectile.angleRad), Sin(projectile.angleRad) } * skill.swingRadius;
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Arc)
			{
				const double t = Clamp(projectile.ageSec / Max(0.05, projectile.maxLifeSec), 0.0, 1.0);
				const Vec2 linePos = projectile.startPosition.lerp(projectile.endPosition, t);
				const Vec2 delta = projectile.endPosition - projectile.startPosition;
				const Vec2 normal = (delta.lengthSq() > 1.0) ? Vec2{ -delta.y, delta.x }.normalized() : Vec2{ 0.0, -1.0 };
				projectile.position = linePos + normal * (Sin(t * Math::Pi) * skill.arcHeight);
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Parabola)
			{
				const double t = Clamp(projectile.ageSec / Max(0.05, projectile.maxLifeSec), 0.0, 1.0);
				projectile.position = projectile.startPosition.lerp(projectile.endPosition, t);
				projectile.height = Sin(t * Math::Pi) * skill.arcHeight;
			}
			else if (projectile.motion == SkillProjectileMotion::Drop)
			{
				projectile.position = projectile.endPosition;
				projectile.height = (skill.projectileSpeed >= 0.0)
					? Max(0.0, Max(1.0, skill.arcHeight) - Abs(skill.projectileSpeed) * projectile.ageSec)
					: Abs(skill.projectileSpeed) * projectile.ageSec;
			}
			else if (projectile.motion == SkillProjectileMotion::Static)
			{
				projectile.height = 0.0;
			}
			else
			{
				if (skill.projectileHoming)
				{
					const Vec2 toTarget = projectile.endPosition - projectile.position;
					if (toTarget.length() > 1.0)
					{
						projectile.velocity = projectile.velocity.lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
						projectile.angleRad = Math::Atan2(projectile.velocity.y, projectile.velocity.x);
					}
				}
				projectile.position += projectile.velocity * dt;
				projectile.height = 0.0;
			}

			if (ProcessSkillSandboxProjectileHit(editor, defs, projectile, skill))
			{
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}
			++i;
		}
	}
}
