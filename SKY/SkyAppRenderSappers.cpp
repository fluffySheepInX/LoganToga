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
      constexpr int32 AttackRangeSegmentCount = 56;
		constexpr double Tau = 6.283185307179586;
           constexpr double TierBadgeWidth = 34.0;
			constexpr double TierBadgeHeight = 18.0;
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

		void DrawProjectedCircumference(const AppCamera3D& camera, const Vec3& center, const double radius, const double thickness, const ColorF& color)
		{
			for (int32 i = 0; i < AttackRangeSegmentCount; ++i)
			{
				const double startAngle = (Tau * i / AttackRangeSegmentCount);
				const double endAngle = (Tau * (i + 1) / AttackRangeSegmentCount);
				const Vec3 worldStart = center.movedBy((Math::Cos(startAngle) * radius), 0, (Math::Sin(startAngle) * radius));
				const Vec3 worldEnd = center.movedBy((Math::Cos(endAngle) * radius), 0, (Math::Sin(endAngle) * radius));
				const Optional<Vec2> screenStart = ProjectToScreen(camera, worldStart);
				const Optional<Vec2> screenEnd = ProjectToScreen(camera, worldEnd);

				if (screenStart && screenEnd)
				{
					Line{ *screenStart, *screenEnd }.draw(thickness, color);
				}
			}
		}
	}

  Optional<size_t> HitTestSpawnedSapper(const Array<SpawnedSapper>& spawnedSappers, const AppCamera3D& camera, const ModelHeightSettings& modelHeightSettings)
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
            const double scale = Max(ModelScaleMin, GetSpawnedSapperModelScale(modelHeightSettings, spawnedSappers[i]));
			const Vec3 hitCenter = GetSpawnedSapperRenderPosition(spawnedSappers[i]).movedBy(0, (1.4 * scale), 0);
			const Sphere hitSphere{ hitCenter, Max(0.45, (1.2 * scale)) };

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

	void DrawSelectedSapperAttackRange(const AppCamera3D& camera, const SpawnedSapper& sapper)
	{
		const Vec3 rangeCenter = GetSpawnedSapperBasePosition(sapper).movedBy(0, 0.05, 0);
		const double pulse = (0.76 + (0.24 * Periodic::Sine0_1(1.4s)));
		const ColorF outerColor{ 1.0, 0.80, 0.24, 0.16 + 0.10 * pulse };
		const ColorF innerColor{ 1.0, 0.93, 0.52, 0.72 + 0.12 * pulse };

		DrawProjectedCircumference(camera, rangeCenter, sapper.attackRange, 5.0, outerColor);
		DrawProjectedCircumference(camera, rangeCenter, sapper.attackRange, 2.2, innerColor);
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

			if (sapper.tier > 1)
			{
				const double tierRate = Math::Saturate((sapper.tier - 1.0) / Max(1.0, static_cast<double>(MaximumSapperTier - 1)));
				const RoundRect tierBadge{ Arg::center = screenAnchor->movedBy(0, -34), TierBadgeWidth, TierBadgeHeight, 4.0 };
				const ColorF badgeColor{ 1.0, (0.76 + 0.16 * tierRate), (0.22 + 0.18 * tierRate), 0.94 };
				tierBadge.draw(badgeColor);
				tierBadge.drawFrame(1.5, ColorF{ 0.45, 0.25, 0.06, 0.92 });
				SimpleGUI::GetFont()(U"T{}"_fmt(sapper.tier)).drawAt(tierBadge.center(), ColorF{ 0.30, 0.18, 0.04, 0.98 });
			}
		}
	}

    void DrawSpawnedSappers(const Array<SpawnedSapper>& spawnedSappers, const BirdModel& sapperModel, const BirdModel& sugoiCarModel, const ModelHeightSettings& modelHeightSettings, const ColorF& color)
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
			const BirdModel& drawModel = ((sapper.unitType == SapperUnitType::SugoiCar) ? sugoiCarModel : sapperModel);
          const double drawScale = (sapper.unitType == SapperUnitType::SugoiCar)
				? GetModelScale(modelHeightSettings, ModelHeightTarget::SugoiCar)
				: ((sapper.team == UnitTeam::Enemy)
					? GetModelScale(modelHeightSettings, ModelHeightTarget::Ashigaru)
					: GetModelScale(modelHeightSettings, ModelHeightTarget::Bird));

         if (drawModel.isLoaded())
			{
				const double appearance = Max(0.72, (0.72 + 0.28 * EaseOutBack(popIn)));
             const ColorF unitColor = GetSpawnedSapperTint(sapper, color);
				const ColorF tint{ unitColor.r * appearance, unitColor.g * appearance, unitColor.b * appearance, unitColor.a };

				if (IsSapperSuppressed(sapper))
				{
					const double pulse = (0.82 + 0.18 * Periodic::Sine0_1(2.0s));
					Sphere{ renderPosition.movedBy(0, 1.25, 0), (1.05 + 0.10 * pulse) }.draw(ColorF{ SuppressionAuraColor.r, SuppressionAuraColor.g, SuppressionAuraColor.b, (SuppressionAuraColor.a + 0.05 * pulse) }.removeSRGBCurve());
				}

                drawModel.draw(renderPosition, GetSpawnedSapperYaw(sapper), tint.removeSRGBCurve(), drawScale);
			}
			else
			{
				const double radius = (0.22 + (0.68 * EaseOutBack(popIn)));
               Sphere{ renderPosition.movedBy(0, radius, 0), radius }.draw(GetSpawnedSapperTint(sapper, color).removeSRGBCurve());
			}
		}
	}
}
