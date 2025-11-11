#pragma once
#include <Siv3D.hpp>

// Simple runtime storage for cooldowns: unitId -> (skillTag -> readyAtSeconds)
class CooldownService {
public:
	bool isReady(long long unitId, const String& skillTag, double now) const {
		if (auto itU = table.find(unitId); itU != table.end()) {
			if (auto itS = itU->second.find(skillTag); itS != itU->second.end()) {
				return now >= itS->second;
			}
		}
		return true;
	}

	double remaining(long long unitId, const String& skillTag, double now) const {
		if (auto itU = table.find(unitId); itU != table.end()) {
			if (auto itS = itU->second.find(skillTag); itS != itU->second.end()) {
				return Max(0.0, itS->second - now);
			}
		}
		return 0.0;
	}

	void commit(long long unitId, const String& skillTag, double cdSeconds, double now) {
		table[unitId][skillTag] = now + cdSeconds;
	}

	void clearUnit(long long unitId) {
		table.erase(unitId);
	}

	void clearAll() { table.clear(); }

private:
	HashTable<long long, HashTable<String, double>> table;
};
