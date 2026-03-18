#include "RewardUiLayout.h"

namespace
{
	void AppendTomlLine(String& content, const String& key, const String& value)
	{
		content += key + U" = " + value + U"\n";
	}

	void AppendVec2Section(String& content, const String& section, const Vec2& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
	}

	void AppendRectSection(String& content, const String& section, const RectF& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
		AppendTomlLine(content, U"w", Format(value.w));
		AppendTomlLine(content, U"h", Format(value.h));
	}

	void TryLoadVec2Section(const TOMLReader& toml, const String& section, Vec2& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
		}
		catch (const std::exception&)
		{
		}
	}

	void TryLoadRectSection(const TOMLReader& toml, const String& section, RectF& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
			value.w = Max(8.0, toml[section][U"w"].get<double>());
			value.h = Max(8.0, toml[section][U"h"].get<double>());
		}
		catch (const std::exception&)
		{
		}
	}

	void RepairRectSizeFromDefault(RectF& value, const RectF& defaults, const double minWidth, const double minHeight)
	{
		if (value.w < minWidth)
		{
			value.w = defaults.w;
		}

		if (value.h < minHeight)
		{
			value.h = defaults.h;
		}
	}

	[[nodiscard]] String BuildRewardUiLayoutTomlContentInternal(const RewardUiLayout& layout)
	{
		String content;
		AppendTomlLine(content, U"schemaVersion", Format(RewardUi::SchemaVersion));
		AppendVec2Section(content, U"title", layout.titlePos);
		AppendVec2Section(content, U"subtitle", layout.subtitlePos);
		AppendVec2Section(content, U"hint", layout.hintPos);
		AppendRectSection(content, U"card1", layout.card1Rect);
		AppendRectSection(content, U"card2", layout.card2Rect);
		AppendRectSection(content, U"card3", layout.card3Rect);
		AppendVec2Section(content, U"acquiredLabel", layout.acquiredLabelPos);
		AppendVec2Section(content, U"acquiredCardName", layout.acquiredCardNamePos);
		return content;
	}

	RewardUiLayout& GetRewardUiLayoutStorage()
	{
		static RewardUiLayout layout = RewardUi::LoadRewardUiLayoutFromDisk();
		return layout;
	}
}

namespace RewardUi
{
	String GetLocalLayoutPath()
	{
		return U"save/reward_ui_layout.toml";
	}

	String GetAppDataLayoutPath()
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(SpecialFolder::LocalAppData), U"LoganToga2Remake2/save/reward_ui_layout.toml");
	}

	String GetLayoutPathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataLayoutPath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalLayoutPath();
		}
	}

	String GetLayoutPath()
	{
		return GetLayoutPathForLocation(GetContinueRunSaveLocation());
	}

	RewardUiLayout MakeDefaultRewardUiLayout()
	{
		return {};
	}

	void RepairRewardUiLayout(RewardUiLayout& layout)
	{
		const RewardUiLayout defaults = MakeDefaultRewardUiLayout();
		RepairRectSizeFromDefault(layout.card1Rect, defaults.card1Rect, 180.0, 180.0);
		RepairRectSizeFromDefault(layout.card2Rect, defaults.card2Rect, 180.0, 180.0);
		RepairRectSizeFromDefault(layout.card3Rect, defaults.card3Rect, 180.0, 180.0);
	}

	RewardUiLayout LoadRewardUiLayoutFromDisk()
	{
		RewardUiLayout layout = MakeDefaultRewardUiLayout();
		const TOMLReader toml{ GetLayoutPath() };
		if (!toml)
		{
			return layout;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != SchemaVersion)
			{
				return MakeDefaultRewardUiLayout();
			}
		}
		catch (const std::exception&)
		{
			return MakeDefaultRewardUiLayout();
		}

		TryLoadVec2Section(toml, U"title", layout.titlePos);
		TryLoadVec2Section(toml, U"subtitle", layout.subtitlePos);
		TryLoadVec2Section(toml, U"hint", layout.hintPos);
		TryLoadRectSection(toml, U"card1", layout.card1Rect);
		TryLoadRectSection(toml, U"card2", layout.card2Rect);
		TryLoadRectSection(toml, U"card3", layout.card3Rect);
		TryLoadVec2Section(toml, U"acquiredLabel", layout.acquiredLabelPos);
		TryLoadVec2Section(toml, U"acquiredCardName", layout.acquiredCardNamePos);
		RepairRewardUiLayout(layout);
		return layout;
	}

	const RewardUiLayout& GetRewardUiLayout()
	{
		return GetRewardUiLayoutStorage();
	}

	RewardUiLayout ReloadRewardUiLayout()
	{
		GetRewardUiLayoutStorage() = LoadRewardUiLayoutFromDisk();
		return GetRewardUiLayoutStorage();
	}

	bool SaveRewardUiLayout(const RewardUiLayout& layout)
	{
		const String path = GetLayoutPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildRewardUiLayoutTomlContentInternal(layout));
		GetRewardUiLayoutStorage() = layout;
		return true;
	}
}
