#pragma once

#include "Localization.Registry.h"
#include <memory>

namespace Localization
{
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

	[[nodiscard]] inline const String& GetCurrentDisplayName()
	{
		return GetDisplayName(GetLanguage());
	}
}
