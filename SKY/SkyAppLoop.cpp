# include "SkyAppLoop.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"
# include "SkyAppUi.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		constexpr double SelectionDragThreshold = 12.0;

		[[nodiscard]] ColorF GetResourceAreaColor(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return ColorF{ 0.96, 0.82, 0.22, 0.28 };

			case ResourceType::Gunpowder:
				return ColorF{ 0.92, 0.42, 0.26, 0.28 };

			case ResourceType::Mana:
				return ColorF{ 0.42, 0.60, 0.98, 0.28 };

			default:
				return ColorF{ 0.72, 0.72, 0.72, 0.28 };
			}
		}

		[[nodiscard]] ColorF GetTeamColor(const Optional<UnitTeam>& team)
		{
			if (team && (*team == UnitTeam::Player))
			{
				return ColorF{ 0.92, 0.95, 1.0, 0.95 };
			}

			if (team && (*team == UnitTeam::Enemy))
			{
				return ColorF{ 1.0, 0.78, 0.74, 0.95 };
			}

			return ColorF{ 0.72, 0.78, 0.84, 0.90 };
		}

		[[nodiscard]] StringView ToDisplayLabel(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return U"予算";

			case ResourceType::Gunpowder:
				return U"火薬";

			case ResourceType::Mana:
				return U"魔力";

			default:
				return U"資源";
			}
		}

		[[nodiscard]] StringView ToOwnerLabel(const Optional<UnitTeam>& team)
		{
			if (team && (*team == UnitTeam::Player))
			{
				return U"自軍";
			}

			if (team && (*team == UnitTeam::Enemy))
			{
				return U"敵軍";
			}

			return U"中立";
		}

		[[nodiscard]] Optional<Vec2> ProjectToScreen(const DebugCamera3D& camera, const Vec3& worldPosition)
		{
			const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(worldPosition.x), static_cast<float>(worldPosition.y), static_cast<float>(worldPosition.z) });

			if (screenPosition.z <= 0.0f)
			{
				return none;
			}

			return Vec2{ screenPosition.x, screenPosition.y };
		}

		void AddResource(ResourceStock& stock, const ResourceType type, const double amount)
		{
			switch (type)
			{
			case ResourceType::Budget:
				stock.budget += amount;
				return;

			case ResourceType::Gunpowder:
				stock.gunpowder += amount;
				return;

			case ResourceType::Mana:
				stock.mana += amount;
				return;

			default:
				return;
			}
		}

		[[nodiscard]] double GetResourceIncomeAmount(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return BudgetAreaIncome;

			case ResourceType::Gunpowder:
				return GunpowderAreaIncome;

			case ResourceType::Mana:
				return ManaAreaIncome;

			default:
				return 0.0;
			}
		}

		void ResetResourceState(SkyAppState& state)
		{
			state.playerResources = ResourceStock{ .budget = StartingResources };
			state.enemyResources = {};
			state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
		}

		void SpawnEnemyReinforcement(SkyAppState& state, const bool moveImmediately)
		{
			static const Array<Vec3> spawnOffsets{
				Vec3{ -2.5, 0, -4.2 },
				Vec3{ 0.0, 0, -3.0 },
				Vec3{ 2.3, 0, -4.0 },
			};

			const Vec3 spawnPosition = state.mapData.enemyBasePosition.movedBy(spawnOffsets[state.enemyReinforcementCount % spawnOffsets.size()]);
			SpawnEnemySapper(state.enemySappers, spawnPosition, 180_deg);

			if (moveImmediately)
			{
				const size_t enemyIndex = (state.enemySappers.size() - 1);
				SetSpawnedSapperTarget(state.enemySappers[enemyIndex], GetSapperPopTargetPosition(state.mapData.playerBasePosition, enemyIndex));
			}

			++state.enemyReinforcementCount;
		}

		void ResetMatch(SkyAppState& state)
		{
			state.spawnedSappers.clear();
			state.enemySappers.clear();
			state.selectedSapperIndices.clear();
			state.selectionDragStart.reset();
			state.showBlacksmithMenu = false;
			state.playerBaseHitPoints = BaseMaxHitPoints;
			state.enemyBaseHitPoints = BaseMaxHitPoints;
          ResetResourceState(state);
			state.nextEnemyReinforcementAt = (Scene::Time() + EnemyReinforcementInterval);
			state.enemyReinforcementCount = 0;
			state.playerWon.reset();
			SpawnEnemyReinforcement(state, true);
			SpawnEnemyReinforcement(state, true);
			SpawnEnemyReinforcement(state, true);
		}

		String ReloadMapAndResetMatch(SkyAppState& state)
		{
			const MapDataLoadResult loadResult = LoadMapDataWithStatus(MapDataPath);
			state.mapData = loadResult.mapData;
			state.mapEditor.hoveredGroundPosition.reset();
			ResetMatch(state);
			return loadResult.message;
		}

		void UpdateBaseCombat(Array<SpawnedSapper>& attackers, const Vec3& basePosition, double& baseHitPoints)
		{
			if (baseHitPoints <= 0.0)
			{
				return;
			}

			for (auto& attacker : attackers)
			{
				if (attacker.hitPoints <= 0.0)
				{
					continue;
				}

				const double attackDistance = (attacker.attackRange + BaseCombatRadius);
				const double distanceSq = GetSpawnedSapperBasePosition(attacker).distanceFromSq(basePosition);

				if (Square(attackDistance) < distanceSq)
				{
					continue;
				}

				if ((Scene::Time() - attacker.lastAttackAt) < attacker.attackInterval)
				{
					continue;
				}

				attacker.lastAttackAt = Scene::Time();
				baseHitPoints = Max(0.0, (baseHitPoints - attacker.attackDamage));
			}
		}

		void UpdateResourceAreas(SkyAppState& state)
		{
			const double deltaTime = Scene::DeltaTime();

			if (state.resourceAreaStates.size() != state.mapData.resourceAreas.size())
			{
				state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
			}

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const ResourceArea& area = state.mapData.resourceAreas[i];
				ResourceAreaState& areaState = state.resourceAreaStates[i];
				int32 playerInside = 0;
				int32 enemyInside = 0;
				const double radiusSq = Square(area.radius);

				for (const auto& sapper : state.spawnedSappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++playerInside;
					}
				}

				for (const auto& sapper : state.enemySappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++enemyInside;
					}
				}

				Optional<UnitTeam> occupyingTeam;
				if ((0 < playerInside) && (enemyInside == 0))
				{
					occupyingTeam = UnitTeam::Player;
				}
				else if ((0 < enemyInside) && (playerInside == 0))
				{
					occupyingTeam = UnitTeam::Enemy;
				}

				if (occupyingTeam)
				{
					if (areaState.ownerTeam && (*areaState.ownerTeam == *occupyingTeam))
					{
						areaState.capturingTeam.reset();
						areaState.captureProgress = ResourceAreaCaptureSeconds;
					}
					else
					{
						if ((not areaState.capturingTeam) || (*areaState.capturingTeam != *occupyingTeam))
						{
							areaState.capturingTeam = *occupyingTeam;
							areaState.captureProgress = 0.0;
						}

						areaState.captureProgress += deltaTime;

						if (ResourceAreaCaptureSeconds <= areaState.captureProgress)
						{
							areaState.ownerTeam = *occupyingTeam;
							areaState.capturingTeam.reset();
							areaState.captureProgress = ResourceAreaCaptureSeconds;
							areaState.incomeProgress = 0.0;
						}
					}
				}
				else if (areaState.capturingTeam)
				{
					areaState.captureProgress = Max(0.0, (areaState.captureProgress - deltaTime));
					if (areaState.captureProgress <= 0.0)
					{
						areaState.capturingTeam.reset();
					}
				}

				if (areaState.ownerTeam)
				{
					areaState.incomeProgress += deltaTime;

					while (ResourceAreaIncomeIntervalSeconds <= areaState.incomeProgress)
					{
						areaState.incomeProgress -= ResourceAreaIncomeIntervalSeconds;
						if (*areaState.ownerTeam == UnitTeam::Player)
						{
							AddResource(state.playerResources, area.type, GetResourceIncomeAmount(area.type));
						}
						else
						{
							AddResource(state.enemyResources, area.type, GetResourceIncomeAmount(area.type));
						}
					}
				}
				else
				{
					areaState.incomeProgress = 0.0;
				}
			}
		}

		void UpdateBattleState(SkyAppState& state)
		{
			if ((state.enemyBaseHitPoints > 0.0) && (state.nextEnemyReinforcementAt <= Scene::Time()))
			{
				SpawnEnemyReinforcement(state, false);
				state.nextEnemyReinforcementAt = (Scene::Time() + EnemyReinforcementInterval);
			}

			for (size_t i = 0; i < state.enemySappers.size(); ++i)
			{
				const Vec3 desiredTarget = GetSapperPopTargetPosition(state.mapData.playerBasePosition, i);

				if (EnemyAdvanceStopDistance < GetSpawnedSapperBasePosition(state.enemySappers[i]).distanceFrom(state.mapData.playerBasePosition)
					&& 0.25 < state.enemySappers[i].targetPosition.distanceFrom(desiredTarget))
				{
					SetSpawnedSapperTarget(state.enemySappers[i], desiredTarget);
				}
			}

			ResolveSapperSpacingAgainstUnits(state.spawnedSappers, state.enemySappers);
			ResolveSapperSpacingAgainstUnits(state.enemySappers, state.spawnedSappers);
			ResolveSapperSpacingAgainstBase(state.spawnedSappers, state.mapData.enemyBasePosition);
			ResolveSapperSpacingAgainstBase(state.enemySappers, state.mapData.playerBasePosition);
			UpdateResourceAreas(state);

			UpdateAutoCombat(state.spawnedSappers, state.enemySappers);
			UpdateAutoCombat(state.enemySappers, state.spawnedSappers);
			UpdateBaseCombat(state.spawnedSappers, state.mapData.enemyBasePosition, state.enemyBaseHitPoints);
			UpdateBaseCombat(state.enemySappers, state.mapData.playerBasePosition, state.playerBaseHitPoints);
			RemoveDefeatedSappers(state.spawnedSappers);
			RemoveDefeatedSappers(state.enemySappers);

			if (state.enemyBaseHitPoints <= 0.0)
			{
				state.playerWon = true;
				state.showBlacksmithMenu = false;
				state.selectedSapperIndices.clear();
			}
			else if (state.playerBaseHitPoints <= 0.0)
			{
				state.playerWon = false;
				state.showBlacksmithMenu = false;
				state.selectedSapperIndices.clear();
			}
		}

		SkyAppFrameState BuildFrameState(SkyAppState& state)
		{
			SkyAppFrameState frame;
			frame.isEditorMode = (state.appMode == AppMode::EditMap);
			state.mapEditor.enabled = frame.isEditorMode;
			frame.showSapperMenu = ((state.selectedSapperIndices.size() == 1) && (not state.playerWon));
			frame.isHoveringUI = frame.panels.isHoveringUi(state.showUI, frame.isEditorMode, state.showBlacksmithMenu, frame.showSapperMenu, state.modelHeightEditMode);
			frame.birdRenderPosition = BirdDisplayPosition.movedBy(0, state.modelHeightSettings.birdOffsetY, 0);
			frame.ashigaruRenderPosition = AshigaruDisplayPosition.movedBy(0, state.modelHeightSettings.ashigaruOffsetY, 0);
			return frame;
		}

		Optional<Vec3> GetGroundIntersection(const DebugCamera3D& camera)
		{
			const Ray ray = camera.screenToRay(Cursor::PosF());
			const InfinitePlane groundPlane3D{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

			if (const auto distance = ray.intersects(groundPlane3D))
			{
				const Vec3 position = ray.point_at(*distance);
				return Vec3{ position.x, 0.0, position.z };
			}

			return none;
		}

		RectF GetSelectionRect(const Optional<Vec2>& selectionDragStart)
		{
			const Vec2 start = *selectionDragStart;
			const Vec2 end = Cursor::PosF();
			return RectF{ Min(start.x, end.x), Min(start.y, end.y), Abs(end.x - start.x), Abs(end.y - start.y) };
		}

		Array<size_t> CollectSappersInSelectionRect(const DebugCamera3D& camera, const Array<SpawnedSapper>& spawnedSappers, const RectF& selectionRect)
		{
			Array<size_t> indices;

			for (size_t i = 0; i < spawnedSappers.size(); ++i)
			{
				const Vec3 renderPosition = GetSpawnedSapperRenderPosition(spawnedSappers[i]).movedBy(0, 1.4, 0);
				const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(renderPosition.x), static_cast<float>(renderPosition.y), static_cast<float>(renderPosition.z) });

				if (screenPosition.z <= 0.0f)
				{
					continue;
				}

				if (selectionRect.intersects(Circle{ Vec2{ screenPosition.x, screenPosition.y }, 12.0 }))
				{
					indices << i;
				}
			}

			return indices;
		}

		void UpdateCameraAndEditor(SkyAppState& state, const SkyAppFrameState& frame)
		{
			state.camera.setView(state.cameraSettings.eye, state.cameraSettings.focus);
			if (not frame.isHoveringUI)
			{
				state.camera.update(4.0);
			}
			state.cameraSettings.eye = state.camera.getEyePosition();
			state.cameraSettings.focus = state.camera.getFocusPosition();

			if (not frame.isHoveringUI)
			{
				UpdateCameraWheelZoom(state.camera, state.cameraSettings, state.mapData.playerBasePosition);
			}

			Graphics3D::SetCameraTransform(state.camera);
			UpdateMapEditor(state.mapEditor, state.mapData, state.camera, (frame.isEditorMode && (not frame.isHoveringUI)));
		}

		void HandleSelectionInput(SkyAppState& state, const SkyAppFrameState& frame)
		{
			const Sphere playerBaseInteractionSphere{ state.mapData.playerBasePosition + Vec3{ 0, 4.0, 0 }, 4.5 };
			const bool blacksmithHovered = playerBaseInteractionSphere.intersects(state.camera.screenToRay(Cursor::PosF())).has_value();
			const Optional<size_t> hoveredSapperIndex = (frame.isEditorMode ? none : HitTestSpawnedSapper(state.spawnedSappers, state.camera));

			if ((not frame.isEditorMode) && (not state.playerWon) && (not frame.isHoveringUI) && MouseL.down())
			{
				state.selectionDragStart = Cursor::PosF();
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && state.selectionDragStart && MouseL.up())
			{
				const RectF selectionRect = GetSelectionRect(state.selectionDragStart);

				if ((SelectionDragThreshold <= selectionRect.w) || (SelectionDragThreshold <= selectionRect.h))
				{
					state.selectedSapperIndices = CollectSappersInSelectionRect(state.camera, state.spawnedSappers, selectionRect);
					state.showBlacksmithMenu = false;
				}
				else if (hoveredSapperIndex)
				{
					state.selectedSapperIndices = { *hoveredSapperIndex };
					state.showBlacksmithMenu = false;
				}
				else if (blacksmithHovered)
				{
					state.showBlacksmithMenu = not state.showBlacksmithMenu;
					state.selectedSapperIndices.clear();
				}
				else
				{
					state.showBlacksmithMenu = false;
					state.selectedSapperIndices.clear();
				}

				state.selectionDragStart.reset();
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && (not state.selectedSapperIndices.isEmpty()) && (not frame.isHoveringUI) && MouseR.down())
			{
				if (const auto targetPosition = GetGroundIntersection(state.camera))
				{
					Array<size_t> validSelectedSapperIndices;

					for (const size_t selectedIndex : state.selectedSapperIndices)
					{
						if (selectedIndex < state.spawnedSappers.size())
						{
							validSelectedSapperIndices << selectedIndex;
						}
					}

					for (size_t i = 0; i < validSelectedSapperIndices.size(); ++i)
					{
						SetSpawnedSapperTarget(state.spawnedSappers[validSelectedSapperIndices[i]], GetSapperPopTargetPosition(*targetPosition, i));
					}

					state.selectedSapperIndices = validSelectedSapperIndices;
					state.showBlacksmithMenu = false;
				}
			}
		}

       void RenderWorld(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
			resources.groundPlane.draw(resources.groundTexture);
			Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const ResourceArea& area = state.mapData.resourceAreas[i];
				const ResourceAreaState resourceState = ((i < state.resourceAreaStates.size()) ? state.resourceAreaStates[i] : ResourceAreaState{});
				Cylinder{ area.position.movedBy(0, 0.02, 0), area.radius, 0.04 }.draw(GetResourceAreaColor(area.type).removeSRGBCurve());
				Cylinder{ area.position.movedBy(0, 0.06, 0), Max(0.35, area.radius * 0.16), 0.12 }.draw(GetTeamColor(resourceState.ownerTeam).removeSRGBCurve());
			}

			resources.blacksmithModel.draw(state.mapData.playerBasePosition);
			resources.blacksmithModel.draw(state.mapData.enemyBasePosition);
			if (resources.birdModel.isLoaded())
			{
				resources.birdModel.draw(frame.birdRenderPosition, BirdDisplayYaw, ColorF{ 0.92, 0.95, 1.0 }.removeSRGBCurve());
			}
			if (resources.ashigaruModel.isLoaded())
			{
				resources.ashigaruModel.draw(frame.ashigaruRenderPosition, BirdDisplayYaw, ColorF{ 0.95, 0.92, 0.90 }.removeSRGBCurve());
			}
			for (const auto& placedModel : state.mapData.placedModels)
			{
				DrawPlacedModel(placedModel, resources.millModel, resources.treeModel, resources.pineModel);
			}

			DrawSpawnedSappers(state.spawnedSappers, resources.birdModel, ColorF{ 0.92, 0.95, 1.0 });
			DrawSpawnedSappers(state.enemySappers, resources.ashigaruModel, ColorF{ 1.0, 0.78, 0.74 });
			DrawMapEditorScene(state.mapEditor, state.mapData);
			UpdateSkyFromTime(state.sky, state.skyTime);
			state.sky.draw();
		}

       void DrawOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
			Graphics3D::Flush();
			resources.renderTexture.resolve();
			Shader::LinearToScreen(resources.renderTexture);

			if (const auto birdGroundPoint = resources.birdModel.groundContactPoint(frame.birdRenderPosition, BirdDisplayYaw))
			{
				DrawGroundContactOverlay(state.camera, *birdGroundPoint);
			}

			if (const auto ashigaruGroundPoint = resources.ashigaruModel.groundContactPoint(frame.ashigaruRenderPosition, BirdDisplayYaw))
			{
				DrawGroundContactOverlay(state.camera, *ashigaruGroundPoint);
			}

			if ((not frame.isEditorMode) && state.showBlacksmithMenu)
			{
				DrawSelectionIndicator(state.camera, state.mapData.playerBasePosition);
			}

			for (const size_t selectedIndex : state.selectedSapperIndices)
			{
				if (selectedIndex < state.spawnedSappers.size())
				{
					DrawSelectedSapperRing(state.camera, state.spawnedSappers[selectedIndex]);
				}
			}

			if ((state.selectedSapperIndices.size() == 1) && (state.selectedSapperIndices.front() < state.spawnedSappers.size()))
			{
				DrawSelectedSapperIcon(state.camera, state.spawnedSappers[state.selectedSapperIndices.front()]);
			}

			DrawSapperHealthBars(state.camera, state.spawnedSappers, ColorF{ 0.36, 0.92, 0.46, 0.95 });
			DrawSapperHealthBars(state.camera, state.enemySappers, ColorF{ 0.96, 0.28, 0.24, 0.95 });

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				if (i >= state.resourceAreaStates.size())
				{
					continue;
				}

				const ResourceArea& area = state.mapData.resourceAreas[i];
				const ResourceAreaState& areaState = state.resourceAreaStates[i];
				const Optional<Vec2> screenAnchor = ProjectToScreen(state.camera, area.position.movedBy(0, 1.1, 0));

				if (not screenAnchor)
				{
					continue;
				}

				const Vec2 anchor = *screenAnchor;
				const RectF backRect{ Arg::center = anchor.movedBy(0, 16), 78, 6 };
				backRect.draw(ColorF{ 0.04, 0.05, 0.06, 0.88 });

				if (areaState.capturingTeam)
				{
					RectF{ backRect.pos, (backRect.w * Math::Saturate(areaState.captureProgress / ResourceAreaCaptureSeconds)), backRect.h }.draw(GetTeamColor(areaState.capturingTeam));
				}
				else if (areaState.ownerTeam)
				{
					backRect.stretched(-1).draw(GetTeamColor(areaState.ownerTeam));
				}

				backRect.drawFrame(1.0, ColorF{ 0.9, 0.95, 1.0, 0.55 });
				SimpleGUI::GetFont()(U"{} [{}]"_fmt(ToDisplayLabel(area.type), ToOwnerLabel(areaState.ownerTeam))).drawAt(anchor.movedBy(0, -2), Palette::White);
			}

			const Rect playerBasePanel{ 260, 20, 220, 66 };
			const Rect enemyBasePanel{ 500, 20, 220, 66 };
            const Rect resourcePanel = frame.panels.resourcePanel;
			playerBasePanel.draw(ColorF{ 0.08, 0.10, 0.12, 0.88 }).drawFrame(2, 0, ColorF{ 0.75, 0.82, 0.90, 0.9 });
			enemyBasePanel.draw(ColorF{ 0.12, 0.08, 0.08, 0.88 }).drawFrame(2, 0, ColorF{ 0.90, 0.55, 0.55, 0.9 });
           resourcePanel.draw(ColorF{ 0.08, 0.10, 0.12, 0.88 }).drawFrame(2, 0, ColorF{ 0.75, 0.82, 0.90, 0.9 });
			SimpleGUI::GetFont()(U"Player Base").draw((playerBasePanel.x + 12), (playerBasePanel.y + 6), Palette::White);
			SimpleGUI::GetFont()(U"Enemy Base").draw((enemyBasePanel.x + 12), (enemyBasePanel.y + 6), Palette::White);
            SimpleGUI::GetFont()(U"資源").draw((resourcePanel.x + 12), (resourcePanel.y + 6), Palette::White);
			RectF{ (playerBasePanel.x + 12), (playerBasePanel.y + 28), 196, 10 }.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
			RectF{ (playerBasePanel.x + 12), (playerBasePanel.y + 28), (196.0 * Math::Saturate(state.playerBaseHitPoints / BaseMaxHitPoints)), 10 }.draw(ColorF{ 0.36, 0.92, 0.46, 0.95 });
			RectF{ (enemyBasePanel.x + 12), (enemyBasePanel.y + 28), 196, 10 }.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
			RectF{ (enemyBasePanel.x + 12), (enemyBasePanel.y + 28), (196.0 * Math::Saturate(state.enemyBaseHitPoints / BaseMaxHitPoints)), 10 }.draw(ColorF{ 0.96, 0.28, 0.24, 0.95 });
          SimpleGUI::GetFont()(U"予算 {:.0f}"_fmt(state.playerResources.budget)).draw((resourcePanel.x + 12), (resourcePanel.y + 28), ColorF{ 0.98, 0.90, 0.38 });
			SimpleGUI::GetFont()(U"火薬 {:.0f}"_fmt(state.playerResources.gunpowder)).draw((resourcePanel.x + 12), (resourcePanel.y + 48), ColorF{ 0.98, 0.56, 0.42 });
			SimpleGUI::GetFont()(U"魔力 {:.0f}"_fmt(state.playerResources.mana)).draw((resourcePanel.x + 112), (resourcePanel.y + 48), ColorF{ 0.58, 0.72, 1.0 });
			const double reinforcementSeconds = Max(0.0, (state.nextEnemyReinforcementAt - Scene::Time()));
			SimpleGUI::GetFont()(U"増援 {:.1f}s"_fmt(reinforcementSeconds)).draw((enemyBasePanel.x + 12), (enemyBasePanel.y + 42), ColorF{ 1.0, 0.88, 0.88 });

			if (state.modelHeightEditMode)
			{
				DrawModelHeightEditor(state.modelHeightSettings, state.modelHeightMessage.text, state.modelHeightMessage.until, frame.panels.modelHeight, frame.birdRenderPosition, frame.ashigaruRenderPosition);
			}

			if ((not frame.isEditorMode) && state.selectionDragStart && MouseL.pressed() && (not frame.isHoveringUI))
			{
				const RectF selectionRect = GetSelectionRect(state.selectionDragStart);
				selectionRect.draw(ColorF{ 0.35, 0.72, 1.0, 0.12 });
				selectionRect.drawFrame(2.0, ColorF{ 0.55, 0.82, 1.0, 0.95 });
			}

			if (state.playerWon)
			{
				const Rect overlay{ Arg::center = Scene::Center(), 340, 140 };
				overlay.draw(ColorF{ 0.06, 0.06, 0.08, 0.88 }).drawFrame(3, 0, *state.playerWon ? ColorF{ 0.45, 0.92, 0.56 } : ColorF{ 0.96, 0.38, 0.30 });
				SimpleGUI::GetFont()(*state.playerWon ? U"Victory" : U"Defeat").drawAt(overlay.center().movedBy(0, -18), Palette::White);
				SimpleGUI::GetFont()(*state.playerWon ? U"Enemy base destroyed" : U"Player base destroyed").drawAt(overlay.center().movedBy(0, 18), ColorF{ 0.92 });
			}
		}

     void DrawHudUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
			if (state.showUI)
			{
				DrawSkySettingsPanel(state.sky, frame.panels);
				DrawCameraSettingsPanel(state.camera, state.cameraSettings, resources.birdModel, resources.ashigaruModel, state.cameraSaveMessage, frame.panels);
			}

          DrawMiniMap(frame.panels, state.camera, state.mapData, state.spawnedSappers, state.enemySappers, state.resourceAreaStates, state.selectedSapperIndices);

			if (frame.isEditorMode)
			{
				DrawMapEditorPanel(state.mapEditor, state.mapData, MapDataPath, frame.panels.mapEditor);
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && state.showBlacksmithMenu)
			{
				DrawBlacksmithMenu(frame.panels, state.spawnedSappers, state.mapData.playerBasePosition, state.mapData.sapperRallyPoint, state.playerResources, SapperCost, state.blacksmithMenuMessage);
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
			{
				DrawSapperMenu(frame.panels, state.spawnedSappers, state.mapData.playerBasePosition, state.mapData.sapperRallyPoint, state.playerResources, SapperCost, state.blacksmithMenuMessage);
			}

			if (DrawTextButton(frame.panels.mapModeToggle, frame.isEditorMode ? U"Map Edit: ON" : U"Map Edit: OFF"))
			{
				state.appMode = frame.isEditorMode ? AppMode::Play : AppMode::EditMap;
				state.showBlacksmithMenu = false;
				state.selectedSapperIndices.clear();
				state.selectionDragStart.reset();
				state.mapEditor.hoveredGroundPosition.reset();
			}

			if (DrawTextButton(frame.panels.modelHeightModeToggle, state.modelHeightEditMode ? U"Model Height: ON" : U"Model Height: OFF"))
			{
				state.modelHeightEditMode = not state.modelHeightEditMode;
			}

			if (DrawTextButton(frame.panels.reloadMapButton, U"保存済みマップ再読込"))
			{
				const String loadMessage = ReloadMapAndResetMatch(state);
				state.mapDataMessage.show(loadMessage.isEmpty() ? U"保存済みマップを再読込" : loadMessage, 4.0);
			}

			if (DrawTextButton(frame.panels.restartButton, U"試合リスタート"))
			{
				ResetMatch(state);
				state.restartMessage.show(U"試合をリスタート");
			}

			if (state.mapDataMessage.isVisible())
			{
				SimpleGUI::GetFont()(state.mapDataMessage.text).draw(20, (Scene::Height() - 132), ColorF{ 0.12 });
			}

			if (state.restartMessage.isVisible())
			{
				SimpleGUI::GetFont()(state.restartMessage.text).draw(20, (Scene::Height() - 108), ColorF{ 0.12 });
			}

			SimpleGUI::CheckBox(state.showUI, U"UI", Vec2{ 20, Scene::Height() - 100 });
			SimpleGUI::Slider(U"time: {:.2f}"_fmt(state.skyTime), state.skyTime, -2.0, 4.0, Vec2{ 20, Scene::Height() - 60 }, 120, Scene::Width() - 160);
		}
	}

	SkyAppResources::SkyAppResources()
		: groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB }
		, blacksmithModel{ U"example/obj/blacksmith.obj" }
		, millModel{ U"example/obj/mill.obj" }
		, treeModel{ U"example/obj/tree.obj" }
		, pineModel{ U"example/obj/pine.obj" }
		, birdModel{ BirdModelPath, BirdDisplayHeight }
		, ashigaruModel{ AshigaruModelPath, BirdDisplayHeight }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
	{
		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.cameraSettings = LoadCameraSettings();
		state.modelHeightSettings = LoadModelHeightSettings();
		const MapDataLoadResult initialMapDataLoad = LoadMapDataWithStatus(MapDataPath);
		state.mapData = initialMapDataLoad.mapData;
		state.camera = DebugCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.cameraSettings.eye, state.cameraSettings.focus };
		ResetMatch(state);

		if (not initialMapDataLoad.message.isEmpty())
		{
			state.mapDataMessage.show(initialMapDataLoad.message, 4.0);
		}
	}

	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state)
	{
		resources.birdModel.update(Scene::DeltaTime());
		resources.ashigaruModel.update(Scene::DeltaTime());
		UpdateSpawnedSappers(state.spawnedSappers);
		UpdateSpawnedSappers(state.enemySappers);

		if (not state.playerWon)
		{
			UpdateBattleState(state);
		}

		state.selectedSapperIndices.remove_if([&state](const size_t index)
			{
				return (state.spawnedSappers.size() <= index);
			});

		const SkyAppFrameState frame = BuildFrameState(state);

		{
			const ScopedRenderTarget3D target{ resources.renderTexture.clear(ColorF{ 0.0 }) };
			UpdateCameraAndEditor(state, frame);
			HandleSelectionInput(state, frame);
			RenderWorld(resources, state, frame);
		}

		DrawOverlay(resources, state, frame);
		DrawHudUi(resources, state, frame);
	}
}
