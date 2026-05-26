#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	enum class BattleNotificationType
	{
		Normal,
		Success,
		Warning,
	};

	struct BattleNotificationEntry
	{
		String message;
		double timeSec = 0.0;
		double currentIndex = 0.0;
		double velocity = 0.0;
		BattleNotificationType type = BattleNotificationType::Normal;
	};

	struct BattleNotificationRuntimeState
	{
		Array<BattleNotificationEntry> entries;
		double lifeTimeSec = 4.5;
	};

	inline void ClearBattleNotifications(BattleNotificationRuntimeState& notifications)
	{
		notifications.entries.clear();
	}

	inline void PushBattleNotification(BattleNotificationRuntimeState& notifications, const String& message, BattleNotificationType type = BattleNotificationType::Normal)
	{
		const double currentIndex = notifications.entries.isEmpty()
			? 0.0
			: (notifications.entries.back().currentIndex + 1.0);
		const double velocity = notifications.entries.isEmpty()
			? 0.0
			: notifications.entries.back().velocity;

		notifications.entries << BattleNotificationEntry{
			.message = message,
			.timeSec = 0.0,
			.currentIndex = currentIndex,
			.velocity = velocity,
			.type = type,
		};
	}
}
