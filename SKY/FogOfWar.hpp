# pragma once
# include <Siv3D.hpp>
# include "MapData.hpp"
# include "SpawnedSapper.hpp"

namespace SkyAppSupport
{
	enum class FogVisibility
	{
		Hidden,
		Explored,
		Visible,
	};

		enum class FogOverlayDarknessCurve
		{
			Linear,
			Soft,
			Strong,
		};

	struct FogOfWarSettings
	{
		bool enabled = true;
		double defaultUnitVisionRange = 8.0;
		double baseVisionRange = 16.0;
		double ownedResourceVisionRange = 8.0;
		double millVisionRange = 10.0;
            FogOverlayDarknessCurve overlayDarknessCurve = FogOverlayDarknessCurve::Linear;
      double overlayDarkness = 0.72;
		double overlayHeight = 3.6;
		int32 mapPaddingCells = 6;
	};

	struct FogOfWarCellState
	{
		bool explored = false;
		double visibleStrength = 0.0;
	};

	struct FogOfWarState
	{
		HashTable<int64, FogOfWarCellState> cells;
		Rect coverageBounds{ 0, 0, 1, 1 };
		uint64 revision = 0;
		bool enabled = true;
	};

	void ResetFogOfWar(FogOfWarState& fogOfWar);
	void UpdateFogOfWar(FogOfWarState& fogOfWar,
		const FogOfWarSettings& settings,
		const MapData& mapData,
		const Array<MainSupport::SpawnedSapper>& spawnedSappers,
		const Array<MainSupport::ResourceAreaState>& resourceAreaStates);
	[[nodiscard]] FogVisibility GetFogVisibilityAt(const FogOfWarState& fogOfWar, const Point& cell);
	[[nodiscard]] FogVisibility GetFogVisibilityAt(const FogOfWarState& fogOfWar, const Vec3& position);
	[[nodiscard]] bool IsFogVisibleAt(const FogOfWarState& fogOfWar, const Point& cell);
	[[nodiscard]] bool IsFogVisibleAt(const FogOfWarState& fogOfWar, const Vec3& position);
	[[nodiscard]] bool IsFogExploredAt(const FogOfWarState& fogOfWar, const Point& cell);
	[[nodiscard]] bool IsFogExploredAt(const FogOfWarState& fogOfWar, const Vec3& position);
}
