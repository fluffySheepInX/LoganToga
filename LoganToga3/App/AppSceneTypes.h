#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	enum class AppSceneState
	{
		Title,
		Battle,
	};

	enum class MusicSceneId
	{
		Title,
		Battle,
		Options,
		Achievements,
	};

	inline Array<MusicSceneId> AllMusicSceneIds()
	{
		return {
			MusicSceneId::Title,
			MusicSceneId::Battle,
			MusicSceneId::Options,
			MusicSceneId::Achievements,
		};
	}

	inline String ToMusicSceneLabel(const MusicSceneId sceneId)
	{
		switch (sceneId)
		{
		case MusicSceneId::Title:
			return U"Title";
		case MusicSceneId::Battle:
			return U"Battle";
		case MusicSceneId::Options:
			return U"Options";
		case MusicSceneId::Achievements:
			return U"Achievements";
		default:
			return U"Unknown";
		}
	}

	inline String ToMusicSceneTomlKey(const MusicSceneId sceneId)
	{
		switch (sceneId)
		{
		case MusicSceneId::Title:
			return U"title";
		case MusicSceneId::Battle:
			return U"battle";
		case MusicSceneId::Options:
			return U"options";
		case MusicSceneId::Achievements:
			return U"achievements";
		default:
			return U"unknown";
		}
	}
}
