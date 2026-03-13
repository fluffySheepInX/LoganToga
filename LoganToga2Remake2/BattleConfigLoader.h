#pragma once

#include "BattleConfigCoreLoader.h"
#include "BattleConfigMapLoader.h"
#include "BattleConfigPathResolver.h"
#include "BattleConfigProgressionLoader.h"
#include "BattleConfigUnitLoader.h"

[[nodiscard]] inline BattleConfigData LoadBattleConfig(const String& path)
{
	const TOMLReader toml{ path };
	if (!toml)
	{
		throw Error{ U"Failed to load battle config: " + path };
	}

	const String coreSource = toml[U"sources"][U"core"].get<String>();
	const String unitsSource = toml[U"sources"][U"units"].get<String>();
	const String mapSource = toml[U"sources"][U"map"].get<String>();
	const String progressionSource = toml[U"sources"][U"progression"].get<String>();

	BattleConfigData config;

	const String corePath = ResolveBattleConfigSourcePath(path, coreSource);
	const TOMLReader coreToml{ corePath };
	if (!coreToml)
	{
		throw Error{ U"Failed to load battle config core: " + corePath };
	}
	LoadBattleCoreConfig(config, coreToml);

	const String unitsPath = ResolveBattleConfigSourcePath(path, unitsSource);
	const TOMLReader unitsToml{ unitsPath };
	if (!unitsToml)
	{
		throw Error{ U"Failed to load battle config units: " + unitsPath };
	}
	LoadBattleUnitConfig(config, unitsToml);

	const String mapPath = ResolveBattleConfigSourcePath(path, mapSource);
	const TOMLReader mapToml{ mapPath };
	if (!mapToml)
	{
		throw Error{ U"Failed to load battle config map: " + mapPath };
	}
	LoadBattleMapConfig(config, mapToml);

	const String progressionPath = ResolveBattleConfigSourcePath(path, progressionSource);
	const TOMLReader progressionToml{ progressionPath };
	if (!progressionToml)
	{
		throw Error{ U"Failed to load battle config progression: " + progressionPath };
	}
	LoadBattleProgressionConfig(config, progressionToml);

	return config;
}
