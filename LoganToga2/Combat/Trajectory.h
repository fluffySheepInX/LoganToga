#pragma once
#include <Siv3D.hpp>
#include "ClassSkill.h"

struct TrajectoryContext {
 Vec2 start;
 Vec2 end;
 double lifeTime; // elapsed seconds
 double duration; // total seconds
};

class ITrajectory {
public:
 virtual ~ITrajectory() = default;
 virtual Vec2 eval(const TrajectoryContext& ctx) const noexcept =0;
};

class LinearTrajectory final : public ITrajectory {
public:
 Vec2 eval(const TrajectoryContext& ctx) const noexcept override {
 double t = (ctx.duration <=0.0) ?1.0 : Saturate(ctx.lifeTime / ctx.duration);
 return ctx.start + (ctx.end - ctx.start) * t;
 }
};

class ArcTrajectory final : public ITrajectory {
public:
 explicit ArcTrajectory(double heightRatio =0.5) : m_heightRatio(heightRatio) {}
 Vec2 eval(const TrajectoryContext& ctx) const noexcept override {
 double t = (ctx.duration <=0.0) ?1.0 : Saturate(ctx.lifeTime / ctx.duration);
 Vec2 flat = ctx.start + (ctx.end - ctx.start) * t;
 double dist = (ctx.end - ctx.start).length();
 double s = (2.0 * t -1.0);
 double yOffset = dist * m_heightRatio * (1.0 - s * s); // parabola peak at t=0.5
 return flat.movedBy(0, -yOffset); // up is -Y
 }
private:
 double m_heightRatio;
};

// NOTE: heightRatio derivation uses Skill.height (0..100) else fallback0.5
inline const ITrajectory& selectTrajectory(const Skill& s) {
 static LinearTrajectory linear;
 static ArcTrajectory arcDefault(0.5); // reused if no custom height
 if (s.MoveType == MoveType::thr) {
 double hr = (s.height ==0) ?0.5 : (static_cast<double>(s.height) /100.0);
 // For simplicity reuse a small pool (could extend with cache if many variants)
 static Array<ArcTrajectory> pool; // height-specific storage
 for (const auto& a : pool) {
 // approximate compare
 if (Abs(a.eval({ Vec2::Zero(), Vec2::One(),0,1 }).y - ArcTrajectory(hr).eval({ Vec2::Zero(), Vec2::One(),0,1 }).y) <0.0001) {
 return a; }
 }
 pool << ArcTrajectory(hr);
 return pool.back();
 }
 return linear;
}
