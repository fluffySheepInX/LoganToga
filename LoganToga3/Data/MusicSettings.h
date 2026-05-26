#pragma once
# include <Siv3D.hpp>
# include "TomlTextUtils.h"
# include "../App/AppSceneSharedData.h"

namespace LT3
{
	inline FilePath ResolveMusicSettingsTomlPath()
	{
		return ResolveFirstExistingPath({
			U"music_settings.toml",
			U"App/music_settings.toml",
			U"LoganToga3/App/music_settings.toml",
		});
	}

	inline void EnsureMusicSettingsDefaults(MusicSettings& settings)
	{
		for (const auto sceneId : AllMusicSceneIds())
		{
			(void)GetMusicTrackSetting(settings, sceneId);
		}
	}

	inline String ToMusicTrackTomlKeyBase(const MusicSceneId sceneId)
	{
		return U"music." + ToMusicSceneTomlKey(sceneId);
	}

	inline double NormalizeMusicTrackVolume(const double volume)
	{
		return Clamp(volume, 0.0, 1.0);
	}

	inline bool LoadMusicSettingsToml(MusicSettings& settings, String& statusText)
	{
		settings = CreateDefaultMusicSettings();
		settings.sourcePath = ResolveMusicSettingsTomlPath();
		const TOMLReader toml{ settings.sourcePath };
		if (!toml)
		{
			statusText = U"Music settings not found: {}"_fmt(settings.sourcePath);
			return false;
		}

		settings.editorOpen = toml[U"editor.open"].getOr<bool>(settings.editorOpen);

		for (const auto sceneId : AllMusicSceneIds())
		{
			MusicTrackSetting& track = GetMusicTrackSetting(settings, sceneId);
			const String keyBase = ToMusicTrackTomlKeyBase(sceneId);
			track.path = toml[keyBase + U".path"].getOr<String>(track.path);
			track.volume = NormalizeMusicTrackVolume(toml[keyBase + U".volume"].getOr<double>(track.volume));
		}

		statusText = U"Loaded music settings: {}"_fmt(settings.sourcePath);
		return true;
	}

	inline bool SaveMusicSettingsToml(const MusicSettings& sourceSettings, String& statusText)
	{
		MusicSettings settings = sourceSettings;
		EnsureMusicSettingsDefaults(settings);
		const FilePath path = settings.sourcePath.isEmpty() ? ResolveMusicSettingsTomlPath() : settings.sourcePath;
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path };
		if (!writer)
		{
			statusText = U"Music settings save failed: {}"_fmt(path);
			return false;
		}

		String tomlText;
		tomlText += U"[editor]\n";
		tomlText += U"open = {}\n"_fmt(settings.editorOpen ? U"true" : U"false");
		tomlText += U"\n";

		for (const auto sceneId : AllMusicSceneIds())
		{
			const MusicTrackSetting& track = GetMusicTrackSetting(settings, sceneId);
			tomlText += U"[" + ToMusicTrackTomlKeyBase(sceneId) + U"]\n";
			tomlText += U"path = \"" + EscapeTomlBasicString(track.path) + U"\"\n";
			tomlText += U"volume = {}\n"_fmt(NormalizeMusicTrackVolume(track.volume));
			tomlText += U"\n";
		}

		writer.write(tomlText);
		statusText = U"Saved music settings: {}"_fmt(path);
		return true;
	}
}
