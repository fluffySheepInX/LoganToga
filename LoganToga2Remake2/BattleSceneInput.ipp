void BattleScene::updateActiveBattle()
{
	if (KeyF6.down())
	{
		m_session.toggleEnemyAiDebugPanel();
	}

	if (KeyF7.down())
	{
		m_session.cycleEnemyAiDebugMode();
	}

	m_camera.update();

	const Vec2 cursorScreenPos = Cursor::PosF();
	const Vec2 constructionCursorWorldPos = screenToWorld(cursorScreenPos);
	m_constructionController.handleInput(m_session, constructionCursorWorldPos);

	if (!(m_session.state().pendingConstructionArchetype || m_session.state().pendingRepairTargeting))
	{
		const bool isCursorOnCommandPanel = m_inputController.isCursorOnCommandPanel(m_session, cursorScreenPos);
		const bool handledCommandPanelClick = m_inputController.handleCommandPanelClick(m_session, cursorScreenPos);
		if (isCursorOnCommandPanel)
		{
			resetCameraPan();
		}

		const Vec2 cursorWorldPos = screenToWorld(cursorScreenPos);
		if (!isCursorOnCommandPanel)
		{
			const bool allowClickSelection = updateCameraPan();
			m_inputController.handleSelectionInput(m_session, cursorWorldPos, allowClickSelection);
			m_inputController.handleCommandInput(m_session, cursorWorldPos);
		}
		else if (handledCommandPanelClick)
		{
			m_session.state().isSelecting = false;
			m_session.state().selectionRect = RectF{ 0, 0, 0, 0 };
		}
	}
	else
	{
		resetCameraPan();
	}

	handleProductionInput();

	const size_t fixedSteps = m_clock.beginFrame();
	for (size_t i = 0; i < fixedSteps; ++i)
	{
		m_session.update(m_clock.stepSeconds());
	}

	playBattleSoundEffects();
}

void BattleScene::playBattleSoundEffects()
{
	m_lightBattleSeCooldown = Max(m_lightBattleSeCooldown - Scene::DeltaTime(), 0.0);
	m_heavyBattleSeCooldown = Max(m_heavyBattleSeCooldown - Scene::DeltaTime(), 0.0);
	m_deathBattleSeCooldown = Max(m_deathBattleSeCooldown - Scene::DeltaTime(), 0.0);

	auto& state = m_session.state();
	if (state.battleAudioEvents.isEmpty())
	{
		return;
	}

	const RectF audibleRect = getCameraWorldRect(48.0);
	struct PrioritizedAudioEvent
	{
		const BattleAudioEvent* event = nullptr;
		int32 priority = 0;
	};

	Array<PrioritizedAudioEvent> candidates;
	candidates.reserve(state.battleAudioEvents.size());
	for (const auto& event : state.battleAudioEvents)
	{
		if (!audibleRect.intersects(event.position))
		{
			continue;
		}

		candidates << PrioritizedAudioEvent{ &event, GetBattleAudioPriority(event) };
	}

	std::sort(candidates.begin(), candidates.end(), [](const PrioritizedAudioEvent& a, const PrioritizedAudioEvent& b)
	{
		return a.priority > b.priority;
	});

	int32 playedCount = 0;
	for (const auto& candidate : candidates)
	{
		if ((playedCount >= 3) || !candidate.event)
		{
			break;
		}

		const auto& event = *candidate.event;
		if (event.kind == BattleAudioEventKind::Death)
		{
			if (m_deathBattleSeCooldown > 0.0)
			{
				continue;
			}

			PlayBattleImpactSe((event.targetOwner == Owner::Player) ? 0.24 : 0.18);
			m_deathBattleSeCooldown = 0.16;
			++playedCount;
			continue;
		}

		if (IsHeavyBattleAudioEvent(event))
		{
			if (m_heavyBattleSeCooldown > 0.0)
			{
				continue;
			}

			PlayBattleImpactSe((event.targetOwner == Owner::Player) ? 0.20 : 0.15);
			m_heavyBattleSeCooldown = 0.11;
			++playedCount;
			continue;
		}

		if (m_lightBattleSeCooldown > 0.0)
		{
			continue;
		}

		PlayBattleHitSe((event.targetOwner == Owner::Player) ? 0.13 : 0.09);
		m_lightBattleSeCooldown = 0.075;
		++playedCount;
	}

	state.battleAudioEvents.clear();
}

bool BattleScene::isCommandSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 1:
		return Key1.down();
	case 2:
		return Key2.down();
	case 3:
		return Key3.down();
	case 4:
		return Key4.down();
	case 5:
		return Key5.down();
	case 6:
		return Key6.down();
	case 7:
		return Key7.down();
	case 8:
		return Key8.down();
	case 9:
		return Key9.down();
	case 0:
		return Key0.down();
	default:
		return false;
	}
}

void BattleScene::handleProductionInput()
{
	if (KeyX.down())
	{
		m_session.cancelLastPlayerProduction();
	}

	for (const auto& command : CollectCommandEntries(m_session.state(), m_session.config()))
	{
		if (!isCommandSlotTriggered(command.slot) || !command.isEnabled)
		{
			continue;
		}

		switch (command.kind)
		{
		case CommandKind::Production:
			m_session.trySpawnPlayerUnit(command.archetype);
			return;
		case CommandKind::Repair:
			m_session.state().pendingConstructionArchetype.reset();
			m_session.state().pendingRepairTargeting = true;
			m_session.state().isSelecting = false;
			m_session.state().selectionRect = RectF{ 0, 0, 0, 0 };
			return;
		case CommandKind::Upgrade:
			if (command.turretUpgradeType)
			{
				m_session.tryUpgradeSelectedTurret(*command.turretUpgradeType);
			}
			return;
		case CommandKind::Detonate:
			m_session.enqueue(IssueGoliathDetonationCommand{ m_session.getSelectedPlayerUnitIds() });
			return;
		default:
			break;
		}
	}
}
