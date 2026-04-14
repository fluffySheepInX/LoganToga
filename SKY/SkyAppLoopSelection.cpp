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
				state.selectionDragStart.reset();
				return;
			}

			const Sphere playerBaseInteractionSphere{ state.mapData.playerBasePosition + Vec3{ 0, 4.0, 0 }, 4.5 };
			const Optional<Ray> cursorRay = TryScreenToRay(state.camera, Cursor::PosF());
			const bool blacksmithHovered = (cursorRay && playerBaseInteractionSphere.intersects(*cursorRay).has_value());
         const Optional<size_t> hoveredSapperIndex = (frame.isEditorMode ? none : HitTestSpawnedSapper(state.spawnedSappers, state.camera, state.modelHeightSettings));
			const Optional<size_t> hoveredMillIndex = (frame.isEditorMode ? none : HitTestMill(state.mapData.placedModels, cursorRay));

			if ((not frame.isEditorMode) && (not state.playerWon) && (not frame.isHoveringUI) && MouseL.down())
			{
				state.selectionDragStart = Cursor::PosF();
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && state.selectionDragStart && MouseL.up())
			{
				const RectF selectionRect = GetSelectionRect(state.selectionDragStart);

				if ((SelectionDragThreshold <= selectionRect.w) || (SelectionDragThreshold <= selectionRect.h))
				{
                 state.selectedSapperIndices = CollectSappersInSelectionRect(state.camera, state.spawnedSappers, selectionRect, state.modelHeightSettings);
					state.selectedMillIndex.reset();
					state.showBlacksmithMenu = false;
				}
				else if (hoveredSapperIndex)
				{
					state.selectedSapperIndices = { *hoveredSapperIndex };
					state.selectedMillIndex.reset();
					state.showBlacksmithMenu = false;
				}
				else if (hoveredMillIndex)
				{
					state.selectedMillIndex = *hoveredMillIndex;
					state.selectedSapperIndices.clear();
					state.showBlacksmithMenu = false;
				}
				else if (blacksmithHovered)
				{
					state.showBlacksmithMenu = not state.showBlacksmithMenu;
					state.selectedMillIndex.reset();
					state.selectedSapperIndices.clear();
				}
				else
				{
					state.showBlacksmithMenu = false;
					state.selectedMillIndex.reset();
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
                        if ((selectedIndex < state.spawnedSappers.size()) && IsSpawnedSapperSelectable(state.spawnedSappers[selectedIndex]))
						{
							validSelectedSapperIndices << selectedIndex;
						}
					}

					for (size_t i = 0; i < validSelectedSapperIndices.size(); ++i)
					{
                     SetSpawnedSapperMoveOrder(state.spawnedSappers[validSelectedSapperIndices[i]], GetSapperPopTargetPosition(*targetPosition, i), state.mapData, state.modelHeightSettings);
					}

					if (not validSelectedSapperIndices.isEmpty())
					{
						state.moveOrderIndicator = MoveOrderIndicator{
							.position = *targetPosition,
							.startedAt = Scene::Time(),
						};
					}

					state.selectedSapperIndices = validSelectedSapperIndices;
					state.showBlacksmithMenu = false;
				}
			}
		}
	}
}
