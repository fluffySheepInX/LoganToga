# pragma once
# include "GameConstants.h"

namespace ff
{
	enum class TimeOfDay
	{
		Day,
		Evening,
		Night,
	};

	inline constexpr size_t FormationSlotCount = 8;
	inline constexpr size_t FormationPresetCount = 3;
	inline constexpr int32 FormationSaveSchemaVersion = 3;
	using FormationSlotUnit = Optional<UnitId>;
	using FormationSlots = Array<FormationSlotUnit>;

	struct FormationEditState
	{
		FormationSlots slots = FormationSlots(FormationSlotCount);
		Optional<UnitId> selectedUnit = UnitId::GuardPlayer;
	};

	[[nodiscard]] inline FormationSlots MakeEmptyFormationSlots()
	{
		return FormationSlots(FormationSlotCount);
	}

	[[nodiscard]] inline Array<FormationSlots> MakeDefaultFormationPresets()
	{
		Array<FormationSlots> presets;
		presets.reserve(FormationPresetCount);

		for (size_t index = 0; index < FormationPresetCount; ++index)
		{
			presets << MakeEmptyFormationSlots();
		}

		return presets;
	}

	[[nodiscard]] inline int32 CountAssignedFormationUnits(const FormationSlots& slots)
	{
		return static_cast<int32>(std::count_if(slots.begin(), slots.end(), [](const auto& slot)
			{
				return static_cast<bool>(slot);
			}));
	}

	[[nodiscard]] inline bool HasAssignedFormationSlot(const FormationSlots& slots)
	{
		return std::any_of(slots.begin(), slots.end(), [](const auto& slot)
			{
				return static_cast<bool>(slot);
			});
	}

	[[nodiscard]] inline FormationSlots MakeRandomFormationSlots()
	{
		const auto& unitTypes = GetAvailableUnitIds();
		FormationSlots slots = MakeEmptyFormationSlots();

		for (auto& slot : slots)
		{
			slot = unitTypes[Random(unitTypes.size() - 1)];
		}

		return slots;
	}

	inline void ClearFormationSlots(FormationSlots& slots)
	{
		for (auto& slot : slots)
		{
			slot.reset();
		}
	}

	[[nodiscard]] inline bool AssignSelectedFormationUnit(FormationEditState& state, const size_t index)
	{
		if ((index >= state.slots.size()) || (not state.selectedUnit))
		{
			return false;
		}

		state.slots[index] = state.selectedUnit;
		return true;
	}

	[[nodiscard]] inline bool ClearFormationSlot(FormationSlots& slots, const size_t index)
	{
		if (index >= slots.size())
		{
			return false;
		}

		slots[index].reset();
		return true;
	}

	[[nodiscard]] inline String GetFormationSavePath()
	{
		return U"save/formation.toml";
	}
}
