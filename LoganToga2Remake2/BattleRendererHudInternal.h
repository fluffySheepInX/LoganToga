#pragma once

#include <unordered_map>

#include "BattleRenderer.h"
#include "BattleCommandUi.h"

namespace BattleRendererHudInternal
{
	[[nodiscard]] String GetCommandIconGlyph(UnitArchetype archetype);
	[[nodiscard]] ColorF GetCommandIconColor(UnitArchetype archetype);
	[[nodiscard]] String GetCommandKindLabel(CommandKind kind);
	[[nodiscard]] ColorF GetCommandAvailabilityColor(const CommandIconEntry& command);
	void DrawCommandSection(const CommandPanelLayout& layout, const GameData& gameData);

	struct QueueDisplayTarget
	{
		const BuildingState* building = nullptr;
		const UnitState* unit = nullptr;
	};

	[[nodiscard]] Optional<QueueDisplayTarget> FindQueueDisplayTarget(const BattleState& state, const std::unordered_map<int32, const BuildingState*>& buildingsByUnitId);
	void DrawQueuePanel(const RectF& panelRect, const QueueDisplayTarget& target, const GameData& gameData);
}
