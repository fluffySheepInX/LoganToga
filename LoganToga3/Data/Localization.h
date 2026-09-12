#pragma once
# include <Siv3D.hpp>
# include "TomlTextUtils.h"

namespace LT3
{
	inline constexpr StringView DefaultLocale = U"ja-JP";

	struct LocalizationCatalog
	{
		String locale = String{ DefaultLocale };
		HashTable<String, String> texts;
	};

	// UI ローカライズ対象のキー一覧を返します。
	inline const Array<String>& LocalizationTextKeys()
	{
		static const Array<String> keys = {
			U"title.skirmish",
			U"battle.result.draw",
			U"battle.result.victory",
			U"battle.result.defeat",
			U"battle.result.aborted",
			U"battle.result.exit_hint",
			U"battle.time_limit",
			U"battle.capture_remaining_seconds",
			U"music_editor.title",
			U"music_editor.description",
			U"music_editor.show",
			U"music_editor.hide",
			U"music_editor.selected_scene",
			U"music_editor.file",
			U"music_editor.volume",
			U"music_editor.preview_playing",
			U"music_editor.preview_stopped",
			U"music_editor.unsaved_changes",
			U"music_editor.saved",
			U"music_editor.browse",
			U"music_editor.preview",
			U"music_editor.stop",
			U"music_editor.clear",
			U"music_editor.save_settings",
			U"music_editor.reload",
			U"music_editor.status_selected_scene",
			U"music_editor.status_assigned",
			U"music_editor.status_preview_stopped",
			U"music_editor.status_cleared",
			U"music_editor.status_volume",
		};
		return keys;
	}

	// 言語リソース TOML のパスを解決します。
	inline FilePath ResolveLocalizationTomlPath(StringView locale)
	{
		const String fileName = String{ locale } + U".toml";
		return ResolveFirstExistingPath({
			U"000_Warehouse/000_DefaultGame/070_Scenario/InfoLocalization/" + fileName,
			U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoLocalization/" + fileName,
			U"LoganToga3/App/000_Warehouse/000_DefaultGame/070_Scenario/InfoLocalization/" + fileName,
		});
	}

	// 指定ロケールの言語リソースを読み込みます。
	inline bool LoadLocalizationToml(StringView locale, LocalizationCatalog& catalog)
	{
		catalog = LocalizationCatalog{};
		catalog.locale = String{ locale };
		const TOMLReader toml{ ResolveLocalizationTomlPath(locale) };
		if (!toml)
		{
			return false;
		}

		for (const String& key : LocalizationTextKeys())
		{
			const Optional<String> value = toml[U"text." + key].getOpt<String>();
			if (value)
			{
				catalog.texts.emplace(key, *value);
			}
		}
		return true;
	}

	// 指定ロケールと標準ロケールを読み込みます。
	inline void LoadLocalizationCatalogs(StringView locale, LocalizationCatalog& localized, LocalizationCatalog& defaults)
	{
		LoadLocalizationToml(DefaultLocale, defaults);
		if (locale == DefaultLocale)
		{
			localized = defaults;
			return;
		}
		LoadLocalizationToml(locale, localized);
	}

	// キーに対応する表示文言を返します。
	inline String LocalizedText(const LocalizationCatalog& localized, const LocalizationCatalog& defaults, StringView key)
	{
		const String lookupKey{ key };
		if (const auto it = localized.texts.find(lookupKey); it != localized.texts.end())
		{
			return it->second;
		}
		if (const auto it = defaults.texts.find(lookupKey); it != defaults.texts.end())
		{
			return it->second;
		}
		return lookupKey;
	}

	// 1 個のプレースホルダーを含む表示文言を整形します。
	inline String FormatLocalizedText(const LocalizationCatalog& localized, const LocalizationCatalog& defaults, StringView key, StringView value)
	{
		return LocalizedText(localized, defaults, key).replaced(U"{}", value);
	}

	// 2 個のプレースホルダーを含む表示文言を整形します。
	inline String FormatLocalizedText(const LocalizationCatalog& localized, const LocalizationCatalog& defaults, StringView key, StringView firstValue, StringView secondValue)
	{
		return LocalizedText(localized, defaults, key).replaced(U"{}", firstValue).replaced(U"{}", secondValue);
	}
}
