# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "SelectionSystem.h"
# include "BattleInputTypes.h"
# include "../UI/MapEditor.h"
# include "../UI/QuarterView.h"

namespace LT3
{
	inline Array<UnitId> PickUnitsInScreenRect(const BattleWorld& world, const DefinitionStores& defs, const RectF& screenRect, Faction faction)
	{
		Array<UnitId> units;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != faction)
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			const Vec2 pos = ToQuarterViewportScreen(world.units.position[unit]);
			if (screenRect.intersects(Circle{ pos, Max(6.0, UnitSelectionRadius(def) * GetQuarterViewCameraScale()) }))
			{
				units << unit;
			}
		}

		return units;
	}

	inline void UpdateAreaSelectionDrag(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, BattleInputIntent& intent)
	{
		if (world.selection.actionPlacementActive)
		{
			world.selection.areaDragging = false;
			return;
		}

		if (IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
			world.selection.areaDragging = false;
			return;
		}

		const Vec2 screenMouse = Cursor::PosF();
		if (MouseL.down())
		{
			world.selection.areaDragging = true;
			world.selection.areaDragStartScreen = screenMouse;
			world.selection.areaDragCurrentScreen = screenMouse;
		}
		else if (world.selection.areaDragging && MouseL.pressed())
		{
			world.selection.areaDragCurrentScreen = screenMouse;
		}
		else if (world.selection.areaDragging && MouseL.up())
		{
			world.selection.areaDragCurrentScreen = screenMouse;
			const RectF rect = MakeDragSelectionRect(world.selection.areaDragStartScreen, world.selection.areaDragCurrentScreen);
			if (rect.size.lengthSq() >= 36.0)
			{
				Array<UnitId> pickedUnits = PickUnitsInScreenRect(world, defs, rect, Faction::Player);
				if (pickedUnits.isEmpty())
				{
					pickedUnits = PickUnitsInScreenRect(world, defs, rect, Faction::Enemy);
				}
				intent.commands << BattleInputCommand{ BattleInputCommandType::SelectUnit, InvalidUnitId, pickedUnits, Vec2{ 0, 0 }, Vec2{ 0, 0 }, InvalidBuildActionDefId, {}, false, false };
			}
			world.selection.areaDragging = false;
		}
	}

	inline void ResetFormationPlacementPreview(BattleWorld& world)
	{
		world.selection.formationPlacementActive = false;
		world.selection.formationUnits.clear();
	}

	inline void UpdateFormationPlacementPreview(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& worldMouse, BattleInputIntent& intent)
	{
		if (world.selection.actionPlacementActive)
		{
			ResetFormationPlacementPreview(world);
			return;
		}

		const bool canStart = IsCursorOnMapArea(mapEditor, world, defs)
			&& !IsCursorOnBuildCommandUi(world, defs, mapEditor)
			&& !GetSelectedUnits(world).isEmpty();

		if (MouseR.down())
		{
			if (canStart)
			{
				world.selection.formationPlacementActive = true;
				world.selection.formationUnits = GetSelectedUnits(world);
				world.selection.formationDestinationWorld = worldMouse;
				world.selection.formationCurrentWorld = worldMouse;
			}
			else
			{
				ResetFormationPlacementPreview(world);
			}
			return;
		}

		if (!world.selection.formationPlacementActive)
		{
			return;
		}

		if (MouseR.pressed())
		{
			world.selection.formationCurrentWorld = worldMouse;
			return;
		}

		if (MouseR.up())
		{
			world.selection.formationCurrentWorld = worldMouse;
			const Array<UnitId> previewUnits = world.selection.formationUnits;
			if (!previewUnits.isEmpty())
			{
				if (previewUnits.size() == 1)
				{
					intent.commands << BattleInputCommand{ BattleInputCommandType::MoveUnit, previewUnits.front(), {}, world.selection.formationDestinationWorld, Vec2{ 0, 0 }, InvalidBuildActionDefId, {}, false, false };
				}
				else
				{
					intent.commands << BattleInputCommand{
						BattleInputCommandType::MoveUnit,
						previewUnits.front(),
						previewUnits,
						world.selection.formationDestinationWorld,
						ResolveFormationFacingDirection(world, previewUnits, world.selection.formationDestinationWorld, world.selection.formationCurrentWorld),
						InvalidBuildActionDefId,
						{},
						true,
						false
					};
				}
			}
			ResetFormationPlacementPreview(world);
			return;
		}

		ResetFormationPlacementPreview(world);
	}
}
