# include "UnitEditorScene.h"

Optional<String> UnitEditorScene::GetHoveredTooltipText() const
{
	if (GetTopHelpIcon().mouseOver())
	{
		return U"各アイコンにマウスを合わせると項目名や操作説明を表示します";
	}

	if (GetUnitPanelHeaderIcon().mouseOver())
	{
       return IsEnemyEditor() ? U"エネミー一覧" : U"ユニット一覧";
	}

	if (GetPreviewPanelHeaderIcon().mouseOver())
	{
		return U"プレビュー / メタ情報";
	}

	if (GetEditPanelHeaderIcon().mouseOver())
	{
		return U"編集";
	}

	if (GetInfoPanelHeaderIcon().mouseOver())
	{
		return U"差分 / 派生ステータス";
	}

	if (GetUnitListHelpIcon().mouseOver())
	{
      return IsEnemyEditor() ? U"クリックで編集中エネミーを切り替えます" : U"クリックで編集中ユニットを切り替えます";
	}

	if (GetPreviewLineIcon(0).mouseOver())
	{
       return IsEnemyEditor() ? U"エネミーID" : U"ユニットID";
	}

	if (GetPreviewLineIcon(1).mouseOver())
	{
      return IsEnemyEditor() ? U"編集中エネミー" : U"編集中ユニット";
	}

	if (GetPreviewLineIcon(2).mouseOver())
	{
		return U"保存先";
	}

	if (GetLabelFieldIcon().mouseOver())
	{
		return U"表示名";
	}

	if (GetRoleFieldIcon().mouseOver())
	{
		return U"説明文";
	}

	for (size_t index = 0; index < FieldCount; ++index)
	{
		if (GetFieldIconRect(index).mouseOver())
		{
			return String{ GetFieldLabel(index) };
		}
	}

	for (size_t index = 0; index < ColorChannelCount; ++index)
	{
		if (GetColorIconRect(index).mouseOver())
		{
			return U"カラー {}"_fmt(GetColorLabel(index));
		}
	}

	if (GetColorPreviewIcon().mouseOver())
	{
		return U"最終カラー";
	}

	if (GetKeyboardHelpIcon().mouseOver())
	{
		return U"↑↓で選択中数値を調整\nShift 併用で大きく変更";
	}

	if (GetMetricIcon(0).mouseOver())
	{
		return U"DPS";
	}

	if (GetMetricIcon(1).mouseOver())
	{
        return IsEnemyEditor() ? U"移動速度" : U"HP / Cost";
	}

	if (GetMetricIcon(2).mouseOver())
	{
       return IsEnemyEditor() ? U"10タイル接近時間" : U"100HP 撃破目安";
	}

	if (GetMetricIcon(3).mouseOver())
	{
     return IsEnemyEditor() ? U"Speed x DPS" : U"射程 x DPS";
	}

	if (GetChangeListIcon().mouseOver())
	{
		return U"変更差分";
	}

	if (GetResetButton().mouseOver())
	{
		return U"初期値に戻す";
	}

	if (GetReloadButton().mouseOver())
	{
		return U"定義を再読込";
	}

	if (GetSaveButton().mouseOver())
	{
		return U"保存";
	}

	if (GetBackButton().mouseOver())
	{
      return GetBackButtonTooltipText();
	}

	return none;
}

String UnitEditorScene::GetPendingActionMessage() const
{
	switch (m_pendingAction)
	{
	case PendingAction::BackToFormation:
        return getData().unitEditorReturnToWaveEditor ? U"Wave編集へ戻ります" : U"編成画面へ戻ります";
	case PendingAction::SwitchUnit:
		return m_pendingUnitId ? U"{} に切り替えます"_fmt(ff::GetUnitDefinition(*m_pendingUnitId).label) : U"別ユニットへ切り替えます";
 case PendingAction::SwitchEnemy:
		return m_pendingEnemyKind ? U"{} に切り替えます"_fmt(ff::GetEnemyDefinition(*m_pendingEnemyKind).label) : U"別エネミーへ切り替えます";
	case PendingAction::ReloadFromDisk:
		return U"ディスクから再読込します";
	case PendingAction::ResetToDefault:
		return U"初期値へ戻します";
	case PendingAction::None:
	default:
		return U"変更を破棄します";
	}
}
