#pragma once
# include <Siv3D.hpp>
# include "../App/BattleNotificationState.h"

namespace LT3
{
	inline void UpdateBattleNotifications(BattleNotificationRuntimeState& notifications, double dt)
	{
		for (auto& entry : notifications.entries)
		{
			entry.timeSec += dt;
		}

		notifications.entries.remove_if([lifeTimeSec = notifications.lifeTimeSec](const BattleNotificationEntry& entry)
		{
			return lifeTimeSec < entry.timeSec;
		});

		for (size_t i = 0; i < notifications.entries.size(); ++i)
		{
			auto& entry = notifications.entries[i];
			entry.currentIndex = Math::SmoothDamp(entry.currentIndex,
				static_cast<double>(i), entry.velocity, 0.15, 9999.0, dt);
		}
	}

	inline void DrawBattleNotifications(const BattleNotificationRuntimeState& notifications, const Font& uiFont)
	{
		const double baseY = Scene::Center().y - 96.0;
		const double baseX = 24.0;
		const double width = 360.0;

		for (const auto& entry : notifications.entries)
		{
			double xScale = 1.0;
			double alpha = 1.0;

			if (entry.timeSec < 0.2)
			{
				xScale = alpha = (entry.timeSec / 0.2);
			}
			else if ((notifications.lifeTimeSec - 0.4) < entry.timeSec)
			{
				alpha = ((notifications.lifeTimeSec - entry.timeSec) / 0.4);
			}

			alpha = EaseOutExpo(Clamp(alpha, 0.0, 1.0));
			xScale = EaseOutExpo(Clamp(xScale, 0.0, 1.0));

			ColorF backgroundColor{ 0.02, 0.03, 0.045, 0.84 * alpha };
			ColorF frameColor{ 1.0, 1.0, 1.0, 0.20 * alpha };
			ColorF accentColor{ 0.70, 0.80, 0.95, alpha };
			if (entry.type == BattleNotificationType::Success)
			{
				accentColor = ColorF{ 1.0, 0.84, 0.0, alpha };
			}
			else if (entry.type == BattleNotificationType::Warning)
			{
				accentColor = ColorF{ 1.0, 0.34, 0.34, alpha };
			}
			const ColorF textColor{ 0.97, 0.98, 1.0, alpha };

			const RectF rect{ baseX, baseY + (entry.currentIndex * 42.0), width * xScale, 34.0 };
			rect.rounded(5.0).draw(backgroundColor).drawFrame(1.0, 0.0, frameColor);
			RectF{ rect.x, rect.y, 5.0, rect.h }.rounded(5.0, 0.0, 0.0, 5.0).draw(accentColor);
			uiFont(entry.message).draw(16, Arg::leftCenter = rect.leftCenter().movedBy(18.0, -1.0), textColor);
		}
	}
}
