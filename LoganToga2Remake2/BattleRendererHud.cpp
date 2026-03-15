#include "BattleRendererHudInternal.h"

#include <unordered_map>

namespace
{
	Array<String> BuildWrappedHudLines(const Array<String>& entries, const Font& font, const double maxWidth, const String& prefix)
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

	ColorF GetFormationAccentColor(const FormationType formation)
	{
		switch (formation)
		{
		case FormationType::Line:
			return ColorF{ 0.24, 0.68, 0.96 };
		case FormationType::Column:
			return ColorF{ 0.44, 0.84, 0.54 };
		case FormationType::Square:
			return ColorF{ 0.96, 0.72, 0.30 };
		default:
			return ColorF{ 0.72, 0.76, 0.84 };
		}
	}

	[[nodiscard]] String GetEnemyAiModeLabel(const EnemyAiMode mode)
	{
		switch (mode)
		{
		case EnemyAiMode::StagingAssault:
			return U"STAGING";
		case EnemyAiMode::Default:
		default:
			return U"DEFAULT";
		}
	}

	[[nodiscard]] ColorF GetEnemyAiDebugAccentColor(const EnemyAiMode mode)
	{
		switch (mode)
		{
		case EnemyAiMode::StagingAssault:
			return ColorF{ 0.96, 0.62, 0.28 };
		case EnemyAiMode::Default:
		default:
			return ColorF{ 0.36, 0.74, 0.98 };
		}
	}

