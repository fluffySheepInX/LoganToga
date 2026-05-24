#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleResourceRenderer.h"
# include "QuarterView.h"
# include "BattleNineSlice.h"

namespace LT3
{
	inline void DrawSelectedUnitSkillRangeOutline(const BattleWorld& world, const DefinitionStores& defs)
	{
		const UnitId selected = GetSelectedUnit(world);
		if (!IsValidUnit(world, selected) || world.units.defId[selected] >= defs.units.size())
		{
			return;
		}

		const UnitDef& def = defs.units[world.units.defId[selected]];
		if (def.skill == InvalidSkillDefId || def.skill >= defs.skills.size())
		{
			return;
		}

		const SkillDef& skill = defs.skills[def.skill];
		const double range = ResolveEffectiveAttackRange(def, skill);
		if (range <= 1.0)
		{
			return;
		}

		const Vec2 center = ToQuarterScreen(world.units.position[selected]);
		const double radiusX = range * QuarterViewScale.x;
		const double radiusY = range * QuarterViewScale.y;
		const ColorF color = (world.units.faction[selected] == Faction::Player)
			? ColorF{ 0.10, 0.95, 1.0, 0.78 }
			: ColorF{ 1.0, 0.45, 0.20, 0.82 };

		Ellipse{ center, radiusX, radiusY }.drawFrame(2.0, color);
	}

	inline void DrawAreaSelectionFrame(const BattleWorld& world, const BattleRenderAssets& assets)
	{
		if (!IsDragSelectionActive(world))
		{
			return;
		}

		const RectF rect = MakeDragSelectionRect(world.selection.areaDragStartScreen, world.selection.areaDragCurrentScreen);
		const FilePath framePath = ResolveSystemImagePath(U"areaWaku.png");
		if (!FileSystem::Exists(framePath))
		{
			rect.drawFrame(2, ColorF{ 0.2, 0.8, 1.0, 0.9 });
			return;
		}

		if (!assets.iconTextureCache.contains(framePath))
		{
			assets.iconTextureCache.emplace(framePath, Texture{ framePath });
		}

		DrawNineSliceTexture(assets.iconTextureCache.at(framePath), rect, 16.0);
	}

	inline void DrawFormationPlacementPreview(const BattleWorld& world, const DefinitionStores& defs)
	{
		if (!world.selection.formationPlacementActive || world.selection.formationUnits.isEmpty())
		{
			return;
		}

		const Vec2 destinationWorld = world.selection.formationDestinationWorld;
		const Vec2 facing = ResolveFormationFacingDirection(world, world.selection.formationUnits, destinationWorld, world.selection.formationCurrentWorld);
		const Array<Vec2> targets = BuildFormationMoveTargets(world, defs, world.selection.formationUnits, destinationWorld, facing);

		for (const Vec2& target : targets)
		{
			const Vec2 screen = ToQuarterViewportScreen(target);
			Circle{ screen, 12.0 }.draw(ColorF{ 0.0, 0.75, 1.0, 0.10 }).drawFrame(2.0, ColorF{ 0.0, 0.85, 1.0, 0.85 });
		}

		const Vec2 destinationScreen = ToQuarterViewportScreen(destinationWorld);
		Vec2 arrowTargetWorld = world.selection.formationCurrentWorld;
		if ((arrowTargetWorld - destinationWorld).lengthSq() < 16.0)
		{
			arrowTargetWorld = destinationWorld + (facing * 120.0);
		}
		const Vec2 arrowTargetScreen = ToQuarterViewportScreen(arrowTargetWorld);

		Circle{ destinationScreen, 7.0 }.draw(ColorF{ 1.0, 0.84, 0.0, 0.90 });
		Line{ destinationScreen, arrowTargetScreen }.drawArrow(4.0, Vec2{ 18.0, 18.0 }, ColorF{ 1.0, 0.84, 0.0, 0.95 });
	}

