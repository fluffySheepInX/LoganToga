#pragma once

#include "GameData.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class TitleScene : public SceneBase
{
public:
	explicit TitleScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	enum class DataClearAction
	{
		None,
		ContinueRunSave,
		GameSettings,
	};

	Optional<ContinueRunPreview> m_continuePreview;
	bool m_hasContinue = false;
	bool m_isQuickGuideOpen = false;
	DataClearAction m_dataClearAction = DataClearAction::None;
	bool m_isExitDialogOpen = false;

	void updateQuickGuide();
	void refreshContinueState();
	void executeDataClearAction();

	[[nodiscard]] static RectF getContinuePreviewRect();
	[[nodiscard]] static String getContinuePreviewHeadline(const ContinueRunPreview& preview);
	[[nodiscard]] static String getContinuePreviewDetail(const ContinueRunPreview& preview);
	static void drawContinuePreview(const ContinueRunPreview& preview, const GameData& data);
	static void drawQuickGuide(const GameData& data);
	static void drawDataClearDialog(DataClearAction action, const GameData& data);
	static void drawExitDialog(const GameData& data);

	[[nodiscard]] static bool isButtonClicked(const RectF& rect);
	static void drawButton(const RectF& rect, const String& label, const Font& font, bool selected = false);
	[[nodiscard]] static RectF getPanelRect();
	[[nodiscard]] static RectF getMenuButtonRect(double yOffset);
	[[nodiscard]] static RectF getQuickGuidePanelRect();
	[[nodiscard]] static RectF getQuickGuideTutorialButtonRect();
	[[nodiscard]] static RectF getQuickGuideCloseButtonRect();
	[[nodiscard]] static RectF getDataClearDialogRect();
	[[nodiscard]] static RectF getDataClearDialogYesButtonRect();
	[[nodiscard]] static RectF getDataClearDialogNoButtonRect();
	[[nodiscard]] static RectF getExitDialogRect();
	[[nodiscard]] static RectF getExitDialogYesButtonRect();
	[[nodiscard]] static RectF getExitDialogNoButtonRect();
	[[nodiscard]] static Vec2 getResolutionLabelPos();
	[[nodiscard]] static RectF getResolutionButtonRect(size_t index);
	[[nodiscard]] static Vec2 getSaveLocationLabelPos();
	[[nodiscard]] static RectF getSaveLocationButtonRect();
	[[nodiscard]] static RectF getClearContinueRunButtonRect();
	[[nodiscard]] static RectF getClearSettingsButtonRect();
	[[nodiscard]] static RectF getMapEditButtonRect();
	[[nodiscard]] static RectF getTransitionPresetButtonRect();
	[[nodiscard]] static RectF getBalanceEditButtonRect();
};
