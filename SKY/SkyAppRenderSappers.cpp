# include "SkyAppSupport.hpp"
# include "BirdModel.hpp"
# include "MainUi.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
        [[nodiscard]] const UnitModel& SelectUnitRenderModel(const UnitRenderModelRegistryView& renderModels, const UnitRenderModel renderModel)
		{
            const size_t index = GetUnitRenderModelIndex(renderModel);
			if ((index < renderModels.models.size()) && renderModels.models[index])
			{
             return *renderModels.models[index];
			}

			return *renderModels.models[GetUnitRenderModelIndex(UnitRenderModel::Bird)];
		}

		constexpr double HealthBarWidth = 36.0;
		constexpr double HealthBarHeight = 5.0;
      constexpr int32 AttackRangeSegmentCount = 56;
		constexpr double Tau = 6.283185307179586;
           constexpr double TierBadgeWidth = 34.0;
			constexpr double TierBadgeHeight = 18.0;
		constexpr ColorF SuppressionAccentColor{ 0.70, 0.90, 1.0, 0.95 };
		constexpr ColorF SuppressionAuraColor{ 0.34, 0.68, 1.0, 0.18 };
		constexpr ColorF SuppressionLabelColor{ 0.92, 0.98, 1.0, 0.96 };
		constexpr int32 FootprintMarkerSegments = 40;
		constexpr double FootprintMarkerHeight = 0.028;
		constexpr double FootprintBaseY = 0.018;

		[[nodiscard]] Vec2 ToFootprintForward(const double facingYaw)
		{
			const double angle = (facingYaw - BirdDisplayYaw - SapperFacingYawOffset);
			return Vec2{ Math::Sin(angle), Math::Cos(angle) };
		}

		void DrawFootprintMarker(const Vec3& position, const double radius, const ColorF& color)
		{
			const ColorF shadowColor{ 0.02, 0.03, 0.05, (0.28 + color.a * 0.30) };
			Cylinder{ position.movedBy(0, FootprintBaseY, 0), (radius + 0.02), (FootprintMarkerHeight + 0.01) }.draw(shadowColor.removeSRGBCurve());
			Cylinder{ position.movedBy(0, FootprintBaseY + 0.008, 0), radius, FootprintMarkerHeight }.draw(color.removeSRGBCurve());
		}

		void DrawFootprintPolyline(const Array<Vec3>& points, const double markerRadius, const ColorF& color)
		{
			for (const Vec3& point : points)
			{
				DrawFootprintMarker(point, markerRadius, color);
			}
		}

		void DrawCircleFootprint(const Vec3& position, const double radius, const ColorF& color)
		{
			Array<Vec3> points;
			points.reserve(FootprintMarkerSegments);

			for (int32 i = 0; i < FootprintMarkerSegments; ++i)
			{
				const double angle = (Math::TwoPi * i / FootprintMarkerSegments);
				points << position.movedBy(Math::Cos(angle) * radius, 0, Math::Sin(angle) * radius);
			}

			DrawFootprintPolyline(points, Clamp(radius * 0.11, 0.07, 0.16), color);
		}

		void DrawCapsuleFootprint(const Vec3& position, const double facingYaw, const double radius, const double halfLength, const ColorF& color)
		{
			if (halfLength <= 0.01)
			{
				DrawCircleFootprint(position, radius, color);
				return;
			}

			const Vec2 axis = ToFootprintForward(facingYaw).normalized_or(Vec2{ 0, 1 });
			Array<Vec3> points;
			points.reserve(FootprintMarkerSegments);

			for (int32 i = 0; i < FootprintMarkerSegments; ++i)
			{
				const double angle = (Math::TwoPi * i / FootprintMarkerSegments);
				const Vec2 direction{ Math::Cos(angle), Math::Sin(angle) };
				const Vec2 axisOffset = (Math::Sign(direction.dot(axis)) * halfLength * axis);
				const Vec2 point = (axisOffset + direction * radius);
				points << position.movedBy(point.x, 0, point.y);
			}

			DrawFootprintPolyline(points, Clamp(radius * 0.11, 0.07, 0.16), color);
			DrawFootprintMarker(position.movedBy(axis.x * (halfLength + radius), 0, axis.y * (halfLength + radius)), Clamp(radius * 0.16, 0.08, 0.19), ColorF{ Min(color.r + 0.10, 1.0), Min(color.g + 0.10, 1.0), Min(color.b + 0.06, 1.0), Min(color.a + 0.10, 1.0) });
		}

		void DrawFootprint(const Vec3& position, const double facingYaw, const UnitParameters& parameters, const ColorF& color)
		{
			const double radius = Max(0.1, parameters.footprintRadius);
			if (parameters.footprintType == UnitFootprintType::Capsule)
			{
				DrawCapsuleFootprint(position, facingYaw, radius, Max(0.0, parameters.footprintHalfLength), color);
				return;
			}

			DrawCircleFootprint(position, radius, color);
		}

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
            if (not IsSpawnedSapperSelectable(spawnedSappers[i]))
			{
				continue;
			}

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

	void DrawSelectedSapperFootprint(const SpawnedSapper& sapper, const ColorF& color)
	{
		if ((sapper.hitPoints <= 0.0) || IsSapperRetreatedHidden(sapper))
		{
			return;
		}

		const UnitParameters parameters{
			.movementType = sapper.movementType,
			.aiRole = UnitAiRole::SecureResources,
			.maxHitPoints = sapper.maxHitPoints,
			.moveSpeed = sapper.moveSpeed,
			.attackRange = sapper.attackRange,
			.stopDistance = sapper.stopDistance,
			.attackDamage = sapper.baseAttackDamage,
			.attackInterval = sapper.baseAttackInterval,
			.footprintType = sapper.footprintType,
			.footprintRadius = sapper.footprintRadius,
			.footprintHalfLength = sapper.footprintHalfLength,
		};
		DrawFootprint(GetSpawnedSapperBasePosition(sapper), GetSpawnedSapperYaw(sapper), parameters, color);
	}

	void DrawUnitFootprintPreview(const Vec3& position, const double yaw, const UnitParameters& parameters, const ColorF& color)
	{
		DrawFootprint(position, yaw, parameters, color);
	}

	void DrawSelectedSapperAttackRange(const AppCamera3D& camera, const SpawnedSapper& sapper)
	{
		const Vec3 rangeCenter = GetSpawnedSapperBasePosition(sapper).movedBy(0, 0.05, 0);
		const double pulse = (0.76 + (0.24 * Periodic::Sine0_1(1.4s)));
        const ColorF attackOuterColor{ 0.38, 0.74, 1.0, 0.14 + 0.10 * pulse };
		const ColorF attackInnerColor{ 0.68, 0.88, 1.0, 0.74 + 0.10 * pulse };
		const ColorF stopOuterColor{ 1.0, 0.48, 0.22, 0.16 + 0.12 * pulse };
		const ColorF stopInnerColor{ 1.0, 0.72, 0.44, 0.82 + 0.10 * pulse };

       DrawProjectedCircumference(camera, rangeCenter, sapper.attackRange, 5.0, attackOuterColor);
		DrawProjectedCircumference(camera, rangeCenter, sapper.attackRange, 2.2, attackInnerColor);

		if (0.0 < sapper.stopDistance)
		{
			DrawProjectedCircumference(camera, rangeCenter, sapper.stopDistance, 4.2, stopOuterColor);
			DrawProjectedCircumference(camera, rangeCenter, sapper.stopDistance, 1.8, stopInnerColor);
		}
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
            if ((sapper.hitPoints <= 0.0) || IsSapperRetreatedHidden(sapper))
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

  void DrawSpawnedSappers(const Array<SpawnedSapper>& spawnedSappers, const UnitRenderModelRegistryView& renderModels, const ModelHeightSettings& modelHeightSettings, const ColorF& color)
	{
		for (const auto& sapper : spawnedSappers)
		{
            if ((sapper.hitPoints <= 0.0) || IsSapperRetreatedHidden(sapper))
			{
				continue;
			}

			const double elapsed = (Scene::Time() - sapper.spawnedAt);
			const double popIn = Min(elapsed / 0.25, 1.0);
			const Vec3 renderPosition = GetSpawnedSapperRenderPosition(sapper);
         const UnitRenderModel renderModel = GetSpawnedSapperRenderModel(sapper);
           const UnitModel& drawModel = SelectUnitRenderModel(renderModels, renderModel);
          const double drawScale = GetModelScale(modelHeightSettings, renderModel);

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