	inline void DrawBuildActionPlacementPreview(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
	{
		if (!world.selection.actionPlacementActive || world.selection.actionId >= defs.buildActions.size())
		{
			return;
		}

		const auto buildPreviewTile = [](const Vec2& targetWorld)
		{
			const Vec2 bottomCenter = ToQuarterViewportScreen(targetWorld);
			const double scale = GetQuarterViewCameraScale();
			const Vec2 tileOffset = QuarterTileOffset * scale;
			const double tileThickness = QuarterTileThickness * scale;
			return Quad{
				bottomCenter.movedBy(0, -(tileThickness + tileOffset.y * 2.0)),
				bottomCenter.movedBy(tileOffset.x, -(tileThickness + tileOffset.y)),
				bottomCenter.movedBy(0, -tileThickness),
				bottomCenter.movedBy(-tileOffset.x, -(tileThickness + tileOffset.y))
			};
		};
		const Vec2 previewCenterOffset{ 0, -(QuarterTileOffset.y + QuarterTileThickness) * GetQuarterViewCameraScale() };
		const BuildActionDef& action = defs.buildActions[world.selection.actionId];
		if (action.placementMode == BuildPlacementMode::Line && action.useRightDragPlacement)
		{
			Array<Vec2> targets = world.selection.actionLineTargets;
			if (targets.isEmpty())
			{
				targets << world.selection.actionTargetWorld;
			}

			for (const Vec2& target : targets)
			{
				const Quad tile = buildPreviewTile(target);
				const BuildPlacementCellState state = EvaluateBuildPlacementCell(world, defs, target);
				if (state == BuildPlacementCellState::Allowed)
				{
					tile.draw(ColorF{ 0.0, 0.75, 1.0, 0.14 }).drawFrame(2.0, ColorF{ 0.0, 0.85, 1.0, 0.90 });
				}
				else
				{
					tile.draw(ColorF{ 1.0, 0.56, 0.10, 0.18 }).drawFrame(2.0, ColorF{ 1.0, 0.64, 0.18, 0.95 });
				}
			}

			if (targets.size() >= 2)
			{
				Line{ ToQuarterViewportScreen(targets.front()) + previewCenterOffset, ToQuarterViewportScreen(targets.back()) + previewCenterOffset }.draw(5.0, ColorF{ 0.0, 0.85, 1.0, 0.45 });
			}

			const Vec2 labelScreen = ToQuarterViewportScreen(targets.back()) + previewCenterOffset + Vec2{ 0, 34 };
			uiFont(U"右ドラッグ範囲: {} x{}"_fmt(action.name, targets.size())).drawAt(14, labelScreen, Palette::Gold);
			return;
		}

		const Vec2 screen = ToQuarterViewportScreen(world.selection.actionTargetWorld) + previewCenterOffset;
		buildPreviewTile(world.selection.actionTargetWorld).draw(ColorF{ 0.0, 0.75, 1.0, 0.12 }).drawFrame(3.0, ColorF{ 0.0, 0.85, 1.0, 0.90 });
		if (!DrawBuildActionIcon(action, defs, assets, screen.movedBy(0, -28), 42.0))
		{
			uiFont(action.name).drawAt(12, screen.movedBy(0, -30), Palette::White);
		}
		uiFont(U"配置: {}"_fmt(action.name)).drawAt(14, screen.movedBy(0, 28), Palette::Gold);
	}

	inline void DrawPlacedBattleObjects(const BattleWorld& world, const Font& uiFont, const BattleRenderAssets& assets)
	{
		for (size_t i = 0; i < world.placedObjects.position.size(); ++i)
		{
			const Vec2 screen = ToQuarterScreen(world.placedObjects.position[i]);
			const String& iconName = world.placedObjects.icon[i];
			const FilePath iconPath = iconName.isEmpty() ? FilePath{} : ResolveBuildIconPath(iconName);
			if (!iconPath.isEmpty() && FileSystem::Exists(iconPath))
			{
				if (!assets.iconTextureCache.contains(iconPath))
				{
					assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
				}
				assets.iconTextureCache.at(iconPath).resized(54.0, 54.0).drawAt(screen.movedBy(0, -24));
			}
			else
			{
				RectF{ Arg::center = screen.movedBy(0, -18), 42.0, 42.0 }.draw(ColorF{ 0.26, 0.26, 0.30, 0.88 }).drawFrame(2.0, ColorF{ 1, 1, 1, 0.18 });
				uiFont(world.placedObjects.tag[i].isEmpty() ? U"obj" : world.placedObjects.tag[i]).drawAt(10, screen.movedBy(0, -18), Palette::White);
			}
		}
	}
}
