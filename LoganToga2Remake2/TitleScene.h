#pragma once

#include "GameData.h"

class TitleScene : public SceneBase
{
public:
	explicit TitleScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

	void update() override
	{
		if (KeyEnter.down() || SimpleGUI::Button(U"Battle Start", Vec2{ 530, 420 }, 220))
		{
			changeScene(U"Battle");
		}
	}

	void draw() const override
	{
		Scene::Rect().draw(ColorF{ 0.08, 0.10, 0.14 });
		RectF{ 120, 100, 1040, 520 }.draw(ColorF{ 0.13, 0.16, 0.20 });
		RectF{ 120, 100, 1040, 520 }.drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

		const auto& data = getData();
		data.titleFont(U"LoganToga2Remake2").drawAt(Scene::CenterF().movedBy(0, -170), Palette::White);
		data.uiFont(U"OpenSiv3D の SceneManager で再構成した起動版").drawAt(Scene::CenterF().movedBy(0, -100), ColorF{ 0.75, 0.86, 1.0 });
		data.smallFont(U"・タイトル -> 戦闘 の遷移を SceneManager で管理").drawAt(Scene::CenterF().movedBy(0, -20), Palette::White);
		data.smallFont(U"・戦闘ロジックは LoganToga2Remake から移植").drawAt(Scene::CenterF().movedBy(0, 12), Palette::White);
		data.smallFont(U"・勝敗後は Title に戻るか、その場で再戦可能").drawAt(Scene::CenterF().movedBy(0, 44), Palette::White);
		data.smallFont(U"Enter でも開始できます").drawAt(Scene::CenterF().movedBy(0, 112), Palette::Yellow);
	}
};
