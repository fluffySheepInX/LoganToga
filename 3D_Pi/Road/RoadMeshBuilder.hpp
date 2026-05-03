# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"

namespace road
{
    inline constexpr double RoadYOffset = 0.02;
    inline constexpr double MiterAngleCap = 0.15;
    inline constexpr double MinRoadWidth = 0.5;
    inline constexpr double MinRoadTextureRepeat = 0.25;
    inline constexpr double ShoulderWidthExpand = 1.6;

    struct MiterInfo
    {
        Optional<Vec3> startOffset;
        Optional<Vec3> endOffset;
    };

    [[nodiscard]] inline Vec3 SafeNormalize(const Vec3& value, const Vec3& fallback)
    {
        if (value.lengthSq() <= 0.000001)
        {
            return fallback;
        }

        return value.normalized();
    }

    [[nodiscard]] inline Vec3 ForwardToRight(const Vec3& fwd)
    {
        return Vec3{ fwd.z, 0.0, -fwd.x };
    }

    [[nodiscard]] inline Vec3 GetRoadStartFwd(const RoadPath& road)
    {
        return SafeNormalize(road.points[1] - road.points[0], Vec3{ 1, 0, 0 });
    }

    [[nodiscard]] inline Vec3 GetRoadEndFwd(const RoadPath& road)
    {
        const size_t n = road.points.size();
        return SafeNormalize(road.points[n - 1] - road.points[n - 2], Vec3{ 1, 0, 0 });
    }

    [[nodiscard]] inline Vec3 CalcMiterOffset(const Vec3& rightA, const Vec3& rightB, const double halfWidth)
    {
        const Vec3 miterDir = SafeNormalize(rightA + rightB, rightA);
        const double dot = Max(miterDir.dot(rightA), MiterAngleCap);
        return miterDir * (halfWidth / dot);
    }

    [[nodiscard]] inline Vec3 BuildRightOffsetAtPoint(const RoadPath& road, const size_t i, const size_t pointCount,
        const double halfWidth, const Optional<Vec3>& startMiterOffset = none, const Optional<Vec3>& endMiterOffset = none)
    {
        Vec3 forward;

        if (i == 0)
        {
            forward = road.points[1] - road.points[0];
        }
        else if ((i + 1) == pointCount)
        {
            forward = road.points[pointCount - 1] - road.points[pointCount - 2];
        }
        else
        {
            const Vec3 prev = SafeNormalize(road.points[i] - road.points[i - 1], Vec3{ 1, 0, 0 });
            const Vec3 next = SafeNormalize(road.points[i + 1] - road.points[i], Vec3{ 1, 0, 0 });
            forward = SafeNormalize(prev + next, next);
        }

        forward.y = 0.0;
        forward = SafeNormalize(forward, Vec3{ 1, 0, 0 });

        if ((i == 0) && startMiterOffset)
        {
            return *startMiterOffset;
        }

        if (((i + 1) == pointCount) && endMiterOffset)
        {
            return *endMiterOffset;
        }

        return ForwardToRight(forward) * halfWidth;
    }

