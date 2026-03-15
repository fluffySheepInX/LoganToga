#pragma once

#include "GameData.h"

namespace TitleUiText
{
	inline const String Title = U"LoganToga2Remake2";
	inline const String Subtitle = U"RTS run prototype";
	inline const Array<String> SummaryLines =
	{
		U"・3-5 battles per run",
		U"・Choose 1 of 3 reward cards after each victory",
		U"・Lose once and the run ends",
	};
	inline const String ViewedBonusRoomsPrefix = U"・Viewed bonus rooms: ";
	inline const String ViewedBonusRoomsPreview = U"・Viewed bonus rooms: 2 / 6";
	inline const String ContinueEnterHint = U"Press Enter to continue the saved run";
	inline const String StartEnterHint = U"Press Enter to start a new run";
	inline const String ContinueButton = U"Continue";
	inline const String BonusRoomsHint = U"Bonus Rooms can be revisited from this menu";
	inline const String BonusRoomsButton = U"Bonus Rooms";
	inline const String TutorialButton = U"Tutorial";
	inline const String QuickGuideButton = U"クイック操作説明";
	inline const String NewRunButton = U"New Run";
	inline const String StartRunButton = U"Start Run";
	inline const String QuickGuideHint = U"";
	inline const String ResolutionLabel = U"解像度";
	inline const String CurrentPrefix = U"現在: ";
	inline const String SaveLocationLabel = U"セーブ保存先";
	inline const String SaveLocationButton = U"Local ⇔ AppData";
	inline const String DataManagementLabel = U"データ管理";
	inline const String ClearContinueButton = U"セーブ削除";
	inline const String ClearSettingsButton = U"設定初期化";
	inline const String DataManagementHint = U"現在の保存先のみ削除 / 設定は既定値へ戻ります";
	inline const String DebugUnlockHint = U"DEBUG: Start with all unlockable units/buildings";
	inline const String DebugFullUnlockButton = U"Debug Full Unlock";
	inline const String MapEditButton = U"Map Edit";
	inline const String BalanceEditButton = U"Balance Edit";
	inline const String TransitionPresetPrefix = U"Fade: ";
	inline const String TitleUiEditorButton = U"Title UI Editor";
	inline const String QuickGuideTitle = U"クイック操作説明";
	inline const String QuickGuideSubtitle = U"まずはここだけ覚えればOK。詳しくは Tutorial で確認できます。";
	inline const Array<String> QuickGuideBodyLines =
	{
		U"1. 左クリックでユニットや建物を選択",
		U"2. Shift + 左クリックで選択追加",
		U"3. 右クリックで単体移動 / 攻撃指示、",
		U"　　右ドラッグで複数指定、その後再び右ドラッグで部隊移動",
		U"4. 下のコマンドパネルで生産 / 建築 / 強化",
		U"5. Esc でポーズ。困ったら一度止める",
		U"6. 勝利後は報酬カードを1枚選んで次の戦闘へ",
		U"7. 敗北するとラン終了。再挑戦はタイトルから",
	};
	inline const String QuickGuideFlow = U"流れ: 選択 → 右クリックで行動 → 建築と生産で戦力を増やす → 勝利後に強化";
	inline const String QuickGuideTutorialButton = U"Tutorial へ";
	inline const String CloseButton = U"閉じる";
	inline const String QuickGuideEscHint = U"Esc でも閉じられます";
	inline const String ContinuePreviewTitle = U"CONTINUE";
	inline const String ContinuePreviewHeadline = U"Battle 2/5";
	inline const String ContinuePreviewDetail = U"Resume from battle start checkpoint";
	inline const String ContinuePreviewCardsPrefix = U"Cards selected: ";
	inline const String DataClearQuestion = U"設定ファイルを初期化しますか？";
	inline const String DataClearBody = U"解像度・フルスクリーン・音量を既定値へ戻します";
	inline const String ExitQuestion = U"ゲームを終了しますか？";
	inline const String Yes = U"はい";
	inline const String No = U"いいえ";

	[[nodiscard]] inline const String& GetEnterHintText(const bool hasContinue)
	{
		return hasContinue ? ContinueEnterHint : StartEnterHint;
	}

	[[nodiscard]] inline const String& GetStartButtonText(const bool hasContinue)
	{
		return hasContinue ? NewRunButton : StartRunButton;
	}
}
