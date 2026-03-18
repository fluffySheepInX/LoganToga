#pragma once

#include "Remake2Common.h"
#include <algorithm>
#include <memory>
#include <utility>

using AppLanguage = String;

namespace Localization
{
   struct LanguageDefinition
	{
		AppLanguage language;
		String displayName;
		String resourcePath;
		int32 sortOrder = 1000;
		Array<String> persistenceAliases;
	};

	inline constexpr StringView LocalizationIndexFileName = U"index.toml";

	[[nodiscard]] inline const AppLanguage& GetDefaultLanguage();
	[[nodiscard]] inline AppLanguage ParsePersistenceLabel(const String& label);

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

	[[nodiscard]] inline AppLanguage& CurrentLanguageStorage()
	{
      static AppLanguage language = GetDefaultLanguage();
		return language;
	}

	[[nodiscard]] inline bool& LanguageChangedStorage()
	{
		static bool changed = false;
		return changed;
	}

	[[nodiscard]] inline std::unique_ptr<s3d::TOMLReader>& LanguageTomlStorage()
	{
		static std::unique_ptr<s3d::TOMLReader> toml;
		return toml;
	}

	[[nodiscard]] inline std::unique_ptr<s3d::TOMLReader>& DefaultLanguageTomlStorage()
	{
		static std::unique_ptr<s3d::TOMLReader> toml;
		return toml;
	}

	[[nodiscard]] inline String TryReadLocalizedValue(const s3d::TOMLReader* toml, const String& key)
	{
		if (!toml)
		{
			return U"";
		}

		try
		{
          const String value = (*toml)[key].get<String>();
			if (!value.isEmpty())
			{
				return value;
			}
		}
		catch (const std::exception&)
		{
		}

		return U"";
	}

	[[nodiscard]] inline const s3d::TOMLReader* GetDefaultLanguageToml()
	{
        if (CurrentLanguageStorage() == GetDefaultLanguage())
		{
			return LanguageTomlStorage().get();
		}

		auto& toml = DefaultLanguageTomlStorage();
		if (!toml)
		{
            const String resourcePath = GetLanguageDefinition(GetDefaultLanguage()).resourcePath;
			if (!FileSystem::Exists(resourcePath))
			{
				return nullptr;
			}

			auto reader = std::make_unique<s3d::TOMLReader>(resourcePath);
			if (*reader)
			{
				toml = std::move(reader);
			}
		}

		return toml.get();
	}

	inline void ReloadLanguageToml(const AppLanguage& language)
	{
		const String resourcePath = GetLanguageDefinition(language).resourcePath;
		auto& toml = LanguageTomlStorage();
		toml.reset();
      if (resourcePath.isEmpty() || !FileSystem::Exists(resourcePath))
		{
			return;
		}

		auto reader = std::make_unique<s3d::TOMLReader>(resourcePath);
		if (*reader)
		{
			toml = std::move(reader);
		}
	}

	inline void InitializeLanguage(const AppLanguage& language)
	{
		CurrentLanguageStorage() = ParsePersistenceLabel(language);
		ReloadLanguageToml(CurrentLanguageStorage());
		LanguageChangedStorage() = false;
	}

	[[nodiscard]] inline AppLanguage GetLanguage()
	{
		return CurrentLanguageStorage();
	}

	inline void SetLanguage(const AppLanguage& language)
	{
		const AppLanguage normalizedLanguage = ParsePersistenceLabel(language);
		if (CurrentLanguageStorage() == normalizedLanguage)
		{
			return;
		}

		CurrentLanguageStorage() = normalizedLanguage;
		ReloadLanguageToml(normalizedLanguage);
		LanguageChangedStorage() = true;
	}

	[[nodiscard]] inline AppLanguage GetNextLanguage(const AppLanguage& language)
	{
		const auto& definitions = GetLanguageDefinitions();
     if (definitions.isEmpty())
		{
			return GetDefaultLanguage();
		}

		for (size_t i = 0; i < definitions.size(); ++i)
		{
			if (definitions[i].language == language)
			{
				return definitions[(i + 1) % definitions.size()].language;
			}
		}

     return GetDefaultLanguage();
	}

