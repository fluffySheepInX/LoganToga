#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "../UI/BattleNotifications.h"
# include "../Systems/BattleInputSystem.h"
# include "../Systems/CameraInputSystem.h"
# include "../Systems/EditorInputSystem.h"
# include "../UI/MapEditorAmbientSound.h"
# include "App/AppStateData.h"

namespace LT3
{
	inline constexpr double ResourceFlagRaiseDurationSec = 1.2;

	inline void UpdateResourceFlagRuntimeState(AppRuntimeState& runtime)
	{
		SyncResourceFlagRuntimeState(runtime);

		const size_t nodeCount = runtime.resourceFlags.nodes.size();
		for (size_t i = 0; i < nodeCount; ++i)
		{
			auto& flagState = runtime.resourceFlags.nodes[i];
			const Faction owner = (i < runtime.world.resourceNodes.owner.size())
				? runtime.world.resourceNodes.owner[i]
				: Faction::Neutral;
			const double captureProgress = (i < runtime.world.resourceNodes.captureProgress.size())
				? runtime.world.resourceNodes.captureProgress[i]
				: 0.0;

			if (owner != flagState.lastOwner)
			{
				flagState.lastOwner = owner;
				flagState.displayFaction = owner;
				flagState.raiseTimerSec = 0.0;
				flagState.raising = (owner != Faction::Neutral && captureProgress >= 1.0);
				flagState.visible = (owner != Faction::Neutral && captureProgress >= 1.0 && !flagState.raising);
			}

			if (flagState.raising)
			{
				flagState.raiseTimerSec = Min(ResourceFlagRaiseDurationSec, flagState.raiseTimerSec + Scene::DeltaTime());
				if (flagState.raiseTimerSec >= ResourceFlagRaiseDurationSec)
				{
					flagState.raiseTimerSec = ResourceFlagRaiseDurationSec;
					flagState.raising = false;
					flagState.visible = true;
				}
			}
			else if (owner == Faction::Neutral || captureProgress < 1.0)
			{
				flagState.visible = false;
				flagState.displayFaction = owner;
				flagState.raiseTimerSec = 0.0;
			}
		}
	}

	inline void ProcessInput(AppRuntimeState& runtime, AppDefinitionState& definitions, AppUiState& ui)
	{
		if (GaussianFSAddon::IsModalActive())
		{
			return;
		}

		if (HandleDebugClipboardCaptureButton(ui))
		{
			return;
		}

		if (HandleDebugClipboardCaptureShortcut(ui))
		{
			return;
		}

		if (HandleDebugNewGameButtons(ui))
		{
			return;
		}

		if (HandleDebugEnemyMoveMarkersToggle(ui))
		{
			return;
		}

		UpdateQuarterViewCamera(ui.mapEditor, runtime.world, definitions.defs);

		const Vec2 screenMouse = Cursor::PosF();
		const Vec2 worldMouse = ToWorldPos(screenMouse);
		if (HandleEditorInput(ui.mapEditor, runtime.world, definitions.defs, definitions.unitCatalog, screenMouse))
		{
			if (ui.mapEditor.unitCatalogDirty || ui.mapEditor.buildLineIconsDirty || ui.mapEditor.skillDefsDirty)
			{
				definitions.defs = CreateDefaultDefinitions(definitions.unitCatalog);
				definitions.renderAssets = BuildBattleRenderAssets(definitions.unitCatalog, &definitions.defs);
				ui.mapEditor.unitCatalogDirty = false;
				ui.mapEditor.buildLineIconsDirty = false;
				ui.mapEditor.skillDefsDirty = false;
			}
			return;
		}
		if (ui.mapEditor.enabled)
		{
			return;
		}

		HandleBattleInput(runtime.world, definitions.defs, ui.mapEditor, screenMouse, worldMouse);
	}

	inline void UpdateDecalAmbientSound(AppRuntimeState& runtime, const AppUiState& ui)
	{
		if (ui.mapEditor.enabled)
		{
			runtime.decalAmbientCooldownSec = 0.0;
			return;
		}

		runtime.decalAmbientCooldownSec = Max(0.0, runtime.decalAmbientCooldownSec - Scene::DeltaTime());
		if (runtime.decalAmbientCooldownSec > 0.0)
		{
			return;
		}

		const Array<DecalAmbientSoundCandidate> candidates = CollectDecalAmbientSoundCandidatesNearMouseOrCenter(ui.mapEditor);
		if (candidates.isEmpty())
		{
			runtime.decalAmbientCooldownSec = 0.8;
			return;
		}

		double totalWeight = 0.0;
		for (const auto& candidate : candidates)
		{
			totalWeight += Max(0.0, candidate.weight);
		}
		if (totalWeight <= 0.0)
		{
			runtime.decalAmbientCooldownSec = 1.0;
			return;
		}

		double roll = Random(0.0, totalWeight);
		const DecalAmbientSoundCandidate* selected = nullptr;
		for (const auto& candidate : candidates)
		{
			roll -= Max(0.0, candidate.weight);
			if (roll <= 0.0)
			{
				selected = &candidate;
				break;
			}
		}
		if (!selected)
		{
			selected = &candidates.back();
		}

		auto cacheIt = runtime.decalAmbientAudioCache.find(selected->path);
		if (cacheIt == runtime.decalAmbientAudioCache.end())
		{
			cacheIt = runtime.decalAmbientAudioCache.emplace(selected->path, Audio{ selected->path }).first;
		}
		if (cacheIt->second)
		{
			cacheIt->second.playOneShot(Clamp(selected->volume, 0.0, 1.0));
		}

		runtime.decalAmbientCooldownSec = Random(1.2, 2.6);
	}

	inline void UpdateAppRuntimeState(AppRuntimeState& runtime, AppDefinitionState& definitions, AppUiState& ui)
	{
		if (ui.debugNewGameRequest != DebugNewGameRequest::None)
		{
			const bool enemyAiStopped = (ui.debugNewGameRequest == DebugNewGameRequest::EnemyAiStopped);
			ResetBattleRuntimeState(runtime, definitions.defs, enemyAiStopped);
			SyncBattleWorldMapFromEditor(ui.mapEditor, runtime.world, definitions.defs);
			SyncResourceFlagRuntimeState(runtime);
			ui.debugNewGameRequest = DebugNewGameRequest::None;
		}

		ProcessInput(runtime, definitions, ui);
		if (!ui.mapEditor.enabled)
		{
			UpdateBattleWorld(runtime.world, definitions.defs, Scene::DeltaTime(), &runtime.notifications);
		}

		UpdateResourceFlagRuntimeState(runtime);
		UpdateDecalAmbientSound(runtime, ui);
		UpdateBattleNotifications(runtime.notifications, Scene::DeltaTime());
	}

	inline void UpdateApp(AppState& app)
	{
		UpdateAppUiState(app.ui);
		UpdateAppRuntimeState(app.runtime, app.definitions, app.ui);
	}
}
