#pragma once
# include <Siv3D.hpp>
# include "BattleUnitRendererAssetOps.h"
# include "../Systems/SelectionSystem.h"

namespace LT3
{
	inline void DrawHealthBar(const Vec2& pos, double radius, int32 hp, int32 maxHp, bool selected = false)
	{
		const Vec2 barCenter = pos + Vec2{ 0, -radius - (selected ? 14.0 : 12.0) };
		const RectF back{ Arg::center = barCenter, radius * (selected ? 2.6 : 2.2), selected ? 8.0 : 5.0 };
		back.draw(ColorF{ 0.08, 0.08, 0.08, 0.75 });
		const double rate = maxHp > 0 ? Clamp(static_cast<double>(hp) / maxHp, 0.0, 1.0) : 0.0;
		ColorF barColor{ 1.0, 0.25, 0.20 };
		if (rate > 0.35)
		{
			barColor = ColorF{ 0.20, 0.80, 0.20 };
		}
		RectF{ back.pos, back.w * rate, back.h }.draw(barColor);
		back.drawFrame(selected ? 2.0 : 1.0, selected ? ColorF{ 1.0, 0.92, 0.48, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.14 });
		if (selected)
		{
			back.stretched(2.0).drawFrame(1.5, ColorF{ 1.0, 1.0, 1.0, 0.70 });
		}
	}

	inline ColorF GetUnitOutlineColor(const BattleWorld& world, UnitId unit)
	{
		if (world.units.faction[unit] == Faction::Player)
		{
			return ColorF{ 0.0, 1.0, 1.0 };
		}

		return ColorF{ 1.0, 0.15, 0.10 };
	}

