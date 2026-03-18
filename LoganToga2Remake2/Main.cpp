#include "GameData.h"
#include "BattleScene.h"
#include "BalanceEditScene.h"
#include "BonusRoomScene.h"
#include "GameSettings.h"
#include "MapEditScene.h"
#include "RewardEditorScene.h"
#include "RewardScene.h"
#include "TitleScene.h"
#include "TitleUiEditorScene.h"
#include "WindowChromeAddon.h"
#include <ctime>

namespace
{
	inline constexpr int32 StartupAvailableDays = 14;
	inline constexpr int32 StartupLimitSchemaVersion = 1;
	inline constexpr int64_t SecondsPerDay = 24LL * 60 * 60;

	[[nodiscard]] String GetStartupLimitPath()
	{
		return FileSystem::PathAppend(FileSystem::ParentPath(GetContinueRunSavePath()), U"startup_limit.toml");
	}

	[[nodiscard]] int64_t GetCurrentUnixTime()
	{
		return static_cast<int64_t>(std::time(nullptr));
	}

	[[nodiscard]] Optional<int64_t> LoadStartupFirstLaunchUnixTime()
	{
		const TOMLReader toml{ GetStartupLimitPath() };
		if (!toml)
		{
			return none;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != StartupLimitSchemaVersion)
			{
				return none;
			}

			const int64_t firstLaunchUnixTime = toml[U"firstLaunchUnixTime"].get<int64_t>();
			return (firstLaunchUnixTime > 0)
				? Optional<int64_t>{ firstLaunchUnixTime }
				: none;
		}
		catch (const std::exception&)
		{
			return none;
		}
	}

	[[nodiscard]] bool SaveStartupFirstLaunchUnixTime(const int64_t firstLaunchUnixTime)
	{
		const String path = GetStartupLimitPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));

		String content;
		AppendTomlLine(content, U"schemaVersion", Format(StartupLimitSchemaVersion));
		AppendTomlLine(content, U"firstLaunchUnixTime", Format(firstLaunchUnixTime));

		TextWriter writer{ path };
		if (!writer)
		{
			return false;
		}

		writer.write(content);
		return true;
	}

	[[nodiscard]] bool IsStartupExpired()
	{
		const int64_t currentUnixTime = GetCurrentUnixTime();
		Optional<int64_t> firstLaunchUnixTime = LoadStartupFirstLaunchUnixTime();
		if (!firstLaunchUnixTime)
		{
			if (!SaveStartupFirstLaunchUnixTime(currentUnixTime))
			{
				return false;
			}

			firstLaunchUnixTime = currentUnixTime;
		}

		const int64_t expirationUnixTime = (*firstLaunchUnixTime + (static_cast<int64_t>(StartupAvailableDays) * SecondsPerDay));
		return (currentUnixTime >= expirationUnixTime);
	}

	void RunStartupExpiredScreen()
	{
		const Font titleFont{ FontMethod::MSDF, 34, Typeface::Bold };
		const Font bodyFont{ FontMethod::MSDF, 22 };
		const Font hintFont{ FontMethod::MSDF, 16 };
		const String title = Localization::GetText(U"startup_limit.expired_title", U"起動期限が終了しました", U"Startup period ended");
		const String body = Localization::FormatText(U"startup_limit.expired_body", U"このビルドは初回起動から {0} 日を過ぎたため、起動できません。", U"This build can no longer start because {0} days have passed since first launch.", StartupAvailableDays);
		const String hint = Localization::GetText(U"startup_limit.expired_hint", U"Esc またはウィンドウを閉じて終了してください", U"Press Esc or close the window to exit");

		while (System::Update())
		{
			if (KeyEscape.down())
			{
				break;
			}

			Scene::Rect().draw(ColorF{ U"#011B05" });
			const RectF panel{ Scene::Center().x - 420, Scene::Center().y - 120, 840, 240 };
			panel.draw(ColorF{ 0.06, 0.10, 0.08, 0.96 });
			panel.drawFrame(2, ColorF{ 0.58, 0.24, 0.24, 1.0 });
			titleFont(title).drawAt(Scene::Center().movedBy(0, -54), Palette::White);
			bodyFont(body).drawAt(Scene::Center().movedBy(0, 2), ColorF{ 1.0, 0.90, 0.90 });
			hintFont(hint).drawAt(Scene::Center().movedBy(0, 72), ColorF{ 0.82, 0.88, 0.84 });
		}
	}
}

void Main()
{
	const PersistentGameSettings settings = GameSettings::GetGameSettings();
	Localization::InitializeLanguage(settings.language);
	ApplyDisplaySettings(settings.displaySettings);
	Window::SetTitle(U"LoganToga2Remake2");
	s3d::Addon::Register<WindowChromeAddon>(WindowChromeAddon::AddonName);
	WindowChromeAddon::Configure(U"LoganToga2Remake2");
	s3d::GlobalAudio::SetVolume(settings.masterVolume);
	s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::BgmBus, settings.bgmVolume);
	s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::SeBus, settings.seVolume);
	Window::SetFullscreen(settings.fullscreen);
	//Scene::SetBackground(ColorF{ 0.11, 0.13, 0.16 });
	Scene::SetBackground(ColorF{ U"#011B05" });
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	//if (IsStartupExpired())
	//{
	//	RunStartupExpiredScreen();
	//	return;
	//}

	App manager;
	manager.get()->displaySettings = settings.displaySettings;
	manager.add<TitleScene>(U"Title");
	manager.add<BattleScene>(U"Battle");
	manager.add<BalanceEditScene>(U"BalanceEdit");
	manager.add<BonusRoomScene>(U"BonusRoom");
	manager.add<MapEditScene>(U"MapEdit");
	manager.add<RewardScene>(U"Reward");
	manager.add<RewardEditorScene>(U"RewardEditor");
	manager.add<TitleUiEditorScene>(U"TitleUiEditor");

	while (System::Update())
	{
		if (!manager.update())
		{
			break;
		}
	}
}
