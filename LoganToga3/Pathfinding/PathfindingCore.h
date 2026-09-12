#pragma once
# include <cstddef>
# include <cstdint>
# include <limits>
# include <queue>
# include <vector>

namespace LT3::PathfindingCore
{
	struct Cell
	{
		int32_t x = 0;
		int32_t y = 0;

		[[nodiscard]] constexpr bool operator==(const Cell& rhs) const
		{
			return x == rhs.x && y == rhs.y;
		}
	};

	struct GridView
	{
		int32_t width = 0;
		int32_t height = 0;
		const uint8_t* blocked = nullptr;
	};

	enum class PathSearchStatus : uint8_t
	{
		Found,
		NoPath,
		InvalidArgument,
		BufferTooSmall,
	};

	// グリッドが有効で、そのセル数をuint32_tで安全に表せるか判定する。
	[[nodiscard]] inline bool TryGetCellCount(const GridView& grid, uint32_t& cellCount)
	{
		if (grid.width <= 0 || grid.height <= 0 || grid.blocked == nullptr)
		{
			return false;
		}

		const uint64_t total = static_cast<uint64_t>(grid.width) * static_cast<uint64_t>(grid.height);
		if (total > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())
			|| total > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
		{
			return false;
		}

		cellCount = static_cast<uint32_t>(total);
		return true;
	}

	// セルがグリッド範囲内か判定する。
	[[nodiscard]] inline bool IsInBounds(const GridView& grid, const Cell& cell)
	{
		return cell.x >= 0 && cell.y >= 0 && cell.x < grid.width && cell.y < grid.height;
	}

	// 範囲内セルを一次元インデックスへ変換する。
	[[nodiscard]] inline uint32_t GetIndex(const GridView& grid, const Cell& cell)
	{
		return static_cast<uint32_t>(cell.y * grid.width + cell.x);
	}

	// 一次元インデックスをセルへ変換する。
	[[nodiscard]] inline Cell GetCell(const GridView& grid, const uint32_t index)
	{
		return Cell{
			static_cast<int32_t>(index % static_cast<uint32_t>(grid.width)),
			static_cast<int32_t>(index / static_cast<uint32_t>(grid.width))
		};
	}

	// セルが範囲内かつ通行可能か判定する。
	[[nodiscard]] inline bool IsPassable(const GridView& grid, const Cell& cell)
	{
		return IsInBounds(grid, cell) && grid.blocked[GetIndex(grid, cell)] == 0;
	}

	// 指定セルから右、左、下、上の順で探索し、最寄りの通行可能セルを返す。
	[[nodiscard]] inline bool TryFindNearestPassableCell(const GridView& grid, const Cell& center, Cell& result)
	{
		uint32_t cellCount = 0;
		if (!TryGetCellCount(grid, cellCount) || !IsInBounds(grid, center))
		{
			return false;
		}

		if (IsPassable(grid, center))
		{
			result = center;
			return true;
		}

		std::vector<uint8_t> visited(cellCount, 0u);
		std::vector<Cell> cells;
		cells.reserve(cellCount);
		cells.push_back(center);
		visited[GetIndex(grid, center)] = 1u;

		constexpr Cell offsets[4] = {
			Cell{ 1, 0 },
			Cell{ -1, 0 },
			Cell{ 0, 1 },
			Cell{ 0, -1 },
		};

		size_t head = 0;
		while (head < cells.size())
		{
			const Cell cell = cells[head++];
			for (const Cell& offset : offsets)
			{
				const Cell next{ cell.x + offset.x, cell.y + offset.y };
				if (!IsInBounds(grid, next))
				{
					continue;
				}

				const uint32_t index = GetIndex(grid, next);
				if (visited[index] != 0u)
				{
					continue;
				}

				visited[index] = 1u;
				if (IsPassable(grid, next))
				{
					result = next;
					return true;
				}

				cells.push_back(next);
			}
		}

		return false;
	}

