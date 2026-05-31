#pragma once
# include <Siv3D.hpp>
# include "MapEditorUiLayout.h"

namespace LT3
{
	struct ClickDebugState
	{
		Vec2 currentScreen{ 0, 0 };
		Vec2 currentWorld{ 0, 0 };
		Optional<Point> currentCell;
		Optional<Vec2> lastLeftScreen;
		Optional<Vec2> lastLeftWorld;
		Optional<Point> lastLeftCell;
		Optional<Vec2> lastRightScreen;
		Optional<Vec2> lastRightWorld;
		Optional<Point> lastRightCell;
	};

	// 勢力名をデバッグ表示用の文字列へ変換する。
	inline String FormatFaction(Faction faction)
	{
		switch (faction)
		{
		case Faction::Player:
			return U"Player";
		case Faction::Enemy:
			return U"Enemy";
		default:
			return U"Neutral";
		}
	}

	// クリック位置デバッグオーバーレイを描画する。
	inline void DrawClickDebugOverlay(const ClickDebugState& debugState, const Font& uiFont)
	{
		const RectF panel{ 24, 72, 520, 176 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Click Debug").draw(44, 88, Palette::White);
		uiFont(U"Cursor screen: ({:.1f}, {:.1f})"_fmt(debugState.currentScreen.x, debugState.currentScreen.y)).draw(13, 44, 116, Palette::Lightgray);
		uiFont(U"Cursor world : ({:.1f}, {:.1f})"_fmt(debugState.currentWorld.x, debugState.currentWorld.y)).draw(13, 44, 136, Palette::Lightgray);
		uiFont(debugState.currentCell
			? U"Cursor cell  : ({}, {})"_fmt(debugState.currentCell->x, debugState.currentCell->y)
			: U"Cursor cell  : (n/a)").draw(13, 44, 156, Palette::Lightgray);

		const String lastLeft = debugState.lastLeftScreen
			? U"L screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
				debugState.lastLeftScreen->x,
				debugState.lastLeftScreen->y,
				debugState.lastLeftWorld ? debugState.lastLeftWorld->x : 0.0,
				debugState.lastLeftWorld ? debugState.lastLeftWorld->y : 0.0,
				debugState.lastLeftCell ? U"({}, {})"_fmt(debugState.lastLeftCell->x, debugState.lastLeftCell->y) : U"n/a")
			: U"L screen=(n/a) world=(n/a) cell=n/a";
		const String lastRight = debugState.lastRightScreen
			? U"R screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
				debugState.lastRightScreen->x,
				debugState.lastRightScreen->y,
				debugState.lastRightWorld ? debugState.lastRightWorld->x : 0.0,
				debugState.lastRightWorld ? debugState.lastRightWorld->y : 0.0,
				debugState.lastRightCell ? U"({}, {})"_fmt(debugState.lastRightCell->x, debugState.lastRightCell->y) : U"n/a")
			: U"R screen=(n/a) world=(n/a) cell=n/a";

		uiFont(lastLeft).draw(13, 44, 184, Palette::Skyblue);
		uiFont(lastRight).draw(13, 44, 204, Palette::Orange);
	}

	// UI レイアウト編集用ドラッグハンドルを描画する。
	inline void DrawUiLayoutDragHandle(const RectF& panelRect, bool active)
	{
		if (!active)
		{
			return;
		}

		const RectF handle = UiLayoutDragHandleRect(panelRect);
		handle.draw(ColorF{ 1.0, 0.84, 0.0, 0.18 }).drawFrame(2.0, ColorF{ 1.0, 0.84, 0.0, 0.90 });
	}

	// UI レイアウト編集用の上端固定トグルを描画する。
	inline void DrawUiLayoutTopAnchorToggle(const RectF& panelRect, bool active, bool topAnchor)
	{
		if (!active)
		{
			return;
		}

		const RectF toggleRect = UiLayoutTopAnchorToggleRect(UiLayoutDragHandleRect(panelRect));
		toggleRect.draw(topAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2.0, toggleRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.18 });

		const Vec2 center = toggleRect.center();
		Line{ center.x - 5.0, center.y - 4.0, center.x + 5.0, center.y - 4.0 }.draw(2.0, topAnchor ? Palette::White : Palette::Lightgray);
		Triangle{ Vec2{ center.x, center.y - 7.0 }, Vec2{ center.x - 4.0, center.y }, Vec2{ center.x + 4.0, center.y } }.draw(topAnchor ? Palette::White : Palette::Lightgray);
	}
}
