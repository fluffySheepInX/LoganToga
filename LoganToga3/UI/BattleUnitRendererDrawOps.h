#pragma once
# include <Siv3D.hpp>
# include "BattleUnitRendererAssetOps.h"
# include "../Systems/SelectionSystem.h"

namespace LT3
{
	inline void DrawHealthBar(const Vec2& pos, double radius, int32 hp, int32 maxHp)
	{
		const RectF back{ Arg::center = pos + Vec2{ 0, -radius - 12 }, radius * 2.2, 5 };
		back.draw(ColorF{ 0.08, 0.08, 0.08, 0.75 });
		const double rate = maxHp > 0 ? Clamp(static_cast<double>(hp) / maxHp, 0.0, 1.0) : 0.0;
		ColorF barColor{ 1.0, 0.25, 0.20 };
		if (rate > 0.35)
		{
			barColor = ColorF{ 0.20, 0.80, 0.20 };
		}
		RectF{ back.pos, back.w * rate, back.h }.draw(barColor);
	}

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

	inline ColorF GetUnitOutlineColor(const BattleWorld& world, UnitId unit)
	{
		if (world.units.faction[unit] == Faction::Player)
		{
			return ColorF{ 0.0, 1.0, 1.0 };
		}

		return ColorF{ 1.0, 0.15, 0.10 };
	}

	inline void DrawUnitMovePath(const BattleWorld& world, UnitId unit, const ColorF& outline)
	{
		if (world.units.task[unit] != UnitTask::Moving)
		{
			return;
		}

		const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
		const Vec2 targetPos = ToQuarterScreen(world.units.targetPosition[unit]);
		Line{ pos, targetPos }.draw(1.5, ColorF{ outline, 0.35 });
		Circle{ targetPos, 5 }.drawFrame(1.5, ColorF{ outline, 0.45 });
	}

	inline void DrawProjectiles(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (size_t i = 0; i < world.projectiles.position.size(); ++i)
		{
			const SkillDef& skill = defs.skills[world.projectiles.skill[i]];
			Circle{ ToQuarterScreen(world.projectiles.position[i]), 4 }.draw(skill.color);
		}
	}

	inline void DrawUnitShape(const UnitDef& def, const Vec2& pos, const ColorF& outline)
	{
		Circle{ pos, def.radius }.draw(def.color);

		if (def.role == UnitRole::Base)
		{
			RectF{ Arg::center = pos, def.radius * 1.45, def.radius * 1.45 }.draw(def.color);
			RectF{ Arg::center = pos, def.radius * 1.45, def.radius * 1.45 }.drawFrame(2, outline);
		}
	}

	inline void DrawUnitShape(const UnitDef& def, const Vec2& pos, bool selected, const ColorF& outline)
	{
		Ellipse{ pos + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
		Circle{ pos, def.radius + (selected ? 6.0 : 2.0) }.drawFrame(selected ? 4.0 : 2.0, outline);
		DrawUnitShape(def, pos, outline);
	}

	inline void DrawUnitVisual(const UnitDef& def, const Vec2& pos, bool selected, bool isMoving, const ColorF& outline, const BattleRenderAssets* assets, StringView iconOverride = U"")
	{
		const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
		Vec2 shadowCenter = pos;
		if (assets)
		{
			const UnitVisualInfo visual = FindUnitVisualInfoByTag(*assets, def.unit_id);
			if (!visual.image.isEmpty())
			{
				const Vec2 shadowOffset = Vec2{ static_cast<double>(visual.shadowOffset.x), static_cast<double>(visual.shadowOffset.y) } * visual.visualScale;
				shadowCenter += shadowOffset;
			}
		}

		Ellipse{ shadowCenter + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
		if (!isStaticStructure)
		{
			Circle{ pos, def.radius + (selected ? 6.0 : 2.0) }.drawFrame(selected ? 4.0 : 2.0, outline);
		}

		Vec2 iconOffset{ 0, 0 };
		if (!iconOverride.isEmpty() && assets)
		{
			const UnitVisualInfo visual = FindUnitVisualInfoByTag(*assets, def.unit_id);
			const String overrideName = String{ iconOverride };
			if (!visual.lineIconHorizontalName.isEmpty() && overrideName == visual.lineIconHorizontalName)
			{
				iconOffset = Vec2{ static_cast<double>(visual.lineIconHorizontalOffset.x), static_cast<double>(visual.lineIconHorizontalOffset.y) };
			}
			else if (!visual.lineIconDiagUpRightName.isEmpty() && overrideName == visual.lineIconDiagUpRightName)
			{
				iconOffset = Vec2{ static_cast<double>(visual.lineIconDiagUpRightOffset.x), static_cast<double>(visual.lineIconDiagUpRightOffset.y) };
			}
			else if (!visual.lineIconDiagUpLeftName.isEmpty() && overrideName == visual.lineIconDiagUpLeftName)
			{
				iconOffset = Vec2{ static_cast<double>(visual.lineIconDiagUpLeftOffset.x), static_cast<double>(visual.lineIconDiagUpLeftOffset.y) };
			}
		}

		if (!(assets && DrawUnitTexture(*assets, def, pos, isMoving, iconOverride, iconOffset)))
		{
			DrawUnitShape(def, pos, outline);
		}
	}

	inline void DrawUnits(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const BattleRenderAssets* assets = nullptr, const Array<bool>* visibleMask = nullptr, int32 maskWidth = 0, int32 maskHeight = 0)
	{
		const Vec2 mouse = Cursor::PosF();
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!world.units.alive[unit]) continue;

			if (visibleMask && world.units.faction[unit] == Faction::Enemy)
			{
				const Point unitCell = QuarterWorldToBattleCell(world.units.position[unit], maskWidth, maskHeight);
				const size_t maskIndex = static_cast<size_t>(unitCell.y * maskWidth + unitCell.x);
				if (maskIndex >= visibleMask->size() || !(*visibleMask)[maskIndex])
				{
					continue;
				}
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
			const bool selected = IsUnitSelected(world, unit);
			const bool isMoving = (world.units.task[unit] == UnitTask::Moving);
			const ColorF outline = GetUnitOutlineColor(world, unit);

			DrawUnitMovePath(world, unit, outline);
			const String iconOverride = (unit < world.units.iconOverride.size()) ? world.units.iconOverride[unit] : U"";
			DrawUnitVisual(def, pos, selected, isMoving, outline, assets, iconOverride);

			const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
			const bool hovered = (mouse.distanceFrom(pos) <= (def.radius + 16.0));
			if (!isStaticStructure || hovered)
			{
				DrawHealthBar(pos, def.radius, world.units.hp[unit], def.hp);
			}
		}
	}
}
