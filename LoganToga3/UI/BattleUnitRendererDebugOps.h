#pragma once
# include <Siv3D.hpp>
# include "BattleUnitRendererAssetOps.h"
# include "../Systems/SelectionSystem.h"

namespace LT3
{
	struct BattleDebugCursorState
	{
		Vec2 logicalScreen{ 0, 0 };
		Vec2 world{ 0, 0 };
		Vec2 preCameraScreen{ 0, 0 };
	};

	inline BattleDebugCursorState MakeBattleDebugCursorState(const Vec2& logicalScreen)
	{
		return BattleDebugCursorState{
			logicalScreen,
			ToQuarterWorld(logicalScreen),
			ToQuarterPreCameraScreen(logicalScreen),
		};
	}

	inline bool IsBuildingUnitForClickDebug(const UnitDef& def)
	{
		return (def.role == UnitRole::Base)
			|| !def.building_category.isEmpty();
	}

	inline void DrawBuildingUnitClickDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const BattleDebugCursorState& cursor)
	{
		for (int32 i = static_cast<int32>(world.units.size()) - 1; i >= 0; --i)
		{
			const UnitId unit = static_cast<UnitId>(i);
			if (!IsValidUnit(world, unit))
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (!IsBuildingUnitForClickDebug(def))
			{
				continue;
			}

			const double pickRadius = UnitSelectionRadius(def) + 6.0;
			const bool hovered = Circle{ world.units.position[unit], pickRadius }.intersects(cursor.world);
			const Vec2 screenPos = ToQuarterScreen(world.units.position[unit]);
			const ColorF fillColor = hovered ? ColorF{ 1.0, 0.80, 0.20, 0.20 } : ColorF{ 0.20, 0.85, 1.0, 0.10 };
			const ColorF frameColor = hovered ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 0.30, 0.90, 1.0, 0.55 };

			Circle{ screenPos, pickRadius }.draw(fillColor).drawFrame(1.5, frameColor);
		}
	}

	inline void DrawSelectionDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
	{
		const RectF panel{ 24, 256, 640, 160 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Selection Debug").draw(44, 272, Palette::White);

		String selectionText = U"Selected unit: id=n/a unit_id=n/a";
		String cellText = U"Cell: n/a";
		String visualText = U"Visual: kind=n/a image=n/a exists=n/a";
		String pathText = U"Path: n/a";
		const UnitId selected = GetSelectedUnit(world);
		if (selected != InvalidUnitId)
		{
			const UnitDef& def = defs.units[world.units.defId[selected]];
			selectionText = U"Selected unit: id={} unit_id={} role={}"_fmt(selected, def.unit_id, static_cast<int32>(def.role));
			const Point cell = QuarterWorldToBattleCell(world.units.position[selected], world.mapWidth, world.mapHeight);
			cellText = U"Cell: x={} y={} world=({:.1f}, {:.1f})"_fmt(cell.x, cell.y, world.units.position[selected].x, world.units.position[selected].y);

			const UnitVisualInfo visualInfo = FindUnitVisualInfoByTag(assets, def.unit_id);
			const FilePath resolvedPath = ResolveCatalogVisualPath(visualInfo.kind, visualInfo.image);

			const String visualKind = visualInfo.kind.isEmpty() ? U"<empty>" : visualInfo.kind;
			const String visualImage = visualInfo.image.isEmpty() ? U"<empty>" : visualInfo.image;
			const String visualExists = (!resolvedPath.isEmpty() && FileSystem::Exists(resolvedPath)) ? U"yes" : U"no";
			visualText = U"Visual: kind={} image={} exists={}"_fmt(visualKind, visualImage, visualExists);
			pathText = U"Path: {}"_fmt(resolvedPath.isEmpty() ? U"<empty>" : resolvedPath);
		}

		uiFont(selectionText).draw(13, 44, 296, Palette::Lightgray);
		uiFont(cellText).draw(13, 44, 316, Palette::Aqua);
		uiFont(visualText).draw(13, 44, 336, Palette::Skyblue);
		uiFont(pathText).draw(11, 44, 356, Palette::Lightgray);
		uiFont(U"Map: {} x {}"_fmt(world.mapWidth, world.mapHeight)).draw(11, 44, 378, ColorF{ 0.76, 0.80, 0.88 });
	}

	inline void DrawResourceNodeClickDebugOverlay(const BattleWorld& world, const BattleDebugCursorState& cursor)
	{
		const bool hasSelectedUnits = !GetSelectedUnits(world).isEmpty();
		const double screenRadius = ResourceNodeHoverScreenRadius(hasSelectedUnits);
		const double preCameraRadius = ResourceNodeHoverPreCameraRadius(screenRadius);

		for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
		{
			if (world.resourceNodes.amount[node] <= 0)
			{
				continue;
			}

			const Vec2 hoverCenter = ResourceNodeHoverPreCameraCenter(world.resourceNodes.position[node]);
			const bool hovered = Circle{ hoverCenter, preCameraRadius }.intersects(cursor.preCameraScreen);
			const Vec2 screenCenter = QuarterTileFaceCenterScreen(world.resourceNodes.position[node]);
			const ColorF fillColor = hovered ? ColorF{ 1.0, 0.82, 0.24, 0.14 } : ColorF{ 0.35, 0.90, 1.0, 0.08 };
			const ColorF frameColor = hovered ? ColorF{ 1.0, 0.90, 0.25, 0.95 } : ColorF{ 0.35, 0.90, 1.0, 0.55 };

			Circle{ screenCenter, screenRadius }.draw(fillColor).drawFrame(2.0, frameColor);
		}
	}
}
