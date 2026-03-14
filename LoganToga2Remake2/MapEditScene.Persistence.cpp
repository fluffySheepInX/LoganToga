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

Array<MapEditScene::EditableMapEntry> MapEditScene::loadEditableMaps(const String& battleConfigPath)
{
	Array<EditableMapEntry> editableMaps;

	const TOMLReader rootToml{ battleConfigPath };
	if (!rootToml)
	{
		throw Error{ U"Failed to load battle config: " + battleConfigPath };
	}

	const String baseMapSource = rootToml[U"sources"][U"map"].get<String>();
	editableMaps << EditableMapEntry{
		ResolveBattleConfigSourcePath(battleConfigPath, baseMapSource),
		U"Base: {}"_fmt(FileSystem::BaseName(baseMapSource))
	};

	if (const auto progressionSource = rootToml[U"sources"][U"progression"].getOpt<String>())
	{
		const String progressionPath = ResolveBattleConfigSourcePath(battleConfigPath, *progressionSource);
		const TOMLReader progressionToml{ progressionPath };
		if (!progressionToml)
		{
			throw Error{ U"Failed to load battle progression config: " + progressionPath };
		}

		for (const auto& table : progressionToml[U"enemy_progression"].tableArrayView())
		{
			const auto mapSource = table[U"map_source"].getOpt<String>();
			if (!mapSource)
			{
				continue;
			}

			const String mapPath = ResolveBattleConfigSourcePath(progressionPath, *mapSource);
			const bool alreadyAdded = editableMaps.any([&mapPath](const EditableMapEntry& entry)
			{
				return entry.mapConfigPath == mapPath;
			});
			if (alreadyAdded)
			{
				continue;
			}

			editableMaps << EditableMapEntry{
				mapPath,
				U"Battle {}: {}"_fmt(table[U"battle"].getOr<int32>(0), FileSystem::BaseName(*mapSource))
			};
		}
	}

	return editableMaps;
}

void MapEditScene::reloadConfig()
{
	m_config = LoadBattleConfig(m_battleConfigPath);

	if (m_mapConfigPath != resolveMapConfigPath(m_battleConfigPath))
	{
		const TOMLReader mapToml{ m_mapConfigPath };
		if (!mapToml)
		{
			throw Error{ U"Failed to load map config: " + m_mapConfigPath };
		}

		LoadBattleMapConfig(m_config, mapToml);
	}

	m_selection = {};
	m_dragOffset.reset();
	m_statusMessage = U"Reloaded {}"_fmt(getCurrentMapLabel());
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
	m_statusMessage = U"Saved {}"_fmt(getCurrentMapLabel());
}

void MapEditScene::selectEditableMap(const int32 mapIndex)
{
	if ((mapIndex < 0) || (static_cast<size_t>(mapIndex) >= m_editableMaps.size()))
	{
		return;
	}

	m_selectedMapIndex = mapIndex;
	m_mapConfigPath = m_editableMaps[static_cast<size_t>(m_selectedMapIndex)].mapConfigPath;
	reloadConfig();
}

String MapEditScene::getCurrentMapLabel() const
{
	if ((m_selectedMapIndex < 0) || (static_cast<size_t>(m_selectedMapIndex) >= m_editableMaps.size()))
	{
		return FileSystem::BaseName(m_mapConfigPath);
	}

	return m_editableMaps[static_cast<size_t>(m_selectedMapIndex)].label;
}

void MapEditScene::startTestPlay()
{
	auto& data = getData();
	data.baseBattleConfig = m_config;
	BeginNewRun(data.runState, data.baseBattleConfig, true);
	ResetBonusRoomSceneState(data.bonusRoomProgress);
	SaveContinueRun(data, ContinueResumeScene::Battle);
	RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}
