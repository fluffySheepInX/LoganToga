# include "../stdafx.h"
# include "RoadEditor.hpp"

Optional<Vec3> RoadEditor::cursorToGround(const BasicCamera3D& camera) const{
        const Ray ray = camera.screenToRay(Cursor::PosF());
        if (const auto hit = ray.intersectsAt(InfinitePlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } }))
        {
            return Vec3{ hit->x, 0.0, hit->z };
        }

        return none;
    }



Vec3 RoadEditor::getCurrentInputPoint() const{
        if (m_snapPoint)
        {
            return *m_snapPoint;
        }

        if (m_hoverPoint)
        {
            return *m_hoverPoint;
        }

        return Vec3{ 0, 0, 0 };
    }



Optional<Vec3> RoadEditor::findSnapPoint(const Optional<Vec3>& point) const{
        if (not point)
        {
            return none;
        }

        Optional<Vec3> result;
        double nearestDistance = m_snapDistance;

        for (const auto& road : m_roads)
        {
            if (road.points.isEmpty())
            {
                continue;
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                const double distance = endpoint.distanceFrom(*point);
                if (distance <= nearestDistance)
                {
                    nearestDistance = distance;
                    result = endpoint;
                }
            }
        }

        if (const auto ghostSnap = m_ghost.findSnapPoint(*point, nearestDistance))
        {
            result = ghostSnap;
        }

        return result;
    }



    void RoadEditor::appendPoint(const Vec3& point){
        const Vec3 snappedPoint{ point.x, 0.0, point.z };

        if (not m_editingRoad)
        {
            m_editingRoad = RoadPath{};
            m_editingRoad->points << snappedPoint;
            rebuildEditingMesh();
            return;
        }

        if (m_editingRoad->points.isEmpty())
        {
            m_editingRoad->points << snappedPoint;
            rebuildEditingMesh();
            return;
        }

        if (m_editingRoad->points.back().distanceFrom(snappedPoint) < PointSpacing)
        {
            return;
        }

        m_editingRoad->points << snappedPoint;
        rebuildEditingMesh();
    }



    void RoadEditor::confirmEditingRoad(){
        if ((not m_editingRoad) || (m_editingRoad->points.size() < 2))
        {
            m_editingRoad.reset();
            m_editingMesh.reset();
            m_statusMessage = U"Road confirmation skipped";
            return;
        }

        if (m_snapPoint)
        {
            const Vec3 snappedEnd{ m_snapPoint->x, 0.0, m_snapPoint->z };
            if (m_editingRoad->points.back().distanceFrom(snappedEnd) < PointSpacing)
            {
                m_editingRoad->points.back() = snappedEnd;
            }
            else
            {
                m_editingRoad->points << snappedEnd;
            }
        }

        m_roads << *m_editingRoad;
        m_editingRoad.reset();
        m_editingMesh.reset();
        rebuildAllMeshes();
        m_statusMessage = U"Road confirmed";
    }



    void RoadEditor::cancelEditingRoad(){
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_statusMessage = U"Current road canceled";
    }



    void RoadEditor::clearAllPlacedData(){
        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_intersectionClusters.clear();
        m_scatterItems.clear();
        m_hoverScatterItemIndex.reset();
        m_statusMessage = U"Placed road data cleared";
    }



    void RoadEditor::undo(){
        if (m_editingRoad && (not m_editingRoad->points.isEmpty()))
        {
            m_editingRoad->points.pop_back();
            if (m_editingRoad->points.size() < 2)
            {
                m_editingMesh.reset();
            }
            else
            {
                rebuildEditingMesh();
            }

            if (m_editingRoad->points.isEmpty())
            {
                m_editingRoad.reset();
            }

            m_statusMessage = U"Editing point removed";
            return;
        }

        if (not m_roads.isEmpty())
        {
            m_roads.pop_back();
            rebuildAllMeshes();
            m_statusMessage = U"Last road removed";
        }
    }



    void RoadEditor::rebuildAllMeshes(){
        road::RebuildRoadMeshes(m_roads, m_materialSettings, m_roadMeshes, m_roadShoulderMeshes, m_connectionPatchMeshes);
        m_intersectionClusters = road::BuildIntersectionClusters(m_roads);
    }



    void RoadEditor::rebuildEditingMesh(){
        if (m_editingRoad)
        {
            m_editingMesh = road::BuildRoadMesh(*m_editingRoad);
        }
        else
        {
            m_editingMesh.reset();
        }
    }



    void RoadEditor::drawPathGuide(const Optional<RoadPath>& road, const ColorF& lineColor, const ColorF& pointColor) const{
        if (not road)
        {
            return;
        }

        for (size_t i = 1; i < road->points.size(); ++i)
        {
            Line3D{ road->points[i - 1] + Vec3{ 0, GuideYOffset, 0 }, road->points[i] + Vec3{ 0, GuideYOffset, 0 } }.draw(lineColor);
        }

        for (const auto& point : road->points)
        {
            Sphere{ point + Vec3{ 0, 0.08, 0 }, 0.10 }.draw(pointColor);
        }
    }



    void RoadEditor::drawGhostRoads3D() const{
        if (not m_ghost.visible)
        {
            return;
        }

        const ColorF ghostRoad{ 0.72, 0.82, 1.0, m_ghost.opacity };
        const ColorF ghostShoulder{ 0.72, 0.82, 1.0, m_ghost.opacity * 0.6 };

        const size_t count = Min(m_ghost.roadMeshes.size(), m_ghost.shoulderMeshes.size());
        for (size_t i = 0; i < count; ++i)
        {
            if (m_ghost.shoulderMeshes[i])
            {
                m_ghost.shoulderMeshes[i]->draw(m_roadShoulderTexture, ghostShoulder);
            }
            if (m_ghost.roadMeshes[i])
            {
                m_ghost.roadMeshes[i]->draw(m_roadTexture, ghostRoad);
            }
        }

        for (const auto& road : m_ghost.roads)
        {
            if (road.points.size() < 2)
            {
                continue;
            }

            for (size_t i = 1; i < road.points.size(); ++i)
            {
                Line3D{ road.points[i - 1] + Vec3{ 0, GuideYOffset * 2.0, 0 },
                        road.points[i]     + Vec3{ 0, GuideYOffset * 2.0, 0 } }
                    .draw(ColorF{ 0.60, 0.76, 1.0, m_ghost.opacity * 1.4 });
            }

            for (const auto& endpoint : { road.points.front(), road.points.back() })
            {
                Sphere{ endpoint + Vec3{ 0, 0.10, 0 }, 0.14 }
                    .draw(ColorF{ 0.50, 0.70, 1.0, m_ghost.opacity * 1.8 });
            }
        }
    }