    [[nodiscard]] inline Optional<Mesh> BuildRoadMeshWithWidth(const RoadPath& road, const double requestedWidth, const Optional<Vec3>& startMiterOffset = none, const Optional<Vec3>& endMiterOffset = none)
    {
        if (road.points.size() < 2)
        {
            return none;
        }

        const double width = Max(requestedWidth, MinRoadWidth);
        const double halfWidth = width * 0.5;
        const double textureRepeat = Max(road.textureRepeat, MinRoadTextureRepeat);
        const size_t pointCount = road.points.size();
        Array<double> lengths(pointCount, 0.0);
        Array<Vertex3D> vertices;
        Array<TriangleIndex32> indices;
        vertices.reserve(pointCount * 2);
        indices.reserve((pointCount - 1) * 2);

        for (size_t i = 1; i < pointCount; ++i)
        {
            lengths[i] = (lengths[i - 1] + road.points[i - 1].distanceFrom(road.points[i]));
        }

        for (size_t i = 0; i < pointCount; ++i)
        {
            const Vec3 rightOffset = BuildRightOffsetAtPoint(road, i, pointCount, halfWidth, startMiterOffset, endMiterOffset);

            const Vec3 center = road.points[i] + Vec3{ 0, RoadYOffset, 0 };
            const Vec3 leftPos = center - rightOffset;
            const Vec3 rightPos = center + rightOffset;
            const float v = static_cast<float>(lengths[i] / textureRepeat);
            vertices << Vertex3D{ Float3{ static_cast<float>(leftPos.x), static_cast<float>(leftPos.y), static_cast<float>(leftPos.z) }, Float3{ 0, 1, 0 }, Float2{ 0.0f, v } };
            vertices << Vertex3D{ Float3{ static_cast<float>(rightPos.x), static_cast<float>(rightPos.y), static_cast<float>(rightPos.z) }, Float3{ 0, 1, 0 }, Float2{ 1.0f, v } };
        }

        for (uint32 i = 0; i + 1 < pointCount; ++i)
        {
            const uint32 base = (i * 2);
            indices << TriangleIndex32{ base, (base + 1), (base + 2) };
            indices << TriangleIndex32{ (base + 2), (base + 1), (base + 3) };
        }

        return Mesh{ MeshData{ std::move(vertices), std::move(indices) } };
    }

    [[nodiscard]] inline Optional<Mesh> BuildRoadMesh(const RoadPath& road, const Optional<Vec3>& startMiterOffset = none, const Optional<Vec3>& endMiterOffset = none)
    {
        return BuildRoadMeshWithWidth(road, road.width, startMiterOffset, endMiterOffset);
    }

    [[nodiscard]] inline Optional<Mesh> BuildRoadShoulderMesh(const RoadPath& road, const double shoulderWidthExpand,
        const Optional<Vec3>& innerStartMiterOffset = none, const Optional<Vec3>& innerEndMiterOffset = none,
        const Optional<Vec3>& outerStartMiterOffset = none, const Optional<Vec3>& outerEndMiterOffset = none)
    {
        if (road.points.size() < 2)
        {
            return none;
        }

        constexpr float LeftOuterU = 0.0f;
        constexpr float LeftInnerU = 0.26f;
        constexpr float RightInnerU = 0.74f;
        constexpr float RightOuterU = 1.0f;

        const double innerWidth = Max(road.width, MinRoadWidth);
        constexpr double InnerOverlap = 0.22;
        const double outerHalfWidth = (innerWidth * 0.5) + Max(shoulderWidthExpand, 0.0);
        const double innerHalfWidth = Max(innerWidth * 0.5 - InnerOverlap, 0.0);
        const double textureRepeat = Max(road.textureRepeat, MinRoadTextureRepeat);
        const size_t pointCount = road.points.size();
        Array<double> lengths(pointCount, 0.0);
        Array<Vertex3D> vertices;
        Array<TriangleIndex32> indices;
        vertices.reserve(pointCount * 4);
        indices.reserve((pointCount - 1) * 4);

        for (size_t i = 1; i < pointCount; ++i)
        {
            lengths[i] = (lengths[i - 1] + road.points[i - 1].distanceFrom(road.points[i]));
        }

        const double noiseAmplitude = Clamp(shoulderWidthExpand * 0.18, 0.10, 0.48);
        const double noiseFreq = 0.72;
        auto shoulderEdgeNoise = [&](const double t, const double phase) -> double
            {
                return Math::Sin(t * noiseFreq * 1.00 + phase) * 0.38
                     + Math::Sin(t * noiseFreq * 2.13 + phase * 1.71) * 0.28
                     + Math::Sin(t * noiseFreq * 4.07 + phase * 0.93) * 0.20
                     + Math::Sin(t * noiseFreq * 7.31 + phase * 2.17) * 0.14;
            };

        for (size_t i = 0; i < pointCount; ++i)
        {
            const Vec3 innerOffset = BuildRightOffsetAtPoint(road, i, pointCount, innerHalfWidth, innerStartMiterOffset, innerEndMiterOffset);
            const Vec3 outerOffset = BuildRightOffsetAtPoint(road, i, pointCount, outerHalfWidth, outerStartMiterOffset, outerEndMiterOffset);

            const Vec3 center = road.points[i] + Vec3{ 0, RoadYOffset, 0 };
            const Vec3 rightDir = SafeNormalize(outerOffset, Vec3{ 1, 0, 0 });
            const double noiseL = shoulderEdgeNoise(lengths[i], 3.14) * noiseAmplitude;
            const double noiseR = shoulderEdgeNoise(lengths[i], 7.83) * noiseAmplitude;
            const Vec3 leftOuterPos = (center - outerOffset) - (rightDir * noiseL);
            const Vec3 leftInnerPos = center - innerOffset;
            const Vec3 rightInnerPos = center + innerOffset;
            const Vec3 rightOuterPos = (center + outerOffset) + (rightDir * noiseR);
            const float v = static_cast<float>(lengths[i] / textureRepeat);

            vertices << Vertex3D{ Float3{ static_cast<float>(leftOuterPos.x), static_cast<float>(leftOuterPos.y), static_cast<float>(leftOuterPos.z) }, Float3{ 0, 1, 0 }, Float2{ LeftOuterU, v } };
            vertices << Vertex3D{ Float3{ static_cast<float>(leftInnerPos.x), static_cast<float>(leftInnerPos.y), static_cast<float>(leftInnerPos.z) }, Float3{ 0, 1, 0 }, Float2{ LeftInnerU, v } };
            vertices << Vertex3D{ Float3{ static_cast<float>(rightInnerPos.x), static_cast<float>(rightInnerPos.y), static_cast<float>(rightInnerPos.z) }, Float3{ 0, 1, 0 }, Float2{ RightInnerU, v } };
            vertices << Vertex3D{ Float3{ static_cast<float>(rightOuterPos.x), static_cast<float>(rightOuterPos.y), static_cast<float>(rightOuterPos.z) }, Float3{ 0, 1, 0 }, Float2{ RightOuterU, v } };
        }

        for (uint32 i = 0; i + 1 < pointCount; ++i)
        {
            const uint32 base = (i * 4);
            const uint32 next = (base + 4);

            indices << TriangleIndex32{ base, (base + 1), next };
            indices << TriangleIndex32{ next, (base + 1), (next + 1) };
            indices << TriangleIndex32{ (base + 2), (base + 3), (next + 2) };
            indices << TriangleIndex32{ (next + 2), (base + 3), (next + 3) };
        }

        return Mesh{ MeshData{ std::move(vertices), std::move(indices) } };
    }

