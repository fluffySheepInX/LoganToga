#pragma once
# include "SkillEditorInput.Sandbox.h"
# include "SkillEditorInput.ListInput.h"
# include "SkillEditorInput.DetailInput.h"
# include "SkillEditorInput.AssetInput.h"
# include "SkillEditorInput.PropertyInput.h"

namespace LT3
{
	/// <summary>
	/// SkillEditor 全体の入力ディスパッチを処理します。
	/// </summary>
	inline bool ProcessSkillEditorInput(MapEditorState& editor, const BattleWorld& world, DefinitionStores& defs, UnitCatalog& catalog)
	{
		if (!editor.showSkillEditor)
		{
			return false;
		}

		EnsureSkillEditorValueSteps(editor);

		const RectF panel = SkillEditorPanelRect();
		const RectF sandboxPreview = SkillEditorSandboxPreviewRect();
		if (!panel.mouseOver() && !(editor.showSkillSandboxPreview && sandboxPreview.mouseOver()))
		{
			return false;
		}

		if (ProcessSkillEditorSandboxInput(editor, defs, catalog))
		{
			return true;
		}

		if (ProcessSkillEditorPanelInput(editor, defs))
		{
			return true;
		}

		if (ProcessSkillEditorRenameInput(editor, defs, catalog))
		{
			return true;
		}

		if (ProcessSkillEditorContextMenuInput(editor, defs))
		{
			return true;
		}

		if (ProcessSkillEditorUnitContextMenuInput(editor, defs, catalog))
		{
			return true;
		}

		if (ProcessSkillEditorListInput(editor, defs))
		{
			return true;
		}

		if (ProcessSkillEditorUnitBindingInput(editor, defs, catalog))
		{
			return true;
		}

		if (!HasSelectedSkill(editor, defs))
		{
			return true;
		}

		SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		EnsureSkillEditorResourceCostSteps(editor, skill);
		const double scroll = editor.skillDetailScroll;

		if (ProcessSkillEditorDetailTextInput(editor, defs))
		{
			return true;
		}

		if (ProcessSkillEditorDetailInput(editor, defs, skill))
		{
			return true;
		}

		if (ProcessSkillEditorAssetInput(editor, world, defs, scroll))
		{
			return true;
		}

		if (ProcessSkillEditorPropertyInput(editor, defs, scroll))
		{
			return true;
		}

		return true;
	}
}
