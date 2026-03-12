#pragma once

#include "BattleSession.h"

namespace BattleSessionInternal
{
	[[nodiscard]] inline int32 MakeFormationColumnCount(const int32 count)
	{
		int32 columns = 1;
		while ((columns * columns) < count)
		{
			++columns;
		}

		return columns;
	}

	[[nodiscard]] inline Vec2 MakeFormationForward(const Array<int32>& unitIds, const BattleState& state, const Vec2& destination, const Vec2& facingDirection)
	{
		if (facingDirection.lengthSq() >= 1.0)
		{
			return facingDirection.normalized();
		}

		Vec2 center = Vec2::Zero();
		int32 count = 0;
		for (const auto unitId : unitIds)
		{
			if (const auto* unit = state.findUnit(unitId))
			{
				center += unit->position;
				++count;
			}
		}

		if (count <= 0)
		{
			return Vec2{ 1.0, 0.0 };
		}

		center /= count;
		Vec2 forward = (destination - center);
		if (forward.lengthSq() < 1.0)
		{
			return Vec2{ 1.0, 0.0 };
		}

		return forward.normalized();
	}

	[[nodiscard]] inline Array<Vec2> MakeClusterOffsets(const Array<int32>& unitIds, const Vec2& forward, const Vec2& right)
	{
		Array<Vec2> offsets;
		offsets.reserve(unitIds.size());

		const double spacing = 34.0;
		const double jitterAmount = 4.0;
		const int32 columns = MakeFormationColumnCount(unitIds.size());
		const int32 rows = (unitIds.size() + columns - 1) / columns;

		for (int32 i = 0; i < unitIds.size(); ++i)
		{
			const int32 row = (i / columns);
			const int32 column = (i % columns);
			const double x = (column - ((columns - 1) * 0.5)) * spacing;
			const double y = (row - ((rows - 1) * 0.5)) * spacing;
			const uint32 seed = static_cast<uint32>((unitIds[i] * 1103515245u) + 12345u);
			const double jitterX = ((((seed >> 0) & 0xFF) / 255.0) - 0.5) * jitterAmount;
			const double jitterY = ((((seed >> 8) & 0xFF) / 255.0) - 0.5) * jitterAmount;
			offsets << (right * (x + jitterX)) + (forward * (y + jitterY));
		}

		return offsets;
	}

	[[nodiscard]] inline Array<Vec2> MakeRowOffsets(const Array<int32>& unitIds, const Vec2& right)
	{
		Array<Vec2> offsets;
		offsets.reserve(unitIds.size());

		const double spacing = 32.0;
		for (int32 i = 0; i < unitIds.size(); ++i)
		{
			const double sideOffset = (i - ((unitIds.size() - 1) * 0.5)) * spacing;
			offsets << (right * sideOffset);
		}

		return offsets;
	}

	[[nodiscard]] inline Array<Vec2> MakeSquareOffsets(const Array<int32>& unitIds, const BattleState& state, const Vec2& forward, const Vec2& right)
	{
		Array<Vec2> offsets;
		for (int32 i = 0; i < unitIds.size(); ++i)
		{
			offsets << Vec2::Zero();
		}

		if (unitIds.isEmpty())
		{
			return offsets;
		}

		Array<Array<int32>> groups;
		Array<int32> groupedSquadIds;
		for (const auto unitId : unitIds)
		{
			if (const auto* unit = state.findUnit(unitId))
			{
				if (unit->squadId)
				{
					int32 groupIndex = -1;
					for (int32 i = 0; i < groupedSquadIds.size(); ++i)
					{
						if (groupedSquadIds[i] == *unit->squadId)
						{
							groupIndex = i;
							break;
						}
					}

					if (groupIndex < 0)
					{
						groupIndex = groups.size();
						groups << Array<int32>{};
						groupedSquadIds << *unit->squadId;
					}

					groups[groupIndex] << unitId;
				}
				else
				{
					groups << Array<int32>{ unitId };
				}
			}
		}

		if (groups.isEmpty())
		{
			return offsets;
		}

		int32 maxGroupSize = 1;
		for (const auto& group : groups)
		{
			if (group.size() > maxGroupSize)
			{
				maxGroupSize = group.size();
			}
		}

		const double unitSpacing = 32.0;
		const double groupSpacing = unitSpacing * Max(maxGroupSize + 1, 3);
		const int32 columns = MakeFormationColumnCount(groups.size());
		const int32 rows = (groups.size() + columns - 1) / columns;

		for (int32 groupIndex = 0; groupIndex < groups.size(); ++groupIndex)
		{
			const auto& group = groups[groupIndex];
			const int32 row = (groupIndex / columns);
			const int32 column = (groupIndex % columns);
			const Vec2 groupCenter
			{
				right * ((column - ((columns - 1) * 0.5)) * groupSpacing)
				+ forward * ((row - ((rows - 1) * 0.5)) * groupSpacing)
			};

			for (int32 memberIndex = 0; memberIndex < group.size(); ++memberIndex)
			{
				const double sideOffset = (memberIndex - ((group.size() - 1) * 0.5)) * unitSpacing;
				const Vec2 offset = groupCenter + (right * sideOffset);

				for (int32 unitIndex = 0; unitIndex < unitIds.size(); ++unitIndex)
				{
					if (unitIds[unitIndex] == group[memberIndex])
					{
						offsets[unitIndex] = offset;
						break;
					}
				}
			}
		}

		return offsets;
	}

	[[nodiscard]] inline Array<Vec2> MakeFormationOffsets(const Array<int32>& unitIds, const BattleState& state, const Vec2& destination, const FormationType formation, const Vec2& facingDirection)
	{
		if (unitIds.isEmpty())
		{
			return {};
		}

		const Vec2 forward = MakeFormationForward(unitIds, state, destination, facingDirection);
		const Vec2 right{ -forward.y, forward.x };

		switch (formation)
		{
		case FormationType::Square:
			return MakeSquareOffsets(unitIds, state, forward, right);
		case FormationType::Column:
			return MakeRowOffsets(unitIds, right);
		case FormationType::Line:
		default:
			return MakeClusterOffsets(unitIds, forward, right);
		}
	}
}
