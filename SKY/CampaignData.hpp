# pragma once
# include <Siv3D.hpp>
# include "MainContext.hpp"

namespace SkyCampaign
{
	inline constexpr FilePathView CampaignDirectoryPath = U"settings/campaigns";
	inline constexpr FilePathView CampaignProgressDirectoryPath = U"settings/campaign_progress";

	struct CampaignMissionDefinition
	{
		String displayName = U"Mission 1";
		FilePath mapFile = FilePath{ MainSupport::MapDataPath };
	};

	struct CampaignDefinition
	{
		String id;
		String displayName = U"New Campaign";
		String description;
		bool visibleInTitle = true;
		Array<CampaignMissionDefinition> missions{ CampaignMissionDefinition{} };
	};

	struct CampaignProgress
	{
		String campaignId;
		size_t unlockedMissionCount = 1;
		size_t currentMissionIndex = 0;
		bool completed = false;
	};

	namespace Detail
	{
		[[nodiscard]] inline String EscapeTomlString(const StringView value)
		{
			String escaped;
			escaped.reserve(value.size());

			for (const auto ch : value)
			{
				switch (ch)
				{
				case U'\\':
					escaped += U"\\\\";
					break;
				case U'\"':
					escaped += U"\\\"";
					break;
				case U'\r':
				case U'\n':
					escaped += U' ';
					break;
				default:
					escaped.push_back(ch);
					break;
				}
			}

			return escaped;
		}

		[[nodiscard]] inline String SanitizeCampaignId(const StringView value)
		{
			String result;
			bool lastWasSeparator = false;

			for (auto ch : value)
			{
				if ((U'a' <= ch) && (ch <= U'z'))
				{
					result.push_back(ch);
					lastWasSeparator = false;
				}
				else if ((U'A' <= ch) && (ch <= U'Z'))
				{
					result.push_back(static_cast<char32>(ch - U'A' + U'a'));
					lastWasSeparator = false;
				}
				else if ((U'0' <= ch) && (ch <= U'9'))
				{
					result.push_back(ch);
					lastWasSeparator = false;
				}
				else if ((ch == U' ') || (ch == U'_') || (ch == U'-'))
				{
					if ((not result.isEmpty()) && (not lastWasSeparator))
					{
						result.push_back(U'_');
						lastWasSeparator = true;
					}
				}
			}

			while (result.ends_with(U"_"))
			{
				result.pop_back();
			}

			if (result.isEmpty())
			{
				return U"campaign";
			}

			return result;
		}

		[[nodiscard]] inline String MakeCampaignFilePath(const StringView id)
		{
			return FileSystem::PathAppend(CampaignDirectoryPath, (String{ id } + U".campaign.toml"));
		}

		[[nodiscard]] inline String EnsureUniqueCampaignId(const StringView baseId)
		{
			String candidate = SanitizeCampaignId(baseId);
			int32 suffix = 2;

			while (FileSystem::Exists(MakeCampaignFilePath(candidate)))
			{
				candidate = U"{}_{}"_fmt(SanitizeCampaignId(baseId), suffix);
				++suffix;
			}

			return candidate;
		}

		[[nodiscard]] inline String MakeCampaignProgressFilePath(const StringView id)
		{
			return FileSystem::PathAppend(CampaignProgressDirectoryPath, (String{ id } + U".progress.toml"));
		}
	}

	[[nodiscard]] inline Array<CampaignDefinition> LoadCampaignDefinitions()
	{
		FileSystem::CreateDirectories(CampaignDirectoryPath);
		Array<CampaignDefinition> campaigns;

		for (const auto& filePath : FileSystem::DirectoryContents(CampaignDirectoryPath))
		{
			if ((not FileSystem::IsFile(filePath)) || (not FileSystem::FileName(filePath).ends_with(U".campaign.toml")))
			{
				continue;
			}

			const TOMLReader toml{ filePath };
			if (not toml)
			{
				continue;
			}

			CampaignDefinition definition;
			definition.id = toml[U"id"].getOpt<String>().value_or(FileSystem::BaseName(FileSystem::FileName(filePath)));
			definition.displayName = toml[U"displayName"].getOpt<String>().value_or(definition.id);
			definition.description = toml[U"description"].getOpt<String>().value_or(U"");
			definition.visibleInTitle = toml[U"visibleInTitle"].getOpt<bool>().value_or(true);
			definition.missions.clear();

			try
			{
				for (const auto& missionTable : toml[U"missions"].tableArrayView())
				{
					definition.missions << CampaignMissionDefinition{
						.displayName = missionTable[U"displayName"].getOpt<String>().value_or(U"Mission {}"_fmt(definition.missions.size() + 1)),
						.mapFile = missionTable[U"mapFile"].getOpt<String>().value_or(FilePath{ MainSupport::MapDataPath }),
					};
				}
			}
			catch (const std::exception&)
			{
			}

			if (definition.missions.isEmpty())
			{
				definition.missions << CampaignMissionDefinition{};
			}

			if (definition.visibleInTitle)
			{
				campaigns << definition;
			}
		}

		std::sort(campaigns.begin(), campaigns.end(), [](const CampaignDefinition& a, const CampaignDefinition& b)
			{
				return a.displayName < b.displayName;
			});
		return campaigns;
	}

