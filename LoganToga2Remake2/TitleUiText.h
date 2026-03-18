#pragma once

#include "GameData.h"
#include "Localization.h"

namespace TitleUiText
{
	inline const LocalizedText Title{ U"title.title", U"LoganToga2Remake2", U"LoganToga2Remake2" };
	inline const LocalizedText Subtitle{ U"title.subtitle", U"RTSラン試作", U"RTS run prototype" };
	inline const Array<LocalizedText> SummaryLines =
	{
		{ U"title.summary.line1", U"・1ランは3〜5戦", U"・3-5 battles per run" },
		{ U"title.summary.line2", U"・勝利後、3枚から報酬カードを1枚選択", U"・Choose 1 of 3 reward cards after each victory" },
		{ U"title.summary.line3", U"・1度でも敗北するとラン終了", U"・Lose once and the run ends" },
	};
	inline const LocalizedText ViewedBonusRoomsPrefix{ U"title.viewed_bonus_rooms_prefix", U"・閲覧済みボーナスルーム: ", U"・Viewed bonus rooms: " };
	inline const LocalizedText ViewedBonusRoomsPreview{ U"title.viewed_bonus_rooms_preview", U"・閲覧済みボーナスルーム: 2 / 6", U"・Viewed bonus rooms: 2 / 6" };
	inline const LocalizedText ContinueEnterHint{ U"title.continue_enter_hint", U"Enterでセーブ済みランを再開", U"Press Enter to continue the saved run" };
	inline const LocalizedText StartEnterHint{ U"title.start_enter_hint", U"Enterで新しいランを開始", U"Press Enter to start a new run" };
	inline const LocalizedText ContinueButton{ U"title.continue_button", U"続きから", U"Continue" };
	inline const LocalizedText BonusRoomsHint{ U"title.bonus_rooms_hint", U"Bonus Rooms はこのメニューから再訪できます", U"Bonus Rooms can be revisited from this menu" };
	inline const LocalizedText BonusRoomsButton{ U"title.bonus_rooms_button", U"Bonus Rooms", U"Bonus Rooms" };
	inline const LocalizedText TutorialButton{ U"title.tutorial_button", U"Tutorial", U"Tutorial" };
	inline const LocalizedText QuickGuideButton{ U"title.quick_guide_button", U"クイック操作説明", U"Quick Guide" };
	inline const LocalizedText NewRunButton{ U"title.new_run_button", U"新しいラン", U"New Run" };
	inline const LocalizedText StartRunButton{ U"title.start_run_button", U"ラン開始", U"Start Run" };
	inline const LocalizedText QuickGuideHint{ U"title.quick_guide_hint", U"", U"" };
	inline const LocalizedText ResolutionLabel{ U"title.resolution_label", U"解像度", U"Resolution" };
	inline const LocalizedText CurrentPrefix{ U"common.current_prefix", U"現在: ", U"Current: " };
	inline const LocalizedText SaveLocationLabel{ U"title.save_location_label", U"セーブ保存先", U"Save Location" };
	inline const LocalizedText SaveLocationButton{ U"title.save_location_button", U"Local ⇔ AppData", U"Local ⇔ AppData" };
	inline const LocalizedText DataManagementLabel{ U"title.data_management_label", U"データ管理", U"Data" };
	inline const LocalizedText ClearContinueButton{ U"title.clear_continue_button", U"セーブ削除", U"Delete Save" };
	inline const LocalizedText ClearSettingsButton{ U"title.clear_settings_button", U"設定初期化", U"Reset Settings" };
	inline const LocalizedText ExitButton{ U"title.exit_button", U"終了", U"Exit" };
	inline const LocalizedText DataManagementHint{ U"title.data_management_hint", U"現在の保存先のみ削除 / 設定は既定値へ戻ります", U"Delete only current storage / settings return to defaults" };
	inline const LocalizedText DebugUnlockHint{ U"title.debug_unlock_hint", U"DEBUG: すべての解放対象ユニット/建物を所持した状態で開始", U"DEBUG: Start with all unlockable units/buildings" };
	inline const LocalizedText DebugFullUnlockButton{ U"title.debug_full_unlock_button", U"Debug Full Unlock", U"Debug Full Unlock" };
	inline const LocalizedText MapEditButton{ U"title.map_edit_button", U"Map Edit", U"Map Edit" };
	inline const LocalizedText BalanceEditButton{ U"title.balance_edit_button", U"Balance Edit", U"Balance Edit" };
	inline const LocalizedText TransitionPresetPrefix{ U"title.transition_preset_prefix", U"Fade: ", U"Fade: " };
	inline const LocalizedText TitleUiEditorButton{ U"title.title_ui_editor_button", U"Title UI Editor", U"Title UI Editor" };
	inline const LocalizedText RewardEditorButton{ U"title.reward_editor_button", U"Reward Editor", U"Reward Editor" };
  inline const LocalizedText BonusRoomEditorButton{ U"title.bonus_room_editor_button", U"Bonus Room Editor", U"Bonus Room Editor" };
	inline const LocalizedText QuickGuideTitle{ U"title.quick_guide_title", U"クイック操作説明", U"Quick Guide" };
	inline const LocalizedText QuickGuideSubtitle{ U"title.quick_guide_subtitle", U"まずはここだけ覚えればOK。詳しくは Tutorial で確認できます。", U"Learn just this first. Check Tutorial for the full explanation." };
	inline const Array<LocalizedText> QuickGuideBodyLines =
	{
		{ U"title.quick_guide.line1", U"1. 左クリックでユニットや建物を選択", U"1. Left click to select units or buildings" },
		{ U"title.quick_guide.line2", U"2. Shift + 左クリックで選択追加", U"2. Shift + left click to add to selection" },
		{ U"title.quick_guide.line3", U"3. 右クリックで単体移動 / 攻撃指示、", U"3. Right click to move or attack with one unit," },
		{ U"title.quick_guide.line4", U"　　右ドラッグで複数指定、その後再び右ドラッグで部隊移動", U"   right drag to target multiple units, then right drag again to move the squad" },
		{ U"title.quick_guide.line5", U"4. 下のコマンドパネルで生産 / 建築 / 強化", U"4. Use the bottom command panel for production / building / upgrades" },
		{ U"title.quick_guide.line6", U"5. Esc でポーズ。困ったら一度止める", U"5. Press Esc to pause. Stop first if you get overwhelmed" },
		{ U"title.quick_guide.line7", U"6. 勝利後は報酬カードを1枚選んで次の戦闘へ", U"6. After victory, pick 1 reward card and move to the next battle" },
		{ U"title.quick_guide.line8", U"7. 敗北するとラン終了。再挑戦はタイトルから", U"7. Defeat ends the run. Retry from the title screen" },
	};
	inline const LocalizedText QuickGuideFlow{ U"title.quick_guide_flow", U"流れ: 選択 → 右クリックで行動 → 建築と生産で戦力を増やす → 勝利後に強化", U"Flow: Select → right click to act → build and produce to grow stronger → upgrade after victory" };
	inline const LocalizedText QuickGuideTutorialButton{ U"title.quick_guide_tutorial_button", U"Tutorial へ", U"Open Tutorial" };
	inline const LocalizedText CloseButton{ U"common.close_button", U"閉じる", U"Close" };
	inline const LocalizedText QuickGuideEscHint{ U"title.quick_guide_esc_hint", U"Esc でも閉じられます", U"You can also close with Esc" };
	inline const LocalizedText ContinuePreviewTitle{ U"title.continue_preview_title", U"CONTINUE", U"CONTINUE" };
	inline const LocalizedText ContinuePreviewHeadline{ U"title.continue_preview_headline", U"戦闘 2/5", U"Battle 2/5" };
	inline const LocalizedText ContinuePreviewDetail{ U"title.continue_preview_detail", U"戦闘開始チェックポイントから再開", U"Resume from battle start checkpoint" };
	inline const LocalizedText ContinuePreviewCardsPrefix{ U"title.continue_preview_cards_prefix", U"選択済みカード: ", U"Cards selected: " };
	inline const LocalizedText DataClearQuestion{ U"title.data_clear_question", U"設定ファイルを初期化しますか？", U"Reset the settings file?" };
	inline const LocalizedText DataClearBody{ U"title.data_clear_body", U"解像度・フルスクリーン・音量を既定値へ戻します", U"Resolution, fullscreen, and volume return to defaults" };
	inline const LocalizedText DataClearContinueQuestion{ U"title.data_clear_continue_question", U"セーブデータを削除しますか？", U"Delete the continue save?" };
	inline const LocalizedText DataClearContinueBody{ U"title.data_clear_continue_body", U"現在の保存先にある continue データを削除します", U"Delete the continue data in the current save location" };
	inline const LocalizedText DataClearImmediateHint{ U"title.data_clear_immediate_hint", U"この操作はタイトルメニューからすぐ反映されます", U"This change is reflected in the title menu immediately" };
	inline const LocalizedText DialogEnterYesNoHint{ U"common.dialog_enter_yes_no_hint", U"Enter: はい / Esc: いいえ", U"Enter: Yes / Esc: No" };
	inline const LocalizedText ExitQuestion{ U"common.exit_question", U"ゲームを終了しますか？", U"Exit the game?" };
	inline const LocalizedText Yes{ U"common.yes", U"はい", U"Yes" };
	inline const LocalizedText No{ U"common.no", U"いいえ", U"No" };

	[[nodiscard]] inline const LocalizedText& GetEnterHintText(const bool hasContinue)
	{
		return hasContinue ? ContinueEnterHint : StartEnterHint;
	}

	[[nodiscard]] inline const LocalizedText& GetStartButtonText(const bool hasContinue)
	{
		return hasContinue ? NewRunButton : StartRunButton;
	}
}
