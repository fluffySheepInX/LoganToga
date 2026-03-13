#include "BattleRendererHudInternal.h"

namespace BattleRendererHudInternal
{
	Optional<QueueDisplayTarget> FindQueueDisplayTarget(const BattleState& state)
	{
		for (const auto& unit : state.units)
		{
			if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && IsBuildingArchetype(unit.archetype)))
			{
				continue;
			}

			if (const auto* building = state.findBuildingByUnitId(unit.id))
			{
				return QueueDisplayTarget{ building, &unit };
			}
		}

		for (const auto& unit : state.units)
		{
			if (!(unit.isAlive && (unit.owner == Owner::Player) && IsBuildingArchetype(unit.archetype)))
			{
				continue;
			}

			const auto* building = state.findBuildingByUnitId(unit.id);
			if (!(building && (!building->isConstructed || !building->productionQueue.isEmpty())))
			{
				continue;
			}

			return QueueDisplayTarget{ building, &unit };
		}

		return none;
	}

	namespace
	{
		void DrawQueueIcon(const RectF& iconRect, const UnitArchetype archetype, const GameData& gameData, const bool compact = false, const double alpha = 1.0)
		{
			const ColorF baseColor = GetCommandIconColor(archetype);
			const ColorF iconColor{ baseColor.r, baseColor.g, baseColor.b, alpha };
			const ColorF backgroundColor{ 0.08, 0.10, 0.14, 0.96 * alpha };
			const RectF frameRect{ iconRect.x, iconRect.y, iconRect.w, iconRect.h };
			frameRect.draw(backgroundColor);
			frameRect.drawFrame(2.0, iconColor);

			const double circleRadius = compact ? 12.0 : 18.0;
			Circle{ frameRect.center(), circleRadius }.draw(iconColor);
			if (compact)
			{
				gameData.smallFont(GetCommandIconGlyph(archetype)).drawAt(frameRect.center(), Palette::White);
			}
			else
			{
				gameData.uiFont(GetCommandIconGlyph(archetype)).drawAt(frameRect.center().movedBy(0.0, -1.0), Palette::White);
			}
		}
	}

	void DrawQueuePanel(const RectF& panelRect, const QueueDisplayTarget& target, const GameData& gameData)
	{
		const RectF contentRect{ panelRect.x + 14.0, panelRect.y + 34.0, panelRect.w - 28.0, panelRect.h - 48.0 };
		const RectF primaryIconRect{ contentRect.x, contentRect.y, 58.0, 58.0 };
		const RectF detailRect{ contentRect.x + 70.0, contentRect.y, contentRect.w - 70.0, 88.0 };
		const bool isConstructing = !target.building->isConstructed;
		const bool hasQueuedUnit = !target.building->productionQueue.isEmpty();
		const UnitArchetype primaryArchetype = hasQueuedUnit ? target.building->productionQueue.front().archetype : target.unit->archetype;

		panelRect.draw(ColorF{ 0.02, 0.04, 0.08, 0.82 });
		panelRect.drawFrame(2.0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
		gameData.uiFont(U"QUEUE").draw(panelRect.x + 14.0, panelRect.y + 6.0, Palette::White);
		gameData.smallFont(GetArchetypeLabel(target.unit->archetype)).draw(panelRect.x + 112.0, panelRect.y + 11.0, ColorF{ 0.76, 0.82, 0.92 });

		DrawQueueIcon(primaryIconRect, primaryArchetype, gameData);

		String titleText = U"Idle";
		String subtitleText = U"No queued units";
		double progress = 0.0;
		String progressText;
		ColorF progressColor{ 0.30, 0.72, 0.98 };

		if (isConstructing)
		{
			titleText = U"Constructing";
			subtitleText = GetArchetypeLabel(target.unit->archetype);
			if (target.building->constructionTotal > 0.0)
			{
				progress = Clamp(1.0 - (target.building->constructionRemaining / target.building->constructionTotal), 0.0, 1.0);
			}
			progressText = s3d::Format(target.building->constructionRemaining, U"s");
			progressColor = ColorF{ 0.84, 0.62, 0.24 };
		}
		else if (hasQueuedUnit)
		{
			const auto& currentItem = target.building->productionQueue.front();
			titleText = GetArchetypeLabel(currentItem.archetype);
			subtitleText = s3d::Format(U"@ ", GetArchetypeLabel(target.unit->archetype), U" / ", target.building->productionQueue.size(), U" in queue");
			if (currentItem.totalTime > 0.0)
			{
				progress = Clamp(1.0 - (currentItem.remainingTime / currentItem.totalTime), 0.0, 1.0);
			}
			progressText = s3d::Format(currentItem.remainingTime, U"s");
		}
		else
		{
			subtitleText = s3d::Format(U"Select ", GetArchetypeLabel(target.unit->archetype), U" and queue a unit");
		}

		gameData.smallFont(titleText).draw(detailRect.x, detailRect.y + 4.0, Palette::White);
		gameData.smallFont(subtitleText).draw(detailRect.x, detailRect.y + 24.0, ColorF{ 0.74, 0.80, 0.88 });

		const RectF barRect{ detailRect.x, detailRect.y + 54.0, detailRect.w, 10.0 };
		barRect.draw(ColorF{ 0.08, 0.10, 0.14, 0.96 });
		if (progress > 0.0)
		{
			RectF{ barRect.x, barRect.y, barRect.w * progress, barRect.h }.draw(progressColor);
		}
		barRect.drawFrame(1.5, ColorF{ 0.30, 0.38, 0.48, 0.95 });
		if (!progressText.isEmpty())
		{
			gameData.smallFont(progressText).draw(detailRect.x, detailRect.y + 68.0, ColorF{ 0.96, 0.97, 0.99 });
		}

		const double smallIconTop = primaryIconRect.bottomY() + 10.0;
		const double smallIconSize = 34.0;
		const double smallIconGap = 8.0;
		const int32 additionalCount = hasQueuedUnit ? Max(static_cast<int32>(target.building->productionQueue.size()) - 1, 0) : 0;
		const int32 visibleCount = Min(additionalCount, 3);
		for (int32 index = 0; index < visibleCount; ++index)
		{
			const RectF iconRect{ primaryIconRect.x + 12.0, smallIconTop + ((smallIconSize + smallIconGap) * index), smallIconSize, smallIconSize };
			DrawQueueIcon(iconRect, target.building->productionQueue[index + 1].archetype, gameData, true);
		}

		if (additionalCount > visibleCount)
		{
			const RectF overflowRect{ primaryIconRect.x + 6.0, smallIconTop + ((smallIconSize + smallIconGap) * visibleCount), 46.0, 24.0 };
			overflowRect.draw(ColorF{ 0.08, 0.10, 0.14, 0.96 });
			overflowRect.drawFrame(2.0, ColorF{ 0.34, 0.52, 0.86, 0.95 });
			gameData.smallFont(s3d::Format(U"+", additionalCount - visibleCount)).drawAt(overflowRect.center(), Palette::White);
		}
	}
}