	// 4方向・直交コスト10のA*で、開始セルから目標セルまでのセル列を出力する。
	[[nodiscard]] inline PathSearchStatus FindPathByAStar(const GridView& grid, const Cell& start, const Cell& goal, Cell* outputCells, const uint32_t outputCapacity, uint32_t& outputCount)
	{
		outputCount = 0;
		uint32_t cellCount = 0;
		if (!TryGetCellCount(grid, cellCount) || !IsInBounds(grid, start) || !IsInBounds(grid, goal)
			|| (outputCells == nullptr && outputCapacity != 0u))
		{
			return PathSearchStatus::InvalidArgument;
		}

		if (start == goal)
		{
			if (outputCapacity == 0u)
			{
				return PathSearchStatus::BufferTooSmall;
			}

			outputCells[0] = start;
			outputCount = 1;
			return PathSearchStatus::Found;
		}

		struct OpenEntry
		{
			int32_t fScore = 0;
			uint32_t index = 0;

			bool operator<(const OpenEntry& rhs) const
			{
				return fScore > rhs.fScore;
			}
		};

		const auto heuristic = [](const Cell& a, const Cell& b)
		{
			const int32_t dx = (a.x >= b.x) ? (a.x - b.x) : (b.x - a.x);
			const int32_t dy = (a.y >= b.y) ? (a.y - b.y) : (b.y - a.y);
			return (dx + dy) * 10;
		};

		const uint32_t startIndex = GetIndex(grid, start);
		const uint32_t goalIndex = GetIndex(grid, goal);
		std::vector<int32_t> gScore(cellCount, std::numeric_limits<int32_t>::max());
		std::vector<uint32_t> cameFrom(cellCount, std::numeric_limits<uint32_t>::max());
		std::vector<uint8_t> closed(cellCount, 0u);
		std::priority_queue<OpenEntry> open;
		gScore[startIndex] = 0;
		open.push(OpenEntry{ heuristic(start, goal), startIndex });

		constexpr Cell offsets[4] = {
			Cell{ 1, 0 },
			Cell{ -1, 0 },
			Cell{ 0, 1 },
			Cell{ 0, -1 },
		};

		bool found = false;
		while (!open.empty())
		{
			const OpenEntry currentEntry = open.top();
			open.pop();

			const uint32_t current = currentEntry.index;
			if (closed[current] != 0u)
			{
				continue;
			}

			if (current == goalIndex)
			{
				found = true;
				break;
			}

			closed[current] = 1u;
			const Cell currentCell = GetCell(grid, current);
			for (const Cell& offset : offsets)
			{
				const Cell nextCell{ currentCell.x + offset.x, currentCell.y + offset.y };
				if (!IsInBounds(grid, nextCell))
				{
					continue;
				}

				if (!IsPassable(grid, nextCell) && nextCell != goal)
				{
					continue;
				}

				const uint32_t next = GetIndex(grid, nextCell);
				if (closed[next] != 0u)
				{
					continue;
				}

				const int32_t nextG = gScore[current] + 10;
				if (nextG >= gScore[next])
				{
					continue;
				}

				cameFrom[next] = current;
				gScore[next] = nextG;
				const int32_t nextF = nextG + heuristic(nextCell, goal);
				open.push(OpenEntry{ nextF, next });
			}
		}

		if (!found)
		{
			return PathSearchStatus::NoPath;
		}

		std::vector<Cell> reversed;
		reversed.reserve(cellCount);
		uint32_t current = goalIndex;
		while (true)
		{
			reversed.push_back(GetCell(grid, current));
			if (current == startIndex)
			{
				break;
			}

			const uint32_t parent = cameFrom[current];
			if (parent == std::numeric_limits<uint32_t>::max())
			{
				return PathSearchStatus::NoPath;
			}

			current = parent;
		}

		if (reversed.size() > outputCapacity)
		{
			return PathSearchStatus::BufferTooSmall;
		}

		outputCount = static_cast<uint32_t>(reversed.size());
		for (uint32_t i = 0; i < outputCount; ++i)
		{
			outputCells[i] = reversed[outputCount - i - 1u];
		}

		return PathSearchStatus::Found;
	}
}
