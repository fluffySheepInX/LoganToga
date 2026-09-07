#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "MapEditor.h"
# include "BattleUiPanels.Build.h"

namespace LT3
{
	// 戦闘結果オーバーレイを描画する。
	inline void DrawResultOverlay(const BattleWorld& world, const Font& uiFont, const Font& titleFont)
	{
		if (!IsTerminalBattleOutcome(world.outcome))
		{
			return;
		}

		Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0, 0, 0, 0.58 });
		String resultText = U"DRAW";
		ColorF resultColor{ 0.75, 0.80, 0.90 };
		switch (world.outcome)
		{
		case BattleOutcome::Victory:
			resultText = U"VICTORY";
			resultColor = ColorF{ 1.0, 0.84, 0.0 };
			break;
		case BattleOutcome::Defeat:
			resultText = U"DEFEAT";
			resultColor = ColorF{ 1.0, 0.25, 0.20 };
			break;
		case BattleOutcome::Aborted:
			resultText = U"ABORTED";
			resultColor = ColorF{ 0.85, 0.70, 0.45 };
			break;
		default:
			break;
		}
		titleFont(resultText).drawAt(90, Vec2{ 800, 410 }, resultColor);
		uiFont(U"Press ESC or close from the Gaussian menu.").drawAt(800, 500, Palette::White);
	}

	// 残り時間を mm:ss 形式へ整形する。
	inline String FormatBattleTimerText(double totalSec)
	{
		const int32 remainingSec = Max(0, static_cast<int32>(Ceil(totalSec)));
		const int32 minutes = remainingSec / 60;
		const int32 seconds = remainingSec % 60;
		return U"{:02}:{:02}"_fmt(minutes, seconds);
	}

	// 戦闘制限時間オーバーレイを描画する。
	inline void DrawBattleTimerOverlay(const BattleWorld& world, const Font& uiFont)
	{
		if (world.outcomeRules.timeLimitSec <= 0.0)
		{
			return;
		}

		const double remainingSec = Max(0.0, world.outcomeRules.timeLimitSec - world.elapsedSec);
		const bool urgent = remainingSec <= 60.0;
		ColorF timerColor{ 1.0, 1.0, 1.0 };
		ColorF frameColor{ 1.0, 0.84, 0.0, 0.60 };
		if (urgent)
		{
			timerColor = ColorF{ 1.0, 0.35, 0.30 };
			frameColor = ColorF{ 1.0, 0.30, 0.26, 0.92 };
		}
		const RectF panel{ 636.0, 18.0, 328.0, 54.0 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(2.0, 0.0, frameColor);
		uiFont(U"Time Limit").drawAt(13, panel.center().movedBy(0.0, -12.0), ColorF{ 0.0, 1.0, 1.0 });
		uiFont(FormatBattleTimerText(remainingSec)).drawAt(26, panel.center().movedBy(0.0, 10.0), timerColor);
	}
}
