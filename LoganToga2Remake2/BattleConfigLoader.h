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
	if (HasTomlOverride(corePath))
	{
		const String coreOverridePath = ResolveTomlOverridePath(corePath);
		const TOMLReader coreOverrideToml{ coreOverridePath };
		if (!coreOverrideToml)
		{
			throw Error{ U"Failed to load battle config core override: " + coreOverridePath };
		}
		ApplyBattleCoreConfigOverrides(config, coreOverrideToml);
	}
	if (HasTomlEditorOverride(corePath))
	{
		const String coreEditorOverridePath = ResolveTomlEditorOverridePath(corePath);
		const TOMLReader coreEditorOverrideToml{ coreEditorOverridePath };
		if (!coreEditorOverrideToml)
		{
			throw Error{ U"Failed to load battle config core editor override: " + coreEditorOverridePath };
		}
		ApplyBattleCoreConfigOverrides(config, coreEditorOverrideToml);
	}

	const String unitsPath = ResolveBattleConfigSourcePath(path, unitsSource);
	const TOMLReader unitsToml{ unitsPath };
	if (!unitsToml)
	{
		throw Error{ U"Failed to load battle config units: " + unitsPath };
	}
	LoadBattleUnitConfig(config, unitsToml);
	if (HasTomlOverride(unitsPath))
	{
		const String unitsOverridePath = ResolveTomlOverridePath(unitsPath);
		const TOMLReader unitsOverrideToml{ unitsOverridePath };
		if (!unitsOverrideToml)
		{
			throw Error{ U"Failed to load battle config units override: " + unitsOverridePath };
		}
		ApplyBattleUnitConfigOverrides(config, unitsOverrideToml);
	}
	if (HasTomlEditorOverride(unitsPath))
	{
		const String unitsEditorOverridePath = ResolveTomlEditorOverridePath(unitsPath);
		const TOMLReader unitsEditorOverrideToml{ unitsEditorOverridePath };
		if (!unitsEditorOverrideToml)
		{
			throw Error{ U"Failed to load battle config units editor override: " + unitsEditorOverridePath };
		}
		ApplyBattleUnitConfigOverrides(config, unitsEditorOverrideToml);
	}

	const String mapPath = ResolveBattleConfigSourcePath(path, mapSource);
	const TOMLReader mapToml{ mapPath };
	if (!mapToml)
	{
		throw Error{ U"Failed to load battle config map: " + mapPath };
	}
	LoadBattleMapConfig(config, mapToml);
	if (HasTomlOverride(mapPath))
	{
		const String mapOverridePath = ResolveTomlOverridePath(mapPath);
		const TOMLReader mapOverrideToml{ mapOverridePath };
		if (!mapOverrideToml)
		{
			throw Error{ U"Failed to load battle config map override: " + mapOverridePath };
		}
		ApplyBattleMapConfigOverrides(config, mapOverrideToml);
	}
	if (HasTomlEditorOverride(mapPath))
	{
		const String mapEditorOverridePath = ResolveTomlEditorOverridePath(mapPath);
		const TOMLReader mapEditorOverrideToml{ mapEditorOverridePath };
		if (!mapEditorOverrideToml)
		{
			throw Error{ U"Failed to load battle config map editor override: " + mapEditorOverridePath };
		}
		ApplyBattleMapConfigOverrides(config, mapEditorOverrideToml);
	}

	const String progressionPath = ResolveBattleConfigSourcePath(path, progressionSource);
	const TOMLReader progressionToml{ progressionPath };
	if (!progressionToml)
	{
		throw Error{ U"Failed to load battle config progression: " + progressionPath };
	}
	LoadBattleProgressionConfig(config, progressionToml, progressionPath);
	if (HasTomlOverride(progressionPath))
	{
		const String progressionOverridePath = ResolveTomlOverridePath(progressionPath);
		const TOMLReader progressionOverrideToml{ progressionOverridePath };
		if (!progressionOverrideToml)
		{
			throw Error{ U"Failed to load battle config progression override: " + progressionOverridePath };
		}
		ApplyBattleProgressionConfigOverrides(config, progressionOverrideToml, progressionOverridePath);
	}
	if (HasTomlEditorOverride(progressionPath))
	{
		const String progressionEditorOverridePath = ResolveTomlEditorOverridePath(progressionPath);
		const TOMLReader progressionEditorOverrideToml{ progressionEditorOverridePath };
		if (!progressionEditorOverrideToml)
		{
			throw Error{ U"Failed to load battle config progression editor override: " + progressionEditorOverridePath };
		}
		ApplyBattleProgressionConfigOverrides(config, progressionEditorOverrideToml, progressionEditorOverridePath);
	}

	return config;
}
