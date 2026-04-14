# include "SkyAppRenderOverlayInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
       [[nodiscard]] size_t GetResourceTypeIndex(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return 0;

			case ResourceType::Gunpowder:
				return 1;

			case ResourceType::Mana:
			default:
				return 2;
			}
		}

		[[nodiscard]] const Optional<Texture>& GetResourceAreaOverlayTexture(const ResourceType type)
		{
			static bool initialized = false;
			static std::array<Optional<Texture>, 3> textures;

			if (not initialized)
			{
				initialized = true;
				const std::array<std::pair<ResourceType, std::array<FilePath, 4>>, 3> candidates{{
					{ ResourceType::Budget, { U"App/texture/ui/resource_budget.png", U"App/ui/resource_budget.png", U"App/texture/resource_budget.png", U"App/resource_budget.png" } },
					{ ResourceType::Gunpowder, { U"App/texture/ui/resource_gunpowder.png", U"App/ui/resource_gunpowder.png", U"App/texture/resource_gunpowder.png", U"App/resource_gunpowder.png" } },
					{ ResourceType::Mana, { U"App/texture/ui/resource_mana.png", U"App/ui/resource_mana.png", U"App/texture/resource_mana.png", U"App/resource_mana.png" } },
				}};

				for (const auto& [resourceType, paths] : candidates)
				{
					for (const auto& path : paths)
					{
						if (FileSystem::Exists(path))
						{
							textures[GetResourceTypeIndex(resourceType)] = Texture{ path };
							break;
						}
					}
				}
			}

			return textures[GetResourceTypeIndex(type)];
		}

		void DrawFallbackResourceAreaIcon(const ResourceType type, const Vec2& center, const double size)
		{
			const ColorF accent = OverlayDetail::GetResourceTextColor(type);
			const ColorF dark{ 0.10, 0.12, 0.16, 0.96 };

			switch (type)
			{
			case ResourceType::Budget:
				Circle{ center, (size * 0.28) }.draw(accent);
				Circle{ center, (size * 0.28) }.drawFrame(2.0, ColorF{ 1.0, 0.97, 0.72, 0.95 });
				Line{ center.movedBy(0, -(size * 0.18)), center.movedBy(0, (size * 0.18)) }.draw(2.2, dark);
				Line{ center.movedBy(-(size * 0.12), -(size * 0.06)), center.movedBy((size * 0.12), -(size * 0.06)) }.draw(2.0, dark);
				Line{ center.movedBy(-(size * 0.12), (size * 0.06)), center.movedBy((size * 0.12), (size * 0.06)) }.draw(2.0, dark);
				return;

			case ResourceType::Gunpowder:
				RectF{ Arg::center = center.movedBy(0, (size * 0.05)), (size * 0.38), (size * 0.34) }.rounded(4).draw(accent).drawFrame(1.5, ColorF{ 0.98, 0.88, 0.82, 0.95 });
				Triangle{ center.movedBy(0, -(size * 0.18)), center.movedBy(-(size * 0.10), -(size * 0.02)), center.movedBy((size * 0.10), -(size * 0.02)) }.draw(ColorF{ 0.98, 0.84, 0.42, 0.95 });
				Line{ center.movedBy(0, -(size * 0.28)), center.movedBy(0, -(size * 0.14)) }.draw(2.0, ColorF{ 0.96, 0.94, 0.88, 0.95 });
				return;

			case ResourceType::Mana:
			default:
				Circle{ center, (size * 0.26) }.draw(accent);
				Circle{ center.movedBy(-(size * 0.08), -(size * 0.08)), (size * 0.10) }.draw(ColorF{ 0.92, 0.97, 1.0, 0.92 });
				Line{ center.movedBy(-(size * 0.18), 0), center.movedBy((size * 0.18), 0) }.draw(1.8, ColorF{ 0.92, 0.97, 1.0, 0.88 });
				Line{ center.movedBy(0, -(size * 0.18)), center.movedBy(0, (size * 0.18)) }.draw(1.8, ColorF{ 0.92, 0.97, 1.0, 0.88 });
				return;
			}
		}

		void DrawResourceAreaIconBadge(const ResourceType type, const Optional<UnitTeam>& ownerTeam, const Vec2& center)
		{
			const RectF badgeRect{ Arg::center = center, 28, 28 };
			badgeRect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.12, 0.88 })
				.drawFrame(1.5, 0.0, OverlayDetail::GetTeamColor(ownerTeam));

			if (const auto& texture = GetResourceAreaOverlayTexture(type))
			{
               texture->resized(20, 20).drawAt(center);
				return;
			}

			DrawFallbackResourceAreaIcon(type, center, 20.0);
		}

		void DrawLaserAttackEffect(const AppCamera3D& camera, const AttackEffectInstance& effect)
		{
			const Optional<Vec2> start = OverlayDetail::ProjectToScreen(camera, effect.startPosition);
			const Optional<Vec2> end = OverlayDetail::ProjectToScreen(camera, effect.endPosition);

			if ((not start) || (not end))
			{
				return;
			}

			const double elapsed = Max(0.0, (Scene::Time() - effect.startedAt));
			const double fade = (1.0 - Math::Saturate(elapsed / Max(effect.lifetime, 0.001)));
			if (fade <= 0.0)
			{
				return;
			}

			Line{ *start, *end }.draw((effect.thickness * 2.6), OverlayDetail::WithAlphaScaled(effect.color, (0.14 * fade)));
			Line{ *start, *end }.draw((effect.thickness * 1.4), OverlayDetail::WithAlphaScaled(effect.color, (0.34 * fade)));
			Line{ *start, *end }.draw(effect.thickness, OverlayDetail::WithAlphaScaled(ColorF{ 1.0, 1.0, 1.0, 1.0 }, (0.92 * fade)));
			Circle{ *start, (effect.thickness * 0.8) }.draw(OverlayDetail::WithAlphaScaled(effect.color, (0.55 * fade)));
			Circle{ *end, (effect.thickness * 1.2) }.draw(OverlayDetail::WithAlphaScaled(effect.color, (0.72 * fade)));
			Circle{ *end, (effect.thickness * 0.55) }.draw(OverlayDetail::WithAlphaScaled(ColorF{ 1.0, 1.0, 1.0, 1.0 }, (0.95 * fade)));
		}

		void DrawExplosionAttackEffect(const AppCamera3D& camera, const AttackEffectInstance& effect)
		{
			const Optional<Vec2> center = OverlayDetail::ProjectToScreen(camera, effect.startPosition);
			const Optional<Vec2> edge = OverlayDetail::ProjectToScreen(camera, effect.startPosition.movedBy(effect.radius, 0, 0));

			if ((not center) || (not edge))
			{
				return;
			}

			const double elapsed = Max(0.0, (Scene::Time() - effect.startedAt));
			const double duration = Max(effect.lifetime, 0.001);
			const double t = Math::Saturate(elapsed / duration);
			const double fade = (1.0 - t);
			if (fade <= 0.0)
			{
				return;
			}

			const double baseRadius = Max(8.0, center->distanceFrom(*edge));
			const double outerRadius = (baseRadius * (0.35 + t * 1.15));
			const double innerRadius = (outerRadius * 0.52);
			Circle{ *center, outerRadius }.drawFrame((effect.thickness * 0.55), OverlayDetail::WithAlphaScaled(effect.color, (0.88 * fade)));
			Circle{ *center, innerRadius }.draw(OverlayDetail::WithAlphaScaled(ColorF{ 1.0, 0.92, 0.72, 1.0 }, (0.24 * fade)));
			Circle{ *center, (effect.thickness * (0.55 + 0.85 * fade)) }.draw(OverlayDetail::WithAlphaScaled(ColorF{ 1.0, 1.0, 1.0, 1.0 }, (0.92 * fade)));
		}

		void DrawAttackEffect(const AppCamera3D& camera, const AttackEffectInstance& effect)
		{
			switch (effect.type)
			{
			case AttackEffectType::Laser:
				DrawLaserAttackEffect(camera, effect);
				return;

			case AttackEffectType::Explosion:
				DrawExplosionAttackEffect(camera, effect);
				return;

			default:
				return;
			}
		}

		void DrawMoveOrderIndicator(const AppCamera3D& camera, const MoveOrderIndicator& indicator)
		{
			const double elapsed = Max(0.0, (Scene::Time() - indicator.startedAt));
			const double duration = Max(indicator.lifetime, 0.001);
			const double t = Math::Saturate(elapsed / duration);
			const double fade = (1.0 - t);
			if (fade <= 0.0)
			{
				return;
			}

			const double bobOffset = (-18.0 - 12.0 * Math::Sin(t * Math::TwoPi * 2.0));
			const double ringPulse = (1.0 + 0.24 * Math::Sin(t * Math::TwoPi * 2.5));
			const Optional<Vec2> groundAnchor = OverlayDetail::ProjectToScreen(camera, indicator.position.movedBy(0, 0.08, 0));
			const Optional<Vec2> arrowAnchor = OverlayDetail::ProjectToScreen(camera, indicator.position.movedBy(0, 1.5, 0));

			if ((not groundAnchor) || (not arrowAnchor))
			{
				return;
			}

			const Vec2 arrowCenter = arrowAnchor->movedBy(0, bobOffset);
			const ColorF glowColor{ 0.26, 0.86, 1.0, 0.30 * fade };
			const ColorF ringColor{ 0.58, 0.92, 1.0, 0.92 * fade };
			const ColorF arrowColor{ 1.0, 0.96, 0.58, 0.96 * fade };
			const ColorF outlineColor{ 0.22, 0.18, 0.06, 0.92 * fade };

			Circle{ *groundAnchor, 28.0 * ringPulse }.drawFrame(6.0, glowColor);
			Circle{ *groundAnchor, 18.0 + 3.0 * ringPulse }.drawFrame(2.5, ringColor);
			Line{ arrowCenter.movedBy(0, 12), groundAnchor->movedBy(0, -6) }.draw(2.4, ColorF{ 0.70, 0.94, 1.0, 0.50 * fade });

			const RectF shaftRect{ Arg::center = arrowCenter.movedBy(0, -14), 10, 18 };
			shaftRect.rounded(3).draw(arrowColor);
			shaftRect.rounded(3).drawFrame(1.5, outlineColor);
			Triangle{
				arrowCenter.movedBy(0, 12),
				arrowCenter.movedBy(-13, -2),
				arrowCenter.movedBy(13, -2)
			}.draw(arrowColor);
			Triangle{
				arrowCenter.movedBy(0, 12),
				arrowCenter.movedBy(-13, -2),
				arrowCenter.movedBy(13, -2)
			}.drawFrame(1.4, 0.0, outlineColor);
		}
	}

	namespace OverlayDetail
	{
		void DrawGroundContactOverlays(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
         for (const UnitRenderModel renderModel : GetUnitRenderModels())
			{
                if (const auto groundPoint = resources.GetUnitRenderModel(renderModel).groundContactPoint(frame.GetPreviewRenderPosition(renderModel), BirdDisplayYaw, GetModelScale(state.modelHeightSettings, renderModel)))
				{
					DrawGroundContactOverlay(state.camera, *groundPoint);
				}
			}
		}

		void DrawBattleOverlays(SkyAppState& state, const SkyAppFrameState& frame)
		{
			if ((not frame.isEditorMode) && state.showBlacksmithMenu)
			{
				DrawSelectionIndicator(state.camera, state.mapData.playerBasePosition);
			}

			for (const auto& attackEffect : state.attackEffects)
			{
				DrawAttackEffect(state.camera, attackEffect);
			}

			if (state.moveOrderIndicator)
			{
				DrawMoveOrderIndicator(state.camera, *state.moveOrderIndicator);
			}

			if (IsValidMillIndex(state, state.selectedMillIndex))
			{
				DrawSelectionIndicator(state.camera, state.mapData.placedModels[*state.selectedMillIndex].position);
			}

			for (const size_t selectedIndex : state.selectedSapperIndices)
			{
				if (selectedIndex < state.spawnedSappers.size())
				{
                   DrawSelectedSapperFootprint(state.spawnedSappers[selectedIndex]);
					DrawSelectedSapperAttackRange(state.camera, state.spawnedSappers[selectedIndex]);
					DrawSelectedSapperRing(state.camera, state.spawnedSappers[selectedIndex]);
				}
			}

			if ((state.selectedSapperIndices.size() == 1) && (state.selectedSapperIndices.front() < state.spawnedSappers.size()))
			{
				DrawSelectedSapperIcon(state.camera, state.spawnedSappers[state.selectedSapperIndices.front()]);
			}

			DrawSapperHealthBars(state.camera, state.spawnedSappers, ColorF{ 0.36, 0.92, 0.46, 0.95 });
			DrawSapperHealthBars(state.camera, state.enemySappers, ColorF{ 0.96, 0.28, 0.24, 0.95 });
		}

		void DrawResourceAreaOverlays(SkyAppState& state)
		{
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
				DrawResourceAreaIconBadge(area.type, areaState.ownerTeam, anchor.movedBy(0, -2));
				const RectF backRect{ Arg::center = anchor.movedBy(0, 18), 78, 6 };
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
			}
		}

		void DrawSelectionDragOverlay(const SkyAppState& state, const SkyAppFrameState& frame)
		{
			if ((not frame.isEditorMode) && state.selectionDragStart && MouseL.pressed() && (not frame.isHoveringUI))
			{
				const RectF selectionRect = GetSelectionRect(state.selectionDragStart);
				selectionRect.draw(ColorF{ 0.35, 0.72, 1.0, 0.12 });
				selectionRect.drawFrame(2.0, ColorF{ 0.55, 0.82, 1.0, 0.95 });
			}
		}
	}
}
