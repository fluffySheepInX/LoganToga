# include "MapDataTomlInternal.hpp"
# include <fstream>

namespace MapDataTomlDetail
{
    void WriteTomlVec3(TextWriter& writer, const String& key, const Vec3& value)
    {
        writer.writeln(U"{} = [{:.3f}, {:.3f}, {:.3f}]"_fmt(key, value.x, value.y, value.z));
    }

    void AppendLoadMessage(String& message, const StringView detail)
    {
        if (message.isEmpty())
        {
            message = detail;
            return;
        }

        message += U" / ";
        message += detail;
    }

    bool HasTomlTableArraySection(FilePathView path, const String& key)
    {
        std::ifstream reader{ Unicode::ToUTF8(FileSystem::FullPath(path)) };

        if (not reader)
        {
            return false;
        }

        const std::string sectionHeader = ("[[" + Unicode::ToUTF8(key) + "]]");
        std::string line;

        while (std::getline(reader, line))
        {
            if ((3 <= line.size())
                && (static_cast<unsigned char>(line[0]) == 0xEF)
                && (static_cast<unsigned char>(line[1]) == 0xBB)
                && (static_cast<unsigned char>(line[2]) == 0xBF))
            {
                line.erase(0, 3);
            }

            const size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos)
            {
                continue;
            }

            const size_t end = line.find_last_not_of(" \t");
            if (line.substr(start, (end - start + 1)) == sectionHeader)
            {
                return true;
            }
        }

        return false;
    }

    namespace
    {
        template <class T, size_t N>
        [[nodiscard]] Optional<T> LookupName(const std::array<std::pair<StringView, T>, N>& table, const StringView name)
        {
            for (const auto& [n, v] : table)
            {
                if (n == name) return v;
            }
            return none;
        }
    }

    Optional<PlaceableModelType> ParsePlaceableModelType(const String& value)
    {
        static constexpr std::array<std::pair<StringView, PlaceableModelType>, 8> Table{ {
            { U"Mill",           PlaceableModelType::Mill },
            { U"Tree",           PlaceableModelType::Tree },
            { U"Pine",           PlaceableModelType::Pine },
            { U"GrassPatch",     PlaceableModelType::GrassPatch },
            { U"Rock",           PlaceableModelType::Rock },
            { U"Wall",           PlaceableModelType::Wall },
            { U"Road",           PlaceableModelType::Road },
            { U"TireTrackDecal", PlaceableModelType::TireTrackDecal },
        } };
        return LookupName(Table, value);
    }

    Optional<MainSupport::UnitTeam> ParseUnitTeam(const String& value)
    {
        static constexpr std::array<std::pair<StringView, MainSupport::UnitTeam>, 2> Table{ {
            { U"Player", MainSupport::UnitTeam::Player },
            { U"Enemy",  MainSupport::UnitTeam::Enemy },
        } };
        return LookupName(Table, value);
    }

    Optional<TerrainCellType> ParseTerrainCellType(const String& value)
    {
        static constexpr std::array<std::pair<StringView, TerrainCellType>, 4> Table{ {
            { U"Grass", TerrainCellType::Grass },
            { U"Dirt",  TerrainCellType::Dirt },
            { U"Sand",  TerrainCellType::Sand },
            { U"Rock",  TerrainCellType::Rock },
        } };
        return LookupName(Table, value);
    }

    Optional<MainSupport::ResourceType> ParseResourceType(const String& value)
    {
        static constexpr std::array<std::pair<StringView, MainSupport::ResourceType>, 3> Table{ {
            { U"Budget",    MainSupport::ResourceType::Budget },
            { U"Gunpowder", MainSupport::ResourceType::Gunpowder },
            { U"Mana",      MainSupport::ResourceType::Mana },
        } };
        return LookupName(Table, value);
    }
}