    [[nodiscard]] inline Optional<Mesh> BuildConnectionPatchMesh(const Vec3& center, const double radius)
    {
        constexpr uint32 Segments = 20;
        if (radius <= 0.05)
        {
            return none;
        }

        Array<Vertex3D> vertices;
        Array<TriangleIndex32> indices;
        vertices.reserve(Segments + 2);
        indices.reserve(Segments);

        const Vec3 patchCenter = center + Vec3{ 0, RoadYOffset + 0.003, 0 };
        vertices << Vertex3D{ Float3{ static_cast<float>(patchCenter.x), static_cast<float>(patchCenter.y), static_cast<float>(patchCenter.z) }, Float3{ 0.0f, 1.0f, 0.0f }, Float2{ 0.5f, 0.5f } };

        for (uint32 i = 0; i <= Segments; ++i)
        {
            const double angle = (Math::TwoPi * i / Segments);
            const Vec3 pos = patchCenter + Vec3{ (Cos(angle) * radius), 0.0, (Sin(angle) * radius) };
            const float u = static_cast<float>(0.5 + (Cos(angle) * 0.5));
            const float v = static_cast<float>(0.5 + (Sin(angle) * 0.5));
            vertices << Vertex3D{ Float3{ static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z) }, Float3{ 0.0f, 1.0f, 0.0f }, Float2{ u, v } };
        }

        for (uint32 i = 1; i <= Segments; ++i)
        {
            indices << TriangleIndex32{ 0, i, (i + 1) };
        }

