#pragma once

#include "Localization.Types.h"
#include <algorithm>
#include <utility>

namespace Localization
{
	inline constexpr StringView LocalizationIndexFileName = U"index.toml";

	[[nodiscard]] inline String GetLocalizationDirectoryPath()
	{
		const String runtimeRelativePath = U"localization";
		if (FileSystem::Exists(runtimeRelativePath))
		{
			return runtimeRelativePath;
		}

		const String projectRelativePath = U"App/localization";
		if (FileSystem::Exists(projectRelativePath))
		{
			return projectRelativePath;
		}

		return runtimeRelativePath;
	}

	[[nodiscard]] inline String GetLocalizationIndexPath()
	{
		return FileSystem::PathAppend(GetLocalizationDirectoryPath(), String{ LocalizationIndexFileName });
	}

	[[nodiscard]] inline String GetLanguageCodeFromPath(const String& resourcePath)
	{
		return FileSystem::BaseName(FileSystem::FileName(resourcePath));
	}

	template <class TomlLike>
	[[nodiscard]] inline String ReadTomlString(const TomlLike& toml, const String& key, const String& fallback)
	{
		try
		{
			const String value = toml[key].get<String>();
			return value.isEmpty() ? fallback : value;
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	template <class TomlLike>
	[[nodiscard]] inline int32 ReadTomlInt(const TomlLike& toml, const String& key, const int32 fallback)
	{
		try
		{
			return toml[key].get<int32>();
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	template <class TomlLike>
	[[nodiscard]] inline Array<String> ReadTomlStringArray(const TomlLike& toml, const String& key)
	{
		Array<String> values;
		try
		{
			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<String>();
			}
		}
		catch (const std::exception&)
		{
		}

		return values;
	}

	[[nodiscard]] inline bool ContainsPersistenceLabel(const LanguageDefinition& definition, const String& label)
	{
		if (definition.language == label)
		{
			return true;
		}

		for (const auto& alias : definition.persistenceAliases)
		{
			if (alias == label)
			{
				return true;
			}
		}

		return false;
	}

	[[nodiscard]] inline void EnsureCanonicalPersistenceLabel(LanguageDefinition& definition)
	{
		for (const auto& alias : definition.persistenceAliases)
		{
			if (alias == definition.language)
			{
				return;
			}
		}

		definition.persistenceAliases << definition.language;
	}

	[[nodiscard]] inline LanguageDefinition BuildLanguageDefinitionFromResourcePath(const String& resourcePath)
	{
		const String fallbackCode = GetLanguageCodeFromPath(resourcePath);
		s3d::TOMLReader toml{ resourcePath };
		if (!toml)
		{
			return { fallbackCode, fallbackCode, resourcePath, 1000, { fallbackCode } };
		}

		LanguageDefinition definition
		{
			ReadTomlString(toml, U"_meta.code", fallbackCode),
			ReadTomlString(toml, U"_meta.displayName", fallbackCode),
			resourcePath,
			ReadTomlInt(toml, U"_meta.order", 1000),
			ReadTomlStringArray(toml, U"_meta.aliases")
		};
		EnsureCanonicalPersistenceLabel(definition);
		return definition;
	}

	[[nodiscard]] inline Array<LanguageDefinition> BuildLanguageDefinitionsFromIndex()
	{
		Array<LanguageDefinition> definitions;
		const String indexPath = GetLocalizationIndexPath();
		if (!FileSystem::Exists(indexPath))
		{
			return definitions;
		}

		s3d::TOMLReader toml{ indexPath };
		if (!toml)
		{
			return definitions;
		}

		const String localizationDirectory = GetLocalizationDirectoryPath();
		for (const auto& table : toml[U"languages"].tableArrayView())
		{
			const String languageCode = ReadTomlString(table, U"code", U"");
			const String relativeFilePath = ReadTomlString(table, U"file", U"");
			if (languageCode.isEmpty() || relativeFilePath.isEmpty())
			{
				continue;
			}

			LanguageDefinition definition
			{
				languageCode,
				ReadTomlString(table, U"displayName", languageCode),
				FileSystem::PathAppend(localizationDirectory, relativeFilePath),
				ReadTomlInt(table, U"order", 1000),
				ReadTomlStringArray(table, U"aliases")
			};
			EnsureCanonicalPersistenceLabel(definition);
			definitions << std::move(definition);
		}

		return definitions;
	}

	[[nodiscard]] inline Array<LanguageDefinition> BuildLanguageDefinitionsFromDirectory()
	{
		Array<LanguageDefinition> definitions;
		for (const auto& path : FileSystem::DirectoryContents(GetLocalizationDirectoryPath()))
		{
			if (!FileSystem::IsFile(path))
			{
				continue;
			}

			if (!FileSystem::FileName(path).ends_with(U".toml") || (FileSystem::FileName(path) == LocalizationIndexFileName))
			{
				continue;
			}

			definitions << BuildLanguageDefinitionFromResourcePath(path);
		}

		return definitions;
	}

	[[nodiscard]] inline Array<LanguageDefinition> BuildLanguageDefinitions()
	{
		Array<LanguageDefinition> definitions = BuildLanguageDefinitionsFromIndex();
		if (definitions.isEmpty())
		{
			definitions = BuildLanguageDefinitionsFromDirectory();
		}

		std::sort(definitions.begin(), definitions.end(), [](const LanguageDefinition& a, const LanguageDefinition& b)
		{
			if (a.sortOrder != b.sortOrder)
			{
				return a.sortOrder < b.sortOrder;
			}

			return a.language < b.language;
		});
		return definitions;
	}

	[[nodiscard]] inline const Array<LanguageDefinition>& GetLanguageDefinitions()
	{
		static const Array<LanguageDefinition> definitions = BuildLanguageDefinitions();
		return definitions;
	}

	[[nodiscard]] inline const LanguageDefinition& EmptyLanguageDefinitionStorage()
	{
		static const LanguageDefinition definition{ U"", U"", U"", 1000, {} };
		return definition;
	}

	[[nodiscard]] inline AppLanguage BuildDefaultLanguage()
	{
		const String indexPath = GetLocalizationIndexPath();
		if (FileSystem::Exists(indexPath))
		{
			s3d::TOMLReader toml{ indexPath };
			if (toml)
			{
				const String configuredDefault = ReadTomlString(toml, U"_meta.defaultLanguage", U"");
				for (const auto& definition : GetLanguageDefinitions())
				{
					if (definition.language == configuredDefault)
					{
						return definition.language;
					}
				}
			}
		}

		const auto& definitions = GetLanguageDefinitions();
		if (!definitions.isEmpty())
		{
			return definitions.front().language;
		}

		return U"";
	}

	[[nodiscard]] inline const AppLanguage& GetDefaultLanguage()
	{
		static const AppLanguage defaultLanguage = BuildDefaultLanguage();
		return defaultLanguage;
	}

	[[nodiscard]] inline const LanguageDefinition& GetLanguageDefinition(const AppLanguage& language)
	{
		for (const auto& definition : GetLanguageDefinitions())
		{
			if (definition.language == language)
			{
				return definition;
			}
		}

		const auto& definitions = GetLanguageDefinitions();
		if (!definitions.isEmpty())
		{
			return definitions.front();
		}

		return EmptyLanguageDefinitionStorage();
	}

	[[nodiscard]] inline AppLanguage ParsePersistenceLabel(const String& label)
	{
		for (const auto& definition : GetLanguageDefinitions())
		{
			if (ContainsPersistenceLabel(definition, label))
			{
				return definition.language;
			}
		}

		if (!label.isEmpty())
		{
			return GetDefaultLanguage();
		}

		return GetDefaultLanguage();
	}

	[[nodiscard]] inline String GetPersistenceLabel(const AppLanguage& language)
	{
		const AppLanguage normalizedLanguage = ParsePersistenceLabel(language);
		return normalizedLanguage.isEmpty() ? language : normalizedLanguage;
	}

	[[nodiscard]] inline const String& GetDisplayName(const AppLanguage& language)
	{
		return GetLanguageDefinition(ParsePersistenceLabel(language)).displayName;
	}
}
