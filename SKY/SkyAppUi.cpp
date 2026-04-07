# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
 namespace
	{
      constexpr double ArcaneInfantryCost = 90.0;

		[[nodiscard]] double GetSliderMin(const double value, const double defaultMin)
		{
			return Min(defaultMin, (value - 1.0));
		}

		[[nodiscard]] double GetSliderMax(const double value, const double defaultMax)
		{
			return Max(defaultMax, (value + 1.0));
		}

		[[nodiscard]] double GetTierUpgradeCost(const int32 currentTier)
		{
			return (TierUpgradeBaseCost * Max(1, currentTier));
		}

		[[nodiscard]] StringView ToUnitDisplayName(const SapperUnitType unitType)
		{
			switch (unitType)
			{
			case SapperUnitType::ArcaneInfantry:
				return U"魔導兵(仮)";

			case SapperUnitType::Infantry:
			default:
				return U"兵";
			}
		}

		bool TrySpawnPlayerUnit(Array<SpawnedSapper>& spawnedSappers,
			const Vec3& playerBasePosition,
			const Vec3& rallyPoint,
			ResourceStock& playerResources,
			const SapperUnitType unitType,
			const double manaCost,
			TimedMessage& message)
		{
			if (manaCost <= playerResources.mana)
			{
				SpawnSapper(spawnedSappers, playerBasePosition, rallyPoint, unitType);
				playerResources.mana -= manaCost;
				message.show(U"{}を出撃"_fmt(ToUnitDisplayName(unitType)));
				return true;
			}

			message.show(U"魔力不足");
			return false;
		}
	}

	void DrawSkySettingsPanel(Sky& sky, const SkyAppPanels& panels)
	{
		panels.skySettings.draw(ColorF{ 1.0, 0.92 });
		Rect{ 20, 20, 480, 76 }.draw();
		SimpleGUI::GetFont()(U"zenith:").draw(28, 24, ColorF{ 0.11 });
		Rect{ 100, 26, 28 }.draw(sky.zenithColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"horizon:").draw(148, 24, ColorF{ 0.11 });
		Rect{ 230, 26, 28 }.draw(sky.horizonColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"cloud:").draw(276, 24, ColorF{ 0.11 });
		Rect{ 340, 26, 28 }.draw(sky.cloudColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sun:").draw(386, 24, ColorF{ 0.11 });
		Rect{ 430, 26, 28 }.draw(Graphics3D::GetSunColor().gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sunDir: {:.2f}   cloudTime: {:.1f}"_fmt(Graphics3D::GetSunDirection(), sky.cloudTime)).draw(28, 60, ColorF{ 0.11 });

		SimpleGUI::Slider(U"cloudiness: {:.3f}"_fmt(sky.cloudiness), sky.cloudiness, Vec2{ 20, 100 }, 180, 300);
		SimpleGUI::Slider(U"cloudScale: {:.2f}"_fmt(sky.cloudScale), sky.cloudScale, 0.0, 2.0, Vec2{ 20, 140 }, 180, 300);
		SimpleGUI::Slider(U"cloudHeight: {:.0f}"_fmt(sky.cloudPlaneHeight), sky.cloudPlaneHeight, 20.0, 6000.0, Vec2{ 20, 180 }, 180, 300);
		SimpleGUI::Slider(U"orientation: {:.0f}"_fmt(Math::ToDegrees(sky.cloudOrientation)), sky.cloudOrientation, 0.0, Math::TwoPi, Vec2{ 20, 220 }, 180, 300);
		SimpleGUI::Slider(U"fogHeightSky: {:.2f}"_fmt(sky.fogHeightSky), sky.fogHeightSky, Vec2{ 20, 260 }, 180, 300, false);
		SimpleGUI::Slider(U"star: {:.2f}"_fmt(sky.starBrightness), sky.starBrightness, Vec2{ 20, 300 }, 180, 300, false);
		SimpleGUI::Slider(U"starF: {:.2f}"_fmt(sky.starBrightnessFactor), sky.starBrightnessFactor, Vec2{ 20, 340 }, 180, 300);
		SimpleGUI::Slider(U"starSat: {:.2f}"_fmt(sky.starSaturation), sky.starSaturation, 0.0, 1.0, Vec2{ 20, 380 }, 180, 300);
		SimpleGUI::CheckBox(sky.sunEnabled, U"sun", Vec2{ 20, 420 }, 120, false);
		SimpleGUI::CheckBox(sky.cloudsEnabled, U"clouds", Vec2{ 150, 420 }, 120);
		SimpleGUI::CheckBox(sky.cloudsLightingEnabled, U"cloudsLighting", Vec2{ 280, 420 }, 220);
	}

 void DrawCameraSettingsPanel(AppCamera3D& camera,
		CameraSettings& cameraSettings,
		BirdModel& birdModel,
		BirdModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels)
	{
        Vec3 editedEye = cameraSettings.eye;
		Vec3 editedFocus = cameraSettings.focus;
		bool cameraChanged = false;

		panels.cameraSettings.draw(ColorF{ 1.0, 0.92 });
		SimpleGUI::GetFont()(U"Camera eye").draw(540, 28, ColorF{ 0.11 });
     cameraChanged = SimpleGUI::Slider(U"eyeX: {:.2f}"_fmt(editedEye.x), editedEye.x, GetSliderMin(cameraSettings.eye.x, -50.0), GetSliderMax(cameraSettings.eye.x, 50.0), Vec2{ 540, 60 }, 140, 180) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"eyeY: {:.2f}"_fmt(editedEye.y), editedEye.y, GetSliderMin(cameraSettings.eye.y, -10.0), GetSliderMax(cameraSettings.eye.y, 50.0), Vec2{ 540, 100 }, 140, 180) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"eyeZ: {:.2f}"_fmt(editedEye.z), editedEye.z, GetSliderMin(cameraSettings.eye.z, -50.0), GetSliderMax(cameraSettings.eye.z, 50.0), Vec2{ 540, 140 }, 140, 180) || cameraChanged;
		SimpleGUI::GetFont()(U"Camera focus").draw(540, 190, ColorF{ 0.11 });
      cameraChanged = SimpleGUI::Slider(U"focusX: {:.2f}"_fmt(editedFocus.x), editedFocus.x, GetSliderMin(cameraSettings.focus.x, -50.0), GetSliderMax(cameraSettings.focus.x, 50.0), Vec2{ 540, 220 }, 140, 180) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"focusY: {:.2f}"_fmt(editedFocus.y), editedFocus.y, GetSliderMin(cameraSettings.focus.y, -10.0), GetSliderMax(cameraSettings.focus.y, 50.0), Vec2{ 540, 260 }, 140, 180) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"focusZ: {:.2f}"_fmt(editedFocus.z), editedFocus.z, GetSliderMin(cameraSettings.focus.z, -50.0), GetSliderMax(cameraSettings.focus.z, 50.0), Vec2{ 540, 300 }, 140, 180) || cameraChanged;

		if (cameraChanged)
		{
			cameraSettings.eye = editedEye;
			cameraSettings.focus = editedFocus;
			EnsureValidCameraSettings(cameraSettings);
		}

		if (DrawTextButton(Rect{ 540, 330, 150, 30 }, U"Save TOML"))
		{
			cameraSaveMessage.show(SaveCameraSettings(cameraSettings)
				? U"Saved: {}"_fmt(CameraSettingsPath)
				: U"Save failed");
		}

		if (DrawTextButton(Rect{ 710, 330, 150, 30 }, U"視点初期化"))
		{
			cameraSettings.eye = DefaultCameraEye;
			cameraSettings.focus = DefaultCameraFocus;
           EnsureValidCameraSettings(cameraSettings);
			ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"DrawCameraSettingsPanel: reset button");
			camera.setView(cameraSettings.eye, cameraSettings.focus);
			cameraSaveMessage.show(U"Camera reset");
		}

		if (cameraSaveMessage.isVisible())
		{
			SimpleGUI::GetFont()(cameraSaveMessage.text).draw(540, 372, ColorF{ 0.11 });
		}

		if (not birdModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"bird.glb load failed").draw(540, 396, ColorF{ 0.75, 0.2, 0.2 });
		}

		if (not ashigaruModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"ashigaru_v2.1.glb load failed").draw(540, 420, ColorF{ 0.75, 0.2, 0.2 });
		}

		DrawAnimationClipSelector(birdModel, U"Bird Clips", 540, 420, 150);
		DrawAnimationClipSelector(ashigaruModel, U"Ashigaru Clips", 710, 420, 150);
	}

	void DrawBlacksmithMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
		const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
		ResourceStock& playerResources,
        int32& playerTier,
		const double sapperCost,
		TimedMessage& blacksmithMenuMessage)
	{
       const double tierUpgradeCost = GetTierUpgradeCost(playerTier);
		panels.blacksmithMenu.draw(ColorF{ 0.98, 0.95 });
		panels.blacksmithMenu.drawFrame(2, 0, ColorF{ 0.25 });
       SimpleGUI::GetFont()(U"兵生産メニュー").draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 12), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 34), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"魔力: {:.0f} / Tier {}"_fmt(playerResources.mana, playerTier)).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 54), ColorF{ 0.12 });

       const Rect produceSapperButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 84), (panels.blacksmithMenu.w - 32), 28 };
        const Rect produceArcaneButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 116), (panels.blacksmithMenu.w - 32), 28 };
		const Rect tierUpgradeButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 148), (panels.blacksmithMenu.w - 32), 28 };

      if (DrawTextButton(produceSapperButton, U"兵を出撃 ({:.0f} 魔力)"_fmt(sapperCost)))
		{
          TrySpawnPlayerUnit(spawnedSappers, playerBasePosition, rallyPoint, playerResources, SapperUnitType::Infantry, sapperCost, blacksmithMenuMessage);
		}

		if (DrawTextButton(produceArcaneButton, U"魔導兵(仮) ({:.0f} 魔力)"_fmt(ArcaneInfantryCost)))
		{
			TrySpawnPlayerUnit(spawnedSappers, playerBasePosition, rallyPoint, playerResources, SapperUnitType::ArcaneInfantry, ArcaneInfantryCost, blacksmithMenuMessage);
		}

       if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード ({:.0f} 予算)"_fmt(tierUpgradeCost)))
		{
           if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
				++playerTier;
				blacksmithMenuMessage.show(U"Tier {} に上昇"_fmt(playerTier));
			}
			else
			{
				blacksmithMenuMessage.show(U"予算不足");
			}
		}

		if (blacksmithMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(blacksmithMenuMessage.text).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y - 28), ColorF{ 0.12 });
		}
	}

	void DrawSapperMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
		const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
		ResourceStock& playerResources,
        int32& playerTier,
		const double sapperCost,
		TimedMessage& sapperMenuMessage)
	{
       const double tierUpgradeCost = GetTierUpgradeCost(playerTier);
		panels.sapperMenu.draw(ColorF{ 0.97, 0.95 });
		panels.sapperMenu.drawFrame(2, 0, ColorF{ 0.25 });
       SimpleGUI::GetFont()(U"兵メニュー").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 12), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 38), ColorF{ 0.12 });
       SimpleGUI::GetFont()(U"魔力: {:.0f} / Tier {}"_fmt(playerResources.mana, playerTier)).draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 60), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"生産").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 86), ColorF{ 0.22 });
     SimpleGUI::GetFont()(U"スキル").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 208), ColorF{ 0.22 });

       const Rect produceSapperButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 110), (panels.sapperMenu.w - 32), 28 };
        const Rect produceArcaneButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 142), (panels.sapperMenu.w - 32), 28 };
		const Rect tierUpgradeButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 174), (panels.sapperMenu.w - 32), 28 };
		const Rect scoutingSkillButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 232), (panels.sapperMenu.w - 32), 28 };
		const Rect fortifySkillButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 264), (panels.sapperMenu.w - 32), 28 };

      if (DrawTextButton(produceSapperButton, U"兵を出撃 ({:.0f} 魔力)"_fmt(sapperCost)))
		{
          TrySpawnPlayerUnit(spawnedSappers, playerBasePosition, rallyPoint, playerResources, SapperUnitType::Infantry, sapperCost, sapperMenuMessage);
		}

		if (DrawTextButton(produceArcaneButton, U"魔導兵(仮) ({:.0f} 魔力)"_fmt(ArcaneInfantryCost)))
		{
			TrySpawnPlayerUnit(spawnedSappers, playerBasePosition, rallyPoint, playerResources, SapperUnitType::ArcaneInfantry, ArcaneInfantryCost, sapperMenuMessage);
		}

       if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード ({:.0f} 予算)"_fmt(tierUpgradeCost)))
		{
           if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
				++playerTier;
				sapperMenuMessage.show(U"Tier {} に上昇"_fmt(playerTier));
			}
			else
			{
				sapperMenuMessage.show(U"予算不足");
			}
		}

		if (DrawTextButton(scoutingSkillButton, U"索敵スキル"))
		{
         sapperMenuMessage.show(U"兵が索敵スキルを準備");
		}

		if (DrawTextButton(fortifySkillButton, U"陣地化スキル"))
		{
            sapperMenuMessage.show(U"兵が陣地化スキルを準備");
		}

		if (sapperMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(sapperMenuMessage.text).draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y - 28), ColorF{ 0.12 });
		}
	}
}

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	void DrawHudUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (state.showUI)
		{
			DrawSkySettingsPanel(state.sky, frame.panels);
			DrawCameraSettingsPanel(state.camera, state.cameraSettings, resources.birdModel, resources.ashigaruModel, state.cameraSaveMessage, frame.panels);
		}

		DrawMiniMap(frame.panels, state.camera, state.mapData, state.spawnedSappers, state.enemySappers, state.resourceAreaStates, state.selectedSapperIndices);

		if (frame.isEditorMode)
		{
			DrawMapEditorPanel(state.mapEditor, state.mapData, MapDataPath, frame.panels.mapEditor);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && state.showBlacksmithMenu)
		{
           DrawBlacksmithMenu(frame.panels, state.spawnedSappers, state.mapData.playerBasePosition, state.mapData.sapperRallyPoint, state.playerResources, state.playerTier, SapperCost, state.blacksmithMenuMessage);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
		{
           DrawSapperMenu(frame.panels, state.spawnedSappers, state.mapData.playerBasePosition, state.mapData.sapperRallyPoint, state.playerResources, state.playerTier, SapperCost, state.blacksmithMenuMessage);
		}

		if (frame.showMillStatusEditor && state.selectedMillIndex)
		{
			DrawMillStatusEditor(frame.panels, state.mapData, *state.selectedMillIndex, MapDataPath, state.mapDataMessage);
		}

		if (DrawTextButton(frame.panels.mapModeToggle, frame.isEditorMode ? U"Map Edit: ON" : U"Map Edit: OFF"))
		{
			state.appMode = frame.isEditorMode ? AppMode::Play : AppMode::EditMap;
			state.showBlacksmithMenu = false;
			state.selectedSapperIndices.clear();
			state.selectedMillIndex.reset();
			state.selectionDragStart.reset();
			state.mapEditor.hoveredGroundPosition.reset();
		}

		if (DrawTextButton(frame.panels.modelHeightModeToggle, state.modelHeightEditMode ? U"Model Height: ON" : U"Model Height: OFF"))
		{
			state.modelHeightEditMode = not state.modelHeightEditMode;
		}

		if (DrawTextButton(frame.panels.reloadMapButton, U"保存済みマップ再読込"))
		{
			const String loadMessage = ReloadMapAndResetMatch(state);
			state.mapDataMessage.show(loadMessage.isEmpty() ? U"保存済みマップを再読込" : loadMessage, 4.0);
		}

		if (DrawTextButton(frame.panels.restartButton, U"試合リスタート"))
		{
			ResetMatch(state);
			state.restartMessage.show(U"試合をリスタート");
		}

		if (state.mapDataMessage.isVisible())
		{
			SimpleGUI::GetFont()(state.mapDataMessage.text).draw(20, (Scene::Height() - 132), ColorF{ 0.12 });
		}

		if (state.restartMessage.isVisible())
		{
			SimpleGUI::GetFont()(state.restartMessage.text).draw(20, (Scene::Height() - 108), ColorF{ 0.12 });
		}

		SimpleGUI::CheckBox(state.showUI, U"UI", Vec2{ 20, Scene::Height() - 100 });
		SimpleGUI::Slider(U"time: {:.2f}"_fmt(state.skyTime), state.skyTime, -2.0, 4.0, Vec2{ 20, Scene::Height() - 60 }, 120, Scene::Width() - 160);
	}
}
