#pragma once

#include "GameData.h"
#include "Localization.h"

namespace TitleUiText
{
 inline const LocalizedText Title{ U"title.title" };
	inline const LocalizedText Subtitle{ U"title.subtitle" };
	inline const Array<LocalizedText> SummaryLines =
	{
      { U"title.summary.line1" },
		{ U"title.summary.line2" },
		{ U"title.summary.line3" },
	};
   inline const LocalizedText ViewedBonusRoomsPrefix{ U"title.viewed_bonus_rooms_prefix" };
	inline const LocalizedText ViewedBonusRoomsPreview{ U"title.viewed_bonus_rooms_preview" };
	inline const LocalizedText ContinueEnterHint{ U"title.continue_enter_hint" };
	inline const LocalizedText StartEnterHint{ U"title.start_enter_hint" };
	inline const LocalizedText ContinueButton{ U"title.continue_button" };
	inline const LocalizedText BonusRoomsHint{ U"title.bonus_rooms_hint" };
	inline const LocalizedText BonusRoomsButton{ U"title.bonus_rooms_button" };
	inline const LocalizedText TutorialButton{ U"title.tutorial_button" };
	inline const LocalizedText QuickGuideButton{ U"title.quick_guide_button" };
	inline const LocalizedText NewRunButton{ U"title.new_run_button" };
	inline const LocalizedText StartRunButton{ U"title.start_run_button" };
	inline const LocalizedText QuickGuideHint{ U"title.quick_guide_hint" };
	inline const LocalizedText ResolutionLabel{ U"title.resolution_label" };
	inline const LocalizedText CurrentPrefix{ U"common.current_prefix" };
	inline const LocalizedText SaveLocationLabel{ U"title.save_location_label" };
	inline const LocalizedText SaveLocationButton{ U"title.save_location_button" };
	inline const LocalizedText DataManagementLabel{ U"title.data_management_label" };
	inline const LocalizedText ClearContinueButton{ U"title.clear_continue_button" };
	inline const LocalizedText ClearSettingsButton{ U"title.clear_settings_button" };
	inline const LocalizedText ExitButton{ U"title.exit_button" };
	inline const LocalizedText DataManagementHint{ U"title.data_management_hint" };
	inline const LocalizedText DebugUnlockHint{ U"title.debug_unlock_hint" };
	inline const LocalizedText DebugFullUnlockButton{ U"title.debug_full_unlock_button" };
	inline const LocalizedText MapEditButton{ U"title.map_edit_button" };
	inline const LocalizedText BalanceEditButton{ U"title.balance_edit_button" };
	inline const LocalizedText TransitionPresetPrefix{ U"title.transition_preset_prefix" };
	inline const LocalizedText TitleUiEditorButton{ U"title.title_ui_editor_button" };
	inline const LocalizedText RewardEditorButton{ U"title.reward_editor_button" };
	inline const LocalizedText BonusRoomEditorButton{ U"title.bonus_room_editor_button" };
	inline const LocalizedText QuickGuideTitle{ U"title.quick_guide_title" };
	inline const LocalizedText QuickGuideSubtitle{ U"title.quick_guide_subtitle" };
	inline const Array<LocalizedText> QuickGuideBodyLines =
	{
      { U"title.quick_guide.line1" },
		{ U"title.quick_guide.line2" },
		{ U"title.quick_guide.line3" },
		{ U"title.quick_guide.line4" },
		{ U"title.quick_guide.line5" },
		{ U"title.quick_guide.line6" },
		{ U"title.quick_guide.line7" },
		{ U"title.quick_guide.line8" },
	};
    inline const LocalizedText QuickGuideFlow{ U"title.quick_guide_flow" };
	inline const LocalizedText QuickGuideTutorialButton{ U"title.quick_guide_tutorial_button" };
	inline const LocalizedText CloseButton{ U"common.close_button" };
	inline const LocalizedText QuickGuideEscHint{ U"title.quick_guide_esc_hint" };
	inline const LocalizedText ContinuePreviewTitle{ U"title.continue_preview_title" };
	inline const LocalizedText ContinuePreviewHeadline{ U"title.continue_preview_headline" };
	inline const LocalizedText ContinuePreviewDetail{ U"title.continue_preview_detail" };
	inline const LocalizedText ContinuePreviewCardsPrefix{ U"title.continue_preview_cards_prefix" };
	inline const LocalizedText DataClearQuestion{ U"title.data_clear_question" };
	inline const LocalizedText DataClearBody{ U"title.data_clear_body" };
	inline const LocalizedText DataClearContinueQuestion{ U"title.data_clear_continue_question" };
	inline const LocalizedText DataClearContinueBody{ U"title.data_clear_continue_body" };
	inline const LocalizedText DataClearImmediateHint{ U"title.data_clear_immediate_hint" };
	inline const LocalizedText DialogEnterYesNoHint{ U"common.dialog_enter_yes_no_hint" };
	inline const LocalizedText ExitQuestion{ U"common.exit_question" };
	inline const LocalizedText Yes{ U"common.yes" };
	inline const LocalizedText No{ U"common.no" };

	[[nodiscard]] inline const LocalizedText& GetEnterHintText(const bool hasContinue)
	{
		return hasContinue ? ContinueEnterHint : StartEnterHint;
	}

	[[nodiscard]] inline const LocalizedText& GetStartButtonText(const bool hasContinue)
	{
		return hasContinue ? NewRunButton : StartRunButton;
	}
}