	inline void DrawUnitMovePath(const BattleWorld& world, UnitId unit, const ColorF& outline, bool showEnemyMoveMarkers)
	{
		if (world.units.task[unit] != UnitTask::Moving)
		{
			return;
		}
		if (world.units.faction[unit] == Faction::Enemy && !showEnemyMoveMarkers)
		{
			return;
		}

		const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
		const Vec2 targetPos = ToQuarterScreen(world.units.targetPosition[unit]);
		Line{ pos, targetPos }.draw(1.5, ColorF{ outline, 0.35 });
		Circle{ targetPos, 5 }.drawFrame(1.5, ColorF{ outline, 0.45 });
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

	inline String UniqueUnitPopupText(const UnitDef& def)
	{
		return def.name.isEmpty() ? def.unit_id : def.name;
	}

	inline const Texture* ResolveUniquePortraitTexture(const BattleRenderAssets& assets, const UnitVisualInfo& visual)
	{
		if (visual.portraitImage.isEmpty())
		{
			return nullptr;
		}

		const FilePath portraitPath = ResolveUnitPortraitPath(visual.portraitImage);
		if (portraitPath.isEmpty() || !FileSystem::Exists(portraitPath))
		{
			return nullptr;
		}

		if (!assets.portraitTextureCache.contains(portraitPath))
		{
			assets.portraitTextureCache.emplace(portraitPath, Texture{ portraitPath });
		}
		return &assets.portraitTextureCache.at(portraitPath);
	}

	inline bool ShouldShowUniqueUnitPopup(const UnitVisualInfo& visual, double timeSec, UnitId unit)
	{
		const double intervalSec = Max(0.5, visual.uniqueSpeechIntervalSec);
		const double visibleSec = Min(Max(0.2, visual.uniqueSpeechVisibleSec), intervalSec);
		const double phase = Fmod(timeSec + static_cast<double>((unit * 17) % 23) * 0.19, intervalSec);
		return phase < visibleSec;
	}

	inline void DrawUniqueUnitPopup(const BattleRenderAssets& assets, const UnitDef& def, UnitId unit, const Vec2& unitPos, double timeSec, const Font& uiFont)
	{
		const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, def.unit_id);
		if (!visual.unique || !ShouldShowUniqueUnitPopup(visual, timeSec, unit))
		{
			return;
		}

		const Texture* portrait = ResolveUniquePortraitTexture(assets, visual);
		const String speech = visual.uniqueSpeechLines.isEmpty() ? UniqueUnitPopupText(def) : visual.uniqueSpeechLines[static_cast<size_t>((static_cast<int32>(timeSec / Max(0.5, visual.uniqueSpeechIntervalSec)) + unit) % static_cast<int32>(visual.uniqueSpeechLines.size()))];
		const Vec2 popupCenter = unitPos + Vec2{ 0.0, -def.radius - 76.0 };
		const double portraitSize = portrait ? 44.0 : 0.0;
		const double textWidth = Min(visual.uniqueSpeechBubbleWidth, Max(72.0, uiFont(speech).region(12).w + 18.0));
		const double bubbleWidth = portraitSize + textWidth + (portrait ? 18.0 : 0.0);
		RectF bubble{ Arg::center = popupCenter, bubbleWidth, visual.uniqueSpeechBubbleHeight };
		const Vec2 screenTopLeft = ToQuarterPreCameraScreen(Vec2{ 0.0, 0.0 });
		const Vec2 screenBottomRight = ToQuarterPreCameraScreen(Vec2{ static_cast<double>(Scene::Width()), static_cast<double>(Scene::Height()) });
		const double visibleLeft = Min(screenTopLeft.x, screenBottomRight.x) + 4.0;
		const double visibleTop = Min(screenTopLeft.y, screenBottomRight.y) + 4.0;
		const double visibleRight = Max(screenTopLeft.x, screenBottomRight.x) - 4.0;
		const double visibleBottom = Max(screenTopLeft.y, screenBottomRight.y) - 4.0;
		bubble.x = Clamp(bubble.x, visibleLeft, Max(visibleLeft, visibleRight - bubble.w));
		bubble.y = Clamp(bubble.y, visibleTop, Max(visibleTop, visibleBottom - bubble.h));

		bubble.rounded(8).draw(ColorF{ 0.02, 0.03, 0.045, 0.88 }).drawFrame(1.5, ColorF{ 1.0, 0.84, 0.0, 0.75 });
		Triangle{ bubble.bottomCenter() + Vec2{ -8.0, 0.0 }, bubble.bottomCenter() + Vec2{ 8.0, 0.0 }, unitPos + Vec2{ 0.0, -def.radius - 8.0 } }.draw(ColorF{ 0.02, 0.03, 0.045, 0.88 });

		double textX = bubble.x + 10.0;
		if (portrait)
		{
			const RectF portraitRect{ bubble.x + 7.0, bubble.y + 5.0, portraitSize, portraitSize };
			portraitRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
			const double fitScale = Min((portraitRect.w - 4.0) / Max(1.0, static_cast<double>(portrait->width())), (portraitRect.h - 4.0) / Max(1.0, static_cast<double>(portrait->height())));
			portrait->scaled(fitScale).drawAt(portraitRect.center());
			textX = portraitRect.x + portraitRect.w + 9.0;
		}

		uiFont(speech).draw(12, textX, bubble.y + 12.0, Palette::White);
		uiFont(U"...").draw(10, textX, bubble.y + 32.0, Palette::Aqua);
	}

