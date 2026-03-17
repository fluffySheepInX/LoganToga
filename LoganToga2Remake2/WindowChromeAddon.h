#pragma once

#include "GameSettings.h"
#include "Remake2Common.h"
#include <utility>

class WindowChromeAddon : public s3d::IAddon
{
public:
	static constexpr s3d::StringView AddonName = U"WindowChromeAddon";
	static constexpr s3d::MixBus BgmBus = s3d::MixBus::Index1;
	static constexpr s3d::MixBus SeBus = s3d::MixBus::Index2;

	static void Configure(const String& title, const ColorF& frameColor = ColorF{ 0.32, 0.58, 0.92 }, const ColorF& glowColor = ColorF{ 0.92, 0.56, 0.78 });

private:
	bool update() override;
	void draw() const override;
	void ensureTextures(const s3d::Size& sceneSize);

	[[nodiscard]] RectF getBottomBarRect() const;
	[[nodiscard]] RectF getLeftBorderRect() const;
	[[nodiscard]] RectF getRightBorderRect() const;
	[[nodiscard]] RectF getCloseButtonRect() const;
	[[nodiscard]] RectF getFullscreenButtonRect() const;
	[[nodiscard]] RectF getAudioButtonRect() const;
	[[nodiscard]] RectF getLanguageButtonRect() const;
	[[nodiscard]] RectF getLanguagePanelRect() const;
	[[nodiscard]] RectF getLanguageOptionRect(size_t index) const;
	[[nodiscard]] RectF getVolumePanelRect() const;
	[[nodiscard]] RectF getVolumeRowRect(int32 rowIndex) const;
	[[nodiscard]] RectF getVolumeDecreaseButtonRect(int32 rowIndex) const;
	[[nodiscard]] RectF getVolumeIncreaseButtonRect(int32 rowIndex) const;
	[[nodiscard]] RectF getVolumeGaugeRect(int32 rowIndex) const;
	[[nodiscard]] RectF getCloseDialogRect() const;
	[[nodiscard]] RectF getCloseDialogYesButtonRect() const;
	[[nodiscard]] RectF getCloseDialogNoButtonRect() const;
	[[nodiscard]] RectF getDragRect() const;

	void handleCloseButton();
	void openCloseDialog();
	void handleCloseDialog();
	void handleFullscreenButton();
	void handleAudioButton();
	void handleLanguageButton();
	static double getBusVolume(s3d::MixBus bus);
	static double getMasterVolume();
	static void setBusVolume(s3d::MixBus bus, double volume);
	static void setMasterVolume(double volume);
	void handleVolumeControls();
	void handleMasterVolumeRowControls(int32 rowIndex);
	void handleVolumeRowControls(int32 rowIndex, s3d::MixBus bus);
	void updateLanguageButtonHint();
	void setPersistedMasterVolume(double volume);
	void setPersistedBusVolume(s3d::MixBus bus, double volume);
	void flushPersistedSettings();
	void handleWindowDrag();
	[[nodiscard]] bool isPointerOnInteractiveChrome() const;
	void drawGlow() const;
	void drawGlowSource() const;
	void drawChrome() const;
	void drawControlButton(const RectF& rect, const String& label, bool hovered, bool selected) const;
	void drawLanguageButtonHint(const RectF& rect) const;
	void drawLanguagePanel() const;
	void drawVolumePanel() const;
	void drawMasterVolumeRow(const String& label, int32 rowIndex) const;
	void drawVolumeRow(const String& label, int32 rowIndex, s3d::MixBus bus) const;
	void drawVolumeRowInternal(const String& label, int32 rowIndex, double volume) const;
	void drawCloseDialog() const;
	void drawDialogButton(const RectF& rect, const String& label, bool selected) const;

	String m_title = U"LoganToga2Remake2";
	ColorF m_frameColor{ 0.32, 0.58, 0.92 };
	ColorF m_glowColor{ 0.92, 0.56, 0.78, 0.85 };
	double m_bottomBarHeight = 24.0;
	double m_borderThickness = 2.0;
	double m_innerGlowThickness = 8.0;
	double m_audioButtonWidth = 50.0;
	double m_languageButtonWidth = 94.0;
	double m_languagePanelWidth = 132.0;
	double m_languageRowHeight = 24.0;
	double m_fullscreenButtonWidth = 66.0;
	double m_buttonSpacing = 6.0;
	double m_closeButtonSize = 30.0;
	double m_volumePanelWidth = 190.0;
	double m_volumePanelHeight = 103.0;
	double m_volumeStep = 0.1;
	s3d::Size m_sceneSize{ 0, 0 };
	s3d::RenderTexture m_gaussianA1;
	s3d::RenderTexture m_gaussianB1;
	s3d::RenderTexture m_gaussianA4;
	s3d::RenderTexture m_gaussianB4;
	s3d::RenderTexture m_gaussianA8;
	s3d::RenderTexture m_gaussianB8;
	double m_a1 = 0.0;
	double m_a4 = 0.0;
	double m_a8 = 0.5;
	Font m_titleFont{ FontMethod::MSDF, 12, Typeface::Medium };
	Font m_dialogFont{ FontMethod::MSDF, 24, Typeface::Bold };
	Font m_dialogSmallFont{ 16, Typeface::Medium };
	bool m_isLanguageButtonHintActive = false;
	double m_languageButtonHintElapsed = 0.0;
	double m_languageButtonHintDuration = 2.6;
	bool m_isLanguagePanelOpen = false;
	bool m_isAudioPanelOpen = false;
	bool m_isCloseDialogOpen = false;
	bool m_settingsDirty = false;
	Optional<std::pair<s3d::Point, s3d::Point>> m_dragStartWindow;
};
