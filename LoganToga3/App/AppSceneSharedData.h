#pragma once
# include <Siv3D.hpp>
# include "App/AppDefinitionState.h"
# include "App/AppSceneTypes.h"
# include "../Data/Localization.h"
# include "../Data/MusicTypes.h"
# include "../UI/TitleUiLayout.h"

namespace LT3
{
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
		Optional<TitleUiLayout> layoutBeforePointerEdit;
		String statusText = U"Title UI Editor ready";
	};

	struct AppSharedData
	{
		Font titleFont{ FontMethod::MSDF, 38, Typeface::Bold };
		Font uiFont{ FontMethod::MSDF, 20, Typeface::Medium };
		AppDefinitionState definitions = CreateAppDefinitionState();
		bool modMode = false;
		String requestedModId;
		bool quickBattleRequested = false;
		String quickBattleArgument;
		ModContext activeMod;
		BattleRequest quickBattleRequest;
		String startupErrorText;
		LocalizationCatalog localizedTexts;
		LocalizationCatalog defaultTexts;
		Texture titleImage{ U"000_Warehouse/000_DefaultGame/000_SystemImage/title.png" };
		TitleUiLayout titleUiLayout = CreateDefaultTitleUiLayout();
		TitleUiEditorState titleUiEditor;
		MusicSettings musicSettings = CreateDefaultMusicSettings();
		MusicEditorState musicEditor;
		MusicPlaybackState musicPlayback;
	};

	using AppSceneManager = SceneManager<AppSceneState, AppSharedData>;
}
