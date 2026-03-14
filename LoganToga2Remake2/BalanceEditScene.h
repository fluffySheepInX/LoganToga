#pragma once

#include "BattleConfigLookup.h"
#include "BattleConfigPathResolver.h"
#include "GameData.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class BalanceEditScene : public SceneBase
{
private:
	enum class Tab
	{
		Core,
		Units,
	};

	struct AdjustmentButtonRects
	{
		RectF minusLarge;
		RectF minusSmall;
		RectF plusSmall;
		RectF plusLarge;
	};

	struct HelpText
	{
		String title;
		String body;
	};

public:
	explicit BalanceEditScene(const SceneBase::InitData& init)
		: SceneBase{ init }
	{
		reloadFromDisk(U"Balance editor ready");
	}

	void update() override
	{
		auto& data = getData();
		if (UpdateSceneTransition(data, [this](const String& sceneName)
		{
			changeScene(sceneName);
		}))
		{
			return;
		}

		if (KeyEscape.down() || isButtonClicked(getTopButtonRect(0)))
		{
			RequestSceneTransition(data, U"Title", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		if (isButtonClicked(getTopButtonRect(1)))
		{
			reloadFromDisk(U"Applied current override files");
			return;
		}

		if (isButtonClicked(getTopButtonRect(2)))
		{
			if (saveEditorOverrides())
			{
				reloadFromDisk(U"Saved editor overrides");
			}
			else
			{
				m_statusMessage = U"Failed to save editor overrides";
			}
			return;
		}

		if (isButtonClicked(getTopButtonRect(4)))
		{
			if (clearAllOverrides())
			{
				reloadFromDisk(U"Cleared manual and editor override files");
			}
			else
			{
				m_statusMessage = U"Failed to clear all overrides";
			}
			return;
		}

		if (isButtonClicked(getTopButtonRect(3)))
		{
			if (clearEditorOverrides())
			{
				reloadFromDisk(U"Cleared editor overrides");
			}
			else
			{
				m_statusMessage = U"Failed to clear editor overrides";
			}
			return;
		}

		if (isButtonClicked(getTabButtonRect(Tab::Core)))
		{
			m_tab = Tab::Core;
			return;
		}

		if (isButtonClicked(getTabButtonRect(Tab::Units)))
		{
			m_tab = Tab::Units;
			return;
		}

		if ((m_tab == Tab::Units) && handleUnitListInput())
		{
			return;
		}

		if (m_tab == Tab::Core)
		{
			handleCoreInput();
		}
		else
		{
			handleUnitInput();
		}
	}

	void draw() const override
	{
		Scene::Rect().draw(ColorF{ 0.07, 0.09, 0.13 });
		getLeftPanelRect().draw(ColorF{ 0.12, 0.15, 0.20, 0.98 });
		getRightPanelRect().draw(ColorF{ 0.10, 0.13, 0.18, 0.98 });
		getLeftPanelRect().drawFrame(2, ColorF{ 0.34, 0.48, 0.70 });
		getRightPanelRect().drawFrame(2, ColorF{ 0.34, 0.48, 0.70 });

		const auto& data = getData();
		data.uiFont(U"Balance Editor").draw(getRightPanelRect().x + 16, getRightPanelRect().y + 16, Palette::White);
		data.smallFont(U"Save writes editor-only TOML override files. Manual _override files stay untouched.")
			.draw(getRightPanelRect().x + 16, getRightPanelRect().y + 52, ColorF{ 0.82, 0.90, 1.0 });
		data.smallFont(m_hasUnsavedChanges ? U"Status: edited / unsaved" : U"Status: synced")
			.draw(getRightPanelRect().x + 16, getRightPanelRect().y + 74, m_hasUnsavedChanges ? ColorF{ 1.0, 0.92, 0.25 } : ColorF{ 0.72, 0.88, 0.78 });
		data.smallFont(m_statusMessage).draw(getRightPanelRect().x + 16, getRightPanelRect().y + 96, ColorF{ 0.94, 0.94, 0.98 });

		drawButton(getTopButtonRect(0), U"Back", data.smallFont);
		drawButton(getTopButtonRect(1), U"Apply Overrides", data.smallFont);
		drawButton(getTopButtonRect(2), U"Save", data.smallFont, m_hasUnsavedChanges);
		drawButton(getTopButtonRect(3), U"Clear Editor", data.smallFont);
		drawButton(getTopButtonRect(4), U"Clear All", data.smallFont);

		drawButton(getTabButtonRect(Tab::Core), U"Core", data.smallFont, m_tab == Tab::Core);
		drawButton(getTabButtonRect(Tab::Units), U"Units", data.smallFont, m_tab == Tab::Units);

		drawLeftPanel();
		if (m_tab == Tab::Core)
		{
			drawCorePanel();
		}
		else
		{
			drawUnitPanel();
		}

		drawHelpPanel();

		DrawSceneTransitionOverlay(data);
	}

private:
	BattleConfigData m_editConfig;
	Tab m_tab = Tab::Core;
	int32 m_selectedUnitIndex = 0;
	String m_statusMessage = U"Ready";
	bool m_hasUnsavedChanges = false;

	void reloadFromDisk(const String& statusMessage)
	{
		auto& data = getData();
		ReloadGameConfigData(data);
		m_editConfig = data.baseBattleConfig;
		m_selectedUnitIndex = Clamp(m_selectedUnitIndex, 0, Max(0, static_cast<int32>(m_editConfig.unitDefinitions.size()) - 1));
		m_hasUnsavedChanges = false;
		m_statusMessage = statusMessage;
	}

	[[nodiscard]] bool saveEditorOverrides() const
	{
		FileSystem::CreateDirectories(U"config");
		return saveCoreEditorOverride() && saveUnitsEditorOverride();
	}

	[[nodiscard]] bool clearEditorOverrides() const
	{
		bool success = true;
		if (FileSystem::Exists(getCoreEditorOverridePath()))
		{
			success = success && FileSystem::Remove(getCoreEditorOverridePath());
		}
		if (FileSystem::Exists(getUnitsEditorOverridePath()))
		{
			success = success && FileSystem::Remove(getUnitsEditorOverridePath());
		}
		return success;
	}

	[[nodiscard]] bool clearAllOverrides() const
	{
		bool success = true;
		success = removeFileIfExists(getCoreOverridePath()) && success;
		success = removeFileIfExists(getUnitsOverridePath()) && success;
		success = removeFileIfExists(getCoreEditorOverridePath()) && success;
		success = removeFileIfExists(getUnitsEditorOverridePath()) && success;
		return success;
	}

	[[nodiscard]] static bool writeTextFile(const String& path, const String& content)
	{
		TextWriter writer{ path };
		if (!writer)
		{
			return false;
		}

		writer.write(content);
		return true;
	}

	[[nodiscard]] static bool removeFileIfExists(const String& path)
	{
		return !FileSystem::Exists(path) || FileSystem::Remove(path);
	}

	[[nodiscard]] bool saveCoreEditorOverride() const
	{
		String content;
		content += U"[economy]\n";
		appendTomlLine(content, U"player_gold", Format(m_editConfig.playerGold));
		appendTomlLine(content, U"enemy_gold", Format(m_editConfig.enemyGold));
		content += U"\n[income]\n";
		appendTomlLine(content, U"interval", Format(m_editConfig.income.interval));
		appendTomlLine(content, U"player_amount", Format(m_editConfig.income.playerAmount));
		appendTomlLine(content, U"enemy_amount", Format(m_editConfig.income.enemyAmount));
		content += U"\n[enemy_spawn]\n";
		appendTomlLine(content, U"interval", Format(m_editConfig.enemySpawn.interval));
		appendTomlLine(content, U"advanced_probability", Format(m_editConfig.enemySpawn.advancedProbability));
		appendTomlLine(content, U"random_y_offset", Format(m_editConfig.enemySpawn.randomYOffset));
		content += U"\n[enemy_ai]\n";
		appendTomlLine(content, U"assault_unit_threshold", Format(m_editConfig.enemyAI.assaultUnitThreshold));
		appendTomlLine(content, U"staging_assault_min_units", Format(m_editConfig.enemyAI.stagingAssaultMinUnits));
		return writeTextFile(getCoreEditorOverridePath(), content);
	}

	[[nodiscard]] bool saveUnitsEditorOverride() const
	{
		String content;
		for (const auto& unit : m_editConfig.unitDefinitions)
		{
			content += U"[[units]]\n";
			appendTomlLine(content, U"archetype", quoteTomlString(toUnitArchetypeTomlString(unit.archetype)));
			appendTomlLine(content, U"hp", Format(unit.hp));
			appendTomlLine(content, U"attack_power", Format(unit.attackPower));
			appendTomlLine(content, U"cost", Format(unit.cost));
			appendTomlLine(content, U"production_time", Format(unit.productionTime));
			appendTomlLine(content, U"move_speed", Format(unit.moveSpeed));
			appendTomlLine(content, U"attack_range", Format(unit.attackRange));
			appendTomlLine(content, U"attack_cooldown", Format(unit.attackCooldown));
			appendTomlLine(content, U"aggro_range", Format(unit.aggroRange));
			content += U"\n";
		}

		for (const auto& slot : m_editConfig.playerProductionSlots)
		{
			content += U"[[player_production]]\n";
			appendTomlLine(content, U"slot", Format(slot.slot));
			appendTomlLine(content, U"producer", quoteTomlString(toUnitArchetypeTomlString(slot.producer)));
			appendTomlLine(content, U"archetype", quoteTomlString(toUnitArchetypeTomlString(slot.archetype)));
			appendTomlLine(content, U"cost", Format(slot.cost));
			appendTomlLine(content, U"batch_count", Format(slot.batchCount));
			content += U"\n";
		}

		return writeTextFile(getUnitsEditorOverridePath(), content);
	}

	void drawLeftPanel() const
	{
		const auto& data = getData();
		const RectF panel = getLeftPanelRect();
		data.smallFont(U"Panel").draw(panel.x + 16, panel.y + 16, Palette::White);
		data.smallFont(m_tab == Tab::Core ? U"Global economy / AI values" : U"Select a unit to edit stats and queue values")
			.draw(panel.x + 16, panel.y + 40, ColorF{ 0.84, 0.90, 0.98 });

		if (m_tab != Tab::Units)
		{
			data.smallFont(U"Core tab edits values from battle_core.toml").draw(panel.x + 16, panel.y + 84, Palette::White);
			data.smallFont(U"Units tab edits battle_units.toml and player_production slots").draw(panel.x + 16, panel.y + 108, Palette::White);
			return;
		}

		for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
		{
			const auto& unit = m_editConfig.unitDefinitions[index];
			drawButton(getUnitButtonRect(static_cast<int32>(index)), toUnitArchetypeDisplayString(unit.archetype), data.smallFont, static_cast<int32>(index) == m_selectedUnitIndex);
		}
	}

	void drawCorePanel() const
	{
		const auto& data = getData();
		drawIntRow(0, U"Player Gold", m_editConfig.playerGold, data.smallFont);
		drawIntRow(1, U"Enemy Gold", m_editConfig.enemyGold, data.smallFont);
		drawDoubleRow(2, U"Income Interval", m_editConfig.income.interval, data.smallFont);
		drawIntRow(3, U"Player Income", m_editConfig.income.playerAmount, data.smallFont);
		drawIntRow(4, U"Enemy Income", m_editConfig.income.enemyAmount, data.smallFont);
		drawDoubleRow(5, U"Spawn Interval", m_editConfig.enemySpawn.interval, data.smallFont);
		drawDoubleRow(6, U"Advanced Spawn Rate", m_editConfig.enemySpawn.advancedProbability, data.smallFont);
		drawDoubleRow(7, U"Spawn Random Y", m_editConfig.enemySpawn.randomYOffset, data.smallFont);
		drawIntRow(8, U"Assault Threshold", m_editConfig.enemyAI.assaultUnitThreshold, data.smallFont);
		drawIntRow(9, U"Staging Min Units", m_editConfig.enemyAI.stagingAssaultMinUnits, data.smallFont);
	}

	void drawUnitPanel() const
	{
		const auto& data = getData();
		const auto* unit = getSelectedUnitDefinition();
		if (!unit)
		{
			data.smallFont(U"No unit selected").draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
			return;
		}

		data.uiFont(toUnitArchetypeDisplayString(unit->archetype)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
		const auto* slot = getSelectedProductionSlot();
		data.smallFont(slot ? U"Queue values are backed by player_production." : U"This unit has no player production slot.")
			.draw(getEditorPanelRect().x + 220, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

		int32 row = 0;
		drawIntRow(row++, U"HP", unit->hp, data.smallFont);
		drawIntRow(row++, U"Attack", unit->attackPower, data.smallFont);
		drawIntRow(row++, U"Unit Cost", unit->cost, data.smallFont);
		if (slot)
		{
			drawIntRow(row++, U"Queue Cost", getSelectedProductionCost(), data.smallFont);
			drawIntRow(row++, U"Batch Count", slot->batchCount, data.smallFont);
		}
		drawDoubleRow(row++, U"Production Time", unit->productionTime, data.smallFont);
		drawDoubleRow(row++, U"Move Speed", unit->moveSpeed, data.smallFont);
		drawDoubleRow(row++, U"Attack Range", unit->attackRange, data.smallFont);
		drawDoubleRow(row++, U"Attack Cooldown", unit->attackCooldown, data.smallFont);
		drawDoubleRow(row++, U"Aggro Range", unit->aggroRange, data.smallFont);
	}

	void drawHelpPanel() const
	{
		const auto help = getHoveredHelpText();
		const auto& data = getData();
		const RectF panel = getHelpPanelRect();
		panel.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		panel.drawFrame(1, ColorF{ 0.32, 0.44, 0.64 });
		data.smallFont(U"Hover Help").draw(panel.x + 12, panel.y + 8, ColorF{ 0.82, 0.90, 1.0 });
		data.smallFont(help.title).draw(panel.x + 12, panel.y + 30, Palette::White);
		data.smallFont(help.body).draw(panel.x + 12, panel.y + 54, ColorF{ 0.88, 0.92, 0.98 });
	}

	void handleCoreInput()
	{
		bool changed = false;
		changed = handleIntRowInput(0, m_editConfig.playerGold, 10, 50, 0) || changed;
		changed = handleIntRowInput(1, m_editConfig.enemyGold, 10, 50, 0) || changed;
		changed = handleDoubleRowInput(2, m_editConfig.income.interval, 0.1, 0.5, 0.1, 10.0) || changed;
		changed = handleIntRowInput(3, m_editConfig.income.playerAmount, 1, 5, 0) || changed;
		changed = handleIntRowInput(4, m_editConfig.income.enemyAmount, 1, 5, 0) || changed;
		changed = handleDoubleRowInput(5, m_editConfig.enemySpawn.interval, 0.1, 0.5, 0.1, 30.0) || changed;
		changed = handleDoubleRowInput(6, m_editConfig.enemySpawn.advancedProbability, 0.05, 0.2, 0.0, 1.0) || changed;
		changed = handleDoubleRowInput(7, m_editConfig.enemySpawn.randomYOffset, 5.0, 20.0, 0.0, 1000.0) || changed;
		changed = handleIntRowInput(8, m_editConfig.enemyAI.assaultUnitThreshold, 1, 2, 1) || changed;
		changed = handleIntRowInput(9, m_editConfig.enemyAI.stagingAssaultMinUnits, 1, 2, 1) || changed;
		applyEditedState(changed);
	}

	void handleUnitInput()
	{
		auto* unit = getSelectedUnitDefinition();
		if (!unit)
		{
			return;
		}

		bool changed = false;
		int32 row = 0;
		changed = handleIntRowInput(row++, unit->hp, 1, 10, 1) || changed;
		changed = handleIntRowInput(row++, unit->attackPower, 1, 2, 0) || changed;
		changed = handleIntRowInput(row++, unit->cost, 1, 10, 0) || changed;
		if (auto* slot = getSelectedProductionSlot())
		{
			changed = handleSelectedProductionCostInput(row++, 1, 10) || changed;
			changed = handleIntRowInput(row++, slot->batchCount, 1, 1, 1) || changed;
		}
		changed = handleDoubleRowInput(row++, unit->productionTime, 0.1, 0.5, 0.0, 60.0) || changed;
		changed = handleDoubleRowInput(row++, unit->moveSpeed, 1.0, 10.0, 0.0, 1000.0) || changed;
		changed = handleDoubleRowInput(row++, unit->attackRange, 1.0, 10.0, 0.0, 1000.0) || changed;
		changed = handleDoubleRowInput(row++, unit->attackCooldown, 0.05, 0.25, 0.05, 30.0) || changed;
		changed = handleDoubleRowInput(row++, unit->aggroRange, 1.0, 10.0, 0.0, 1000.0) || changed;
		applyEditedState(changed);
	}

	void applyEditedState(const bool changed)
	{
		if (!changed)
		{
			return;
		}

		m_hasUnsavedChanges = true;
		m_statusMessage = U"Edited values locally";
	}

	[[nodiscard]] bool handleUnitListInput()
	{
		for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
		{
			if (!isButtonClicked(getUnitButtonRect(static_cast<int32>(index))))
			{
				continue;
			}

			m_selectedUnitIndex = static_cast<int32>(index);
			return true;
		}

		return false;
	}

	[[nodiscard]] bool handleSelectedProductionCostInput(const int32 rowIndex, const int32 smallStep, const int32 largeStep)
	{
		auto* slot = getSelectedProductionSlot();
		if (!slot)
		{
			return false;
		}

		int32 value = getSelectedProductionCost();
		if (!handleIntRowInput(rowIndex, value, smallStep, largeStep, 0))
		{
			return false;
		}

		slot->cost = value;
		return true;
	}

	[[nodiscard]] int32 getSelectedProductionCost() const
	{
		const auto* unit = getSelectedUnitDefinition();
		const auto* slot = getSelectedProductionSlot();
		if (!unit)
		{
			return 0;
		}
		if (!slot)
		{
			return unit->cost;
		}

		return (slot->cost > 0) ? slot->cost : unit->cost;
	}

	[[nodiscard]] bool handleIntRowInput(const int32 rowIndex, int32& value, const int32 smallStep, const int32 largeStep, const int32 minValue)
	{
		const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
		const int32 originalValue = value;
		if (isButtonClicked(buttons.minusLarge))
		{
			value = Max(minValue, value - largeStep);
		}
		if (isButtonClicked(buttons.minusSmall))
		{
			value = Max(minValue, value - smallStep);
		}
		if (isButtonClicked(buttons.plusSmall))
		{
			value = Max(minValue, value + smallStep);
		}
		if (isButtonClicked(buttons.plusLarge))
		{
			value = Max(minValue, value + largeStep);
		}
		return value != originalValue;
	}

	[[nodiscard]] HelpText getHoveredHelpText() const
	{
		if (getTopButtonRect(0).mouseOver())
		{
			return{ U"Back", U"Return to the title scene. Unsaved local edits in this editor are discarded." };
		}
		if (getTopButtonRect(1).mouseOver())
		{
			return{ U"Apply Overrides", U"Reload config from disk and re-apply both manual _override and _editor_override files." };
		}
		if (getTopButtonRect(2).mouseOver())
		{
			return{ U"Save", U"Write the current editor values into editor-only override TOML files." };
		}
		if (getTopButtonRect(3).mouseOver())
		{
			return{ U"Clear Editor", U"Delete only _editor_override files generated by this editor and reload the base/manual overrides." };
		}
		if (getTopButtonRect(4).mouseOver())
		{
			return{ U"Clear All", U"Delete both manual _override files and editor _editor_override files for core and unit balance." };
		}
		if (getTabButtonRect(Tab::Core).mouseOver())
		{
			return{ U"Core Tab", U"Edit economy, spawn, and enemy AI pacing values from battle_core data." };
		}
		if (getTabButtonRect(Tab::Units).mouseOver())
		{
			return{ U"Units Tab", U"Edit unit stats and player production slot values from battle_units data." };
		}

		if (m_tab == Tab::Units)
		{
			for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
			{
				const auto& unit = m_editConfig.unitDefinitions[index];
				if (getUnitButtonRect(static_cast<int32>(index)).mouseOver())
				{
					return{
						toUnitArchetypeDisplayString(unit.archetype),
						unit.description.isEmpty() ? U"Select this unit to edit its combat and queue balance values." : unit.description
					};
				}
			}
		}

		const auto rowHelp = (m_tab == Tab::Core)
			? getHoveredCoreRowHelp()
			: getHoveredUnitRowHelp();
		if (rowHelp)
		{
			return *rowHelp;
		}

		return{
			U"Balance Editor",
			U"Hover a control or stat row to see what it affects. Save writes editor-only overrides, while Apply Overrides reloads files from disk."
		};
	}

	[[nodiscard]] Optional<HelpText> getHoveredCoreRowHelp() const
	{
		for (int32 row = 0; row < 10; ++row)
		{
			if (!getEditorRowRect(row).mouseOver())
			{
				continue;
			}

			switch (row)
			{
			case 0:
				return HelpText{ U"Player Gold", U"Player starting gold. Higher values let the player open with more buildings or units." };
			case 1:
				return HelpText{ U"Enemy Gold", U"Enemy starting gold. Raising this makes the enemy establish faster in early battle time." };
			case 2:
				return HelpText{ U"Income Interval", U"Time between automatic gold income ticks. Lower is faster economy for both sides." };
			case 3:
				return HelpText{ U"Player Income", U"Gold gained by the player on each income tick." };
			case 4:
				return HelpText{ U"Enemy Income", U"Gold gained by the enemy on each income tick." };
			case 5:
				return HelpText{ U"Spawn Interval", U"Enemy spawn cadence. Lower values create more constant pressure." };
			case 6:
				return HelpText{ U"Advanced Spawn Rate", U"Chance that enemy spawns the advanced archetype instead of the basic one." };
			case 7:
				return HelpText{ U"Spawn Random Y", U"Vertical random spread applied to enemy spawn positions." };
			case 8:
				return HelpText{ U"Assault Threshold", U"Enemy AI waits until this many units are ready before switching to stronger assault behavior." };
			case 9:
				return HelpText{ U"Staging Min Units", U"Minimum units gathered before staging assault commits. Larger values delay attacks but make pushes denser." };
			default:
				break;
			}
		}

		return none;
	}

	[[nodiscard]] Optional<HelpText> getHoveredUnitRowHelp() const
	{
		const bool hasProductionSlot = (getSelectedProductionSlot() != nullptr);
		const int32 rowCount = hasProductionSlot ? 9 : 7;
		for (int32 row = 0; row < rowCount; ++row)
		{
			if (!getEditorRowRect(row).mouseOver())
			{
				continue;
			}

			if (row == 0)
			{
				return HelpText{ U"HP", U"Unit durability. Higher HP increases time alive and usually lowers control stress by making losses less sharp." };
			}
			if (row == 1)
			{
				return HelpText{ U"Attack", U"Damage dealt per attack event. Raising this improves burst and focus fire efficiency." };
			}
			if (row == 2)
			{
				return HelpText{ U"Unit Cost", U"Default cost taken from the unit definition. Used unless a production slot cost override exists." };
			}

			if (hasProductionSlot)
			{
				switch (row)
				{
				case 3:
					return HelpText{ U"Queue Cost", U"Actual player production command cost. If this is set above zero, it overrides the unit's default cost in the command menu and queue." };
				case 4:
					return HelpText{ U"Batch Count", U"How many units are produced from one queue action. Bigger values increase burst production and reduce input count." };
				case 5:
					return HelpText{ U"Production Time", U"Time required to finish one queue item for this unit." };
				case 6:
					return HelpText{ U"Move Speed", U"Travel speed. Higher values improve reinforcement speed and chase potential." };
				case 7:
					return HelpText{ U"Attack Range", U"Distance at which the unit can attack. Higher values improve safety and first-hit control." };
				case 8:
					return HelpText{ U"Attack Cooldown", U"Delay between attacks. Lower values mean faster repeated attacks." };
				default:
					return HelpText{ U"Aggro Range", U"Detection radius for engaging targets. Higher values make units react earlier." };
				}
			}

			switch (row)
			{
			case 3:
				return HelpText{ U"Production Time", U"Time required to finish one queue item for this unit." };
			case 4:
				return HelpText{ U"Move Speed", U"Travel speed. Higher values improve reinforcement speed and chase potential." };
			case 5:
				return HelpText{ U"Attack Range", U"Distance at which the unit can attack. Higher values improve safety and first-hit control." };
			case 6:
				return HelpText{ U"Attack Cooldown", U"Delay between attacks. Lower values mean faster repeated attacks." };
			default:
				return HelpText{ U"Aggro Range", U"Detection radius for engaging targets. Higher values make units react earlier." };
			}
		}

		if (getSelectedProductionSlot() == nullptr && getEditorRowRect(7).mouseOver())
		{
			return HelpText{ U"Aggro Range", U"Detection radius for engaging targets. Higher values make units react earlier." };
		}

		return none;
	}

	[[nodiscard]] bool handleDoubleRowInput(const int32 rowIndex, double& value, const double smallStep, const double largeStep, const double minValue, const double maxValue)
	{
		const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
		const double originalValue = value;
		if (isButtonClicked(buttons.minusLarge))
		{
			value = Clamp(value - largeStep, minValue, maxValue);
		}
		if (isButtonClicked(buttons.minusSmall))
		{
			value = Clamp(value - smallStep, minValue, maxValue);
		}
		if (isButtonClicked(buttons.plusSmall))
		{
			value = Clamp(value + smallStep, minValue, maxValue);
		}
		if (isButtonClicked(buttons.plusLarge))
		{
			value = Clamp(value + largeStep, minValue, maxValue);
		}
		return value != originalValue;
	}

	void drawIntRow(const int32 rowIndex, const String& label, const int32 value, const Font& font) const
	{
		drawAdjustmentRow(getEditorRowRect(rowIndex), label, Format(value), font);
	}

	void drawDoubleRow(const int32 rowIndex, const String& label, const double value, const Font& font) const
	{
		drawAdjustmentRow(getEditorRowRect(rowIndex), label, Format(value), font);
	}

	void drawAdjustmentRow(const RectF& rowRect, const String& label, const String& valueText, const Font& font) const
	{
		rowRect.draw(ColorF{ 0.14, 0.18, 0.24, 0.96 });
		rowRect.drawFrame(1, ColorF{ 0.32, 0.44, 0.64 });
		font(label).draw(rowRect.x + 12, rowRect.y + 8, Palette::White);
		font(valueText).draw(rowRect.x + 220, rowRect.y + 8, ColorF{ 0.90, 0.94, 1.0 });
		const auto buttons = getAdjustmentButtonRects(rowRect);
		const auto& data = getData();
		drawButton(buttons.minusLarge, U"-L", data.smallFont);
		drawButton(buttons.minusSmall, U"-S", data.smallFont);
		drawButton(buttons.plusSmall, U"+S", data.smallFont);
		drawButton(buttons.plusLarge, U"+L", data.smallFont);
	}

	[[nodiscard]] AdjustmentButtonRects getAdjustmentButtonRects(const RectF& rowRect) const
	{
		const double buttonWidth = 50;
		const double buttonHeight = 28;
		const double gap = 8;
		const double right = rowRect.x + rowRect.w - 12;
		return AdjustmentButtonRects{
			RectF{ right - ((buttonWidth + gap) * 4), rowRect.y + 6, buttonWidth, buttonHeight },
			RectF{ right - ((buttonWidth + gap) * 3), rowRect.y + 6, buttonWidth, buttonHeight },
			RectF{ right - ((buttonWidth + gap) * 2), rowRect.y + 6, buttonWidth, buttonHeight },
			RectF{ right - (buttonWidth + gap), rowRect.y + 6, buttonWidth, buttonHeight },
		};
	}

	[[nodiscard]] UnitDefinition* getSelectedUnitDefinition()
	{
		if ((m_selectedUnitIndex < 0) || (static_cast<size_t>(m_selectedUnitIndex) >= m_editConfig.unitDefinitions.size()))
		{
			return nullptr;
		}

		return &m_editConfig.unitDefinitions[static_cast<size_t>(m_selectedUnitIndex)];
	}

	[[nodiscard]] const UnitDefinition* getSelectedUnitDefinition() const
	{
		if ((m_selectedUnitIndex < 0) || (static_cast<size_t>(m_selectedUnitIndex) >= m_editConfig.unitDefinitions.size()))
		{
			return nullptr;
		}

		return &m_editConfig.unitDefinitions[static_cast<size_t>(m_selectedUnitIndex)];
	}

	[[nodiscard]] ProductionSlot* getSelectedProductionSlot()
	{
		const auto* unit = getSelectedUnitDefinition();
		if (!unit)
		{
			return nullptr;
		}

		for (auto& slot : m_editConfig.playerProductionSlots)
		{
			if (slot.archetype == unit->archetype)
			{
				return &slot;
			}
		}

		return nullptr;
	}

	[[nodiscard]] const ProductionSlot* getSelectedProductionSlot() const
	{
		const auto* unit = getSelectedUnitDefinition();
		if (!unit)
		{
			return nullptr;
		}

		for (const auto& slot : m_editConfig.playerProductionSlots)
		{
			if (slot.archetype == unit->archetype)
			{
				return &slot;
			}
		}

		return nullptr;
	}

	[[nodiscard]] static String getCoreEditorOverridePath()
	{
		return ResolveTomlEditorOverridePath(U"config/battle_core.toml");
	}

	[[nodiscard]] static String getCoreOverridePath()
	{
		return ResolveTomlOverridePath(U"config/battle_core.toml");
	}

	[[nodiscard]] static String getUnitsEditorOverridePath()
	{
		return ResolveTomlEditorOverridePath(U"config/battle_units.toml");
	}

	[[nodiscard]] static String getUnitsOverridePath()
	{
		return ResolveTomlOverridePath(U"config/battle_units.toml");
	}

	static void appendTomlLine(String& content, const String& key, const String& value)
	{
		content += key + U" = " + value + U"\n";
	}

	[[nodiscard]] static String quoteTomlString(const String& value)
	{
		String escaped = value;
		escaped.replace(U"\\", U"\\\\");
		escaped.replace(U"\"", U"\\\"");
		return U"\"" + escaped + U"\"";
	}

	[[nodiscard]] static String toUnitArchetypeDisplayString(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
			return U"Base";
		case UnitArchetype::Barracks:
			return U"Barracks";
		case UnitArchetype::Stable:
			return U"Stable";
		case UnitArchetype::Turret:
			return U"Turret";
		case UnitArchetype::Worker:
			return U"Worker";
		case UnitArchetype::Soldier:
			return U"Soldier";
		case UnitArchetype::Archer:
			return U"Archer";
		case UnitArchetype::Sniper:
			return U"Sniper";
		case UnitArchetype::Katyusha:
			return U"Katyusha";
		case UnitArchetype::MachineGun:
			return U"MachineGun";
		case UnitArchetype::Goliath:
			return U"Goliath";
		case UnitArchetype::Healer:
			return U"Healer";
		case UnitArchetype::Spinner:
		default:
			return U"Spinner";
		}
	}

	[[nodiscard]] static String toUnitArchetypeTomlString(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
			return U"base";
		case UnitArchetype::Barracks:
			return U"barracks";
		case UnitArchetype::Stable:
			return U"stable";
		case UnitArchetype::Turret:
			return U"turret";
		case UnitArchetype::Worker:
			return U"worker";
		case UnitArchetype::Soldier:
			return U"soldier";
		case UnitArchetype::Archer:
			return U"archer";
		case UnitArchetype::Sniper:
			return U"sniper";
		case UnitArchetype::Katyusha:
			return U"katyusha";
		case UnitArchetype::MachineGun:
			return U"machine_gun";
		case UnitArchetype::Goliath:
			return U"goliath";
		case UnitArchetype::Healer:
			return U"healer";
		case UnitArchetype::Spinner:
		default:
			return U"spinner";
		}
	}

	[[nodiscard]] static bool isButtonClicked(const RectF& rect)
	{
		return IsMenuButtonClicked(rect);
	}

	static void drawButton(const RectF& rect, const String& label, const Font& font, const bool selected = false)
	{
		DrawMenuButton(rect, label, font, selected);
	}

	[[nodiscard]] static RectF getLeftPanelRect()
	{
		return RectF{ 12, 12, 220, Scene::Height() - 24 };
	}

	[[nodiscard]] static RectF getRightPanelRect()
	{
		return RectF{ 244, 12, Scene::Width() - 256, Scene::Height() - 24 };
	}

	[[nodiscard]] static RectF getEditorPanelRect()
	{
		const RectF right = getRightPanelRect();
		return RectF{ right.x + 16, right.y + 112, right.w - 32, right.h - 220 };
	}

	[[nodiscard]] static RectF getHelpPanelRect()
	{
		const RectF right = getRightPanelRect();
		return RectF{ right.x + 16, right.y + right.h - 96, right.w - 32, 80 };
	}

	[[nodiscard]] static RectF getTopButtonRect(const int32 index)
	{
		const RectF right = getRightPanelRect();
		return RectF{ right.x + right.w - 650 + (index * 130), right.y + 16, 118, 30 };
	}

	[[nodiscard]] static RectF getTabButtonRect(const Tab tab)
	{
		const RectF right = getRightPanelRect();
		const int32 index = (tab == Tab::Core) ? 0 : 1;
		return RectF{ right.x + 16 + (index * 108), right.y + 112, 96, 30 };
	}

	[[nodiscard]] static RectF getUnitButtonRect(const int32 index)
	{
		const RectF left = getLeftPanelRect();
		return RectF{ left.x + 16, left.y + 84 + (index * 32), left.w - 32, 28 };
	}

	[[nodiscard]] static RectF getEditorRowRect(const int32 index)
	{
		const RectF editor = getEditorPanelRect();
		return RectF{ editor.x, editor.y + 44 + (index * 40), editor.w, 36 };
	}
};
