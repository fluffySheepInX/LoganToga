# pragma once
# include <Siv3D.hpp>

namespace ui::editor_icon
{
    inline constexpr double CollapsedIconSize = 64.0;
 inline constexpr double DockGap = 8.0;
    inline constexpr double CollisionPadding = 8.0;
    inline constexpr double SearchStep = 8.0;

    inline RectF GetPreviewButtonRect()
    {
        const double windowBarHeight = 36.0;
        const double buttonSize = 96.0;
        const double offset = 10.0;
        return RectF{ 12, Scene::Height() - windowBarHeight - offset - buttonSize, buttonSize, buttonSize };
    }

    inline Vec2 GetDockedStackPosition(const size_t slotIndex)
    {
        const RectF previewRect = GetPreviewButtonRect();
        return Vec2{
            previewRect.x,
            previewRect.y - DockGap - CollapsedIconSize - (slotIndex * (CollapsedIconSize + DockGap))
        };
    }

    inline HashTable<String, RectF>& Registry()
    {
        static HashTable<String, RectF> registry;
        return registry;
    }

    inline RectF ClampToScene(const RectF& rect)
    {
        const double maxX = Max(0.0, static_cast<double>(Scene::Width()) - rect.w);
        const double maxY = Max(0.0, static_cast<double>(Scene::Height()) - rect.h);
        return RectF{
            Clamp(rect.x, 0.0, maxX),
            Clamp(rect.y, 0.0, maxY),
            rect.w,
            rect.h
        };
    }

    inline void RegisterCollapsedIcon(const StringView id, const Optional<RectF>& rect)
    {
        const String key{ id };
        if (rect)
        {
            Registry().insert_or_assign(key, ClampToScene(*rect));
        }
        else
        {
            Registry().erase(key);
        }
    }

    inline Array<RectF> GetOtherCollapsedIcons(const StringView id)
    {
        Array<RectF> rects;
        for (const auto& [key, rect] : Registry())
        {
            if (key != id)
            {
                rects << rect;
            }
        }
        return rects;
    }

    inline bool IntersectsAny(const RectF& rect, const Array<RectF>& others)
    {
        const RectF padded = rect.stretched(CollisionPadding);
        for (const auto& other : others)
        {
            if (padded.intersects(other.stretched(CollisionPadding)))
            {
                return true;
            }
        }
        return false;
    }

    inline Vec2 ResolveCollapsedIconPosition(const StringView id, const Vec2& desiredPos, const SizeF& size = SizeF{ CollapsedIconSize, CollapsedIconSize })
    {
        const RectF desiredRect = ClampToScene(RectF{ desiredPos, size });
        const Array<RectF> others = GetOtherCollapsedIcons(id);
        if (not IntersectsAny(desiredRect, others))
        {
            return desiredRect.pos;
        }

        Optional<Vec2> bestPos;
        double bestDistance = Math::Inf;
        const double maxRadius = Max(Scene::Width(), Scene::Height()) + Max(size.x, size.y);

        auto evaluate = [&](const Vec2& candidatePos)
            {
                const RectF candidate = ClampToScene(RectF{ candidatePos, size });
                if (IntersectsAny(candidate, others))
                {
                    return;
                }

                const double distance = candidate.pos.distanceFrom(desiredRect.pos);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestPos = candidate.pos;
                }
            };

        evaluate(desiredRect.pos);
        if (bestPos)
        {
            return *bestPos;
        }

        for (double radius = SearchStep; radius <= maxRadius; radius += SearchStep)
        {
            for (double dx = -radius; dx <= radius; dx += SearchStep)
            {
                evaluate(desiredRect.pos + Vec2{ dx, -radius });
                evaluate(desiredRect.pos + Vec2{ dx, radius });
            }

            for (double dy = (-radius + SearchStep); dy <= (radius - SearchStep); dy += SearchStep)
            {
                evaluate(desiredRect.pos + Vec2{ -radius, dy });
                evaluate(desiredRect.pos + Vec2{ radius, dy });
            }

            if (bestPos)
            {
                return *bestPos;
            }
        }

        return desiredRect.pos;
    }
}
