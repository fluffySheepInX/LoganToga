# pragma once
# include <Siv3D.hpp>
# include <limits>
# include "RoadTypes.hpp"

namespace road
{
    struct RoadPointSample
    {
        size_t roadIndex = 0;
        size_t segmentIndex = 0;
        Vec3 position{ 0, 0, 0 };
        Vec3 tangent{ 1, 0, 0 };
        double arcLength = 0.0;
        double signedLateralDistance = 0.0;
        double halfWidth = 0.0;
    };

    struct IntersectionCluster
    {
        Vec3 position{ 0, 0, 0 };
        size_t count = 0;
        double radius = 1.2;
    };

    struct RoadBoundaryContext
    {
        RoadPointSample sample;
        double edgeDistance = 0.0;
        double warpedEdgeDistance = 0.0;
        double edgeWarp = 0.0;
        double coreMask = 0.0;
        double shoulderMask = 0.0;
        double recoveryMask = 0.0;
        double intersectionMask = 0.0;
    };

    [[nodiscard]] inline double GeometrySaturate(const double value)
    {
        return Clamp(value, 0.0, 1.0);
    }

    [[nodiscard]] inline double GeometrySmooth01(const double value)
    {
        const double t = GeometrySaturate(value);
        return (t * t * (3.0 - (2.0 * t)));
    }

    [[nodiscard]] inline double GeometrySmoothStep(const double edge0, const double edge1, const double value)
    {
        if (Abs(edge1 - edge0) <= 0.000001)
        {
            return (value < edge0 ? 0.0 : 1.0);
        }

        return GeometrySmooth01((value - edge0) / (edge1 - edge0));
    }

    [[nodiscard]] inline Vec3 GeometrySafeNormalize(const Vec3& value, const Vec3& fallback)
    {
        if (value.lengthSq() <= 0.000001)
        {
            return fallback;
        }

        return value.normalized();
    }

    [[nodiscard]] inline Vec2 ToXZ(const Vec3& value)
    {
        return Vec2{ value.x, value.z };
    }

    [[nodiscard]] inline double HashNoise1D(const int32 x)
    {
        uint32 n = static_cast<uint32>(x) * 374761393u + 0x9e3779b9u;
        n = (n << 13u) ^ n;
        const uint32 hashed = (n * (n * n * 15731u + 789221u) + 1376312589u);
        return ((hashed & 0x7fffffffu) / 2147483647.0);
    }

    [[nodiscard]] inline double ValueNoise1D(const double x)
    {
        const int32 ix = static_cast<int32>(std::floor(x));
        const double fx = x - ix;
        const double sx = GeometrySmooth01(fx);
        return Math::Lerp(HashNoise1D(ix), HashNoise1D(ix + 1), sx);
    }

    [[nodiscard]] inline double Fbm1D(const double x, const int32 octaves)
    {
        double total = 0.0;
        double amplitude = 0.5;
        double frequency = 1.0;
        double normalization = 0.0;

        for (int32 i = 0; i < octaves; ++i)
        {
            total += (ValueNoise1D(x * frequency) * amplitude);
            normalization += amplitude;
            amplitude *= 0.5;
            frequency *= 2.0;
        }

        return ((normalization > 0.0) ? (total / normalization) : 0.0);
    }

    [[nodiscard]] inline Optional<RoadPointSample> ProjectPointToRoad(const Vec3& point, const RoadPath& road, const size_t roadIndex)
    {
        if (road.points.size() < 2)
        {
            return none;
        }

        double bestDistanceSq = std::numeric_limits<double>::infinity();
        Optional<RoadPointSample> result;
        double accumulatedLength = 0.0;

        for (size_t i = 0; (i + 1) < road.points.size(); ++i)
        {
            const Vec3 a3 = road.points[i];
            const Vec3 b3 = road.points[i + 1];
            const Vec2 a = ToXZ(a3);
            const Vec2 b = ToXZ(b3);
            const Vec2 p = ToXZ(point);
            const Vec2 ab = (b - a);
            const double abLengthSq = ab.lengthSq();
            const double segmentLength = a3.distanceFrom(b3);

            double t = 0.0;
            if (abLengthSq > 0.000001)
            {
                t = Clamp((p - a).dot(ab) / abLengthSq, 0.0, 1.0);
            }

            const Vec3 projectedPos = a3.lerp(b3, t);
            const Vec2 projectedXZ = a.lerp(b, t);
            const double distanceSq = (p - projectedXZ).lengthSq();

            if (distanceSq < bestDistanceSq)
            {
                const Vec3 tangent = GeometrySafeNormalize(b3 - a3, Vec3{ 1, 0, 0 });
                const Vec3 right = Vec3{ tangent.z, 0.0, -tangent.x };
                bestDistanceSq = distanceSq;
                result = RoadPointSample{
                    .roadIndex = roadIndex,
                    .segmentIndex = i,
                    .position = Vec3{ projectedPos.x, 0.0, projectedPos.z },
                    .tangent = tangent,
                    .arcLength = (accumulatedLength + segmentLength * t),
                    .signedLateralDistance = (point - projectedPos).dot(right),
                    .halfWidth = Max(road.width * 0.5, 0.0)
                };
            }

            accumulatedLength += segmentLength;
        }

        return result;
    }

