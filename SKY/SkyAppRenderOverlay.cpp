# include "SkyAppLoopInternal.hpp"
# include "SkyAppSupport.hpp"
# include "MainUi.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
           [[nodiscard]] ColorF WithAlphaScaled(const ColorF& color, const double alphaScale)
			{
				return ColorF{ color.r, color.g, color.b, Math::Saturate(color.a * alphaScale) };
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

        [[nodiscard]] Optional<Vec2> ProjectToScreen(const AppCamera3D& camera, const Vec3& worldPosition)
		{
			const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(worldPosition.x), static_cast<float>(worldPosition.y), static_cast<float>(worldPosition.z) });

			if (screenPosition.z <= 0.0f)
			{
				return none;
			}

			return Vec2{ screenPosition.x, screenPosition.y };
		}

		void DrawLaserAttackEffect(const AppCamera3D& camera, const AttackEffectInstance& effect)
		{
			const Optional<Vec2> start = ProjectToScreen(camera, effect.startPosition);
			const Optional<Vec2> end = ProjectToScreen(camera, effect.endPosition);

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

			Line{ *start, *end }.draw((effect.thickness * 2.6), WithAlphaScaled(effect.color, (0.14 * fade)));
			Line{ *start, *end }.draw((effect.thickness * 1.4), WithAlphaScaled(effect.color, (0.34 * fade)));
			Line{ *start, *end }.draw(effect.thickness, WithAlphaScaled(ColorF{ 1.0, 1.0, 1.0, 1.0 }, (0.92 * fade)));
			Circle{ *start, (effect.thickness * 0.8) }.draw(WithAlphaScaled(effect.color, (0.55 * fade)));
			Circle{ *end, (effect.thickness * 1.2) }.draw(WithAlphaScaled(effect.color, (0.72 * fade)));
			Circle{ *end, (effect.thickness * 0.55) }.draw(WithAlphaScaled(ColorF{ 1.0, 1.0, 1.0, 1.0 }, (0.95 * fade)));
		}

		void DrawAttackEffect(const AppCamera3D& camera, const AttackEffectInstance& effect)
		{
			switch (effect.type)
			{
			case AttackEffectType::Laser:
				DrawLaserAttackEffect(camera, effect);
				return;

			default:
				return;
			}
		}

		[[nodiscard]] RectF GetSelectionRect(const Optional<Vec2>& selectionDragStart)
		{
			const Vec2 start = *selectionDragStart;
			const Vec2 end = Cursor::PosF();
			return RectF{ Min(start.x, end.x), Min(start.y, end.y), Abs(end.x - start.x), Abs(end.y - start.y) };
		}

       void DrawBaseStatusLabel(const AppCamera3D& camera,
			const Vec3& basePosition,
			const StringView label,
			const double hitPoints,
			const ColorF& fillColor,
			const ColorF& panelColor,
			const StringView secondaryText)
		{
			const Optional<Vec2> screenAnchor = ProjectToScreen(camera, basePosition.movedBy(0, 4.2, 0));

			if (not screenAnchor)
			{
				return;
			}

			const double panelHeight = (secondaryText.isEmpty() ? 58.0 : 78.0);
			const RectF panel{ Arg::center = screenAnchor->movedBy(0, -16), 176, panelHeight };
			panel.draw(panelColor).drawFrame(2, 0, ColorF{ 0.92, 0.96, 1.0, 0.72 });
			SimpleGUI::GetFont()(label).drawAt(panel.center().movedBy(0, secondaryText.isEmpty() ? -16 : -24), Palette::White);

			const RectF backRect{ Arg::center = panel.center().movedBy(0, secondaryText.isEmpty() ? 2 : -2), 140, 8 };
			backRect.draw(ColorF{ 0.05, 0.05, 0.05, 0.88 });
			RectF{ backRect.pos, (backRect.w * Math::Saturate(hitPoints / BaseMaxHitPoints)), backRect.h }.draw(fillColor);
			backRect.drawFrame(1.0, ColorF{ 0.9, 0.95, 1.0, 0.55 });
			SimpleGUI::GetFont()(U"{:.0f} / {:.0f}"_fmt(hitPoints, BaseMaxHitPoints)).drawAt(panel.center().movedBy(0, secondaryText.isEmpty() ? 16 : 14), ColorF{ 0.96, 0.98, 1.0 });

			if (not secondaryText.isEmpty())
			{
				SimpleGUI::GetFont()(secondaryText).drawAt(panel.center().movedBy(0, 32), ColorF{ 1.0, 0.88, 0.88 });
			}
		}
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

		const Rect resourcePanel = frame.panels.resourcePanel;
		resourcePanel.draw(ColorF{ 0.08, 0.10, 0.12, 0.88 }).drawFrame(2, 0, ColorF{ 0.75, 0.82, 0.90, 0.9 });
        SimpleGUI::GetFont()(U"資源").draw(SkyAppUiLayout::ResourcePanelTitlePosition(resourcePanel), Palette::White);
		SimpleGUI::GetFont()(U"予算 {:.0f}"_fmt(state.playerResources.budget)).draw(SkyAppUiLayout::ResourcePanelBudgetPosition(resourcePanel), ColorF{ 0.98, 0.90, 0.38 });
		SimpleGUI::GetFont()(U"火薬 {:.0f}"_fmt(state.playerResources.gunpowder)).draw(SkyAppUiLayout::ResourcePanelGunpowderPosition(resourcePanel), ColorF{ 0.98, 0.56, 0.42 });
		SimpleGUI::GetFont()(U"魔力 {:.0f}"_fmt(state.playerResources.mana)).draw(SkyAppUiLayout::ResourcePanelManaPosition(resourcePanel), ColorF{ 0.58, 0.72, 1.0 });
		const double reinforcementSeconds = Max(0.0, (state.nextEnemyReinforcementAt - Scene::Time()));
		const String reinforcementText = U"増援 {:.1f}s"_fmt(reinforcementSeconds);
		DrawBaseStatusLabel(state.camera,
			state.mapData.playerBasePosition,
			U"自軍拠点",
			state.playerBaseHitPoints,
			ColorF{ 0.36, 0.92, 0.46, 0.95 },
			ColorF{ 0.08, 0.10, 0.12, 0.88 },
			U"");
		DrawBaseStatusLabel(state.camera,
			state.mapData.enemyBasePosition,
			U"敵拠点",
			state.enemyBaseHitPoints,
			ColorF{ 0.96, 0.28, 0.24, 0.95 },
			ColorF{ 0.12, 0.08, 0.08, 0.88 },
			reinforcementText);

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
}
