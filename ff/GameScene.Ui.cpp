# include "GameScene.h"
# include "MenuUi.h"

namespace
{
	StringView GetTimeOfDayLabel(const ff::TimeOfDay timeOfDay)
	{
		switch (timeOfDay)
		{
		case ff::TimeOfDay::Evening:
			return U"夕方";
		case ff::TimeOfDay::Night:
			return U"夜";
		default:
			return U"昼";
		}
	}
}

bool GameScene::UpdateMenu()
{
	if (not m_menuOpen)
	{
		return false;
	}

	if (GetResumeButton().leftClicked())
	{
		m_menuOpen = false;
		return true;
	}

	if (GetExitGameButton().leftClicked())
	{
		System::Exit();
		return true;
	}

	if (GetBackToTitleButton().leftClicked())
	{
		changeScene(U"Title");
		return true;
	}

	return true;
}

void GameScene::UpdateTimeOfDayButtons()
{
	if (GetEveningButton().leftClicked())
	{
		SetTimeOfDay(ff::TimeOfDay::Evening);
		return;
	}

	if (GetNightButton().leftClicked())
	{
		SetTimeOfDay(ff::TimeOfDay::Night);
	}
}

void GameScene::DrawTimeOfDayButtons() const
{
	const ff::TimeOfDay currentTimeOfDay = getData().timeOfDay;
	const RectF labelRect{ Scene::Width() - 264, 16, 240, 32 };
	const auto drawButton = [&](const RectF& rect, const String& label, const ff::TimeOfDay timeOfDay, const ColorF& accentColor)
		{
			const bool selected = (currentTimeOfDay == timeOfDay);
			const bool hovered = rect.mouseOver();
			const ColorF fillColor = selected
				? accentColor
				: ColorF{ 0.10, 0.14, 0.24, 0.84 };
			const ColorF drawColor = hovered
				? fillColor.lerp(ColorF{ 0.92, 0.96, 1.0, 0.94 }, selected ? 0.12 : 0.08)
				: fillColor;
			const ColorF frameColor = selected
				? accentColor.lerp(Palette::White, 0.45)
				: ColorF{ 0.78, 0.86, 0.96, 0.76 };

			if (hovered)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}

			rect.rounded(10).draw(drawColor);
			rect.rounded(10).drawFrame(2, frameColor);
			m_font(label).drawAt(16, rect.center(), Palette::White);
		};

	labelRect.rounded(10).draw(ColorF{ 0.04, 0.06, 0.12, 0.72 });
	labelRect.rounded(10).drawFrame(1.5, ColorF{ 0.78, 0.86, 0.96, 0.52 });
	m_font(U"時間帯: {}"_fmt(GetTimeOfDayLabel(currentTimeOfDay))).drawAt(16, labelRect.center(), Palette::White);

	drawButton(GetEveningButton(), U"夕方", ff::TimeOfDay::Evening, ColorF{ 0.72, 0.38, 0.16, 0.92 });
	drawButton(GetNightButton(), U"夜", ff::TimeOfDay::Night, ColorF{ 0.18, 0.22, 0.46, 0.94 });
}

void GameScene::DrawMenuOverlay() const
{
	const RectF overlay{ Scene::Rect() };
	const RectF menuRect{ Arg::center = Scene::Center(), 340, 280 };

	overlay.draw(ColorF{ 0.0, 0.0, 0.0, 0.45 });
	menuRect.rounded(20).draw(ColorF{ 0.08, 0.12, 0.22, 0.92 });
	menuRect.rounded(20).drawFrame(2, ColorF{ 0.88, 0.92, 1.0, 0.78 });
	m_font(U"メニュー").drawAt(24, menuRect.center().movedBy(0, -92), Palette::White);

	DrawMenuButton(GetResumeButton(), m_font, U"続行");
	DrawMenuButton(GetExitGameButton(), m_font, U"ゲーム終了");
	DrawMenuButton(GetBackToTitleButton(), m_font, U"タイトルバック");
}

RectF GameScene::GetEveningButton() const
{
	return RectF{ Scene::Width() - 264, 56, 112, 40 };
}

RectF GameScene::GetNightButton() const
{
	return RectF{ Scene::Width() - 144, 56, 112, 40 };
}

RectF GameScene::GetResumeButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, -24), 220, 48 };
}

RectF GameScene::GetExitGameButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, 36), 220, 48 };
}

RectF GameScene::GetBackToTitleButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(0, 96), 220, 48 };
}
