#pragma once
#include <Siv3D.hpp>

struct ProjectileHitIntervalState {
 Vec2 lastPos;
 double accum =0.0;
};

class ProjectileHitService {
public:
 bool shouldCheck(uint64 bulletKey, const Vec2& currentPos, double threshold) {
 if (threshold <=0.0) {
 return true; // always check if no threshold
 }
 auto it = state.find(bulletKey);
 if (it == state.end()) {
 state[bulletKey] = { currentPos,0.0 }; // initialize only
 return false; // skip first frame until moved enough
 }
 auto& st = it->second;
 st.accum += (currentPos - st.lastPos).length();
 st.lastPos = currentPos;
 if (st.accum >= threshold) {
 st.accum =0.0;
 return true;
 }
 return false;
 }

 void forgetByBulletNo(int32 bulletNo) {
 Array<uint64> eraseKeys;
 for (const auto& kv : state) {
 int32 upper = static_cast<int32>(kv.first >>32);
 if (upper == bulletNo) eraseKeys.push_back(kv.first);
 }
 for (auto k : eraseKeys) state.erase(k);
 }

 void clearAll() { state.clear(); }

private:
 HashTable<uint64, ProjectileHitIntervalState> state;
};
