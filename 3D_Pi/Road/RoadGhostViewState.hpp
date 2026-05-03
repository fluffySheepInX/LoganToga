# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"
# include "RoadMeshBuilder.hpp"

struct RoadGhostViewState
{
    bool visible = false;
    double opacity = 0.22;
    bool snapEnabled = true;
    Array<RoadPath> roads;
    Array<Optional<Mesh>> roadMeshes;
    Array<Optional<Mesh>> shoulderMeshes;
    Array<Mesh> connectionPatchMeshes;

    void buildFrom(const Array<RoadPath>& sourceRoads, const RoadMaterialSettings& material)
    {
        roads = sourceRoads;
        road::RebuildRoadMeshes(roads, material, roadMeshes, shoulderMeshes, connectionPatchMeshes);
    }

    void clear()
    {
        visible = false;
        roads.clear();
        roadMeshes.clear();
        shoulderMeshes.clear();
        connectionPatchMeshes.clear();
    }

    [[nodiscard]] Optional<Vec3> findSnapPoint(const Vec3& point, const double maxDistance) const
    {
        if (not (visible && snapEnabled))
        {
            return none;
        }

        Optional<Vec3> result;
        double nearest = maxDistance;

        for (const auto& road : roads)
        {
            if (road.points.isEmpty())
            {
                continue;
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                const double dist = endpoint.distanceFrom(point);
                if (dist <= nearest)
                {
                    nearest = dist;
                    result = endpoint;
                }
            }
        }

        return result;
    }
};