        return Mesh{ MeshData{ std::move(vertices), std::move(indices) } };
    }

    [[nodiscard]] inline Array<MiterInfo> BuildRoadMiters(const Array<RoadPath>& roads, const double widthExpand = 0.0)
    {
        Array<MiterInfo> miters(roads.size());

        for (size_t a = 0; a < roads.size(); ++a)
        {
            if (roads[a].points.size() < 2)
            {
                continue;
            }

            const Vec3 aStart = roads[a].points.front();
            const Vec3 aEnd = roads[a].points.back();

            for (size_t b = a + 1; b < roads.size(); ++b)
            {
                if (roads[b].points.size() < 2)
                {
                    continue;
                }

                const Vec3 bStart = roads[b].points.front();
                const Vec3 bEnd = roads[b].points.back();
                const double halfA = Max(roads[a].width + widthExpand, MinRoadWidth) * 0.5;
                const double halfB = Max(roads[b].width + widthExpand, MinRoadWidth) * 0.5;
                const double avgHalf = (halfA + halfB) * 0.5;

                if (aEnd.distanceFrom(bStart) <= 0.01)
                {
                    const Vec3 offset = CalcMiterOffset(ForwardToRight(GetRoadEndFwd(roads[a])), ForwardToRight(GetRoadStartFwd(roads[b])), avgHalf);
                    miters[a].endOffset = offset;
                    miters[b].startOffset = offset;
                }
                else if (aEnd.distanceFrom(bEnd) <= 0.01)
                {
                    const Vec3 offset = CalcMiterOffset(ForwardToRight(GetRoadEndFwd(roads[a])), ForwardToRight(-GetRoadEndFwd(roads[b])), avgHalf);
                    miters[a].endOffset = offset;
                    miters[b].endOffset = offset;
                }
                else if (aStart.distanceFrom(bStart) <= 0.01)
                {
                    const Vec3 offset = CalcMiterOffset(ForwardToRight(-GetRoadStartFwd(roads[a])), ForwardToRight(GetRoadStartFwd(roads[b])), avgHalf);
                    miters[a].startOffset = offset;
                    miters[b].startOffset = offset;
                }
                else if (aStart.distanceFrom(bEnd) <= 0.01)
                {
                    const Vec3 offset = CalcMiterOffset(ForwardToRight(-GetRoadStartFwd(roads[a])), ForwardToRight(-GetRoadEndFwd(roads[b])), avgHalf);
                    miters[a].startOffset = offset;
                    miters[b].endOffset = offset;
                }
            }
        }

        return miters;
    }

    inline void RebuildRoadMeshes(const Array<RoadPath>& roads, const RoadMaterialSettings& materialSettings, Array<Optional<Mesh>>& roadMeshes, Array<Optional<Mesh>>& roadShoulderMeshes, Array<Mesh>& connectionPatchMeshes)
    {
        roadMeshes.clear();
        roadShoulderMeshes.clear();
        connectionPatchMeshes.clear();
        roadMeshes.reserve(roads.size());
        roadShoulderMeshes.reserve(roads.size());

        const double shoulderWidthExpand = Max(materialSettings.shoulderWidthExpand, 0.0);
        const Array<MiterInfo> coreMiters = BuildRoadMiters(roads);
        const Array<MiterInfo> shoulderMiters = BuildRoadMiters(roads, shoulderWidthExpand * 2.0);

        for (size_t i = 0; i < roads.size(); ++i)
        {
            roadShoulderMeshes << BuildRoadShoulderMesh(roads[i], shoulderWidthExpand,
                coreMiters[i].startOffset, coreMiters[i].endOffset,
                shoulderMiters[i].startOffset, shoulderMiters[i].endOffset);
            roadMeshes << BuildRoadMesh(roads[i], coreMiters[i].startOffset, coreMiters[i].endOffset);
        }

        struct EndpointCluster
        {
            Vec3 position;
            size_t count = 0;
            double maxWidth = 0.0;
        };

        Array<EndpointCluster> clusters;
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
                        cluster.maxWidth = Max(cluster.maxWidth, road.width);
                        merged = true;
                        break;
                    }
                }

                if (not merged)
                {
                    clusters << EndpointCluster{ endpoint, 1, road.width };
                }
            }
        }

        for (const auto& cluster : clusters)
        {
            if (cluster.count < 2)
            {
                continue;
            }

            if (const auto patch = BuildConnectionPatchMesh(cluster.position, Max(cluster.maxWidth * 0.75, 1.2)))
            {
                connectionPatchMeshes << *patch;
            }
        }
    }
}
