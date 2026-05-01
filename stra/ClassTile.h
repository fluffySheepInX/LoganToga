#pragma once

using TileIndex = int32;
using TerrainId = int32;
using PlayerId = int32;
using CityId = int32;
using UnitId = int64;
using EventId = int32;

static constexpr TileIndex InvalidTileIndex = -1;
static constexpr TerrainId InvalidTerrainId = -1;
static constexpr PlayerId InvalidPlayerId = -1;
static constexpr CityId InvalidCityId = -1;
static constexpr UnitId InvalidUnitId = -1;

enum class FogState : uint8
{
	Unseen = 0,
	Seen = 1,
	Explored = 2,
};

enum TileFlags : uint8
{
	Passable = 1 << 0,
	Buildable = 1 << 1,
	Road = 1 << 2,
	Fortified = 1 << 3,
};

class ClassTile
{
public:
	TileIndex index = InvalidTileIndex;
	TerrainId terrain = InvalidTerrainId;
	PlayerId owner = InvalidPlayerId;
	Optional<CityId> city;

	// Optional<Array> は思想としてOK。重くなるなら外出し候補。
	Optional<Array<UnitId>> occupants;

	FogState fogState = FogState::Unseen;
	uint16 control = 0;
	uint8 flags = 0;

	Optional<Array<EventId>> effects;

	Array<EventId>& getEffects()
	{
		if (!effects) effects.emplace();
		return *effects;
	}
	Array<UnitId>& getOccupants()
	{
		if (!occupants) occupants.emplace();
		return *occupants;
	}

	// 読み取り用途（constで使える）
	const Array<EventId>& effectsOrEmpty() const
	{
		static const Array<EventId> empty{};
		return effects ? *effects : empty;
	}
	const Array<UnitId>& occupantsOrEmpty() const
	{
		static const Array<UnitId> empty{};
		return occupants ? *occupants : empty;
	}

	// フラグ操作を関数化すると事故が減る
	bool isPassable() const { return (flags & TileFlags::Passable) != 0; }
	bool isBuildable() const { return (flags & TileFlags::Buildable) != 0; }
};
