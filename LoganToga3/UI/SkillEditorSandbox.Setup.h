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

	inline Vec2 SkillSandboxDefaultAllyPos()
	{
		const RectF arena = SkillEditorSandboxArenaRect();
		return Vec2{ arena.x + arena.w * 0.34, arena.center().y - 92.0 };
	}

	inline Array<Vec2> SkillSandboxDefaultExtraAllyPositions()
	{
		const RectF arena = SkillEditorSandboxArenaRect();
		return {
			Vec2{ arena.x + arena.w * 0.26, arena.center().y + 84.0 },
			Vec2{ arena.x + arena.w * 0.44, arena.center().y + 108.0 },
		};
	}

	struct SkillSandboxTargetRef
	{
		Vec2 pos{ 0.0, 0.0 };
		bool isAlly = false;
		int32 index = -1;
	};

	inline Array<Vec2> SkillSandboxDefaultExtraTargetPositions()
	{
		const RectF arena = SkillEditorSandboxArenaRect();
		return {
			Vec2{ arena.x + arena.w * 0.68, arena.center().y - 86.0 },
			Vec2{ arena.x + arena.w * 0.70, arena.center().y + 88.0 },
			Vec2{ arena.x + arena.w * 0.90, arena.center().y - 12.0 },
		};
	}

	inline void ResetSkillSandbox(MapEditorState& editor)
	{
		editor.skillSandboxCasterPos = SkillSandboxDefaultCasterPos();
		editor.skillSandboxAllyPos = SkillSandboxDefaultAllyPos();
		editor.skillSandboxTargetPos = SkillSandboxDefaultTargetPos();
		editor.skillSandboxCasterMaxHp = 100;
		editor.skillSandboxCasterHp = editor.skillSandboxCasterMaxHp;
		editor.skillSandboxAllyMaxHp = 100;
		editor.skillSandboxAllyHp = 55;
		editor.skillSandboxTargetMaxHp = 100;
		editor.skillSandboxTargetHp = editor.skillSandboxTargetMaxHp;
		editor.skillSandboxCooldownLeftSec = 0.0;
		editor.skillSandboxDraggingAlly = false;
		editor.skillSandboxDraggingTarget = false;
		editor.skillSandboxDraggingExtraAllyIndex = none;
		editor.skillSandboxDraggingExtraTargetIndex = none;
		editor.skillSandboxBurstShotsLeft = 0;
		editor.skillSandboxBurstShotTimerSec = 0.0;
		editor.skillSandboxBurstOrder.clear();
		editor.skillSandboxProjectiles.clear();
		editor.skillSandboxExtraAllies.clear();
		for (int32 i = 0; i < static_cast<int32>(SkillSandboxDefaultExtraAllyPositions().size()); ++i)
		{
			const Vec2& pos = SkillSandboxDefaultExtraAllyPositions()[i];
			const int32 hp = (i == 0) ? 38 : 72;
			editor.skillSandboxExtraAllies << SkillSandboxDummyTarget{ pos, hp, 100 };
		}
		editor.skillSandboxExtraTargets.clear();
		for (const Vec2& pos : SkillSandboxDefaultExtraTargetPositions())
		{
			editor.skillSandboxExtraTargets << SkillSandboxDummyTarget{ pos, 100, 100 };
		}
		editor.skillSandboxLastBomCenter = editor.skillSandboxTargetPos;
		editor.skillSandboxLastBomRadius = 0.0;
		editor.skillSandboxLastBomDisplaySec = 0.0;
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
}
