# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	void SpawnSapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, const MapData& mapData, const SapperUnitType unitType)
	{
		const size_t sapperIndex = spawnedSappers.size();
		const Vec3 startPosition = spawnPosition.movedBy(2.4, 0, 2.2);
		const Vec3 targetPosition = GetSapperPopTargetPosition(rallyPoint, sapperIndex);
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = startPosition,
			.position = startPosition,
			.targetPosition = startPosition,
			.destinationPosition = startPosition,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
			.moveDuration = 0.0,
			.facingYaw = SapperInternal::ToSapperYaw((targetPosition - startPosition), BirdDisplayYaw),
			.team = UnitTeam::Player,
			.unitType = unitType,
			.movementType = MovementType::Infantry,
		};
		ApplyUnitParameters(spawnedSappers.back(), MakeDefaultUnitParameters(UnitTeam::Player, unitType));
		SetSpawnedSapperTarget(spawnedSappers.back(), targetPosition, mapData, ModelHeightSettings{});
	}

	void SpawnEnemySapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& position, const double facingYaw, const SapperUnitType unitType)
	{
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = position,
			.position = position,
			.targetPosition = position,
			.destinationPosition = position,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
			.moveDuration = 0.0,
			.facingYaw = facingYaw,
			.team = UnitTeam::Enemy,
			.unitType = unitType,
			.movementType = MovementType::Infantry,
		};
		ApplyUnitParameters(spawnedSappers.back(), MakeDefaultUnitParameters(UnitTeam::Enemy, unitType));
	}
}
