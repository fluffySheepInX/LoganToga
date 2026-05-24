#pragma once
# include "MapEditorCommandEditorInput.RenameContextMenu.h"

namespace LT3
{
	inline bool ProcessCommandInspectInput(MapEditorState& editor, BuildActionDef& action, const RectF& inspectTopViewport)
	{
		bool consumed = false;
		const bool showLineSettings = (action.placementMode == BuildPlacementMode::Line);
		const double inspectContentHeight = showLineSettings ? 576.0 : 492.0;
		const double inspectMaxScroll = Max(0.0, inspectContentHeight - inspectTopViewport.h + 8.0);
		editor.commandInspectScroll = Clamp(editor.commandInspectScroll, 0.0, inspectMaxScroll);
		if (inspectTopViewport.mouseOver())
		{
			editor.commandInspectScroll = Clamp(editor.commandInspectScroll - Mouse::Wheel() * 42.0, 0.0, inspectMaxScroll);
			consumed = true;
		}

		const double scroll = editor.commandInspectScroll;
		const RectF placementToggleRect = EditorCommandPlacementToggleRect(scroll);
		const RectF spawnCountRow = EditorCommandSpawnCountRowRect(scroll);
		const RectF placementModePointRect = EditorCommandPlacementModePointRect(scroll);
		const RectF placementModeLineRect = EditorCommandPlacementModeLineRect(scroll);
		const RectF lineDragPlacementToggleRect = EditorCommandLineDragPlacementToggleRect(scroll);
		const RectF lineAxisAutoRect = EditorCommandLineAxisAutoRect(scroll);
		const RectF lineAxisHorizontalRect = EditorCommandLineAxisHorizontalRect(scroll);
		const RectF lineAxisVerticalRect = EditorCommandLineAxisVerticalRect(scroll);
		const Array<RectF> costRows = { EditorCommandCostRowRect(0, scroll), EditorCommandCostRowRect(1, scroll), EditorCommandCostRowRect(2, scroll) };

		if (placementToggleRect.leftClicked())
		{
			action.isMove = !action.isMove;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command placement toggled: {} -> {}"_fmt(action.name, action.isMove ? U"ON" : U"OFF");
			consumed = true;
		}
		if (action.resultType == BuildActionResultType::Unit)
		{
			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorCommandSpawnCountButtonRect(spawnCountRow, buttonIndex);
				if (!buttonRect.leftClicked())
				{
					continue;
				}

				if (buttonIndex == 0)
				{
					action.createCount = Max(1, action.createCount - 1);
				}
				else if (buttonIndex == 1)
				{
					action.createCount = Min(32, Max(1, action.createCount) + 1);
				}
				else
				{
					action.createCount = 1;
				}

				action.createCount = Clamp(action.createCount, 1, 32);
				editor.commandBindingsDirty = true;
				editor.statusText = U"Command spawn count updated: {} x{}"_fmt(action.name, action.createCount);
				return true;
			}
		}
		else
		{
			action.createCount = 1;
		}

		if (placementModePointRect.leftClicked())
		{
			action.placementMode = BuildPlacementMode::Point;
			action.useRightDragPlacement = false;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command placement mode: {} -> point"_fmt(action.name);
			consumed = true;
		}
		if (placementModeLineRect.leftClicked())
		{
			action.placementMode = BuildPlacementMode::Line;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command placement mode: {} -> line"_fmt(action.name);
			consumed = true;
		}
		if (action.placementMode == BuildPlacementMode::Line && lineDragPlacementToggleRect.leftClicked())
		{
			action.useRightDragPlacement = !action.useRightDragPlacement;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command line drag input: {} -> {}"_fmt(action.name, action.useRightDragPlacement ? U"right-drag" : U"click-only");
			consumed = true;
		}
		if (action.placementMode == BuildPlacementMode::Line && lineAxisAutoRect.leftClicked())
		{
			action.lineAxisMode = BuildLineAxisMode::Auto;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command line axis: {} -> auto"_fmt(action.name);
			consumed = true;
		}
		if (action.placementMode == BuildPlacementMode::Line && lineAxisHorizontalRect.leftClicked())
		{
			action.lineAxisMode = BuildLineAxisMode::HorizontalOnly;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command line axis: {} -> horizontal"_fmt(action.name);
			consumed = true;
		}
		if (action.placementMode == BuildPlacementMode::Line && lineAxisVerticalRect.leftClicked())
		{
			action.lineAxisMode = BuildLineAxisMode::VerticalOnly;
			editor.commandBindingsDirty = true;
			editor.statusText = U"Command line axis: {} -> vertical"_fmt(action.name);
			consumed = true;
		}

		auto adjustCostValue = [&](int32& value, int32 buttonIndex)
		{
			if (buttonIndex == 0)
			{
				value = Max(0, value - 10);
			}
			else if (buttonIndex == 1)
			{
				value = Min(99999, value + 10);
			}
			else
			{
				value = 0;
			}
		};

