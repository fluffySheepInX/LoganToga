# pragma once
# include "FormationState.h"

struct AppData
{
	ff::FormationSlots formationSlots = ff::MakeEmptyFormationSlots();
	Array<ff::FormationSlots> formationPresets = ff::MakeDefaultFormationPresets();
	Optional<ff::UnitId> selectedFormationUnit = ff::UnitId::GuardPlayer;
	Optional<ff::EnemyKind> selectedEnemyKind = ff::EnemyKind::Normal;
	bool editEnemyDefinitions = false;
	bool unitEditorReturnToWaveEditor = false;
	ff::SummonDiscountTraitConfig summonDiscountTraits = ff::MakeDefaultSummonDiscountTraitConfig();
	ff::TimeOfDay timeOfDay = ff::TimeOfDay::Day;
};

[[nodiscard]] inline ff::FormationEditState MakeFormationEditState(const AppData& data)
{
	return ff::FormationEditState{ data.formationSlots, data.selectedFormationUnit };
}

inline void ApplyFormationEditState(AppData& data, const ff::FormationEditState& state)
{
	data.formationSlots = state.slots;
	data.selectedFormationUnit = state.selectedUnit;
}