	[[nodiscard]] inline Optional<CampaignDefinition> LoadCampaignDefinitionById(const StringView campaignId)
	{
		const TOMLReader toml{ Detail::MakeCampaignFilePath(campaignId) };
		if (not toml)
		{
			return none;
		}

		CampaignDefinition definition;
		definition.id = toml[U"id"].getOpt<String>().value_or(String{ campaignId });
		definition.displayName = toml[U"displayName"].getOpt<String>().value_or(definition.id);
		definition.description = toml[U"description"].getOpt<String>().value_or(U"");
		definition.visibleInTitle = toml[U"visibleInTitle"].getOpt<bool>().value_or(true);
		definition.missions.clear();

		try
		{
			for (const auto& missionTable : toml[U"missions"].tableArrayView())
			{
				definition.missions << CampaignMissionDefinition{
					.displayName = missionTable[U"displayName"].getOpt<String>().value_or(U"Mission {}"_fmt(definition.missions.size() + 1)),
					.mapFile = missionTable[U"mapFile"].getOpt<String>().value_or(FilePath{ MainSupport::MapDataPath }),
				};
			}
		}
		catch (const std::exception&)
		{
		}

		if (definition.missions.isEmpty())
		{
			definition.missions << CampaignMissionDefinition{};
		}

		return definition;
	}

	inline bool SaveCampaignDefinition(CampaignDefinition& definition)
	{
		if (definition.displayName.trimmed().isEmpty())
		{
			return false;
		}

		if (definition.id.isEmpty())
		{
			definition.id = Detail::EnsureUniqueCampaignId(definition.displayName);
		}

		if (definition.missions.isEmpty())
		{
			definition.missions << CampaignMissionDefinition{};
		}

		FileSystem::CreateDirectories(CampaignDirectoryPath);
		TextWriter writer{ Detail::MakeCampaignFilePath(definition.id) };
		if (not writer)
		{
			return false;
		}

		writer.writeln(U"id = \"{}\""_fmt(Detail::EscapeTomlString(definition.id)));
		writer.writeln(U"displayName = \"{}\""_fmt(Detail::EscapeTomlString(definition.displayName.trimmed())));
		writer.writeln(U"description = \"{}\""_fmt(Detail::EscapeTomlString(definition.description.trimmed())));
		writer.writeln(U"visibleInTitle = {}"_fmt(definition.visibleInTitle ? U"true" : U"false"));
		writer.writeln(U"");

		for (size_t i = 0; i < definition.missions.size(); ++i)
		{
			const CampaignMissionDefinition& mission = definition.missions[i];
			writer.writeln(U"[[missions]]");
			writer.writeln(U"displayName = \"{}\""_fmt(Detail::EscapeTomlString(mission.displayName.isEmpty() ? U"Mission {}"_fmt(i + 1) : mission.displayName)));
			writer.writeln(U"mapFile = \"{}\""_fmt(Detail::EscapeTomlString(mission.mapFile.isEmpty() ? FilePath{ MainSupport::MapDataPath } : mission.mapFile)));
			writer.writeln(U"");
		}

		return true;
	}

	[[nodiscard]] inline bool HasCampaignProgress(const StringView campaignId)
	{
		return FileSystem::Exists(Detail::MakeCampaignProgressFilePath(campaignId));
	}

	[[nodiscard]] inline CampaignProgress LoadCampaignProgress(const StringView campaignId)
	{
		CampaignProgress progress;
		progress.campaignId = campaignId;
		const TOMLReader toml{ Detail::MakeCampaignProgressFilePath(campaignId) };

		if (not toml)
		{
			return progress;
		}

		progress.unlockedMissionCount = static_cast<size_t>(Max<int64>(1, toml[U"unlockedMissionCount"].getOpt<int64>().value_or(1)));
		progress.currentMissionIndex = static_cast<size_t>(Max<int64>(0, toml[U"currentMissionIndex"].getOpt<int64>().value_or(0)));
		progress.completed = toml[U"completed"].getOpt<bool>().value_or(false);
		return progress;
	}

	inline bool SaveCampaignProgress(const CampaignProgress& progress)
	{
		if (progress.campaignId.isEmpty())
		{
			return false;
		}

		FileSystem::CreateDirectories(CampaignProgressDirectoryPath);
		TextWriter writer{ Detail::MakeCampaignProgressFilePath(progress.campaignId) };
		if (not writer)
		{
			return false;
		}

		writer.writeln(U"campaignId = \"{}\""_fmt(Detail::EscapeTomlString(progress.campaignId)));
		writer.writeln(U"unlockedMissionCount = {}"_fmt(Max<size_t>(1, progress.unlockedMissionCount)));
		writer.writeln(U"currentMissionIndex = {}"_fmt(progress.currentMissionIndex));
		writer.writeln(U"completed = {}"_fmt(progress.completed ? U"true" : U"false"));
		return true;
	}

	inline bool DeleteCampaign(const StringView campaignId)
	{
		bool removed = false;

		const String definitionPath = Detail::MakeCampaignFilePath(campaignId);
		if (FileSystem::Exists(definitionPath))
		{
			removed = FileSystem::Remove(definitionPath);
		}

		const String progressPath = Detail::MakeCampaignProgressFilePath(campaignId);
		if (FileSystem::Exists(progressPath))
		{
			removed = FileSystem::Remove(progressPath) || removed;
		}

		return removed;
	}
}