	[[nodiscard]] String GetTutorialPhaseLabel(const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
			return U"STEP 1";
		case TutorialPhase::BuildStructure:
			return U"STEP 2";
		case TutorialPhase::PrepareDefense:
			return U"WARNING";
		case TutorialPhase::ProduceUnit:
			return U"STEP 3";
		case TutorialPhase::DefendWave:
			return U"STEP 4";
		case TutorialPhase::Completed:
			return U"DONE";
		case TutorialPhase::None:
		default:
			return U"TUTORIAL";
		}
	}

	[[nodiscard]] ColorF GetTutorialAccentColor(const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
			return ColorF{ 0.30, 0.74, 0.98 };
		case TutorialPhase::BuildStructure:
			return ColorF{ 0.38, 0.86, 0.54 };
		case TutorialPhase::PrepareDefense:
		case TutorialPhase::ProduceUnit:
			return ColorF{ 0.96, 0.72, 0.30 };
		case TutorialPhase::DefendWave:
			return ColorF{ 0.96, 0.46, 0.38 };
		case TutorialPhase::Completed:
			return ColorF{ 0.88, 0.92, 0.98 };
		case TutorialPhase::None:
		default:
			return ColorF{ 0.72, 0.80, 0.92 };
		}
	}

	void DrawTutorialPanel(const BattleState& state, const GameData& gameData)
	{
		if (!state.tutorialActive)
		{
			return;
		}

		const RectF panelRect{ 512, 16, 470, 90 };
		const ColorF accentColor = GetTutorialAccentColor(state.tutorialPhase);
		RoundRect{ panelRect, 12 }.draw(ColorF{ 0.03, 0.05, 0.10, 0.88 });
		RoundRect{ panelRect, 12 }.drawFrame(2, 0, accentColor);
		gameData.smallFont(U"TUTORIAL  " + GetTutorialPhaseLabel(state.tutorialPhase)).draw(panelRect.x + 16, panelRect.y + 12, accentColor);

		const Array<String> objectiveLines = BuildWrappedHudLines({ state.tutorialObjective }, gameData.smallFont, panelRect.w - 32.0, U"");
		for (size_t index = 0; index < objectiveLines.size(); ++index)
		{
			gameData.smallFont(objectiveLines[index]).draw(panelRect.x + 16, panelRect.y + 34 + (index * 20), Palette::White);
		}

		if ((state.tutorialPhase == TutorialPhase::ProduceUnit) && (state.tutorialPhaseTimer > 0.0))
		{
			gameData.smallFont(U"Enemy arrival: " + Format(static_cast<int32>(Ceil(state.tutorialPhaseTimer))) + U"s")
				.draw(panelRect.x + 16, panelRect.bottomY() - 22, ColorF{ 0.96, 0.86, 0.62 });
		}
	}

	void DrawFormationPanel(const BattleState& state, const GameData& gameData)
	{
		const FormationPanelLayout layout = BuildFormationPanelLayout(state);
		const Vec2 cursorScreenPos = Cursor::PosF();

		RoundRect{ layout.panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		RoundRect{ layout.panelRect, 12 }.drawFrame(2, 0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.smallFont(layout.title).draw(layout.panelRect.x + 16, layout.panelRect.y + 12, ColorF{ 0.82, 0.86, 0.92 });

		for (const auto& button : layout.buttons)
		{
			const bool isHovered = button.rect.intersects(cursorScreenPos);
			const bool isPressed = isHovered && MouseL.pressed();
			const ColorF accentColor = GetFormationAccentColor(button.button.formation);
			const RectF animatedRect = isPressed
				? RectF{ button.rect.x + 2, button.rect.y + 3, button.rect.w - 4, button.rect.h - 4 }
				: (isHovered ? RectF{ button.rect.x - 1, button.rect.y - 1, button.rect.w + 2, button.rect.h + 2 } : button.rect);
			const ColorF fillColor = button.button.isActive
				? ColorF{ accentColor.r * 0.30 + 0.12, accentColor.g * 0.30 + 0.12, accentColor.b * 0.30 + 0.12, 0.98 }
				: (isHovered ? ColorF{ 0.16, 0.18, 0.24, 0.98 } : ColorF{ 0.10, 0.11, 0.15, 0.96 });
			const double borderThickness = button.button.isActive ? 4.0 : (isHovered ? 3.0 : 2.0);

			RoundRect{ animatedRect, 10 }.draw(fillColor);
			RoundRect{ animatedRect, 10 }.drawFrame(borderThickness, 0, accentColor);
			RectF{ animatedRect.x + 10, animatedRect.bottomY() - 10, animatedRect.w - 20, 4 }
				.draw(button.button.isActive ? accentColor : ColorF{ accentColor.r, accentColor.g, accentColor.b, 0.45 });
			gameData.smallFont(button.button.label).drawAt(animatedRect.center().movedBy(0, -2), Palette::White);
		}
	}

	void DrawEnemyAiDebugPanel(const BattleState& state, const BattleConfigData& config, const GameData& gameData)
	{
		const auto layout = BuildEnemyAiDebugPanelLayout(state, config);
		if (!layout)
		{
			return;
		}

		const Vec2 cursorScreenPos = Cursor::PosF();
		const ColorF accentColor = GetEnemyAiDebugAccentColor(state.enemyAiResolvedMode);
		RoundRect{ layout->panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.88 });
		RoundRect{ layout->panelRect, 12 }.drawFrame(2, 0, accentColor);
		gameData.smallFont(layout->title).draw(layout->panelRect.x + 16, layout->panelRect.y + 12, ColorF{ 0.90, 0.92, 0.96 });

		for (const auto& button : layout->buttons)
		{
			const bool isHovered = button.rect.intersects(cursorScreenPos);
			const bool isPressed = isHovered && MouseL.pressed();
			const RectF animatedRect = isPressed
				? RectF{ button.rect.x + 1, button.rect.y + 2, button.rect.w - 2, button.rect.h - 2 }
				: (isHovered ? RectF{ button.rect.x - 1, button.rect.y - 1, button.rect.w + 2, button.rect.h + 2 } : button.rect);
			const ColorF fillColor = button.button.isActive
				? ColorF{ accentColor.r * 0.24 + 0.14, accentColor.g * 0.24 + 0.14, accentColor.b * 0.24 + 0.14, 0.98 }
				: (isHovered ? ColorF{ 0.16, 0.18, 0.24, 0.98 } : ColorF{ 0.10, 0.11, 0.15, 0.96 });

			RoundRect{ animatedRect, 8 }.draw(fillColor);
			RoundRect{ animatedRect, 8 }.drawFrame(button.button.isActive ? 3.0 : 2.0, 0, accentColor);
			gameData.smallFont(button.button.label).drawAt(animatedRect.center(), Palette::White);
		}

		const String overrideLabel = state.enemyAiDebugOverrideMode
			? GetEnemyAiModeLabel(*state.enemyAiDebugOverrideMode)
			: U"TOML";
		const String targetLabel = state.enemyAiAssaultTargetUnitId
			? Format(*state.enemyAiAssaultTargetUnitId)
			: U"-";
		const double infoBaseY = layout->panelRect.y + 88.0;
		gameData.smallFont(U"F6: Panel  /  F7: Cycle").draw(layout->panelRect.x + 16, infoBaseY, ColorF{ 0.78, 0.82, 0.88 });
		gameData.smallFont(U"Current: " + GetEnemyAiModeLabel(state.enemyAiResolvedMode)).draw(layout->panelRect.x + 16, infoBaseY + 22.0, Palette::White);
		gameData.smallFont(U"TOML: " + GetEnemyAiModeLabel(config.enemyAI.mode) + U"  /  Override: " + overrideLabel).draw(layout->panelRect.x + 16, infoBaseY + 44.0, Palette::White);
		gameData.smallFont(Format(U"Combat: ", state.enemyAiDebugCombatUnitCount, U"  /  Ready: ", state.enemyAiDebugReadyUnitCount)).draw(layout->panelRect.x + 16, infoBaseY + 66.0, Palette::White);
		gameData.smallFont(Format(U"Stage: ", state.enemyAiStagingTimer, U"  /  Commit: ", state.enemyAiAssaultCommitTimer, U"  /  Target: ", targetLabel)).draw(layout->panelRect.x + 16, infoBaseY + 88.0, Palette::White);
	}
}

