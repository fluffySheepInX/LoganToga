#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "SkillEditorLayout.h"
# include "../Data/Loaders/SkillDefLoader.h"
# include "../Systems/ProjectileSystem.h"

namespace LT3
{
	inline Vec2 SkillSandboxDefaultCasterPos()
	{
		const RectF arena = SkillEditorSandboxArenaRect();
		return Vec2{ arena.x + arena.w * 0.18, arena.center().y };
	}

	inline Vec2 SkillSandboxDefaultTargetPos()
	{
		const RectF arena = SkillEditorSandboxArenaRect();
		return Vec2{ arena.x + arena.w * 0.82, arena.center().y };
	}

	inline void ResetSkillSandbox(MapEditorState& editor)
	{
		editor.skillSandboxCasterPos = SkillSandboxDefaultCasterPos();
		editor.skillSandboxTargetPos = SkillSandboxDefaultTargetPos();
		editor.skillSandboxTargetMaxHp = 100;
		editor.skillSandboxTargetHp = editor.skillSandboxTargetMaxHp;
		editor.skillSandboxCooldownLeftSec = 0.0;
		editor.skillSandboxDraggingTarget = false;
		editor.skillSandboxProjectiles.clear();
	}

	inline void EnsureSkillSandboxReady(MapEditorState& editor)
	{
		if (editor.skillSandboxCasterPos == Vec2{ 136.0, 404.0 } && editor.skillSandboxTargetPos == Vec2{ 512.0, 404.0 })
		{
			ResetSkillSandbox(editor);
		}
	}

	inline void ResetSkillSandboxForSkill(MapEditorState& editor)
	{
		if (editor.skillSandboxSkillIndex != editor.selectedSkillIndex)
		{
			editor.skillSandboxSkillIndex = editor.selectedSkillIndex;
			ResetSkillSandbox(editor);
		}
	}

	inline void SpawnSkillSandboxProjectile(MapEditorState& editor, const SkillDef& skill, int32 burstIndex)
	{
		const Vec2 toTarget = editor.skillSandboxTargetPos - editor.skillSandboxCasterPos;
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
			spawnPos = editor.skillSandboxTargetPos;
		}

		SkillSandboxProjectile projectile;
		projectile.position = spawnPos;
		projectile.velocity = (skill.projectileMotion == SkillProjectileMotion::Static) ? Vec2{ 0.0, 0.0 } : (dir * skill.projectileSpeed);
		projectile.startPosition = editor.skillSandboxCasterPos;
		projectile.endPosition = editor.skillSandboxTargetPos;
		projectile.lifeSec = maxLife;
		projectile.maxLifeSec = maxLife;
		projectile.angleRad = startAngleRad;
		projectile.baseAngleRad = startAngleRad;
		projectile.motion = skill.projectileMotion;
		if (skill.projectileMotion == SkillProjectileMotion::Drop)
		{
			projectile.height = (skill.projectileSpeed >= 0.0) ? Max(1.0, skill.arcHeight) : 0.0;
		}
		editor.skillSandboxProjectiles << projectile;
	}

	inline void FireSkillSandbox(MapEditorState& editor, const SkillDef& skill)
	{
		if (editor.skillSandboxTargetHp <= 0)
		{
			editor.skillSandboxTargetHp = editor.skillSandboxTargetMaxHp;
		}
		const int32 burstCount = Max(1, skill.burstCount);
		for (int32 burstIndex = 0; burstIndex < burstCount; ++burstIndex)
		{
			SpawnSkillSandboxProjectile(editor, skill, burstIndex);
		}
		editor.skillSandboxCooldownLeftSec = Max(0.05, skill.cooldownSec);
	}

	inline void ApplySkillSandboxHit(MapEditorState& editor, const SkillDef& skill)
	{
		editor.skillSandboxTargetHp = Max(0, editor.skillSandboxTargetHp - Max(1, skill.damage));
	}

	inline void UpdateSkillSandbox(MapEditorState& editor, const SkillDef& skill, double dt)
	{
		editor.skillSandboxCooldownLeftSec = Max(0.0, editor.skillSandboxCooldownLeftSec - dt);
		if (editor.skillSandboxAutoFire && editor.skillSandboxCooldownLeftSec <= 0.0)
		{
			FireSkillSandbox(editor, skill);
		}

		for (size_t i = 0; i < editor.skillSandboxProjectiles.size();)
		{
			SkillSandboxProjectile& projectile = editor.skillSandboxProjectiles[i];
			projectile.lifeSec -= dt;
			projectile.ageSec += dt;
			if (projectile.lifeSec <= 0.0)
			{
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
				projectile.endPosition = editor.skillSandboxTargetPos;
				const Vec2 linePos = projectile.startPosition.lerp(projectile.endPosition, t);
				const Vec2 delta = projectile.endPosition - projectile.startPosition;
				const Vec2 normal = (delta.lengthSq() > 1.0) ? Vec2{ -delta.y, delta.x }.normalized() : Vec2{ 0.0, -1.0 };
				projectile.position = linePos + normal * (Sin(t * Math::Pi) * skill.arcHeight);
				projectile.height = 0.0;
			}
			else if (projectile.motion == SkillProjectileMotion::Parabola)
			{
				const double t = Clamp(projectile.ageSec / Max(0.05, projectile.maxLifeSec), 0.0, 1.0);
				projectile.endPosition = editor.skillSandboxTargetPos;
				projectile.position = projectile.startPosition.lerp(projectile.endPosition, t);
				projectile.height = Sin(t * Math::Pi) * skill.arcHeight;
			}
			else if (projectile.motion == SkillProjectileMotion::Drop)
			{
				projectile.endPosition = editor.skillSandboxTargetPos;
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
					const Vec2 toTarget = editor.skillSandboxTargetPos - projectile.position;
					if (toTarget.length() > 1.0)
					{
						projectile.velocity = projectile.velocity.lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
						projectile.angleRad = Math::Atan2(projectile.velocity.y, projectile.velocity.x);
					}
				}
				projectile.position += projectile.velocity * dt;
				projectile.height = 0.0;
			}

			if (projectile.position.distanceFrom(editor.skillSandboxTargetPos) <= 34.0)
			{
				ApplySkillSandboxHit(editor, skill);
				editor.skillSandboxProjectiles.remove_at(i);
				continue;
			}
			++i;
		}
	}
}
