#pragma once
# include <Siv3D.hpp>
# include "MusicSettings.h"
# include "TomlTextUtils.h"

namespace LT3
{
	inline FilePath ResolveMusicSettingsTomlPath()
	{
		return ResolveFirstExistingPath({ U"music_settings.toml", U"App/music_settings.toml", U"LoganToga3/App/music_settings.toml" });
	}

	inline void EnsureMusicSettingsDefaults(MusicSettings& settings)
	{
		for (const auto sceneId : AllMusicSceneIds()) (void)GetMusicTrackSetting(settings, sceneId);
	}

	inline String ToMusicTrackTomlKeyBase(MusicSceneId sceneId)
	{
		return U"music." + ToMusicSceneTomlKey(sceneId);
	}

	inline double NormalizeMusicTrackVolume(double volume)
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
			const String key = ToMusicTrackTomlKeyBase(sceneId);
			track.path = toml[key + U".path"].getOr<String>(track.path);
			track.volume = NormalizeMusicTrackVolume(toml[key + U".volume"].getOr<double>(track.volume));
		}
		statusText = U"Loaded music settings: {}"_fmt(settings.sourcePath);
		return true;
	}

	inline bool SaveMusicSettingsToml(const MusicSettings& sourceSettings, String& statusText)
	{
		MusicSettings settings = sourceSettings;
		EnsureMusicSettingsDefaults(settings);
		const FilePath path = settings.sourcePath.isEmpty() ? ResolveMusicSettingsTomlPath() : settings.sourcePath;
		String text = U"[editor]\nopen = {}\n\n"_fmt(settings.editorOpen ? U"true" : U"false");
		for (const auto sceneId : AllMusicSceneIds())
		{
			const MusicTrackSetting& track = GetMusicTrackSetting(settings, sceneId);
			text += U"[{}]\npath = \"{}\"\nvolume = {}\n\n"_fmt(ToMusicTrackTomlKeyBase(sceneId), EscapeTomlBasicString(track.path), NormalizeMusicTrackVolume(track.volume));
		}
		const FilePath temporaryPath = path + U".tmp";
		FileSystem::Remove(temporaryPath);
		if (!WriteUtf8TextFile(temporaryPath, text, statusText)) return false;
		if (!SaveTomlFilesTransaction({ TomlTransactionFile{ path, temporaryPath, path + U".bak" } }, statusText)) return false;
		statusText = U"Saved music settings: {}"_fmt(path);
		return true;
	}
}
