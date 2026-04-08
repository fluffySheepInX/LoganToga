# include "SkyAppSupport.hpp"
# include "BirdModel.hpp"
# include "MainUi.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		constexpr double HealthBarWidth = 36.0;
		constexpr double HealthBarHeight = 5.0;
		constexpr ColorF SuppressionAccentColor{ 0.70, 0.90, 1.0, 0.95 };
		constexpr ColorF SuppressionAuraColor{ 0.34, 0.68, 1.0, 0.18 };
		constexpr ColorF SuppressionLabelColor{ 0.92, 0.98, 1.0, 0.96 };

        [[nodiscard]] Optional<Vec2> ProjectToScreen(const AppCamera3D& camera, const Vec3& worldPosition)
		{
			const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(worldPosition.x), static_cast<float>(worldPosition.y), static_cast<float>(worldPosition.z) });

			if (screenPosition.z <= 0.0f)
			{
				return none;
			}

			return Vec2{ screenPosition.x, screenPosition.y };
		}
	}

  Optional<size_t> HitTestSpawnedSapper(const Array<SpawnedSapper>& spawnedSappers, const AppCamera3D& camera)
	{
		const Optional<Ray> cursorRay = TryScreenToRay(camera, Cursor::PosF());
		if (not cursorRay)
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<size_t> hitIndex;

		for (size_t i = 0; i < spawnedSappers.size(); ++i)
		{
			const Vec3 hitCenter = GetSpawnedSapperRenderPosition(spawnedSappers[i]).movedBy(0, 1.4, 0);
			const Sphere hitSphere{ hitCenter, 1.2 };

			if (const auto distance = cursorRay->intersects(hitSphere))
			{
				if (*distance < nearestDistance)
				{
					nearestDistance = *distance;
					hitIndex = i;
				}
			}
		}

		return hitIndex;
	}

   void DrawSelectedSapperRing(const AppCamera3D& camera, const SpawnedSapper& sapper)
	{
		const Vec3 ringCenter = GetSpawnedSapperBasePosition(sapper).movedBy(0, 0.06, 0);
		const Optional<Vec2> screenCenter = ProjectToScreen(camera, ringCenter);
		const Optional<Vec2> screenEdgeX = ProjectToScreen(camera, ringCenter.movedBy(1.15, 0, 0));
		const Optional<Vec2> screenEdgeZ = ProjectToScreen(camera, ringCenter.movedBy(0, 0, 1.15));

		if ((not screenCenter) || (not screenEdgeX) || (not screenEdgeZ))
		{
			return;
		}

		const double radiusX = screenCenter->distanceFrom(*screenEdgeX);
		const double radiusZ = screenCenter->distanceFrom(*screenEdgeZ);
		const double radius = Max(18.0, (radiusX + radiusZ) * 0.5);
		const double pulse = (0.75 + (0.25 * Periodic::Sine0_1(1.8s)));
		const ColorF outerColor{ 1.0, 0.92, 0.25, 0.30 + 0.25 * pulse };
		const ColorF innerColor{ 1.0, 0.96, 0.55, 0.90 };

		Circle{ *screenCenter, radius + 8.0 * pulse }.drawFrame(7.0, outerColor);
		Circle{ *screenCenter, radius }.drawFrame(3.5, innerColor);
	}

   void DrawSelectedSapperIcon(const AppCamera3D& camera, const SpawnedSapper& sapper)
	{
		const double pulse = (0.65 + (0.35 * Periodic::Sine0_1(1.8s)));
		const Vec3 iconAnchor = GetSpawnedSapperRenderPosition(sapper).movedBy(0, 3.2 + 0.18 * pulse, 0);
		const Optional<Vec2> screenAnchor = ProjectToScreen(camera, iconAnchor);

		if (not screenAnchor)
		{
			return;
		}

		const Vec2 circleCenter = screenAnchor->movedBy(0, -26);
		Circle{ circleCenter, 9.0 + pulse }.draw(ColorF{ 1.0, 0.95, 0.40, 0.95 });
		Circle{ circleCenter, 9.0 + pulse }.drawFrame(2.0, ColorF{ 0.45, 0.32, 0.08, 0.95 });
		Triangle{
			screenAnchor->movedBy(0, -4),
			screenAnchor->movedBy(-9, -20),
			screenAnchor->movedBy(9, -20)
		}.draw(ColorF{ 1.0, 0.93, 0.35, 0.95 });
	}

 void DrawSapperHealthBars(const AppCamera3D& camera, const Array<SpawnedSapper>& spawnedSappers, const ColorF& fillColor)
	{
		for (const auto& sapper : spawnedSappers)
		{
			if (sapper.hitPoints <= 0.0)
			{
				continue;
			}

			const Optional<Vec2> screenAnchor = ProjectToScreen(camera, GetSpawnedSapperRenderPosition(sapper).movedBy(0, 3.0, 0));

			if (not screenAnchor)
			{
				continue;
			}

			const double rate = Math::Saturate(sapper.hitPoints / Max(1.0, sapper.maxHitPoints));
			const RectF backRect{ Arg::center = screenAnchor->movedBy(0, -8), HealthBarWidth, HealthBarHeight };
			const RectF fillRect{ backRect.pos, (backRect.w * rate), backRect.h };
			backRect.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
			fillRect.draw(fillColor);
         backRect.drawFrame(1.0, IsSapperSuppressed(sapper) ? SuppressionAccentColor : ColorF{ 0.9, 0.9, 0.9, 0.65 });

			if (IsSapperSuppressed(sapper))
			{
				RectF{ backRect.x, (backRect.y - 4), backRect.w, 2.0 }.draw(SuppressionAccentColor);
				SimpleGUI::GetFont()(U"SUP").drawAt(screenAnchor->movedBy(0, -22), SuppressionLabelColor);
			}
		}
	}

	void DrawSpawnedSappers(const Array<SpawnedSapper>& spawnedSappers, const BirdModel& sapperModel, const ColorF& color)
	{
		for (const auto& sapper : spawnedSappers)
		{
			if (sapper.hitPoints <= 0.0)
			{
				continue;
			}

			const double elapsed = (Scene::Time() - sapper.spawnedAt);
			const double popIn = Min(elapsed / 0.25, 1.0);
			const Vec3 renderPosition = GetSpawnedSapperRenderPosition(sapper);

			if (sapperModel.isLoaded())
			{
				const double appearance = Max(0.72, (0.72 + 0.28 * EaseOutBack(popIn)));
             const ColorF unitColor = GetSpawnedSapperTint(sapper, color);
				const ColorF tint{ unitColor.r * appearance, unitColor.g * appearance, unitColor.b * appearance, unitColor.a };

				if (IsSapperSuppressed(sapper))
				{
					const double pulse = (0.82 + 0.18 * Periodic::Sine0_1(2.0s));
					Sphere{ renderPosition.movedBy(0, 1.25, 0), (1.05 + 0.10 * pulse) }.draw(ColorF{ SuppressionAuraColor.r, SuppressionAuraColor.g, SuppressionAuraColor.b, (SuppressionAuraColor.a + 0.05 * pulse) }.removeSRGBCurve());
				}

				sapperModel.draw(renderPosition, GetSpawnedSapperYaw(sapper), tint.removeSRGBCurve());
			}
			else
			{
				const double radius = (0.22 + (0.68 * EaseOutBack(popIn)));
               Sphere{ renderPosition.movedBy(0, radius, 0), radius }.draw(GetSpawnedSapperTint(sapper, color).removeSRGBCurve());
			}
		}
	}
}
