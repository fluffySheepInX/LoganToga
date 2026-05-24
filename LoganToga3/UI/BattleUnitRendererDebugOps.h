#pragma once
# include <Siv3D.hpp>
# include "BattleUnitRendererAssetOps.h"
# include "../Systems/SelectionSystem.h"

namespace LT3
{
	inline void DrawSelectionDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
	{
		const RectF panel{ 24, 256, 640, 136 };
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Selection Debug").draw(44, 272, Palette::White);

		String selectionText = U"Selected unit: id=n/a unit_id=n/a";
		String visualText = U"Visual: kind=n/a image=n/a exists=n/a";
		String pathText = U"Path: n/a";
		const UnitId selected = GetSelectedUnit(world);
		if (selected != InvalidUnitId)
		{
			const UnitDef& def = defs.units[world.units.defId[selected]];
			selectionText = U"Selected unit: id={} unit_id={} role={}"_fmt(selected, def.unit_id, static_cast<int32>(def.role));

			const UnitVisualInfo visualInfo = FindUnitVisualInfoByTag(assets, def.unit_id);
			const FilePath resolvedPath = ResolveCatalogVisualPath(visualInfo.kind, visualInfo.image);

			const String visualKind = visualInfo.kind.isEmpty() ? U"<empty>" : visualInfo.kind;
			const String visualImage = visualInfo.image.isEmpty() ? U"<empty>" : visualInfo.image;
			const String visualExists = (!resolvedPath.isEmpty() && FileSystem::Exists(resolvedPath)) ? U"yes" : U"no";
			visualText = U"Visual: kind={} image={} exists={}"_fmt(visualKind, visualImage, visualExists);
			pathText = U"Path: {}"_fmt(resolvedPath.isEmpty() ? U"<empty>" : resolvedPath);
		}

		uiFont(selectionText).draw(13, 44, 296, Palette::Lightgray);
		uiFont(visualText).draw(13, 44, 316, Palette::Skyblue);
		uiFont(pathText).draw(11, 44, 336, Palette::Lightgray);
		uiFont(U"Map: {} x {}"_fmt(world.mapWidth, world.mapHeight)).draw(11, 44, 358, ColorF{ 0.76, 0.80, 0.88 });
	}
}
