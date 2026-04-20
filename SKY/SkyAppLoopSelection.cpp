# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		constexpr double SelectionDragThreshold = 12.0;
		constexpr double MillSelectionRadius = 4.5;

		[[nodiscard]] Optional<size_t> HitTestMill(const Array<PlacedModel>& placedModels, const Optional<Ray>& cursorRay)
		{
			if (not cursorRay)
			{
				return none;
			}

			double nearestDistance = Math::Inf;
			Optional<size_t> nearestIndex;

			for (size_t i = 0; i < placedModels.size(); ++i)
			{
				if (placedModels[i].type != PlaceableModelType::Mill)
				{
					continue;
				}

				const Sphere interactionSphere{ placedModels[i].position.movedBy(0, 4.0, 0), MillSelectionRadius };
				if (const auto distance = cursorRay->intersects(interactionSphere))
				{
					if (*distance < nearestDistance)
					{
						nearestDistance = *distance;
						nearestIndex = i;
					}
				}
			}

			return nearestIndex;
		}

		[[nodiscard]] Optional<Vec3> GetGroundIntersection(const AppCamera3D& camera)
		{
			const Optional<Ray> ray = TryScreenToRay(camera, Cursor::PosF());
			if (not ray)
			{
				return none;
			}

			const InfinitePlane groundPlane3D{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

			if (const auto distance = ray->intersects(groundPlane3D))
			{
				const Vec3 position = ray->point_at(*distance);
				return Vec3{ position.x, 0.0, position.z };
			}

			return none;
		}

		[[nodiscard]] RectF GetSelectionRect(const Optional<Vec2>& selectionDragStart)
		{
			const Vec2 start = *selectionDragStart;
			const Vec2 end = Cursor::PosF();
			return RectF{ Min(start.x, end.x), Min(start.y, end.y), Abs(end.x - start.x), Abs(end.y - start.y) };
		}

        [[nodiscard]] Array<size_t> CollectSappersInSelectionRect(const AppCamera3D& camera, const Array<SpawnedSapper>& spawnedSappers, const RectF& selectionRect, const ModelHeightSettings& modelHeightSettings)
		{
			Array<size_t> indices;

			for (size_t i = 0; i < spawnedSappers.size(); ++i)
			{
             if (not IsSpawnedSapperSelectable(spawnedSappers[i]))
				{
					continue;
				}

               const double scale = Max(ModelScaleMin, GetSpawnedSapperModelScale(modelHeightSettings, spawnedSappers[i]));
				const Vec3 renderPosition = GetSpawnedSapperRenderPosition(spawnedSappers[i]).movedBy(0, (1.4 * scale), 0);
				const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(renderPosition.x), static_cast<float>(renderPosition.y), static_cast<float>(renderPosition.z) });

				if (screenPosition.z <= 0.0f)
				{
					continue;
				}

               if (selectionRect.intersects(Circle{ Vec2{ screenPosition.x, screenPosition.y }, Max(4.0, (12.0 * scale)) }))
				{
					indices << i;
				}
			}

			return indices;
		}
	}

	namespace Detail
	{
		void HandleSelectionInput(SkyAppState& state, const SkyAppFrameState& frame)
		{
			if (frame.showEscMenu)
			{
				state.battle.selectionDragStart.reset();
				return;
			}

			const Sphere playerBaseInteractionSphere{ state.world.mapData.playerBasePosition + Vec3{ 0, 4.0, 0 }, 4.5 };
			const Optional<Ray> cursorRay = TryScreenToRay(state.env.camera, Cursor::PosF());
			const bool blacksmithHovered = (cursorRay && playerBaseInteractionSphere.intersects(*cursorRay).has_value());
         const Optional<size_t> hoveredSapperIndex = (frame.isEditorMode ? none : HitTestSpawnedSapper(state.battle.spawnedSappers, state.env.camera, state.editor.modelHeightSettings));
			const Optional<size_t> hoveredMillIndex = (frame.isEditorMode ? none : HitTestMill(state.world.mapData.placedModels, cursorRay));

			if ((not frame.isEditorMode) && (not state.battle.playerWon) && (not frame.isHoveringUI) && MouseL.down())
			{
				state.battle.selectionDragStart = Cursor::PosF();
			}

			if ((not frame.isEditorMode) && (not state.battle.playerWon) && state.battle.selectionDragStart && MouseL.up())
			{
				const RectF selectionRect = GetSelectionRect(state.battle.selectionDragStart);

				if ((SelectionDragThreshold <= selectionRect.w) || (SelectionDragThreshold <= selectionRect.h))
				{
                 state.battle.selectedSapperIndices = CollectSappersInSelectionRect(state.env.camera, state.battle.spawnedSappers, selectionRect, state.editor.modelHeightSettings);
					state.battle.selectedMillIndex.reset();
					state.hud.showBlacksmithMenu = false;
				}
				else if (hoveredSapperIndex)
				{
					state.battle.selectedSapperIndices = { *hoveredSapperIndex };
					state.battle.selectedMillIndex.reset();
					state.hud.showBlacksmithMenu = false;
				}
				else if (hoveredMillIndex)
				{
					state.battle.selectedMillIndex = *hoveredMillIndex;
					state.battle.selectedSapperIndices.clear();
					state.hud.showBlacksmithMenu = false;
				}
				else if (blacksmithHovered)
				{
					state.hud.showBlacksmithMenu = not state.hud.showBlacksmithMenu;
					state.battle.selectedMillIndex.reset();
					state.battle.selectedSapperIndices.clear();
				}
				else
				{
					state.hud.showBlacksmithMenu = false;
					state.battle.selectedMillIndex.reset();
					state.battle.selectedSapperIndices.clear();
				}

				state.battle.selectionDragStart.reset();
			}

			if ((not frame.isEditorMode) && (not state.battle.playerWon) && (not state.battle.selectedSapperIndices.isEmpty()) && (not frame.isHoveringUI) && MouseR.down())
			{
				if (const auto targetPosition = GetGroundIntersection(state.env.camera))
				{
					Array<size_t> validSelectedSapperIndices;

					for (const size_t selectedIndex : state.battle.selectedSapperIndices)
					{
                        if ((selectedIndex < state.battle.spawnedSappers.size()) && IsSpawnedSapperSelectable(state.battle.spawnedSappers[selectedIndex]))
						{
							validSelectedSapperIndices << selectedIndex;
						}
					}

					for (size_t i = 0; i < validSelectedSapperIndices.size(); ++i)
					{
                     SetSpawnedSapperMoveOrder(state.battle.spawnedSappers[validSelectedSapperIndices[i]], GetSapperPopTargetPosition(*targetPosition, i), state.world.mapData, state.editor.modelHeightSettings);
					}

					if (not validSelectedSapperIndices.isEmpty())
					{
						state.battle.moveOrderIndicator = MoveOrderIndicator{
							.position = *targetPosition,
							.startedAt = Scene::Time(),
						};
					}

					state.battle.selectedSapperIndices = validSelectedSapperIndices;
					state.hud.showBlacksmithMenu = false;
				}
			}
		}
	}
}