		for (int32 rowIndex = 0; rowIndex < static_cast<int32>(costRows.size()); ++rowIndex)
		{
			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorCommandCostButtonRect(costRows[rowIndex], buttonIndex);
				if (!buttonRect.leftClicked())
				{
					continue;
				}

				if (rowIndex == 0)
				{
					adjustCostValue(action.costGold, buttonIndex);
					editor.statusText = U"Command cost updated: {} Gold={}"_fmt(action.name, action.costGold);
				}
				else if (rowIndex == 1)
				{
					adjustCostValue(action.costTrust, buttonIndex);
					editor.statusText = U"Command cost updated: {} Trust={}"_fmt(action.name, action.costTrust);
				}
				else
				{
					adjustCostValue(action.costFood, buttonIndex);
					editor.statusText = U"Command cost updated: {} Food={}"_fmt(action.name, action.costFood);
				}

				editor.commandBindingsDirty = true;
				return true;
			}
		}

		return consumed;
	}

	inline bool ProcessCommandUnitSelectionInput(MapEditorState& editor, UnitCatalog& catalog, DefinitionStores& defs, BuildActionDef& action, const RectF& unitViewport)
	{
		constexpr int32 columns = 5;
		const Array<int32> sortedUnitIndices = SortedUnitCatalogEntryIndicesById(catalog);
		const int32 unitCount = static_cast<int32>(sortedUnitIndices.size());
		const int32 unitRows = (unitCount + columns - 1) / columns;
		const double unitContentHeight = unitRows * 96.0 + 8.0;
		const double unitMaxScroll = Max(0.0, unitContentHeight - unitViewport.h);
		if (editor.commandEditorMode != 2 && unitViewport.mouseOver())
		{
			editor.commandUnitListScroll = Clamp(editor.commandUnitListScroll - Mouse::Wheel() * 42.0, 0.0, unitMaxScroll);
		}

		const double unitViewportBottom = unitViewport.y + unitViewport.h;
		if (editor.commandEditorMode == 2)
		{
			return false;
		}

		for (int32 i = 0; i < unitCount; ++i)
		{
			const UnitCatalogEntry& entry = catalog.entries[sortedUnitIndices[i]];
			const RectF cell = EditorCommandUnitCellRect(unitViewport, i, columns, editor.commandUnitListScroll);
			if ((cell.y + cell.h) < unitViewport.y || unitViewportBottom < cell.y)
			{
				continue;
			}

			if (!cell.leftClicked())
			{
				continue;
			}

			if (editor.commandEditorMode == 0)
			{
				Array<String>& ownerTags = action.ownerTags;
				const bool hasTag = ContainsOwnerTag(ownerTags, entry.unit_id);

				if (hasTag)
				{
					Array<String> filtered;
					for (const auto& value : ownerTags)
					{
						if (!EqualsOwnerTagIgnoreCase(value, entry.unit_id))
						{
							filtered << value;
						}
					}
					ownerTags = std::move(filtered);
				}
				else
				{
					ownerTags << entry.unit_id;
				}

				ownerTags = NormalizeOwnerTagsForEditor(ownerTags);
				action.ownerTag = ownerTags.isEmpty() ? U"" : ownerTags.front();
				action.tag = U"{}:{}"_fmt(action.ownerTag, action.id);
				editor.statusText = U"Command binding updated: {}"_fmt(action.name);
			}
			else if (editor.commandEditorMode == 1)
			{
				action.resultType = BuildActionResultType::Unit;
				Array<String> spawnTags = action.spawnTags;
				if (spawnTags.isEmpty() && !action.spawnTag.isEmpty())
				{
					spawnTags << action.spawnTag;
				}

				const bool allowMultipleSpawns = ActionOwnerIncludesFacilityUnit(action, catalog);
				const bool alreadySelected = ContainsOwnerTag(spawnTags, entry.unit_id);
				if (allowMultipleSpawns)
				{
					if (alreadySelected)
					{
						Array<String> filtered;
						for (const auto& value : spawnTags)
						{
							if (!EqualsOwnerTagIgnoreCase(value, entry.unit_id))
							{
								filtered << value;
							}
						}
						spawnTags = std::move(filtered);
					}
					else
					{
						spawnTags << entry.unit_id;
					}
				}
				else
				{
					spawnTags = alreadySelected ? spawnTags : Array<String>{ entry.unit_id };
				}

				action.spawnTags = std::move(spawnTags);
				RefreshActionSpawnSelection(action, defs);
				editor.statusText = U"Command spawn updated: {} -> {}"_fmt(
					action.name,
					action.spawnTags.isEmpty() ? U"(none)" : action.spawnTags.join(U", "));
			}

			editor.commandBindingsDirty = true;
			return true;
		}

		return false;
	}
}
