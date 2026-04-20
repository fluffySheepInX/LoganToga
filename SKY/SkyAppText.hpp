# pragma once
# include <Siv3D.hpp>

namespace SkyAppText
{
    enum class Language
    {
        Japanese,
        English,
    };

    // Loads / reloads localization tables from `locale/strings.<lang>.toml`.
    // Safe to call multiple times; called automatically on first Tr() use.
    void Initialize();

    void SetLanguage(Language language);
    void SetFallbackLanguage(Language language);

    [[nodiscard]] Language CurrentLanguage();
    [[nodiscard]] Language FallbackLanguage();

    // Look up a localized string by its TOML key.
    // Falls back to the fallback language, then to `[MissingText:<key>]`.
    [[nodiscard]] String Tr(StringView key);

    // Tr + positional `{0}`, `{1}` ... substitution (matches the
    // `_fmt` style used in the TOML resources).
    template <class... Args>
    [[nodiscard]] inline String TrFormat(const StringView key, const Args&... args)
    {
        String text = Tr(key);
        const Array<String> replacements = { Format(args)... };
        for (size_t i = 0; i < replacements.size(); ++i)
        {
            text.replace((U"{" + Format(i) + U"}"), replacements[i]);
        }
        return text;
    }
}
