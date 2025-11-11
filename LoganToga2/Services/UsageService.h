#pragma once
#include <Siv3D.hpp>

class UsageService {
public:
 bool hasUsesLeft(long long unitId, const String& skillTag) const {
 if (auto itU = table.find(unitId); itU != table.end()) {
 if (auto itS = itU->second.find(skillTag); itS != itU->second.end()) {
 return (itS->second >0);
 }
 }
 return true; // unlimited if not set
 }

 void consume(long long unitId, const String& skillTag) {
 if (auto itU = table.find(unitId); itU != table.end()) {
 if (auto itS = itU->second.find(skillTag); itS != itU->second.end() && itS->second >0) {
 --(itS->second);
 }
 }
 }

 void setUses(long long unitId, const String& skillTag, int32 uses) {
 table[unitId][skillTag] = Max(0, uses);
 }

 void clearUnit(long long unitId) { table.erase(unitId); }
 void clearAll() { table.clear(); }

private:
 HashTable<long long, HashTable<String, int32>> table;
};
