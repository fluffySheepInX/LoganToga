#include "MapEditScene.h"
#include "SceneTransition.h"

String MapEditScene::resolveMapConfigPath(const String& battleConfigPath)
{
	const TOMLReader rootToml{ battleConfigPath };
	if (!rootToml)
	{
		throw Error{ U"Failed to load battle config: " + battleConfigPath };
	}

	const String mapSource = rootToml[U"sources"][U"map"].get<String>();
	return ResolveBattleConfigSourcePath(battleConfigPath, mapSource);
}

void MapEditScene::reloadConfig()
{
	m_config = LoadBattleConfig(m_battleConfigPath);
	getData().baseBattleConfig = m_config;
	m_selection = {};
	m_dragOffset.reset();
	m_statusMessage = U"Reloaded map config";
}

void MapEditScene::saveMap()
{
	String output;

	for (const auto& placement : m_config.initialUnits)
	{
		output += U"[[initial_units]]\n";
		output += U"owner = \"{}\"\n"_fmt(toOwnerTomlString(placement.owner));
		output += U"archetype = \"{}\"\n"_fmt(toUnitArchetypeTomlString(placement.archetype));
		output += U"x = {:.1f}\n"_fmt(placement.position.x);
		output += U"y = {:.1f}\n\n"_fmt(placement.position.y);
	}

	for (const auto& obstacle : m_config.obstacles)
	{
		output += U"[[obstacles]]\n";
		output += U"label = \"{}\"\n"_fmt(obstacle.label);
		output += U"x = {:.1f}\n"_fmt(obstacle.rect.x);
		output += U"y = {:.1f}\n"_fmt(obstacle.rect.y);
		output += U"width = {:.1f}\n"_fmt(obstacle.rect.w);
		output += U"height = {:.1f}\n"_fmt(obstacle.rect.h);
		output += U"blocks_movement = {}\n\n"_fmt(obstacle.blocksMovement ? U"true" : U"false");
	}

	for (const auto& resourcePoint : m_config.resourcePoints)
	{
		output += U"[[resource_points]]\n";
		output += U"label = \"{}\"\n"_fmt(resourcePoint.label);
		output += U"owner = \"{}\"\n"_fmt(toOwnerTomlString(resourcePoint.owner));
		output += U"x = {:.1f}\n"_fmt(resourcePoint.position.x);
		output += U"y = {:.1f}\n"_fmt(resourcePoint.position.y);
		output += U"radius = {:.1f}\n"_fmt(resourcePoint.radius);
		output += U"income_amount = {}\n"_fmt(resourcePoint.incomeAmount);
		output += U"capture_time = {:.1f}\n\n"_fmt(resourcePoint.captureTime);
	}

	{
		s3d::TextWriter writer{ m_mapConfigPath };
		if (!writer)
		{
			throw Error{ U"Failed to open map config for write: " + m_mapConfigPath };
		}

		writer.write(output);
	}

	reloadConfig();
	m_statusMessage = U"Saved battle_map.toml";
}

void MapEditScene::startTestPlay()
{
	auto& data = getData();
	data.baseBattleConfig = m_config;
	BeginNewRun(data.runState, true);
	ResetBonusRoomSceneState(data.bonusRoomProgress);
	SaveContinueRun(data, ContinueResumeScene::Battle);
	RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}