	inline void CycleLanguage()
	{
		SetLanguage(GetNextLanguage(GetLanguage()));
	}

	[[nodiscard]] inline bool ConsumeLanguageChanged()
	{
		const bool changed = LanguageChangedStorage();
		LanguageChangedStorage() = false;
		return changed;
	}

	[[nodiscard]] inline String GetPersistenceLabel(const AppLanguage& language)
	{
     const AppLanguage normalizedLanguage = ParsePersistenceLabel(language);
		return normalizedLanguage.isEmpty() ? language : normalizedLanguage;
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

	[[nodiscard]] inline const String& GetDisplayName(const AppLanguage& language)
	{
		return GetLanguageDefinition(ParsePersistenceLabel(language)).displayName;
	}

	[[nodiscard]] inline const String& GetCurrentDisplayName()
	{
		return GetDisplayName(GetLanguage());
	}

 [[nodiscard]] inline String TryGetText(const String& key)
	{
		const String localizedValue = TryReadLocalizedValue(LanguageTomlStorage().get(), key);
		if (!localizedValue.isEmpty())
		{
			return localizedValue;
		}

		const String defaultLanguageValue = TryReadLocalizedValue(GetDefaultLanguageToml(), key);
		if (!defaultLanguageValue.isEmpty())
		{
			return defaultLanguageValue;
		}

		return U"";
	}

	[[nodiscard]] inline String GetText(const String& key)
	{
		const String localizedValue = TryGetText(key);
		return localizedValue.isEmpty() ? key : localizedValue;
	}

	template <class... Args>
	[[nodiscard]] inline String FormatText(const String& key, const Args&... args)
	{
		String text = GetText(key);
		const Array<String> replacements = { s3d::Format(args)... };
		for (size_t i = 0; i < replacements.size(); ++i)
		{
			text.replace((U"{" + s3d::Format(i) + U"}"), replacements[i]);
		}
		return text;
	}

    namespace Legacy
	{
      [[nodiscard]] inline bool PrefersJapaneseFallbackText(const AppLanguage& language)
		{
          return (ParsePersistenceLabel(language) == U"ja");
		}

		[[nodiscard]] inline String SelectFallbackText(const String& japanese, const String& english)
		{
			if (PrefersJapaneseFallbackText(GetLanguage()))
			{
				if (!japanese.isEmpty())
				{
					return japanese;
				}

				return english;
			}

			if (!english.isEmpty())
			{
				return english;
			}

			return japanese;
		}

		[[nodiscard]] inline String GetText(const String& key, const String& japanese, const String& english)
		{
			const String localizedValue = TryGetText(key);
			return localizedValue.isEmpty() ? SelectFallbackText(japanese, english) : localizedValue;
		}

		template <class... Args>
		[[nodiscard]] inline String FormatText(const String& key, const String& japanese, const String& english, const Args&... args)
		{
			String text = GetText(key, japanese, english);
			const Array<String> replacements = { s3d::Format(args)... };
			for (size_t i = 0; i < replacements.size(); ++i)
			{
				text.replace((U"{" + s3d::Format(i) + U"}"), replacements[i]);
			}
			return text;
		}
	}
}

struct LocalizedText
{
	String key;
    String legacyJapanese;
	String legacyEnglish;

	LocalizedText() = default;

	LocalizedText(String keyValue)
		: key{ std::move(keyValue) }
	{
	}

	LocalizedText(String keyValue, String japaneseValue, String englishValue)
		: key{ std::move(keyValue) }
      , legacyJapanese{ std::move(japaneseValue) }
		, legacyEnglish{ std::move(englishValue) }
	{
	}

	[[nodiscard]] bool hasLegacyFallbackText() const
	{
		return !(legacyJapanese.isEmpty() && legacyEnglish.isEmpty());
	}

	[[nodiscard]] String get() const
	{
     if (!hasLegacyFallbackText())
		{
			return Localization::GetText(key);
		}

        return Localization::Legacy::GetText(key, legacyJapanese, legacyEnglish);
	}

	operator String() const
	{
		return get();
	}
};

namespace s3d
{
	inline void Formatter(FormatData& formatData, const ::LocalizedText& value)
	{
		Formatter(formatData, value.get());
	}
}