    [[nodiscard]] inline Optional<RoadPointSample> ProjectPointToRoads(const Vec3& point, const Array<RoadPath>& roads)
    {
        double bestDistanceSq = std::numeric_limits<double>::infinity();
        Optional<RoadPointSample> result;

        for (size_t i = 0; i < roads.size(); ++i)
        {
            if (const auto sample = ProjectPointToRoad(point, roads[i], i))
            {
                const double distanceSq = ToXZ(point - sample->position).lengthSq();
                if (distanceSq < bestDistanceSq)
                {
                    bestDistanceSq = distanceSq;
                    result = *sample;
                }
            }
        }

        return result;
    }

    [[nodiscard]] inline double EdgeDistance(const RoadPointSample& sample)
    {
        return (Abs(sample.signedLateralDistance) - sample.halfWidth);
    }

    [[nodiscard]] inline double EvaluateEdgeWarp(const RoadPointSample& sample, const double amplitude = 0.22, const double frequency = 0.08)
    {
        const double longitudinalNoise = (Fbm1D(sample.arcLength * frequency + 13.7, 4) - 0.5) * 2.0;
        const double roadFactor = Clamp(sample.halfWidth / 3.0, 0.4, 1.0);
        return (longitudinalNoise * amplitude * roadFactor);
    }

    [[nodiscard]] inline double EvaluateCoreMask(const RoadPointSample& sample, const double fadeWidth = 0.35, const double warpedEdgeDistance = std::numeric_limits<double>::quiet_NaN())
    {
        const double distance = (std::isnan(warpedEdgeDistance) ? EdgeDistance(sample) : warpedEdgeDistance);
        const double innerRadius = Max(sample.halfWidth - fadeWidth, 0.0);
        const double pseudoLateral = (distance + sample.halfWidth);
        return (1.0 - GeometrySmoothStep(innerRadius, sample.halfWidth, pseudoLateral));
    }

    [[nodiscard]] inline double EvaluateShoulderMask(const RoadPointSample& sample, const double innerWidth = 0.35, const double outerWidth = 0.9, const double warpedEdgeDistance = std::numeric_limits<double>::quiet_NaN())
    {
        const double d = (std::isnan(warpedEdgeDistance) ? EdgeDistance(sample) : warpedEdgeDistance);
        const double inside = GeometrySmoothStep(-innerWidth, 0.0, d);
        const double outside = (1.0 - GeometrySmoothStep(0.0, outerWidth, d));
        return GeometrySaturate(inside * outside);
    }

    [[nodiscard]] inline double EvaluateRecoveryMask(const RoadPointSample& sample, const double begin = 0.15, const double end = 1.4, const double warpedEdgeDistance = std::numeric_limits<double>::quiet_NaN())
    {
        const double d = (std::isnan(warpedEdgeDistance) ? EdgeDistance(sample) : warpedEdgeDistance);
        return GeometrySmoothStep(begin, end, d);
    }

    [[nodiscard]] inline Array<IntersectionCluster> BuildIntersectionClusters(const Array<RoadPath>& roads)
    {
        Array<IntersectionCluster> clusters;

        for (const auto& road : roads)
        {
            if (road.points.isEmpty())
            {
                continue;
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                bool merged = false;
                for (auto& cluster : clusters)
                {
                    if (cluster.position.distanceFrom(endpoint) <= 0.01)
                    {
                        cluster.count += 1;
                        cluster.radius = Max(cluster.radius, Max(road.width * 0.9, 1.2));
                        merged = true;
                        break;
                    }
                }

                if (not merged)
                {
                    clusters << IntersectionCluster{ endpoint, 1, Max(road.width * 0.9, 1.2) };
                }
            }
        }

        return clusters;
    }

    [[nodiscard]] inline double EvaluateIntersectionMask(const Vec3& point, const Array<IntersectionCluster>& clusters)
    {
        double mask = 0.0;

        for (const auto& cluster : clusters)
        {
            if (cluster.count < 2)
            {
                continue;
            }

            const double distance = cluster.position.distanceFrom(point);
            const double contribution = (1.0 - GeometrySmoothStep(cluster.radius, cluster.radius * 1.8, distance));
            mask = Max(mask, contribution);
        }

        return mask;
    }

    [[nodiscard]] inline Optional<RoadBoundaryContext> EvaluateRoadBoundaryContext(
        const Vec3& point,
        const Array<RoadPath>& roads,
        const Array<IntersectionCluster>& clusters,
        const double shoulderInnerWidth = 0.35,
        const double shoulderOuterWidth = 0.9)
    {
        const auto sample = ProjectPointToRoads(point, roads);
        if (not sample)
        {
            return none;
        }

        const double edgeDistance = EdgeDistance(*sample);
        const double edgeWarp = EvaluateEdgeWarp(*sample);
        const double warpedEdgeDistance = (edgeDistance - edgeWarp);

        return RoadBoundaryContext{
            .sample = *sample,
            .edgeDistance = edgeDistance,
            .warpedEdgeDistance = warpedEdgeDistance,
            .edgeWarp = edgeWarp,
            .coreMask = EvaluateCoreMask(*sample, 0.35, warpedEdgeDistance),
            .shoulderMask = EvaluateShoulderMask(*sample, shoulderInnerWidth, shoulderOuterWidth, warpedEdgeDistance),
            .recoveryMask = EvaluateRecoveryMask(*sample, 0.15, 1.4, warpedEdgeDistance),
            .intersectionMask = EvaluateIntersectionMask(point, clusters)
        };
    }
}
