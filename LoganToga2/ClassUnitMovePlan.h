#pragma once
class ClassUnitMovePlan
{
private:
	int64 targetID = -1;
	Array<Point> path;
	Optional<Point> currentTarget;
	bool pathCompleted = false;
	bool needsRepath = false;
	bool retreating = false;
	Vec2 lastKnownEnemyPos;
public:
	Point lastPoint;
	ClassUnitMovePlan() = default;
	void setLastKnownEnemyPos(const Vec2& pos)
	{
		lastKnownEnemyPos = pos;
	}

	Vec2 getLastKnownEnemyPos() const
	{
		return lastKnownEnemyPos;
	}
	void setTarget(int64 newTargetID)
	{
		targetID = newTargetID;
	}
	void setRetreating(bool va)
	{
		retreating = va;
	}
	bool isRetreating() const
	{
		return retreating;
	}

	int64 getTargetID() const
	{
		return targetID;
	}

	void setPath(const Array<Point>& newPath)
	{
		path = newPath;
		pathCompleted = path.isEmpty();
		currentTarget = pathCompleted ? Optional<Point>{} : Optional<Point>{ path.front() };
	}

	const Array<Point>& getPath() const
	{
		return path;
	}

	Optional<Point> getCurrentTarget() const
	{
		return currentTarget;
	}

	bool isPathCompleted() const
	{
		return pathCompleted;
	}

	void markRepathNeeded()
	{
		needsRepath = true;
	}

	bool isRepathNeeded() const
	{
		return needsRepath;
	}

	void stepToNext()
	{
		if (path.isEmpty())
		{
			pathCompleted = true;
			currentTarget.reset();
			return;
		}

		path.pop_front();

		if (path.isEmpty())
		{
			pathCompleted = true;
			currentTarget.reset();
		}
		else
		{
			currentTarget = path.front();
		}
	}
};

