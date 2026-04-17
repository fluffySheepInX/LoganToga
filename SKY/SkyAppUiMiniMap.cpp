# include "SkyAppUiInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		[[nodiscard]] std::pair<Vec3, Vec3> GetWallEndpoints(const PlacedModel& placedModel);

		[[nodiscard]] Vec2 ToMiniMapPoint(const RectF& mapRect, const double minX, const double minZ, const double spanX, const double spanZ, const Vec3& worldPosition)
		{
			const double x = (worldPosition.x - minX) / spanX;
			const double z = (worldPosition.z - minZ) / spanZ;
			return Vec2{ (mapRect.x + x * mapRect.w), (mapRect.bottomY() - z * mapRect.h) };
		}

		void ExpandMiniMapBounds(double& minX, double& maxX, double& minZ, double& maxZ, const Vec3& worldPosition, const double radius = 0.0)
		{
			minX = Min(minX, (worldPosition.x - radius));
			maxX = Max(maxX, (worldPosition.x + radius));
			minZ = Min(minZ, (worldPosition.z - radius));
			maxZ = Max(maxZ, (worldPosition.z + radius));
		}

		void ExpandMiniMapBounds(double& minX, double& maxX, double& minZ, double& maxZ, const TerrainCell& terrainCell)
		{
			const double cellMinX = (terrainCell.cell.x * TerrainCellSize);
			const double cellMaxX = (cellMinX + TerrainCellSize);
			const double cellMinZ = (terrainCell.cell.y * TerrainCellSize);
			const double cellMaxZ = (cellMinZ + TerrainCellSize);
			minX = Min(minX, cellMinX);
			maxX = Max(maxX, cellMaxX);
			minZ = Min(minZ, cellMinZ);
			maxZ = Max(maxZ, cellMaxZ);
		}

		struct MiniMapBounds
		{
			double minX = 0.0;
			double minZ = 0.0;
			double spanX = 20.0;
			double spanZ = 20.0;
		};

		[[nodiscard]] MiniMapBounds GetMiniMapBounds(const MapData& mapData)
		{
			double minX = mapData.playerBasePosition.x;
			double maxX = mapData.playerBasePosition.x;
			double minZ = mapData.playerBasePosition.z;
			double maxZ = mapData.playerBasePosition.z;

			ExpandMiniMapBounds(minX, maxX, minZ, maxZ, mapData.enemyBasePosition);
			ExpandMiniMapBounds(minX, maxX, minZ, maxZ, mapData.sapperRallyPoint);

			for (const auto& terrainCell : mapData.terrainCells)
			{
				ExpandMiniMapBounds(minX, maxX, minZ, maxZ, terrainCell);
			}

			for (const auto& placedModel : mapData.placedModels)
			{
				if (placedModel.type == PlaceableModelType::Wall)
				{
					const auto [start, end] = GetWallEndpoints(placedModel);
					ExpandMiniMapBounds(minX, maxX, minZ, maxZ, start);
					ExpandMiniMapBounds(minX, maxX, minZ, maxZ, end);
					continue;
				}

				double radius = 0.0;
				switch (placedModel.type)
				{
				case PlaceableModelType::Road:
				case PlaceableModelType::TireTrackDecal:
					radius = (Max(placedModel.roadLength, placedModel.roadWidth) * 0.5);
					break;

				default:
					radius = 1.0;
					break;
				}

				ExpandMiniMapBounds(minX, maxX, minZ, maxZ, placedModel.position, radius);
			}

			for (const auto& resourceArea : mapData.resourceAreas)
			{
				ExpandMiniMapBounds(minX, maxX, minZ, maxZ, resourceArea.position, resourceArea.radius);
			}

			const double padding = 4.0;
			minX -= padding;
			maxX += padding;
			minZ -= padding;
			maxZ += padding;

			const double centerX = ((minX + maxX) * 0.5);
			const double centerZ = ((minZ + maxZ) * 0.5);
			const double spanX = Max((maxX - minX), 20.0);
			const double spanZ = Max((maxZ - minZ), 20.0);
			return MiniMapBounds{
				.minX = (centerX - spanX * 0.5),
				.minZ = (centerZ - spanZ * 0.5),
				.spanX = spanX,
				.spanZ = spanZ,
			};
		}

		[[nodiscard]] RectF FitMiniMapRect(const RectF& availableRect, const double spanX, const double spanZ)
		{
			if ((availableRect.w <= 0.0) || (availableRect.h <= 0.0) || (spanX <= 0.0) || (spanZ <= 0.0))
			{
				return availableRect;
			}

			const double aspect = (spanX / spanZ);
			double width = availableRect.w;
			double height = (width / aspect);

			if (height > availableRect.h)
			{
				height = availableRect.h;
				width = (height * aspect);
			}

			return RectF{ Arg::center = availableRect.center(), width, height };
		}

		[[nodiscard]] bool IsMiniMapVisibleByFog(const bool showFogOfWar, const FogOfWarState& fogOfWar, const Vec3& worldPosition, const bool requireCurrentVision)
		{
			if (not showFogOfWar)
			{
				return true;
			}

			return requireCurrentVision
				? IsFogVisibleAt(fogOfWar, worldPosition)
				: IsFogExploredAt(fogOfWar, worldPosition);
		}

		[[nodiscard]] std::pair<Vec3, Vec3> GetWallEndpoints(const PlacedModel& placedModel)
		{
			const double wallLength = Clamp(placedModel.wallLength, 2.0, 80.0);
			const Vec3 direction{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
			const Vec3 halfDirection = (direction * (wallLength * 0.5));
			return std::pair{ (placedModel.position - halfDirection), (placedModel.position + halfDirection) };
		}

		[[nodiscard]] ColorF GetResourceTypeColor(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return ColorF{ 0.96, 0.82, 0.22, 0.78 };

			case ResourceType::Gunpowder:
				return ColorF{ 0.92, 0.42, 0.26, 0.78 };

			case ResourceType::Mana:
				return ColorF{ 0.42, 0.60, 0.98, 0.78 };

			default:
				return ColorF{ 0.70, 0.70, 0.70, 0.78 };
			}
		}

		[[nodiscard]] ColorF GetOwnerFrameColor(const Optional<UnitTeam>& ownerTeam)
		{
			if (ownerTeam && (*ownerTeam == UnitTeam::Player))
			{
				return ColorF{ 0.96, 0.96, 1.0, 0.95 };
			}

			if (ownerTeam && (*ownerTeam == UnitTeam::Enemy))
			{
				return ColorF{ 0.95, 0.35, 0.30, 0.95 };
			}

			return ColorF{ 0.65, 0.72, 0.82, 0.85 };
		}

		void DrawMiniMapFogLayer(const RectF& mapRect, const double minX, const double minZ, const double spanX, const double spanZ, const FogOfWarState& fogOfWar)
		{
			if (not fogOfWar.enabled)
			{
				return;
			}

			const double cellScale = Min((mapRect.w / spanX), (mapRect.h / spanZ));
			const double cellDrawSize = Max(2.0, (TerrainCellSize * cellScale));
			for (int32 y = fogOfWar.coverageBounds.y; y < fogOfWar.coverageBounds.bottomY(); ++y)
			{
				for (int32 x = fogOfWar.coverageBounds.x; x < fogOfWar.coverageBounds.rightX(); ++x)
				{
					const Point cell{ x, y };
					const FogVisibility visibility = GetFogVisibilityAt(fogOfWar, cell);
					if (visibility == FogVisibility::Visible)
					{
						continue;
					}

					const Vec2 center = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, ToTerrainCellCenter(cell));
					RectF{ Arg::center = center, cellDrawSize + 0.5, cellDrawSize + 0.5 }.draw(
						(visibility == FogVisibility::Explored)
							? ColorF{ 0.03, 0.05, 0.08, 0.42 }
					: ColorF{ 0.01, 0.02, 0.04, 0.88 });
				}
			}
		}
	}

	void DrawMiniMap(bool& isExpanded,
		  const SkyAppPanels& panels,
		const bool uiEditMode,
			  const bool showFogOfWar,
		  const AppCamera3D& camera,
		  const MapData& mapData,
		   const FogOfWarState& fogOfWar,
		  const Array<SpawnedSapper>& spawnedSappers,
		  const Array<SpawnedSapper>& enemySappers,
		  const Array<ResourceAreaState>& resourceAreaStates,
		  const Array<size_t>& selectedSapperIndices)
	{
		const Rect panel = panels.miniMap;
		const String title = (uiEditMode ? U"Mini Map [Drag/Resize]" : U"Mini Map");

		if (UiInternal::DrawAccordionHeader(panel, title, isExpanded, ColorF{ 0.08, 0.10, 0.12, 0.88 }, ColorF{ 0.75, 0.82, 0.90, 0.9 }, Palette::White, MainSupport::PanelSkinTarget::Hud) && (not uiEditMode))
		{
			isExpanded = not isExpanded;
		}

		if (not isExpanded)
		{
			return;
		}

		UiInternal::DrawPanelFrame(panel, U"", ColorF{ 0.08, 0.10, 0.12, 0.88 }, ColorF{ 0.75, 0.82, 0.90, 0.9 }, Palette::White, MainSupport::PanelSkinTarget::Hud);
		UiInternal::DrawAccordionHeader(panel, title, isExpanded, ColorF{ 0.08, 0.10, 0.12, 0.88 }, ColorF{ 0.75, 0.82, 0.90, 0.9 }, Palette::White, MainSupport::PanelSkinTarget::Hud);
     constexpr double panelInnerPadding = 10.0;
		constexpr double compassLabelHeight = 16.0;
		constexpr double mapSymbolPadding = 10.0;
		const RectF availableMapRect{
			(panel.x + panelInnerPadding),
			(panel.y + SkyAppUiLayout::AccordionHeaderHeight + compassLabelHeight + panelInnerPadding),
			Max(0.0, (panel.w - panelInnerPadding * 2.0)),
			Max(0.0, (panel.h - SkyAppUiLayout::AccordionHeaderHeight - compassLabelHeight - panelInnerPadding * 2.0))
		};
		const MiniMapBounds miniMapBounds = GetMiniMapBounds(mapData);
		const double minX = miniMapBounds.minX;
		const double minZ = miniMapBounds.minZ;
		const double spanX = miniMapBounds.spanX;
		const double spanZ = miniMapBounds.spanZ;
       const RectF mapRect = FitMiniMapRect(availableMapRect.stretched(-mapSymbolPadding), spanX, spanZ);

     SimpleGUI::GetFont()(U"N").drawAt((panel.x + panel.w * 0.5), (panel.y + SkyAppUiLayout::AccordionHeaderHeight + 10), ColorF{ 0.85, 0.92, 1.0 });
		mapRect.draw(ColorF{ 0.02, 0.03, 0.05, 0.96 });
		mapRect.drawFrame(1, 0, ColorF{ 0.35, 0.45, 0.55, 0.85 });

		for (const auto& placedModel : mapData.placedModels)
		{
			if (placedModel.type == PlaceableModelType::Wall)
			{
				const auto [start, end] = GetWallEndpoints(placedModel);
				const Vec2 startPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, start);
				const Vec2 endPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, end);
				Line{ startPoint, endPoint }.draw(3.0, ColorF{ 0.78, 0.82, 0.88, 0.95 });
				Circle{ startPoint, 2.2 }.draw(ColorF{ 0.60, 0.66, 0.74, 0.98 });
				Circle{ endPoint, 2.2 }.draw(ColorF{ 0.60, 0.66, 0.74, 0.98 });
			}
			else
			{
				Circle{ ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, placedModel.position), 2.0 }.draw(ColorF{ 0.48, 0.58, 0.48, 0.85 });
			}
		}

		for (size_t i = 0; i < mapData.resourceAreas.size(); ++i)
		{
			const ResourceArea& resourceArea = mapData.resourceAreas[i];
			if (not IsMiniMapVisibleByFog(showFogOfWar, fogOfWar, resourceArea.position, false))
			{
				continue;
			}

			const Optional<UnitTeam> ownerTeam = ((i < resourceAreaStates.size()) ? resourceAreaStates[i].ownerTeam : none);
			const Vec2 resourcePoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, resourceArea.position);
			const double radiusScale = Min((mapRect.w / spanX), (mapRect.h / spanZ));
			const double radius = Max(5.0, (resourceArea.radius * radiusScale));
			Circle{ resourcePoint, (radius + 1.5) }.drawFrame(3.0, ColorF{ 0.02, 0.03, 0.05, 0.92 });
			Circle{ resourcePoint, radius }.drawFrame(2.4, GetResourceTypeColor(resourceArea.type));
			Circle{ resourcePoint, Max(2.2, (radius * 0.20)) }.draw(ColorF{ 0.02, 0.03, 0.05, 0.90 });
			Circle{ resourcePoint, Max(1.4, (radius * 0.14)) }.draw(GetOwnerFrameColor(ownerTeam));
		}

		const Vec2 blacksmithPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, mapData.playerBasePosition);
		RectF{ Arg::center = blacksmithPoint, 8, 8 }.draw(ColorF{ 0.92, 0.74, 0.28, 0.95 });

		if (IsMiniMapVisibleByFog(showFogOfWar, fogOfWar, mapData.enemyBasePosition, false))
		{
			const Vec2 enemyBasePoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, mapData.enemyBasePosition);
			RectF{ Arg::center = enemyBasePoint, 8, 8 }.draw(ColorF{ 0.92, 0.28, 0.24, 0.95 });
		}

		const Vec2 rallyPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, mapData.sapperRallyPoint);
		Quad{
			rallyPoint.movedBy(0, -6),
			rallyPoint.movedBy(6, 0),
			rallyPoint.movedBy(0, 6),
			rallyPoint.movedBy(-6, 0)
		}.draw(ColorF{ 0.45, 0.88, 0.98, 0.95 });

		if (showFogOfWar)
		{
			DrawMiniMapFogLayer(mapRect, minX, minZ, spanX, spanZ, fogOfWar);
		}

		for (size_t i = 0; i < spawnedSappers.size(); ++i)
		{
			if (IsSapperRetreatedHidden(spawnedSappers[i]))
			{
				continue;
			}

			const Vec2 sapperPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, GetSpawnedSapperBasePosition(spawnedSappers[i]));
			const bool isSelected = selectedSapperIndices.contains(i);

			if (isSelected)
			{
				Circle{ sapperPoint, 7.0 }.drawFrame(2.0, ColorF{ 1.0, 0.95, 0.35, 0.95 });
			}

			Circle{ sapperPoint, 4.0 }.draw(isSelected ? ColorF{ 1.0, 0.96, 0.55, 0.98 } : ColorF{ 0.96, 0.96, 1.0, 0.95 });
		}

		for (const auto& enemySapper : enemySappers)
		{
			if (not IsMiniMapVisibleByFog(showFogOfWar, fogOfWar, GetSpawnedSapperBasePosition(enemySapper), true))
			{
				continue;
			}

			const Vec2 enemyPoint = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, GetSpawnedSapperBasePosition(enemySapper));
			RectF{ Arg::center = enemyPoint, 7, 7 }.draw(ColorF{ 0.95, 0.35, 0.30, 0.95 });
		}

		const Vec3 cameraPosition = camera.getEyePosition();
		Vec3 cameraDirection = (camera.getFocusPosition() - cameraPosition);
		cameraDirection.y = 0.0;

		if (cameraDirection.lengthSq() < 0.0001)
		{
			cameraDirection = Vec3{ 0, 0, 1 };
		}
		else
		{
			cameraDirection = cameraDirection.normalized();
		}

		const Vec3 cameraWing = Vec3{ -cameraDirection.z, 0, cameraDirection.x };
		const double cameraRange = Max(3.0, Max(spanX, spanZ) * 0.12);
		const Vec2 cameraCenter = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, cameraPosition);
		const Vec2 cameraTip = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, (cameraPosition + cameraDirection * cameraRange));
		const Vec2 cameraLeft = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, (cameraPosition + cameraDirection * cameraRange * 0.55 + cameraWing * cameraRange * 0.35));
		const Vec2 cameraRight = ToMiniMapPoint(mapRect, minX, minZ, spanX, spanZ, (cameraPosition + cameraDirection * cameraRange * 0.55 - cameraWing * cameraRange * 0.35));

		Line{ cameraCenter, cameraTip }.draw(2.0, ColorF{ 0.98, 0.5, 0.32, 0.95 });
		Triangle{ cameraTip, cameraLeft, cameraRight }.draw(ColorF{ 0.98, 0.5, 0.32, 0.7 });
		Circle{ cameraCenter, 4.0 }.draw(ColorF{ 1.0, 0.72, 0.52, 0.98 });

		if (uiEditMode)
		{
			const Rect resizeHandle = SkyAppUiLayout::MiniMapResizeHandle(panel);
			const Rect resizeHandleVisual = resizeHandle.stretched(-2);
			const Rect resizeHandleOuterFrame = resizeHandleVisual.stretched(4);
			//resizeHandle.movedBy(16,16).draw(ColorF{0.02, 0.04, 0.08, 0.88}).drawFrame(2.0, 0.0, ColorF{0.90, 0.94, 1.0, 0.96});
			resizeHandleOuterFrame.draw(ColorF{ 0.01, 0.02, 0.04, 0.86 }).drawFrame(2.0, 0.0, ColorF{ 0.90, 0.94, 1.0, 0.96 });
			resizeHandleVisual.draw(ColorF{ 0.80, 0.86, 0.96, 0.28 }).drawFrame(1.2, 0.0, ColorF{ 0.74, 0.82, 0.94, 0.88 });
			Line{ resizeHandleVisual.x + 4, resizeHandleVisual.bottomY() - 2, resizeHandleVisual.rightX() - 2, resizeHandleVisual.y + 4 }.draw(1.4, ColorF{ 0.92, 0.96, 1.0, 0.92 });
			Line{ resizeHandleVisual.x + 7, resizeHandleVisual.bottomY() - 2, resizeHandleVisual.rightX() - 2, resizeHandleVisual.y + 7 }.draw(1.2, ColorF{ 0.92, 0.96, 1.0, 0.78 });
		}
	}
}
