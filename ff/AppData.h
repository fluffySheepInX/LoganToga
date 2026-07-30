# pragma once
# include "FormationState.h"

namespace ff
{
	enum class UnitEditorDefinitionKind : uint8
	{
		Unit,
		Enemy,
	};

	enum class UnitEditorReturnTarget : uint8
	{
		Formation,
		WaveEditor,
	};

	struct UnitEditorNavigationRequest
	{
		UnitEditorDefinitionKind definitionKind = UnitEditorDefinitionKind::Unit;
		UnitId unitId = UnitId::GuardPlayer;
		EnemyKind enemyKind = EnemyKind::Normal;
		UnitEditorReturnTarget returnTarget = UnitEditorReturnTarget::Formation;
	};
}

struct AppData
{
	ff::FormationSlots formationSlots = ff::MakeEmptyFormationSlots();
	Array<ff::FormationSlots> formationPresets = ff::MakeDefaultFormationPresets();
	Optional<ff::UnitId> selectedFormationUnit = ff::UnitId::GuardPlayer;
	Optional<ff::UnitEditorNavigationRequest> unitEditorNavigationRequest;
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
