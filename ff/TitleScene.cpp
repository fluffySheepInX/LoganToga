# include "TitleScene.h"
# include "MenuUi.h"

TitleScene::TitleScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 48, Typeface::Heavy }
	, m_buttonFont{ 24 }
{
	Scene::SetBackground(ColorF{ 0.10, 0.16, 0.28 });
}

void TitleScene::update()
{
	if (GetStartButton().leftClicked())
	{
		changeScene(U"Game");
		return;
	}

	if (GetFormationButton().leftClicked())
	{
		changeScene(U"Formation");
		return;
	}

	if (GetExitButton().leftClicked())
	{
		System::Exit();
	}
}

void TitleScene::draw() const
{
	const RectF panel{ Arg::center = Scene::Center(), 460, 400 };

	panel.rounded(24).draw(ColorF{ 0.08, 0.12, 0.22, 0.86 });
	panel.rounded(24).drawFrame(2, ColorF{ 0.84, 0.90, 1.0, 0.70 });
	m_titleFont(U"LoganToga").drawAt(Scene::Center().movedBy(0, -116), ColorF{ 0.97, 0.98, 1.0 });
	m_buttonFont(U"Press a button").drawAt(Scene::Center().movedBy(0, -60), ColorF{ 0.82, 0.88, 1.0, 0.86 });

	DrawMenuButton(GetStartButton(), m_buttonFont, U"Start");
	DrawMenuButton(GetFormationButton(), m_buttonFont, U"編成");
	DrawMenuButton(GetExitButton(), m_buttonFont, U"Exit");
}

RectF TitleScene::GetStartButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, 20), 240, 56 };
}

RectF TitleScene::GetFormationButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, 96), 240, 56 };
}

RectF TitleScene::GetExitButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, 172), 240, 56 };
}
