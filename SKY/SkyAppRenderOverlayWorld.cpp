# include "SkyAppRenderOverlayInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
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
	}

	namespace OverlayDetail
	{
		void DrawGroundContactOverlays(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
              if (const auto birdGroundPoint = resources.birdModel.groundContactPoint(frame.birdRenderPosition, BirdDisplayYaw, GetModelScale(state.modelHeightSettings, ModelHeightTarget::Bird)))
			{
				DrawGroundContactOverlay(state.camera, *birdGroundPoint);
			}

              if (const auto ashigaruGroundPoint = resources.ashigaruModel.groundContactPoint(frame.ashigaruRenderPosition, BirdDisplayYaw, GetModelScale(state.modelHeightSettings, ModelHeightTarget::Ashigaru)))
			{
				DrawGroundContactOverlay(state.camera, *ashigaruGroundPoint);
			}

				if (const auto sugoiCarGroundPoint = resources.sugoiCarModel.groundContactPoint(frame.sugoiCarRenderPosition, BirdDisplayYaw, GetModelScale(state.modelHeightSettings, ModelHeightTarget::SugoiCar)))
				{
					DrawGroundContactOverlay(state.camera, *sugoiCarGroundPoint);
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

			if (IsValidMillIndex(state, state.selectedMillIndex))
			{
				DrawSelectionIndicator(state.camera, state.mapData.placedModels[*state.selectedMillIndex].position);
			}

			for (const size_t selectedIndex : state.selectedSapperIndices)
			{
				if (selectedIndex < state.spawnedSappers.size())
				{
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
