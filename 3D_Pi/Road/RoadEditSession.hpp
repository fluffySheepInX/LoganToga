# pragma once
# include <Siv3D.hpp>
# include "RoadSceneSnapshot.hpp"

struct RoadEditSession
{
    bool active = false;
    Optional<RoadSceneSnapshot> restorePoint;

    void begin(const RoadSceneSnapshot& snapshot)
    {
        restorePoint = snapshot;
        active = true;
    }

    void end()
    {
        active = false;
        restorePoint.reset();
    }

    [[nodiscard]] bool canRestore() const
    {
        return active && restorePoint.has_value();
    }
};
