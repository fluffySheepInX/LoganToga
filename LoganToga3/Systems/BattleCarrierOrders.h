# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
	inline void RemoveStoredUnitFromAllCarriers(BattleWorld& world, UnitId unit)
	{
		for (auto& stored : world.carriers.storedUnits)
		{
			stored.remove(unit);
		}
	}

	inline int32 StoreNearbyUnitsInCarrier(BattleWorld& world, const DefinitionStores& defs, UnitId carrier)
	{
		if (!IsValidUnit(world, carrier) || carrier >= world.carriers.storedUnits.size())
		{
			return 0;
		}

		Array<UnitId>& stored = world.carriers.storedUnits[carrier];
		const Vec2 carrierPos = world.units.position[carrier];
		const Faction faction = world.units.faction[carrier];
		int32 storedCount = 0;

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (unit == carrier || !IsValidUnit(world, unit))
			{
				continue;
			}
			if (world.units.faction[unit] != faction)
			{
				continue;
			}

			const UnitDef& unitDef = defs.units[world.units.defId[unit]];
			if (unitDef.role == UnitRole::Base)
			{
				continue;
			}
			if (carrierPos.distanceFromSq(world.units.position[unit]) > Square(84.0))
			{
				continue;
			}

			RemoveStoredUnitFromAllCarriers(world, unit);
			stored << unit;
			SetUnitAlive(world, unit, false);
			SetUnitIdle(world, unit);
			SetUnitTargetPosition(world, unit, carrierPos);
			ClearUnitAttackTarget(world, unit);
			if (world.selection.selected == unit || world.selection.selectedUnits.contains(unit))
			{
				ClearSelection(world);
			}
			++storedCount;
		}

		return storedCount;
	}

	inline bool ReleaseStoredUnitsFromCarrier(BattleWorld& world, UnitId carrier)
	{
		if (!IsValidUnit(world, carrier) || carrier >= world.carriers.storedUnits.size())
		{
			return false;
		}

		Array<UnitId>& stored = world.carriers.storedUnits[carrier];
		if (stored.isEmpty())
		{
			return false;
		}

		const Vec2 carrierPos = world.units.position[carrier];
		for (size_t i = 0; i < stored.size(); ++i)
		{
			const UnitId unit = stored[i];
			if (unit >= world.units.size())
			{
				continue;
			}

			const double angle = (Math::TwoPi * static_cast<double>(i)) / Max(1.0, static_cast<double>(stored.size()));
			const Vec2 offset = Circular{ 42.0 + 10.0 * static_cast<double>(i / 6), angle };
			SetUnitAlive(world, unit, true);
			world.units.position[unit] = carrierPos + offset;
			SetUnitTargetPosition(world, unit, carrierPos + offset);
			SetUnitIdle(world, unit);
			ClearUnitAttackTarget(world, unit);
		}

		stored.clear();
		return true;
	}

	inline bool TryExecuteCarrierAction(BattleWorld& world, const DefinitionStores& defs, UnitId builder, const BuildActionDef& action)
	{
		const String actionId = action.id.isEmpty() ? action.tag.lowercased() : action.id.lowercased();
		const String category = action.category.lowercased();
		if (category == U"releaseall" || actionId.includes(U"releaseall"))
		{
			return ReleaseStoredUnitsFromCarrier(world, builder);
		}

		StoreNearbyUnitsInCarrier(world, defs, builder);
		return true;
	}
}
