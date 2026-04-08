# include "SkyAppUiInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		[[nodiscard]] Vec2 ToMiniMapPoint(const RectF& mapRect, const double minX, const double minZ, const double worldSpan, const Vec3& worldPosition)
		{
			const double x = (worldPosition.x - minX) / worldSpan;
			const double z = (worldPosition.z - minZ) / worldSpan;
			return Vec2{ (mapRect.x + x * mapRect.w), (mapRect.bottomY() - z * mapRect.h) };
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
	}

    void DrawMiniMap(bool& isExpanded,
		const SkyAppPanels& panels,
        const AppCamera3D& camera,
		const MapData& mapData,
		const Array<SpawnedSapper>& spawnedSappers,
		const Array<SpawnedSapper>& enemySappers,
		const Array<ResourceAreaState>& resourceAreaStates,
		const Array<size_t>& selectedSapperIndices)
	{
		const Rect panel = panels.miniMap;

		if (UiInternal::DrawAccordionHeader(panel, U"Mini Map", isExpanded, ColorF{ 0.08, 0.10, 0.12, 0.88 }, ColorF{ 0.75, 0.82, 0.90, 0.9 }, Palette::White))
		{
			isExpanded = not isExpanded;
		}

		if (not isExpanded)
		{
			return;
		}

		panel.draw(ColorF{ 0.08, 0.10, 0.12, 0.88 });
		panel.drawFrame(2, 0, ColorF{ 0.75, 0.82, 0.90, 0.9 });
		UiInternal::DrawAccordionHeader(panel, U"Mini Map", isExpanded, ColorF{ 0.08, 0.10, 0.12, 0.88 }, ColorF{ 0.75, 0.82, 0.90, 0.9 }, Palette::White);
		const RectF mapRect{ (panel.x + 10), (panel.y + SkyAppUiLayout::AccordionHeaderHeight + 10), (panel.w - 20), Max(0, (panel.h - SkyAppUiLayout::AccordionHeaderHeight - 20)) };
		Array<Vec3> boundsPoints{
			mapData.playerBasePosition,
			mapData.enemyBasePosition,
			mapData.sapperRallyPoint,
			camera.getEyePosition(),
			camera.getFocusPosition(),
		};

		for (const auto& sapper : spawnedSappers)
		{
			boundsPoints << GetSpawnedSapperBasePosition(sapper);
		}

		for (const auto& enemySapper : enemySappers)
		{
			boundsPoints << GetSpawnedSapperBasePosition(enemySapper);
		}

		for (const auto& placedModel : mapData.placedModels)
		{
			boundsPoints << placedModel.position;
		}

		for (const auto& resourceArea : mapData.resourceAreas)
		{
			boundsPoints << resourceArea.position;
		}

		double minX = boundsPoints.front().x;
		double maxX = boundsPoints.front().x;
		double minZ = boundsPoints.front().z;
		double maxZ = boundsPoints.front().z;

		for (const auto& point : boundsPoints)
		{
			minX = Min(minX, point.x);
			maxX = Max(maxX, point.x);
			minZ = Min(minZ, point.z);
			maxZ = Max(maxZ, point.z);
		}

		const double padding = 4.0;
		minX -= padding;
		maxX += padding;
		minZ -= padding;
		maxZ += padding;

		const double centerX = ((minX + maxX) * 0.5);
		const double centerZ = ((minZ + maxZ) * 0.5);
		const double worldSpan = Max({ (maxX - minX), (maxZ - minZ), 20.0 });
		minX = (centerX - worldSpan * 0.5);
		minZ = (centerZ - worldSpan * 0.5);

       SimpleGUI::GetFont()(U"N").drawAt((panel.x + panel.w * 0.5), (panel.y + SkyAppUiLayout::AccordionHeaderHeight + 8), ColorF{ 0.85, 0.92, 1.0 });
		mapRect.draw(ColorF{ 0.02, 0.03, 0.05, 0.96 });
		mapRect.drawFrame(1, 0, ColorF{ 0.35, 0.45, 0.55, 0.85 });

		for (const auto& placedModel : mapData.placedModels)
		{
			Circle{ ToMiniMapPoint(mapRect, minX, minZ, worldSpan, placedModel.position), 2.0 }.draw(ColorF{ 0.48, 0.58, 0.48, 0.85 });
		}

		for (size_t i = 0; i < mapData.resourceAreas.size(); ++i)
		{
			const ResourceArea& resourceArea = mapData.resourceAreas[i];
			const Optional<UnitTeam> ownerTeam = ((i < resourceAreaStates.size()) ? resourceAreaStates[i].ownerTeam : none);
			const Vec2 resourcePoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, resourceArea.position);
			const double radius = Max(5.0, (resourceArea.radius / worldSpan) * mapRect.w);
          Circle{ resourcePoint, (radius + 1.5) }.drawFrame(3.0, ColorF{ 0.02, 0.03, 0.05, 0.92 });
			Circle{ resourcePoint, radius }.drawFrame(2.4, GetResourceTypeColor(resourceArea.type));
			Circle{ resourcePoint, Max(2.2, (radius * 0.20)) }.draw(ColorF{ 0.02, 0.03, 0.05, 0.90 });
			Circle{ resourcePoint, Max(1.4, (radius * 0.14)) }.draw(GetOwnerFrameColor(ownerTeam));
		}

		const Vec2 blacksmithPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.playerBasePosition);
		RectF{ Arg::center = blacksmithPoint, 8, 8 }.draw(ColorF{ 0.92, 0.74, 0.28, 0.95 });

		const Vec2 enemyBasePoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.enemyBasePosition);
		RectF{ Arg::center = enemyBasePoint, 8, 8 }.draw(ColorF{ 0.92, 0.28, 0.24, 0.95 });

		const Vec2 rallyPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.sapperRallyPoint);
		Quad{
			rallyPoint.movedBy(0, -6),
			rallyPoint.movedBy(6, 0),
			rallyPoint.movedBy(0, 6),
			rallyPoint.movedBy(-6, 0)
		}.draw(ColorF{ 0.45, 0.88, 0.98, 0.95 });

		for (size_t i = 0; i < spawnedSappers.size(); ++i)
		{
			const Vec2 sapperPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, GetSpawnedSapperBasePosition(spawnedSappers[i]));
			const bool isSelected = selectedSapperIndices.contains(i);

			if (isSelected)
			{
				Circle{ sapperPoint, 7.0 }.drawFrame(2.0, ColorF{ 1.0, 0.95, 0.35, 0.95 });
			}

			Circle{ sapperPoint, 4.0 }.draw(isSelected ? ColorF{ 1.0, 0.96, 0.55, 0.98 } : ColorF{ 0.96, 0.96, 1.0, 0.95 });
		}

		for (const auto& enemySapper : enemySappers)
		{
			const Vec2 enemyPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, GetSpawnedSapperBasePosition(enemySapper));
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
		const double cameraRange = Max(3.0, worldSpan * 0.12);
		const Vec2 cameraCenter = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, cameraPosition);
		const Vec2 cameraTip = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange));
		const Vec2 cameraLeft = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange * 0.55 + cameraWing * cameraRange * 0.35));
		const Vec2 cameraRight = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange * 0.55 - cameraWing * cameraRange * 0.35));

		Line{ cameraCenter, cameraTip }.draw(2.0, ColorF{ 0.98, 0.5, 0.32, 0.95 });
		Triangle{ cameraTip, cameraLeft, cameraRight }.draw(ColorF{ 0.98, 0.5, 0.32, 0.7 });
		Circle{ cameraCenter, 4.0 }.draw(ColorF{ 1.0, 0.72, 0.52, 0.98 });
	}
}
