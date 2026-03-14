#pragma once

#include "GameData.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class TitleScene : public SceneBase
{
public:
	explicit TitleScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

	void update() override
	{
		if (UpdateSceneTransition(getData(), [this](const String& sceneName)
		{
			changeScene(sceneName);
		}))
		{
			return;
		}

		refreshContinueState();

		auto& data = getData();
		const bool hasContinue = m_hasContinue;
		const double continueButtonOffset = 120;
		const double tutorialButtonOffset = hasContinue ? 172 : 120;
		const double startButtonOffset = hasContinue ? 224 : 172;
		const double bonusButtonOffset = hasContinue ? 276 : 224;
		const double debugButtonOffset = hasContinue ? 328 : 276;

		if (hasContinue && (KeyEnter.down() || isButtonClicked(getMenuButtonRect(continueButtonOffset))))
		{
			ContinueResumeScene resumeScene = ContinueResumeScene::Battle;
			if (LoadContinueRun(data, resumeScene))
			{
				RequestSceneTransition(data, GetContinueResumeSceneName(resumeScene), [this](const String& sceneName)
				{
					changeScene(sceneName);
				});
				return;
			}

			ClearContinueRunSave();
			refreshContinueState();
		}

		if ((!hasContinue && KeyEnter.down()) || isButtonClicked(getMenuButtonRect(startButtonOffset)))
		{
			data.battleLaunchMode = BattleLaunchMode::Run;
			BeginNewRun(data.runState, data.baseBattleConfig, false);
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			SaveContinueRun(data, ContinueResumeScene::Battle);
			RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		if (isButtonClicked(getMenuButtonRect(tutorialButtonOffset)))
		{
			data.battleLaunchMode = BattleLaunchMode::Tutorial;
			RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		auto& bonusRoomProgress = data.bonusRoomProgress;
		const Array<const BonusRoomDefinition*> viewedRooms = CollectViewedBonusRooms(data.bonusRooms, bonusRoomProgress);
		if (!viewedRooms.isEmpty() && isButtonClicked(getMenuButtonRect(bonusButtonOffset)))
		{
			ResetBonusRoomSceneState(bonusRoomProgress);
			bonusRoomProgress.sceneMode = BonusRoomSceneMode::Gallery;
			RequestSceneTransition(data, U"BonusRoom", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		const Array<WindowResolutionPreset> presets =
		{
			WindowResolutionPreset::Small,
			WindowResolutionPreset::Medium,
			WindowResolutionPreset::Large,
		};

		for (size_t i = 0; i < presets.size(); ++i)
		{
			const WindowResolutionPreset preset = presets[i];
			if (!isButtonClicked(getResolutionButtonRect(i)))
			{
				continue;
			}

			data.displaySettings.resolutionPreset = preset;
			ApplyDisplaySettings(data.displaySettings);
		}

#ifdef _DEBUG
		if (isButtonClicked(getMenuButtonRect(debugButtonOffset)))
		{
			data.battleLaunchMode = BattleLaunchMode::Run;
			BeginNewRun(data.runState, data.baseBattleConfig, true);
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			SaveContinueRun(data, ContinueResumeScene::Battle);
			RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		if (isButtonClicked(getTransitionPresetButtonRect()))
		{
			data.sceneTransitionSettings.preset = CycleSceneTransitionPreset(data.sceneTransitionSettings.preset);
			return;
		}

		if (isButtonClicked(getMapEditButtonRect()))
		{
			RequestSceneTransition(data, U"MapEdit", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		if (isButtonClicked(getBalanceEditButtonRect()))
		{
			RequestSceneTransition(data, U"BalanceEdit", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}
#endif
	}

	void draw() const override
	{
		Scene::Rect().draw(ColorF{ 0.08, 0.10, 0.14 });
		getPanelRect().draw(ColorF{ 0.13, 0.16, 0.20 });
		getPanelRect().drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

		const auto& data = getData();
		const bool hasContinue = m_hasContinue;
		const auto& continuePreview = m_continuePreview;
		const double continueButtonOffset = 120;
		const double tutorialButtonOffset = hasContinue ? 172 : 120;
		const double startButtonOffset = hasContinue ? 224 : 172;
		const double bonusButtonOffset = hasContinue ? 276 : 224;
		const double debugButtonOffset = hasContinue ? 328 : 276;
		data.titleFont(U"LoganToga2Remake2").drawAt(Scene::CenterF().movedBy(0, -170), Palette::White);
		data.uiFont(U"RTS run prototype").drawAt(Scene::CenterF().movedBy(0, -100), ColorF{ 0.75, 0.86, 1.0 });
		data.smallFont(U"・3-5 battles per run").drawAt(Scene::CenterF().movedBy(0, -20), Palette::White);
		data.smallFont(U"・Choose 1 of 3 reward cards after each victory").drawAt(Scene::CenterF().movedBy(0, 12), Palette::White);
		data.smallFont(U"・Lose once and the run ends").drawAt(Scene::CenterF().movedBy(0, 44), Palette::White);
		data.smallFont(s3d::Format(U"・Viewed bonus rooms: ", data.bonusRoomProgress.viewedRoomIds.size(), U" / ", data.bonusRooms.size())).drawAt(Scene::CenterF().movedBy(0, 76), Palette::White);
		data.smallFont(hasContinue ? U"Press Enter to continue the saved run" : U"Press Enter to start a new run").drawAt(Scene::CenterF().movedBy(0, 112), Palette::Yellow);
		if (hasContinue)
		{
			drawButton(getMenuButtonRect(continueButtonOffset), U"Continue", data.uiFont, true);
			if (continuePreview)
			{
				drawContinuePreview(*continuePreview, data);
			}
		}
		if (!data.bonusRoomProgress.viewedRoomIds.isEmpty())
		{
			data.smallFont(U"Bonus Rooms can be revisited from this menu").drawAt(Scene::CenterF().movedBy(0, hasContinue ? 156 : 144), ColorF{ 1.0, 0.88, 0.55 });
			drawButton(getMenuButtonRect(bonusButtonOffset), U"Bonus Rooms", data.uiFont);
		}

		drawButton(getMenuButtonRect(tutorialButtonOffset), U"Tutorial", data.uiFont, true);
		drawButton(getMenuButtonRect(startButtonOffset), hasContinue ? U"New Run" : U"Start Run", data.uiFont);
		data.smallFont(U"Move / Build / Produce / Defend").drawAt(Scene::CenterF().movedBy(0, tutorialButtonOffset + 28), ColorF{ 0.88, 0.92, 1.0 });

		const s3d::Size resolutionSize = GetWindowResolutionSize(data.displaySettings.resolutionPreset);
		data.smallFont(U"解像度").draw(getResolutionLabelPos(), Palette::White);
		data.smallFont(s3d::Format(U"現在: ", GetWindowResolutionLabel(data.displaySettings.resolutionPreset), U" (", resolutionSize.x, U"x", resolutionSize.y, U")"))
			.draw(getResolutionLabelPos().movedBy(0, 24), ColorF{ 0.85, 0.92, 1.0 });

		const Array<WindowResolutionPreset> presets =
		{
			WindowResolutionPreset::Small,
			WindowResolutionPreset::Medium,
			WindowResolutionPreset::Large,
		};

		for (size_t i = 0; i < presets.size(); ++i)
		{
			const WindowResolutionPreset preset = presets[i];
			drawButton(getResolutionButtonRect(i), GetWindowResolutionLabel(preset), data.smallFont, data.displaySettings.resolutionPreset == preset);
		}

#ifdef _DEBUG
		data.smallFont(U"DEBUG: Start with all unlockable units/buildings").drawAt(Scene::CenterF().movedBy(0, 178), ColorF{ 1.0, 0.75, 0.45 });
		drawButton(getMenuButtonRect(debugButtonOffset), U"Debug Full Unlock", data.uiFont, true);
		drawButton(getMapEditButtonRect(), U"Map Edit", data.smallFont);
		drawButton(getBalanceEditButtonRect(), U"Balance Edit", data.smallFont);
		drawButton(getTransitionPresetButtonRect(), U"Fade: " + GetSceneTransitionPresetLabel(data.sceneTransitionSettings.preset), data.smallFont, true);
#endif
		DrawSceneTransitionOverlay(data);
	}

private:
	Optional<ContinueRunPreview> m_continuePreview;
	bool m_hasContinue = false;

	void refreshContinueState()
	{
		if (!HasContinueRunSave())
		{
			m_continuePreview.reset();
			m_hasContinue = false;
			return;
		}

		m_continuePreview = LoadContinueRunPreview();
		if (!m_continuePreview)
		{
			ClearContinueRunSave();
		}

		m_hasContinue = m_continuePreview.has_value();
	}

	[[nodiscard]] static RectF getContinuePreviewRect()
	{
		return RectF{ Scene::CenterF().movedBy(156, 118), 308, 92 };
	}

	[[nodiscard]] static String getContinuePreviewHeadline(const ContinueRunPreview& preview)
	{
		switch (preview.resumeScene)
		{
		case ContinueResumeScene::Reward:
			return s3d::Format(U"Reward after battle ", preview.currentBattleIndex + 1, U"/", preview.totalBattles);
		case ContinueResumeScene::BonusRoom:
			return U"Bonus Room after clear";
		case ContinueResumeScene::Battle:
		default:
			return s3d::Format(U"Battle ", preview.currentBattleIndex + 1, U"/", preview.totalBattles);
		}
	}

	[[nodiscard]] static String getContinuePreviewDetail(const ContinueRunPreview& preview)
	{
		switch (preview.resumeScene)
		{
		case ContinueResumeScene::Reward:
			return s3d::Format(U"Reward choices: ", preview.pendingRewardCardCount);
		case ContinueResumeScene::BonusRoom:
			return preview.isCleared ? U"Run cleared" : U"Clear reward available";
		case ContinueResumeScene::Battle:
		default:
			return U"Resume from battle start checkpoint";
		}
	}

	static void drawContinuePreview(const ContinueRunPreview& preview, const GameData& data)
	{
		const RectF rect = getContinuePreviewRect();
		rect.draw(ColorF{ 0.09, 0.12, 0.18, 0.96 });
		rect.drawFrame(2, ColorF{ 0.42, 0.60, 0.92 });
		data.smallFont(U"CONTINUE").draw(rect.x + 14, rect.y + 10, ColorF{ 0.82, 0.90, 1.0 });
		data.smallFont(getContinuePreviewHeadline(preview)).draw(rect.x + 14, rect.y + 32, Palette::White);
		data.smallFont(getContinuePreviewDetail(preview)).draw(rect.x + 14, rect.y + 52, ColorF{ 0.86, 0.90, 0.96 });
		data.smallFont(s3d::Format(U"Cards selected: ", preview.selectedCardCount)).draw(rect.x + 14, rect.y + 72, Palette::Gold);
	}

	[[nodiscard]] static bool isButtonClicked(const RectF& rect)
	{
		return IsMenuButtonClicked(rect);
	}

	static void drawButton(const RectF& rect, const String& label, const Font& font, const bool selected = false)
	{
		DrawMenuButton(rect, label, font, selected);
	}

	[[nodiscard]] static RectF getPanelRect()
	{
		const Vec2 panelSize{ 1040, 520 };
		return RectF{ Arg::center = Scene::CenterF(), panelSize };
	}

	[[nodiscard]] static RectF getMenuButtonRect(const double yOffset)
	{
		return RectF{ Scene::CenterF().movedBy(-110, yOffset), 220, 36 };
	}

	[[nodiscard]] static Vec2 getResolutionLabelPos()
	{
		return Scene::CenterF().movedBy(-150, 250);
	}

	[[nodiscard]] static RectF getResolutionButtonRect(const size_t index)
	{
		return RectF{ getResolutionLabelPos().movedBy(100 + (index * 108), -4), 96, 32 };
	}

	[[nodiscard]] static RectF getMapEditButtonRect()
	{
		const RectF panel = getPanelRect();
		return RectF{ panel.x + panel.w - 150, panel.y + 18, 128, 30 };
	}

	[[nodiscard]] static RectF getTransitionPresetButtonRect()
	{
		const RectF panel = getPanelRect();
		return RectF{ panel.x + panel.w - 214, panel.y + 94, 192, 30 };
	}

	[[nodiscard]] static RectF getBalanceEditButtonRect()
	{
		const RectF panel = getPanelRect();
		return RectF{ panel.x + panel.w - 150, panel.y + 56, 128, 30 };
	}
};
