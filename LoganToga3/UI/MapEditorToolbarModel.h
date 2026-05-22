#pragma once
# include <Siv3D.hpp>
# include "MapEditorToolbarLayoutRects.h"

namespace LT3
{
	enum class MapEditorToolbarAction
	{
		ToggleMapEditor,
		SaveToml,
		DecreaseMapWidth,
		IncreaseMapWidth,
		DecreaseMapHeight,
		IncreaseMapHeight,
		ToggleUnitList,
		ToggleBuildingEditor,
		ToggleDebugInfo,
		ToggleUiLayoutEdit,
		ToggleBattleGrid,
		ToggleCommandEditor,
		ToggleSkillEditor,
		HideBarPreview,
	};

	struct MapEditorToolbarButtonSpec
	{
		int32 buttonIndex = -1;
		MapEditorToolbarAction action = MapEditorToolbarAction::ToggleMapEditor;
		const char32* label = U"";
	};

	inline const Array<MapEditorToolbarButtonSpec>& MapEditorToolbarButtonSpecs()
	{
		static const Array<MapEditorToolbarButtonSpec> specs = {
			{ 0, MapEditorToolbarAction::ToggleMapEditor, U"Map Editor" },
			{ 1, MapEditorToolbarAction::SaveToml, U"Save TOML" },
			{ 2, MapEditorToolbarAction::DecreaseMapWidth, U"W -" },
			{ 3, MapEditorToolbarAction::IncreaseMapWidth, U"W +" },
			{ 4, MapEditorToolbarAction::DecreaseMapHeight, U"H -" },
			{ 5, MapEditorToolbarAction::IncreaseMapHeight, U"H +" },
			{ 6, MapEditorToolbarAction::ToggleUnitList, U"Unit List" },
			{ 7, MapEditorToolbarAction::ToggleBuildingEditor, U"Build Edit" },
			{ 8, MapEditorToolbarAction::ToggleDebugInfo, U"Debug Info" },
			{ 9, MapEditorToolbarAction::ToggleUiLayoutEdit, U"UI Edit" },
			{ 10, MapEditorToolbarAction::ToggleBattleGrid, U"Battle Grid" },
			{ 11, MapEditorToolbarAction::ToggleCommandEditor, U"Command" },
			{ 12, MapEditorToolbarAction::ToggleSkillEditor, U"Skill" },
		};
		return specs;
	}

	inline MapEditorToolbarButtonSpec MapEditorToolbarPreviewHideButtonSpec()
	{
		return MapEditorToolbarButtonSpec{ -1, MapEditorToolbarAction::HideBarPreview, U"Hide 3s" };
	}

	inline bool IsMapEditorToolbarButtonVisible(const MapEditorState& editor, const MapEditorToolbarButtonSpec& spec)
	{
		switch (spec.action)
		{
		case MapEditorToolbarAction::SaveToml:
		case MapEditorToolbarAction::DecreaseMapWidth:
		case MapEditorToolbarAction::IncreaseMapWidth:
		case MapEditorToolbarAction::DecreaseMapHeight:
		case MapEditorToolbarAction::IncreaseMapHeight:
			return editor.enabled;
		default:
			return true;
		}
	}

	inline bool IsMapEditorToolbarButtonActive(const MapEditorState& editor, const MapEditorToolbarButtonSpec& spec)
	{
		switch (spec.action)
		{
		case MapEditorToolbarAction::ToggleMapEditor:
			return editor.enabled;
		case MapEditorToolbarAction::ToggleUnitList:
			return editor.showUnitList;
		case MapEditorToolbarAction::ToggleBuildingEditor:
			return editor.showBuildingEditor;
		case MapEditorToolbarAction::ToggleDebugInfo:
			return editor.showDebugInfo;
		case MapEditorToolbarAction::ToggleUiLayoutEdit:
			return editor.uiLayoutEditEnabled;
		case MapEditorToolbarAction::ToggleBattleGrid:
			return editor.showBattleGrid;
		case MapEditorToolbarAction::ToggleCommandEditor:
			return editor.showCommandEditor;
		case MapEditorToolbarAction::ToggleSkillEditor:
			return editor.showSkillEditor;
		default:
			return false;
		}
	}

	inline RectF MapEditorToolbarButtonRect(const MapEditorState& editor, const MapEditorToolbarButtonSpec& spec)
	{
		return (spec.action == MapEditorToolbarAction::HideBarPreview)
			? EditorToolbarPreviewHideButtonRect()
			: EditorToolbarButtonRect(editor, spec.buttonIndex);
	}
}
