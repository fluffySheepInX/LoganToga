# pragma once
# include <Siv3D.hpp>
# include "RoadGeometry.hpp"
# include "RoadPlacementTypes.hpp"

namespace road
{
    [[nodiscard]] inline double ScatterSaturate(const double value)
    {
        return Clamp(value, 0.0, 1.0);
    }

    [[nodiscard]] inline double EvaluateEdgeBiasFactor(const RoadBoundaryContext& context, const double edgeBias)
    {
        const double bias = Clamp(edgeBias, 0.0, 1.0);
        return Math::Lerp(1.0 - context.recoveryMask, context.shoulderMask, bias);
    }

    [[nodiscard]] inline PlacementDensityProfile EvaluatePlacementDensityProfile(
        const RoadBoundaryContext& context,
        const PlacementSettings& settings)
    {
        const double edgeBiasFactor = EvaluateEdgeBiasFactor(context, settings.edgeBias);
        const double intersectionBoost = (1.0 + context.intersectionMask * settings.intersectionBoost);
        const double shoulder = context.shoulderMask;
        const double recovery = context.recoveryMask;
        const double core = context.coreMask;

        PlacementDensityProfile profile;
        profile.grassDensity = ScatterSaturate((recovery * 0.85 + shoulder * 0.25) * (1.0 - core * 0.9) * edgeBiasFactor * (1.0 - context.intersectionMask * 0.45));
        profile.pebbleDensity = ScatterSaturate((shoulder * 0.85 + recovery * 0.20) * intersectionBoost * Math::Lerp(0.75, 1.0, edgeBiasFactor));
        profile.mudDensity = ScatterSaturate((shoulder * 0.72 + core * 0.18) * (0.85 + context.intersectionMask * settings.intersectionBoost));
        return profile;
    }

    [[nodiscard]] inline String BuildPlacementDensitySummary(const RoadBoundaryContext& context, const PlacementDensityProfile& profile)
    {
        return U"Edge {:.2f} / Warp {:.2f}\nShoulder {:.2f} / Recovery {:.2f}\nGrass {:.2f} Pebble {:.2f} Mud {:.2f}"_fmt(
            context.warpedEdgeDistance,
            context.edgeWarp,
            context.shoulderMask,
            context.recoveryMask,
            profile.grassDensity,
            profile.pebbleDensity,
            profile.mudDensity);
    }
}
