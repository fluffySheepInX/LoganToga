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
	};

	inline const AppLanguage DefaultLanguage = U"en";

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

	[[nodiscard]] inline String GetLanguageCodeFromPath(const String& resourcePath)
	{
		return FileSystem::BaseName(FileSystem::FileName(resourcePath));
	}

	[[nodiscard]] inline String ReadTomlString(const s3d::TOMLReader& toml, const String& key, const String& fallback)
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

	[[nodiscard]] inline int32 ReadTomlInt(const s3d::TOMLReader& toml, const String& key, const int32 fallback)
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

	[[nodiscard]] inline LanguageDefinition BuildLanguageDefinition(const String& resourcePath)
	{
		const String fallbackCode = GetLanguageCodeFromPath(resourcePath);
		s3d::TOMLReader toml{ resourcePath };
		if (!toml)
		{
			return { fallbackCode, fallbackCode, resourcePath, 1000 };
		}

		const String languageCode = ReadTomlString(toml, U"_meta.code", fallbackCode);
		const String displayName = ReadTomlString(toml, U"_meta.displayName", languageCode);
		const int32 sortOrder = ReadTomlInt(toml, U"_meta.order", 1000);
		return { languageCode, displayName, resourcePath, sortOrder };
	}

	[[nodiscard]] inline Array<LanguageDefinition> BuildLanguageDefinitions()
	{
		Array<LanguageDefinition> definitions;
		for (const auto& path : FileSystem::DirectoryContents(GetLocalizationDirectoryPath()))
		{
			if (!FileSystem::IsFile(path))
			{
				continue;
			}

			if (!FileSystem::FileName(path).ends_with(U".toml"))
			{
				continue;
			}

			definitions << BuildLanguageDefinition(path);
		}

		if (definitions.isEmpty())
		{
           const String localizationDirectory = GetLocalizationDirectoryPath();
			definitions =
			{
                { U"en", U"English", FileSystem::PathAppend(localizationDirectory, U"en.toml"), 0 },
				{ U"ja", U"日本語", FileSystem::PathAppend(localizationDirectory, U"ja.toml"), 10 },
			};
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

	[[nodiscard]] inline const LanguageDefinition& GetLanguageDefinition(const AppLanguage& language)
	{
		for (const auto& definition : GetLanguageDefinitions())
		{
			if (definition.language == language)
			{
				return definition;
			}
		}

		return GetLanguageDefinitions().front();
	}

	[[nodiscard]] inline AppLanguage& CurrentLanguageStorage()
	{
		static AppLanguage language = DefaultLanguage;
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
        if (CurrentLanguageStorage() == DefaultLanguage)
		{
			return LanguageTomlStorage().get();
		}

		auto& toml = DefaultLanguageTomlStorage();
		if (!toml)
		{
			const String resourcePath = GetLanguageDefinition(DefaultLanguage).resourcePath;
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
		if (!FileSystem::Exists(resourcePath))
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
		for (size_t i = 0; i < definitions.size(); ++i)
		{
			if (definitions[i].language == language)
			{
				return definitions[(i + 1) % definitions.size()].language;
			}
		}

		return DefaultLanguage;
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
		return ParsePersistenceLabel(language);
	}

	[[nodiscard]] inline AppLanguage ParsePersistenceLabel(const String& label)
	{
		for (const auto& definition : GetLanguageDefinitions())
		{
			if (definition.language == label)
			{
				return definition.language;
			}
		}

		if (label == U"Japanese")
		{
			return U"ja";
		}

		if (label == U"English")
		{
			return U"en";
		}

		return DefaultLanguage;
	}

	[[nodiscard]] inline const String& GetDisplayName(const AppLanguage& language)
	{
		return GetLanguageDefinition(ParsePersistenceLabel(language)).displayName;
	}

	[[nodiscard]] inline const String& GetCurrentDisplayName()
	{
		return GetDisplayName(GetLanguage());
	}

	[[nodiscard]] inline bool UsesJapaneseFallback(const AppLanguage& language)
	{
		return (ParsePersistenceLabel(language) == U"ja");
	}

	[[nodiscard]] inline String GetText(const String& key, const String& japanese, const String& english)
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

		if (UsesJapaneseFallback(GetLanguage()))
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

	[[nodiscard]] inline String Text(const String& japanese, const String& english)
	{
		return UsesJapaneseFallback(GetLanguage()) ? japanese : english;
	}
}

struct LocalizedText
{
	String key;
	String japanese;
	String english;

	LocalizedText() = default;

	LocalizedText(String keyValue, String japaneseValue, String englishValue)
		: key{ std::move(keyValue) }
		, japanese{ std::move(japaneseValue) }
		, english{ std::move(englishValue) }
	{
	}

	[[nodiscard]] String get() const
	{
		return Localization::GetText(key, japanese, english);
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