	inline void DrawUnitShape(const UnitDef& def, const Vec2& pos, bool selected, const ColorF& outline)
	{
		Ellipse{ pos + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
		Circle{ pos, def.radius + 2.0 }.drawFrame(2.0, outline);
		DrawUnitShape(def, pos, outline);
	}

	inline void DrawUnitVisual(const UnitDef& def, const Vec2& pos, bool selected, bool isMoving, const ColorF& outline, const BattleRenderAssets* assets, StringView iconOverride = U"")
	{
		const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
		Vec2 shadowCenter = pos;
		double shadowScale = 1.0;
		double shadowOpacity = 0.28;
		if (assets)
		{
			const UnitVisualInfo visual = FindUnitVisualInfoByTag(*assets, def.unit_id);
			shadowScale = Max(0.1, visual.shadowScale);
			shadowOpacity = Clamp(visual.shadowOpacity, 0.0, 1.0);
			if (!visual.image.isEmpty())
			{
				const Vec2 shadowOffset = Vec2{ static_cast<double>(visual.shadowOffset.x), static_cast<double>(visual.shadowOffset.y) } * visual.visualScale;
				shadowCenter += shadowOffset;
			}
		}

		Ellipse{ shadowCenter + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15 * shadowScale, def.radius * 0.45 * shadowScale }.draw(ColorF{ 0, 0, 0, shadowOpacity });
		if (!isStaticStructure)
		{
			Circle{ pos, def.radius + 2.0 }.drawFrame(2.0, outline);
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

	inline bool IsUnitVisibleForRender(const BattleWorld& world, UnitId unit, const Array<bool>* visibleMask = nullptr, int32 maskWidth = 0, int32 maskHeight = 0)
	{
		if (!world.units.alive[unit])
		{
			return false;
		}

		if (visibleMask && world.units.faction[unit] == Faction::Enemy)
		{
			const Point unitCell = QuarterWorldToBattleCell(world.units.position[unit], maskWidth, maskHeight);
			const size_t maskIndex = static_cast<size_t>(unitCell.y * maskWidth + unitCell.x);
			if (maskIndex >= visibleMask->size() || !(*visibleMask)[maskIndex])
			{
				return false;
			}
		}

		return true;
	}

	inline void DrawUnitRenderContent(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, const Font& uiFont, const BattleRenderAssets* assets = nullptr, bool showEnemyMoveMarkers = false)
	{
		const UnitDef& def = defs.units[world.units.defId[unit]];
		const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
		const bool selected = IsUnitSelected(world, unit);
		const bool isMoving = (world.units.task[unit] == UnitTask::Moving);
		const ColorF outline = GetUnitOutlineColor(world, unit);

		DrawUnitMovePath(world, unit, outline, showEnemyMoveMarkers);
		const String iconOverride = (unit < world.units.iconOverride.size()) ? world.units.iconOverride[unit] : U"";
		DrawUnitVisual(def, pos, selected, isMoving, outline, assets, iconOverride);

		if (assets)
		{
			DrawUniqueUnitPopup(*assets, def, unit, pos, world.elapsedSec, uiFont);
		}
	}

	inline void DrawUnitHealthBarOverlay(const BattleWorld& world, const DefinitionStores& defs, UnitId unit)
	{
		const Vec2 mouse = Cursor::PosF();
		const UnitDef& def = defs.units[world.units.defId[unit]];
		const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
		const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
		const bool hovered = (mouse.distanceFrom(pos) <= (def.radius + 16.0));
		const bool selected = IsUnitSelected(world, unit);
		if (!isStaticStructure || hovered || selected)
		{
			DrawHealthBar(pos, def.radius, world.units.hp[unit], def.hp, selected);
		}
		if (unit < world.cooldowns.skillCastFailureDisplayLeftSec.size()
			&& world.cooldowns.skillCastFailureDisplayLeftSec[unit] > 0.0)
		{
			static const Font failureFont{ 11 };
			const Vec2 labelCenter = pos + Vec2{ 0.0, -def.radius - (selected ? 30.0 : 24.0) };
			const RectF labelRect{ Arg::center = labelCenter, 72.0, 16.0 };
			labelRect.rounded(3.0).draw(ColorF{ 0.0, 0.0, 0.0, 0.82 }).drawFrame(1.0, ColorF{ 0.85, 0.0, 0.0, 0.92 });
			failureFont(U"failure!").drawAt(labelRect.center(), ColorF{ 1.0, 0.18, 0.18, 0.98 });
		}
	}

	inline void DrawUnitHealthBarsOverlay(const BattleWorld& world, const DefinitionStores& defs, const Array<bool>* visibleMask = nullptr, int32 maskWidth = 0, int32 maskHeight = 0)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsUnitVisibleForRender(world, unit, visibleMask, maskWidth, maskHeight))
			{
				continue;
			}

			DrawUnitHealthBarOverlay(world, defs, unit);
		}
	}

	inline void DrawUnits(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const BattleRenderAssets* assets = nullptr, const Array<bool>* visibleMask = nullptr, int32 maskWidth = 0, int32 maskHeight = 0, bool showEnemyMoveMarkers = false)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsUnitVisibleForRender(world, unit, visibleMask, maskWidth, maskHeight))
			{
				continue;
			}

			DrawUnitRenderContent(world, defs, unit, uiFont, assets, showEnemyMoveMarkers);
		}

		static_cast<void>(uiFont);
	}
}
