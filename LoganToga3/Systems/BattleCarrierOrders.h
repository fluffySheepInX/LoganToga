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

	inline int32 StoreNearbyUnitsInCarrier(BattleWorld& world, const DefinitionStores& defs, UnitId carrier, double storeRadiusPx, int32 maxStoreUnits)
	{
		if (!IsValidUnit(world, carrier) || carrier >= world.carriers.storedUnits.size())
		{
			return 0;
		}

		Array<UnitId>& stored = world.carriers.storedUnits[carrier];
		const int32 capacityLeft = (maxStoreUnits > 0) ? Max(0, maxStoreUnits - static_cast<int32>(stored.size())) : INT32_MAX;
		if (capacityLeft <= 0)
		{
			return 0;
		}

		const Vec2 carrierPos = world.units.position[carrier];
		const Faction faction = world.units.faction[carrier];
		const double radiusSq = Square(Max(0.0, storeRadiusPx));
		int32 storedCount = 0;

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (storedCount >= capacityLeft)
			{
				break;
			}
			if (unit == carrier || !IsValidUnit(world, unit))
			{
				continue;
			}
			if (world.units.faction[unit] != faction)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& unitDef = defs.units[world.units.defId[unit]];
			if (unitDef.role == UnitRole::Base || unitDef.role == UnitRole::Barrier)
			{
				continue;
			}
			if (carrierPos.distanceFromSq(world.units.position[unit]) > radiusSq)
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

	inline bool ReleaseStoredUnitsFromCarrier(BattleWorld& world, UnitId carrier, double releaseRadiusPx)
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
		const double radius = Max(24.0, releaseRadiusPx);
		for (size_t i = 0; i < stored.size(); ++i)
		{
			const UnitId unit = stored[i];
			if (unit >= world.units.size())
			{
				continue;
			}

			const double angle = Random(0.0, Math::TwoPi);
			const double distance = Random(24.0, radius);
			const Vec2 offset = Circular{ distance, angle };
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
		if (action.carrierAction == CarrierActionKind::Release)
		{
			return ReleaseStoredUnitsFromCarrier(world, builder, action.carrierRadiusPx);
		}

		const String actionId = action.id.isEmpty() ? action.tag.lowercased() : action.id.lowercased();
		const String category = action.category.lowercased();
		if (category == U"releaseall" || actionId.includes(U"releaseall"))
		{
			return ReleaseStoredUnitsFromCarrier(world, builder, action.carrierRadiusPx);
		}

		StoreNearbyUnitsInCarrier(world, defs, builder, action.carrierRadiusPx, action.carrierMaxUnits);
		return true;
	}
}
