#pragma once
# include <Siv3D.hpp>
# include "App/AppDefinitionState.h"
# include "App/AppSceneTypes.h"
# include "../UI/TitleUiLayout.h"

namespace LT3
{
	struct MusicTrackSetting
	{
		FilePath path;
		double volume = 0.2;
	};

	inline MusicTrackSetting DefaultMusicTrackSetting(const MusicSceneId sceneId)
	{
		MusicTrackSetting setting;
		switch (sceneId)
		{
		case MusicSceneId::Title:
			setting.volume = 0.18;
			break;
		case MusicSceneId::Battle:
			setting.volume = 0.22;
			break;
		case MusicSceneId::Options:
			setting.volume = 0.16;
			break;
		case MusicSceneId::Achievements:
			setting.volume = 0.18;
			break;
		default:
			setting.volume = 0.2;
			break;
		}

		return setting;
	}

	struct MusicEditorState
	{
		MusicSceneId selectedScene = MusicSceneId::Title;
		String statusText = U"Music settings not loaded";
		bool dirty = false;
		bool open = true;
		Audio previewAudio;
		FilePath previewPath;
	};

	struct MusicPlaybackState
	{
		Audio audio;
		Optional<MusicSceneId> activeScene;
		FilePath activePath;
	};

	enum class TitleUiEditableElement
	{
		SkirmishButton,
		MusicEditorToggle,
	};

	struct TitleUiEditorState
	{
		bool open = false;
		Optional<TitleUiEditableElement> selectedElement = TitleUiEditableElement::SkirmishButton;
		Optional<Vec2> dragOffset;
		bool resizing = false;
		double resizeAnchorLeft = 0.0;
		double resizeAnchorY = 0.0;
		String statusText = U"Title UI Editor ready";
	};

	struct MusicSettings
	{
		HashTable<MusicSceneId, MusicTrackSetting> tracks;
		FilePath sourcePath;
		bool editorOpen = true;
	};

	inline MusicSettings CreateDefaultMusicSettings()
	{
		MusicSettings settings;
		for (const auto sceneId : AllMusicSceneIds())
		{
			settings.tracks.emplace(sceneId, DefaultMusicTrackSetting(sceneId));
		}

		return settings;
	}

	inline MusicTrackSetting& GetMusicTrackSetting(MusicSettings& settings, const MusicSceneId sceneId)
	{
		if (const auto it = settings.tracks.find(sceneId); it != settings.tracks.end())
		{
			return it->second;
		}

		return settings.tracks.emplace(sceneId, DefaultMusicTrackSetting(sceneId)).first->second;
	}

	inline const MusicTrackSetting& GetMusicTrackSetting(const MusicSettings& settings, const MusicSceneId sceneId)
	{
		if (const auto it = settings.tracks.find(sceneId); it != settings.tracks.end())
		{
			return it->second;
		}

		static const MusicTrackSetting defaultSetting;
		return defaultSetting;
	}

	struct AppSharedData
	{
		Font titleFont{ FontMethod::MSDF, 38, Typeface::Bold };
		Font uiFont{ FontMethod::MSDF, 20, Typeface::Medium };
		AppDefinitionState definitions = CreateAppDefinitionState();
		bool modMode = false;
		bool quickBattleRequested = false;
		String quickBattleArgument;
		Texture titleImage{ U"000_Warehouse/000_DefaultGame/000_SystemImage/title.png" };
		TitleUiLayout titleUiLayout = CreateDefaultTitleUiLayout();
		TitleUiEditorState titleUiEditor;
		MusicSettings musicSettings = CreateDefaultMusicSettings();
		MusicEditorState musicEditor;
		MusicPlaybackState musicPlayback;
	};

	using AppSceneManager = SceneManager<AppSceneState, AppSharedData>;
}
