# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
	inline UnitId GetSelectedUnit(const BattleWorld& world)
	{
		return IsValidUnit(world, world.selection.selected) ? world.selection.selected : InvalidUnitId;
	}

	inline const Array<UnitId>& GetSelectedUnits(const BattleWorld& world)
	{
		return world.selection.selectedUnits;
	}

	inline bool IsUnitSelected(const BattleWorld& world, UnitId unit)
	{
		return GetSelectedUnit(world) == unit || world.selection.selectedUnits.contains(unit);
	}

	inline RectF MakeDragSelectionRect(const Vec2& start, const Vec2& current)
	{
		return RectF{ Min(start.x, current.x), Min(start.y, current.y), Abs(current.x - start.x), Abs(current.y - start.y) };
	}

	inline bool IsDragSelectionActive(const BattleWorld& world)
	{
		return world.selection.areaDragging
			&& MakeDragSelectionRect(world.selection.areaDragStartScreen, world.selection.areaDragCurrentScreen).size.lengthSq() >= 36.0;
	}

	inline bool HasSelectedUnit(const BattleWorld& world)
	{
		return GetSelectedUnit(world) != InvalidUnitId;
	}

	inline Array<BuildActionUiState> CollectVisibleBuildActionsForSelectedUnit(const BattleWorld& world, const DefinitionStores& defs)
	{
		return CollectVisibleBuildActions(world, defs, GetSelectedUnit(world));
	}

	inline void SelectUnit(BattleWorld& world, UnitId unit)
	{
		world.selection.selected = IsValidUnit(world, unit) ? unit : InvalidUnitId;
        world.selection.selectedUnits.clear();
		if (world.selection.selected != InvalidUnitId)
		{
			world.selection.selectedUnits << world.selection.selected;
		}
	}

	inline void SelectUnits(BattleWorld& world, const Array<UnitId>& units)
	{
		world.selection.selectedUnits.clear();
		for (const UnitId unit : units)
		{
			if (IsValidUnit(world, unit) && !world.selection.selectedUnits.contains(unit))
			{
				world.selection.selectedUnits << unit;
			}
		}
		world.selection.selected = world.selection.selectedUnits.isEmpty() ? InvalidUnitId : world.selection.selectedUnits.front();
	}

	inline void ClearSelection(BattleWorld& world)
	{
		world.selection.selected = InvalidUnitId;
        world.selection.selectedUnits.clear();
       world.selection.formationPlacementActive = false;
		world.selection.formationUnits.clear();
	}

	inline void ClearSelectionIfUnitSelected(BattleWorld& world, UnitId unit)
	{
		if (GetSelectedUnit(world) == unit)
		{
			ClearSelection(world);
		}
	}
}
