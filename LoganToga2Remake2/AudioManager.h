#pragma once

#include "Remake2Common.h"
#include "WindowChromeAddon.h"

namespace AudioManager
{
	enum class BgmTrack
	{
		None,
		Menu,
		Battle,
	};

	inline s3d::Audio& GetMenuBgm()
	{
		static s3d::Audio audio{ U"example/メニュー画面.wav" };
		return audio;
	}

	inline s3d::Audio& GetBattleBgm()
	{
		static s3d::Audio audio{ U"example/ねらいを定めて.wav" };
		return audio;
	}

	inline s3d::Audio& GetUiClickSe()
	{
		static s3d::Audio audio{ U"example/shot.mp3" };
		return audio;
	}

	inline BgmTrack& CurrentBgmTrack()
	{
		static BgmTrack track = BgmTrack::None;
		return track;
	}

	inline s3d::Audio* GetCurrentBgmAudio()
	{
		switch (CurrentBgmTrack())
		{
		case BgmTrack::Menu:
			return &GetMenuBgm();
		case BgmTrack::Battle:
			return &GetBattleBgm();
		case BgmTrack::None:
		default:
			return nullptr;
		}
	}

	inline void PlayBgm(const BgmTrack track)
	{
		if (track == BgmTrack::None)
		{
			if (auto* current = GetCurrentBgmAudio())
			{
				current->stop();
			}
			CurrentBgmTrack() = BgmTrack::None;
			return;
		}

		s3d::Audio& next = (track == BgmTrack::Battle) ? GetBattleBgm() : GetMenuBgm();
		next.setVolume(track == BgmTrack::Battle ? 0.20 : 0.16);

		if (CurrentBgmTrack() == track)
		{
			if (!next.isPlaying())
			{
				next.setLoop(true);
				next.play(WindowChromeAddon::BgmBus);
			}
			return;
		}

		if (auto* current = GetCurrentBgmAudio())
		{
			current->stop();
		}

		CurrentBgmTrack() = track;
		next.setLoop(true);
		next.play(WindowChromeAddon::BgmBus);
	}
}

inline void PlayMenuBgm()
{
	AudioManager::PlayBgm(AudioManager::BgmTrack::Menu);
}

inline void PlayBattleBgm()
{
	AudioManager::PlayBgm(AudioManager::BgmTrack::Battle);
}

inline void StopBgm()
{
	AudioManager::PlayBgm(AudioManager::BgmTrack::None);
}

inline void PlayUiClickSe()
{
	auto& audio = AudioManager::GetUiClickSe();
	audio.playOneShot(WindowChromeAddon::SeBus, 0.55);
}