void BattleRenderer::drawHud(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	int32 playerResourceIncome = 0;
	int32 playerResourceCount = 0;
	for (const auto& resourcePoint : state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Player)
		{
			playerResourceIncome += resourcePoint.incomeAmount;
			++playerResourceCount;
		}
	}

	const double detailBaseY = 66.0;
	const double hudHeight = 140.0;
	const String runText = state.tutorialActive
		? U"Mode: tutorial"
		: (gameData.runState.isActive
			? s3d::Format(U"Run: battle ", gameData.runState.currentBattleIndex + 1, U"/", gameData.runState.totalBattles)
			: U"Run: inactive");
	const auto commandLayout = BuildCommandPanelLayout(state, config);
	std::unordered_map<int32, const BuildingState*> buildingsByUnitId;
	buildingsByUnitId.reserve(state.buildings.size());
	for (const auto& building : state.buildings)
	{
		buildingsByUnitId.try_emplace(building.unitId, &building);
	}
	const auto queueTarget = BattleRendererHudInternal::FindQueueDisplayTarget(state, buildingsByUnitId);

	RoundRect{ 16, 16, 480, hudHeight, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(s3d::Format(U"Resource: ", playerResourceCount, U" pts / +", playerResourceIncome, U" income")).draw(28, detailBaseY, Palette::White);
	gameData.smallFont(runText).draw(28, detailBaseY + 22.0, Palette::White);
	gameData.smallFont(s3d::Format(U"Gold: ", state.playerGold)).draw(28, detailBaseY + 44.0, Palette::Gold);
	DrawTutorialPanel(state, gameData);
	DrawFormationPanel(state, gameData);
	DrawEnemyAiDebugPanel(state, config, gameData);

	if (queueTarget)
	{
		const RectF queuePanelRect = commandLayout
			? RectF{ Max(16.0, commandLayout->panelRect.x - 242.0 - 12.0), commandLayout->panelRect.y, 242.0, 212.0 }
			: RectF{ Scene::Width() - 242.0 - 16.0, Scene::Height() - 212.0 - 16.0, 242.0, 212.0 };
		BattleRendererHudInternal::DrawQueuePanel(queuePanelRect, *queueTarget, gameData);
	}

	if (commandLayout)
	{
		RoundRect{ commandLayout->panelRect, 12 }.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		RoundRect{ commandLayout->panelRect, 12 }.drawFrame(2, 0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.uiFont(commandLayout->title).draw(commandLayout->panelRect.x + 16, commandLayout->panelRect.y + 8, Palette::White);
		BattleRendererHudInternal::DrawCommandSection(*commandLayout, gameData);
	}

	if (!state.statusMessage.isEmpty())
	{
		const RectF messageRect{ Arg::center(Scene::CenterF().movedBy(0, -220)), 420, 42 };
		RoundRect{ messageRect, 10 }.draw(ColorF{ 0.10, 0.05, 0.05, 0.90 });
		RoundRect{ messageRect, 10 }.drawFrame(2, 0, ColorF{ 0.96, 0.42, 0.36, 0.95 });
		gameData.smallFont(state.statusMessage).drawAt(messageRect.center(), Palette::White);
	}

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		String winHint = config.hud.winHint;
		if (!state.tutorialActive && gameData.runState.isActive)
		{
			if ((*state.winner == Owner::Player) && ((gameData.runState.currentBattleIndex + 1) < gameData.runState.totalBattles))
			{
				winHint = U"Enter: choose reward / R: new run";
			}
			else
			{
				winHint = U"Enter: title / R: new run";
			}
		}
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
