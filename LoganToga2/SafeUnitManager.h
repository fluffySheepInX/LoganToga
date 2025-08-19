#pragma once
#include "stdafx.h"
#include "ClassUnit.h"

class SafeUnitManager {
private:
	static inline HashTable<int64, std::weak_ptr<Unit>> unitRegistry;
	static inline std::mutex registryMutex;

public:
	static std::shared_ptr<Unit> GetSafeUnit(int64 unitID) {
		std::scoped_lock lock(registryMutex);

		if (auto it = unitRegistry.find(unitID); it != unitRegistry.end()) {
			if (auto unit = it->second.lock()) {
				return unit;  // 有効な参照
			}
			else {
				unitRegistry.erase(it);
				return nullptr;
			}
		}
		return nullptr;
	}

	static void RegisterUnit(std::shared_ptr<Unit> unit) {
		std::scoped_lock lock(registryMutex);
		unitRegistry[unit->ID] = unit;
	}
};
