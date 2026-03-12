#pragma once

#include "GameData.h"

class TitleScene : public SceneBase
{
public:
	explicit TitleScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

	void update() override
	{
		auto& data = getData();
		if (KeyEnter.down() || isButtonClicked(getMenuButtonRect(120)))
		{
			BeginNewRun(data.runState, false);
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			changeScene(U"Battle");
			return;
		}

		auto& bonusRoomProgress = data.bonusRoomProgress;
		const Array<const BonusRoomDefinition*> viewedRooms = CollectViewedBonusRooms(data.bonusRooms, bonusRoomProgress);
		if (!viewedRooms.isEmpty() && isButtonClicked(getMenuButtonRect(172)))
		{
			ResetBonusRoomSceneState(bonusRoomProgress);
			bonusRoomProgress.sceneMode = BonusRoomSceneMode::Gallery;
			changeScene(U"BonusRoom");
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
		if (isButtonClicked(getMenuButtonRect(224)))
		{
			BeginNewRun(data.runState, true);
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			changeScene(U"Battle");
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
		data.titleFont(U"LoganToga2Remake2").drawAt(Scene::CenterF().movedBy(0, -170), Palette::White);
		data.uiFont(U"RTS run prototype").drawAt(Scene::CenterF().movedBy(0, -100), ColorF{ 0.75, 0.86, 1.0 });
		data.smallFont(U"・3-5 battles per run").drawAt(Scene::CenterF().movedBy(0, -20), Palette::White);
		data.smallFont(U"・Choose 1 of 3 reward cards after each victory").drawAt(Scene::CenterF().movedBy(0, 12), Palette::White);
		data.smallFont(U"・Lose once and the run ends").drawAt(Scene::CenterF().movedBy(0, 44), Palette::White);
		data.smallFont(s3d::Format(U"・Viewed bonus rooms: ", data.bonusRoomProgress.viewedRoomIds.size(), U" / ", data.bonusRooms.size())).drawAt(Scene::CenterF().movedBy(0, 76), Palette::White);
		data.smallFont(U"Press Enter to start a new run").drawAt(Scene::CenterF().movedBy(0, 112), Palette::Yellow);
		if (!data.bonusRoomProgress.viewedRoomIds.isEmpty())
		{
			data.smallFont(U"Bonus Rooms can be revisited from this menu").drawAt(Scene::CenterF().movedBy(0, 144), ColorF{ 1.0, 0.88, 0.55 });
			drawButton(getMenuButtonRect(172), U"Bonus Rooms", data.uiFont);
		}

		drawButton(getMenuButtonRect(120), U"Start Run", data.uiFont);

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
		drawButton(getMenuButtonRect(224), U"Debug Full Unlock", data.uiFont, true);
#endif
	}

private:
	[[nodiscard]] static bool isButtonClicked(const RectF& rect)
	{
		return rect.mouseOver() && MouseL.down();
	}

	static void drawButton(const RectF& rect, const String& label, const Font& font, const bool selected = false)
	{
		const bool hovered = rect.mouseOver();
		const ColorF fillColor = selected
			? (hovered ? ColorF{ 0.34, 0.48, 0.76 } : ColorF{ 0.26, 0.38, 0.64 })
			: (hovered ? ColorF{ 0.24, 0.29, 0.38 } : ColorF{ 0.18, 0.22, 0.29 });
		const ColorF frameColor = hovered ? ColorF{ 0.78, 0.88, 1.0 } : ColorF{ 0.42, 0.56, 0.78 };

		rect.draw(fillColor);
		rect.drawFrame(2, frameColor);
		font(label).drawAt(rect.center(), Palette::White);
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
};
