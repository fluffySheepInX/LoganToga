#include "MenuButtonUi.h"

namespace
{
	constexpr int32 PauseMenuItemCount = 3;

	[[nodiscard]] Array<String> BuildWrappedPauseLines(const Array<String>& entries, const Font& font, const double maxWidth, const String& prefix)
	{
		Array<String> lines;
		String currentLine = prefix;

		for (size_t index = 0; index < entries.size(); ++index)
		{
			const String segment = ((currentLine == prefix) ? U"" : U" / ") + entries[index];
			const String candidate = currentLine + segment;
			if ((currentLine != prefix) && (font(candidate).region().w > maxWidth))
			{
				lines << currentLine;
				currentLine = prefix + entries[index];
				continue;
			}

			currentLine = candidate;
		}

		if (!currentLine.isEmpty())
		{
			lines << currentLine;
		}

		return lines;
	}

	[[nodiscard]] Array<String> GetPauseMenuLabels()
	{
		return{ U"Resume", U"Retry", U"Back to Title" };
	}

	[[nodiscard]] RectF GetPausePanelRect()
	{
		return RectF{ Arg::center(Scene::CenterF()), Scene::Width() - 48.0, 500 };
	}

	[[nodiscard]] RectF GetPauseMenuItemRect(const int32 index)
	{
		const RectF panelRect = GetPausePanelRect();
		return RectF{ Arg::center(panelRect.center().movedBy(0, -46 + (index * 56))), 280, 44 };
	}
}

void BattleScene::clearTransientInputState()
{
	resetCameraPan();
	auto& state = m_session.state();
	state.isSelecting = false;
	state.selectionStart = Vec2::Zero();
	state.selectionRect = RectF{ 0, 0, 0, 0 };
	state.isCommandDragging = false;
	state.commandDragStart = Vec2::Zero();
	state.commandDragCurrent = Vec2::Zero();
}

void BattleScene::togglePause()
{
	if (m_session.state().winner)
	{
		return;
	}

	m_isPaused = !m_isPaused;
	clearTransientInputState();
	if (m_isPaused)
	{
		m_pauseMenuIndex = 0;
	}
}

void BattleScene::updatePauseMenu()
{
	if (KeyEscape.down())
	{
		m_isPaused = false;
		clearTransientInputState();
		return;
	}

	if (KeyUp.down())
	{
		m_pauseMenuIndex = (m_pauseMenuIndex + PauseMenuItemCount - 1) % PauseMenuItemCount;
	}
	else if (KeyDown.down())
	{
		m_pauseMenuIndex = (m_pauseMenuIndex + 1) % PauseMenuItemCount;
	}

	for (int32 i = 0; i < PauseMenuItemCount; ++i)
	{
		if (GetPauseMenuItemRect(i).mouseOver())
		{
			m_pauseMenuIndex = i;
			break;
		}
	}

	const bool decidedByMouse = MouseL.down() && GetPauseMenuItemRect(m_pauseMenuIndex).mouseOver();
	const bool decidedByKey = KeyEnter.down();
	if (!(decidedByMouse || decidedByKey))
	{
		return;
	}

	switch (m_pauseMenuIndex)
	{
	case 0:
		m_isPaused = false;
		clearTransientInputState();
		break;
	case 1:
		RequestSceneTransition(getData(), U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		break;
	case 2:
		if (getData().battleLaunchMode == BattleLaunchMode::Tutorial)
		{
			getData().battleLaunchMode = BattleLaunchMode::Run;
		}
		else
		{
			SaveContinueRun(getData(), ContinueResumeScene::Battle);
		}
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		break;
	default:
		break;
	}
}

void BattleScene::drawPauseMenu() const
{
	const auto& data = getData();
	const auto& config = m_session.config();
	const RectF panelRect = GetPausePanelRect();
	const Array<String> labels = GetPauseMenuLabels();
	Array<String> productionEntries;
	for (const auto& slot : config.playerProductionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableProductionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = (slot.cost > 0)
			? slot.cost
			: (definition ? definition->cost : 0);
		productionEntries << s3d::Format(
			slot.slot,
			U": ",
			GetArchetypeLabel(slot.archetype),
			(slot.batchCount >= 2) ? U" x" + Format(slot.batchCount) : U"",
			U" (",
			cost,
			U"G)");
	}
	const Array<String> productionLines = BuildWrappedPauseLines(productionEntries, data.smallFont, panelRect.w - 80.0, U"Production: ");
	String constructionText = U"Build: none";
	for (const auto& slot : config.playerConstructionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = definition ? definition->cost : 0;
		constructionText = s3d::Format(U"Build: ", slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" (", cost, U"G)");
		break;
	}

	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.45 });
	RoundRect{ panelRect, 14 }.draw(ColorF{ 0.04, 0.06, 0.10, 0.96 });
	RoundRect{ panelRect, 14 }.drawFrame(2, 0, ColorF{ 0.32, 0.48, 0.82, 0.96 });
	data.titleFont(U"PAUSED").drawAt(panelRect.center().movedBy(0, -145), Palette::White);

	for (int32 i = 0; i < labels.size(); ++i)
	{
		const RectF buttonRect = GetPauseMenuItemRect(i);
		DrawMenuButton(buttonRect, labels[i], data.uiFont, m_pauseMenuIndex == i);
	}

	const double infoBaseY = panelRect.center().y + 112.0;
	const double lineStep = 22.0;
	for (size_t index = 0; index < productionLines.size(); ++index)
	{
		data.smallFont(productionLines[index]).drawAt(Vec2{ panelRect.center().x, infoBaseY + (lineStep * index) }, Palette::White);
	}
	const double constructionY = infoBaseY + (lineStep * productionLines.size());
	data.smallFont(constructionText).drawAt(Vec2{ panelRect.center().x, constructionY }, Palette::White);
	data.smallFont(U"Menu: Esc resume / Up-Down select / Enter confirm").drawAt(Vec2{ panelRect.center().x, constructionY + 30.0 }, ColorF{ 0.82, 0.87, 0.94 });
	data.smallFont(config.hud.escapeHint).drawAt(Vec2{ panelRect.center().x, constructionY + 56.0 }, ColorF{ 0.76, 0.82, 0.90 });
	data.smallFont(config.hud.controls).drawAt(Vec2{ panelRect.center().x, constructionY + 82.0 }, ColorF{ 0.72, 0.78, 0.86 });
}
