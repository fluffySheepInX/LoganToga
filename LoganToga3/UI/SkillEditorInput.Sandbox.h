#pragma once
# include "SkillEditorInput.Common.h"

namespace LT3
{
	inline int32 ResolveDefaultSkillSandboxUnitCatalogIndex(const MapEditorState& editor, const DefinitionStores& defs, const UnitCatalog& catalog)
	{
		if (editor.selectedSkillUnitIndex >= 0 && editor.selectedSkillUnitIndex < static_cast<int32>(catalog.entries.size()))
		{
			return editor.selectedSkillUnitIndex;
		}

		if (HasSelectedSkill(editor, defs))
		{
			const String& skillTag = defs.skills[editor.selectedSkillIndex].tag;
			for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
			{
				if (UnitHasSkill(catalog.entries[i], skillTag))
				{
					return i;
				}
			}
		}

		for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
		{
			if (!catalog.entries[i].skills.isEmpty())
			{
				return i;
			}
		}

		return -1;
	}

	/// <summary>
	/// スキルサンドボックス領域の入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorSandboxInput(MapEditorState& editor, DefinitionStores& defs, const UnitCatalog& catalog)
	{
		const RectF sandboxPreview = SkillEditorSandboxPreviewRect();
		if (!(editor.showSkillSandboxPreview && sandboxPreview.mouseOver()))
		{
			return false;
		}

		EnsureSkillSandboxReady(editor);
		ResetSkillSandboxForSkill(editor);

		const bool isUnitMode = (editor.skillSandboxMode == SkillSandboxMode::Unit);

		// モード切替ボタン (index 3)
		if (HandleRectButtonClick(SkillEditorSandboxButtonRect(3)))
		{
			if (isUnitMode)
			{
				editor.skillSandboxMode = SkillSandboxMode::Skill;
				editor.skillSandboxActiveSkillId = InvalidSkillDefId;
				editor.statusText = U"Sandbox: Skill Mode";
			}
			else
			{
				editor.skillSandboxMode = SkillSandboxMode::Unit;
				editor.skillSandboxUnitCatalogIndex = ResolveDefaultSkillSandboxUnitCatalogIndex(editor, defs, catalog);
				editor.selectedSkillUnitIndex = editor.skillSandboxUnitCatalogIndex;
				editor.skillSandboxActiveSkillId = InvalidSkillDefId;
				editor.statusText = (editor.skillSandboxUnitCatalogIndex >= 0)
					? U"Sandbox: Unit Mode"
					: U"Sandbox: Unit Mode (unit none)";
			}
			ResetSkillSandbox(editor);
			return true;
		}

		if (isUnitMode)
		{
			UpdateSkillSandboxUnitMode(editor, defs, Scene::DeltaTime());

			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(0)))
			{
				FireSkillSandboxUnitMode(editor, defs, catalog);
				return true;
			}
			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(1)))
			{
				editor.skillSandboxAutoFire = !editor.skillSandboxAutoFire;
				editor.statusText = editor.skillSandboxAutoFire ? U"Skill sandbox auto fire ON" : U"Skill sandbox auto fire OFF";
				return true;
			}
			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(2)))
			{
				ResetSkillSandbox(editor);
				editor.skillSandboxActiveSkillId = InvalidSkillDefId;
				editor.statusText = U"Skill sandbox reset";
				return true;
			}
			if (editor.skillSandboxAutoFire && editor.skillSandboxCooldownLeftSec <= 0.0)
			{
				FireSkillSandboxUnitMode(editor, defs, catalog);
			}
		}
		else if (HasSelectedSkill(editor, defs))
		{
			SkillDef& skill = defs.skills[editor.selectedSkillIndex];
			UpdateSkillSandbox(editor, defs, static_cast<SkillDefId>(editor.selectedSkillIndex), Scene::DeltaTime());

			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(0)))
			{
				FireSkillSandbox(editor, defs, static_cast<SkillDefId>(editor.selectedSkillIndex));
				return true;
			}
			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(1)))
			{
				editor.skillSandboxAutoFire = !editor.skillSandboxAutoFire;
				editor.statusText = editor.skillSandboxAutoFire ? U"Skill sandbox auto fire ON" : U"Skill sandbox auto fire OFF";
				return true;
			}
			if (HandleRectButtonClick(SkillEditorSandboxButtonRect(2)))
			{
				ResetSkillSandbox(editor);
				editor.statusText = U"Skill sandbox reset";
				return true;
			}
		}

		const RectF arena = SkillEditorSandboxArenaRect();
		if (MouseL.down() && Circle{ editor.skillSandboxAllyPos, 32.0 }.mouseOver())
		{
			editor.skillSandboxDraggingAlly = true;
			editor.skillSandboxDraggingTarget = false;
			editor.skillSandboxDraggingExtraAllyIndex = none;
			editor.skillSandboxDraggingExtraTargetIndex = none;
			return true;
		}
		if (MouseL.down() && Circle{ editor.skillSandboxTargetPos, 34.0 }.mouseOver())
		{
			editor.skillSandboxDraggingAlly = false;
			editor.skillSandboxDraggingTarget = true;
			editor.skillSandboxDraggingExtraAllyIndex = none;
			editor.skillSandboxDraggingExtraTargetIndex = none;
			return true;
		}
		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
		{
			if (MouseL.down() && Circle{ editor.skillSandboxExtraAllies[i].pos, 28.0 }.mouseOver())
			{
				editor.skillSandboxDraggingAlly = false;
				editor.skillSandboxDraggingTarget = false;
				editor.skillSandboxDraggingExtraAllyIndex = i;
				editor.skillSandboxDraggingExtraTargetIndex = none;
				return true;
			}
		}
		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
		{
			if (MouseL.down() && Circle{ editor.skillSandboxExtraTargets[i].pos, 28.0 }.mouseOver())
			{
				editor.skillSandboxDraggingAlly = false;
				editor.skillSandboxDraggingTarget = false;
				editor.skillSandboxDraggingExtraAllyIndex = none;
				editor.skillSandboxDraggingExtraTargetIndex = i;
				return true;
			}
		}
		if (!MouseL.pressed())
		{
			editor.skillSandboxDraggingAlly = false;
			editor.skillSandboxDraggingTarget = false;
			editor.skillSandboxDraggingExtraAllyIndex = none;
			editor.skillSandboxDraggingExtraTargetIndex = none;
		}
		if (editor.skillSandboxDraggingAlly)
		{
			const Vec2 mouse = Cursor::PosF();
			editor.skillSandboxAllyPos = Vec2{
				Clamp(mouse.x, arena.x + 28.0, arena.x + arena.w - 28.0),
				Clamp(mouse.y, arena.y + 28.0, arena.y + arena.h - 28.0)
			};
			return true;
		}
		if (editor.skillSandboxDraggingExtraAllyIndex)
		{
			const Vec2 mouse = Cursor::PosF();
			auto& ally = editor.skillSandboxExtraAllies[*editor.skillSandboxDraggingExtraAllyIndex];
			ally.pos = Vec2{
				Clamp(mouse.x, arena.x + 28.0, arena.x + arena.w - 28.0),
				Clamp(mouse.y, arena.y + 28.0, arena.y + arena.h - 28.0)
			};
			return true;
		}
		if (editor.skillSandboxDraggingTarget)
		{
			const Vec2 mouse = Cursor::PosF();
			editor.skillSandboxTargetPos = Vec2{
				Clamp(mouse.x, arena.x + 34.0, arena.x + arena.w - 34.0),
				Clamp(mouse.y, arena.y + 34.0, arena.y + arena.h - 34.0)
			};
			return true;
		}
		if (editor.skillSandboxDraggingExtraTargetIndex)
		{
			const Vec2 mouse = Cursor::PosF();
			auto& dummy = editor.skillSandboxExtraTargets[*editor.skillSandboxDraggingExtraTargetIndex];
			dummy.pos = Vec2{
				Clamp(mouse.x, arena.x + 28.0, arena.x + arena.w - 28.0),
				Clamp(mouse.y, arena.y + 28.0, arena.y + arena.h - 28.0)
			};
			return true;
		}
		return true;
	}

	/// <summary>
	/// スキルエディタ上部の共通パネル操作を処理します。
	/// </summary>
	inline bool ProcessSkillEditorPanelInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (HandleRectButtonClick(SkillEditorCloseRect()))
		{
			editor.showSkillEditor = false;
			editor.statusText = U"SkillEditor OFF";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSandboxToggleRect()))
		{
			editor.showSkillSandboxPreview = !editor.showSkillSandboxPreview;
			if (editor.showSkillSandboxPreview)
			{
				EnsureSkillSandboxReady(editor);
				ResetSkillSandboxForSkill(editor);
			}
			editor.statusText = editor.showSkillSandboxPreview ? U"Skill sandbox preview ON" : U"Skill sandbox preview OFF";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSaveRect()))
		{
			SaveSkillEditorDefinitions(editor, defs);
			return true;
		}

		return false;
	}
}
