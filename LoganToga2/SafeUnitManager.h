#pragma once
#include "stdafx.h"
#include "ClassUnit.h"
#include <shared_mutex>

class SafeUnitManager {
private:
	static inline HashTable<int64, std::weak_ptr<Unit>> unitRegistry;
	// std::mutex -> std::shared_mutex に変更
	static inline std::shared_mutex registryMutex;

public:
	// 読み取りのみなので shared_lock
	static std::shared_ptr<Unit> GetSafeUnit(int64 unitID) {
		std::shared_lock<std::shared_mutex> lock(registryMutex);

		if (auto it = unitRegistry.find(unitID); it != unitRegistry.end()) {
			if (auto unit = it->second.lock()) {
				return unit;
			}
			else {
				// 削除は書き込みなので一時的にアップグレードする必要あり
				lock.unlock();                       // 共有ロック解除
				{
					std::unique_lock<std::shared_mutex> ulock(registryMutex);
					// 再確認 (他スレッドで状態が変わった可能性)
					if (auto it2 = unitRegistry.find(unitID); it2 != unitRegistry.end() && it2->second.expired()) {
						unitRegistry.erase(it2);
					}
				}
				return nullptr;
			}
		}
		return nullptr;
	}

	// 書き込み: unique_lock (排他)
	static void RegisterUnit(const std::shared_ptr<Unit>& unit) {
		std::unique_lock<std::shared_mutex> lock(registryMutex);
		unitRegistry[unit->ID] = unit;
	}
};
