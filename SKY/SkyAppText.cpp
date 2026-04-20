# include "SkyAppText.hpp"

namespace SkyAppText
{
    namespace
    {
        using Table = HashTable<String, String>;

        struct State
        {
            Language current = Language::Japanese;
            Language fallback = Language::English;
            std::array<Table, 2> tables{};
            bool loaded = false;
        };

        [[nodiscard]] State& GetState()
        {
            static State state;
            return state;
        }

        [[nodiscard]] constexpr size_t LanguageIndex(const Language language)
        {
            return (language == Language::Japanese) ? 0 : 1;
        }

        [[nodiscard]] FilePathView LanguageFilePath(const Language language)
        {
            switch (language)
            {
            case Language::Japanese: return U"locale/strings.ja.toml";
            case Language::English:
            default:                 return U"locale/strings.en.toml";
            }
        }

        void LoadLanguageTable(const Language language, Table& table)
        {
            table.clear();
            const TOMLReader toml{ FilePath{ LanguageFilePath(language) } };
            if (not toml)
            {
                return;
            }

            for (const auto& member : toml.tableView())
            {
                if (member.value.getType() == TOMLValueType::String)
                {
                    table.emplace(String{ member.name }, member.value.getString());
                }
            }
        }

        void EnsureLoaded()
        {
            State& state = GetState();
            if (state.loaded)
            {
                return;
            }

            LoadLanguageTable(Language::Japanese, state.tables[LanguageIndex(Language::Japanese)]);
            LoadLanguageTable(Language::English, state.tables[LanguageIndex(Language::English)]);
            state.loaded = true;
        }

        [[nodiscard]] Optional<String> TryLookup(const Language language, const StringView key)
        {
            const Table& table = GetState().tables[LanguageIndex(language)];
            if (const auto it = table.find(String{ key }); it != table.end())
            {
                if (not it->second.isEmpty())
                {
                    return it->second;
                }
            }

            return none;
        }
    }

    void Initialize()
    {
        State& state = GetState();
        state.loaded = false;
        EnsureLoaded();
    }

    void SetLanguage(const Language language)
    {
        GetState().current = language;
    }

    void SetFallbackLanguage(const Language language)
    {
        GetState().fallback = language;
    }

    Language CurrentLanguage()
    {
        return GetState().current;
    }

    Language FallbackLanguage()
    {
        return GetState().fallback;
    }

    String Tr(const StringView key)
    {
        EnsureLoaded();

        const State& state = GetState();
        if (auto text = TryLookup(state.current, key))
        {
            return *text;
        }

        if (state.current != state.fallback)
        {
            if (auto text = TryLookup(state.fallback, key))
            {
                return *text;
            }
        }

        return U"[MissingText:{0}]"_fmt(key);
    }
}
