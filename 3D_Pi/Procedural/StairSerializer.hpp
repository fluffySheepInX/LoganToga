# pragma once
# include <Siv3D.hpp>
# include <fstream>
# include "ProceduralDocument.hpp"

namespace procedural
{
    namespace
    {
        [[nodiscard]] bool HasTomlTableArraySection(FilePathView path, const String& key)
        {
            std::ifstream reader{ Unicode::ToUTF8(FileSystem::FullPath(path)) };

            if (not reader)
            {
                return false;
            }

            const std::string sectionHeader = "[[" + Unicode::ToUTF8(key) + "]]";
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
                if (line.substr(start, end - start + 1) == sectionHeader)
                {
                    return true;
                }
            }

            return false;
        }
    }

    inline void SaveProceduralData(const FilePath& savePath, const ProceduralDocument& document)
    {
        const FilePath directory = FileSystem::ParentPath(savePath);
        if (not directory.isEmpty())
        {
            FileSystem::CreateDirectories(directory);
        }

        TextWriter writer{ savePath };
        if (not writer)
        {
            return;
        }

        for (const auto& stair : document.stairs)
        {
            writer.writeln(U"[[stairs]]");
            writer.writeln(U"origin = [{:.3f}, {:.3f}, {:.3f}]"_fmt(stair.origin.x, stair.origin.y, stair.origin.z));
            writer.writeln(U"steps = {}"_fmt(stair.steps));
            writer.writeln(U"height = {:.4f}"_fmt(stair.height));
            writer.writeln(U"width = {:.4f}"_fmt(stair.width));
            writer.writeln(U"depth = {:.4f}"_fmt(stair.depth));
            writer.writeln(U"rotation01 = {:.5f}"_fmt(stair.rotation01));
            writer.writeln(U"color = [{:.4f}, {:.4f}, {:.4f}, {:.4f}]"_fmt(stair.color.r, stair.color.g, stair.color.b, stair.color.a));
            writer.writeln(U"useDullNoise = {}"_fmt(stair.useDullNoise ? U"true" : U"false"));
            writer.writeln(U"useColorVariation = {}"_fmt(stair.useColorVariation ? U"true" : U"false"));
            writer.writeln(U"dullNoiseAmount = {:.4f}"_fmt(stair.dullNoiseAmount));
            writer.writeln(U"colorVariationAmount = {:.4f}"_fmt(stair.colorVariationAmount));
            writer.writeln(U"");
        }

        for (const auto& naturalObject : document.natureObjects)
        {
            writer.writeln(U"[[nature]]");
            writer.writeln(U"origin = [{:.3f}, {:.3f}, {:.3f}]"_fmt(naturalObject.origin.x, naturalObject.origin.y, naturalObject.origin.z));
            writer.writeln(U"type = \"{}\""_fmt(naturalObject.type == GeneratedNatureType::Tree ? U"Tree" : U"Mushroom"));
            writer.writeln(U"serial = {}"_fmt(naturalObject.serial));
            writer.writeln(U"variationSeed = {}"_fmt(naturalObject.variationSeed));
            writer.writeln(U"");
        }
    }

    inline bool LoadProceduralData(const FilePath& savePath, ProceduralDocument& document)
    {
        document.stairs.clear();
        document.natureObjects.clear();

        if (not FileSystem::Exists(savePath))
        {
            return false;
        }

        const TOMLReader toml{ savePath };
        if (not toml)
        {
            return false;
        }

        if (HasTomlTableArraySection(savePath, U"stairs"))
        {
            for (const auto& v : toml[U"stairs"].tableArrayView())
            {
                GeneratedStair stair{};
                stair.steps = Max(1, v[U"steps"].getOr<int32>(6));
                stair.height = v[U"height"].getOr<double>(0.35);
                stair.width = v[U"width"].getOr<double>(3.0);
                stair.depth = v[U"depth"].getOr<double>(0.6);
                stair.rotation01 = v[U"rotation01"].getOr<double>(0.0);
                stair.useDullNoise = v[U"useDullNoise"].getOr<bool>(false);
                stair.useColorVariation = v[U"useColorVariation"].getOr<bool>(false);
                stair.dullNoiseAmount = v[U"dullNoiseAmount"].getOr<double>(0.35);
                stair.colorVariationAmount = v[U"colorVariationAmount"].getOr<double>(0.20);

                {
                    Array<double> values;
                    for (const auto& axis : v[U"origin"].arrayView())
                    {
                        if (const auto value = axis.getOpt<double>())
                        {
                            values << *value;
                        }
                    }
                    if (values.size() >= 3)
                    {
                        stair.origin = Vec3{ values[0], values[1], values[2] };
                    }
                }

                {
                    Array<double> values;
                    for (const auto& c : v[U"color"].arrayView())
                    {
                        if (const auto value = c.getOpt<double>())
                        {
                            values << *value;
                        }
                    }
                    if (values.size() >= 3)
                    {
                        stair.color = ColorF{ values[0], values[1], values[2], values.size() >= 4 ? values[3] : 1.0 };
                    }
                }

                document.stairs << stair;
            }
        }

        if (HasTomlTableArraySection(savePath, U"nature"))
        {
            for (const auto& v : toml[U"nature"].tableArrayView())
            {
                GeneratedNatureObject naturalObject{};
                const String type = v[U"type"].getOr<String>(U"Tree");
                naturalObject.type = (type == U"Mushroom") ? GeneratedNatureType::Mushroom : GeneratedNatureType::Tree;
                naturalObject.serial = static_cast<uint32>(v[U"serial"].getOr<int64>(0));
                naturalObject.variationSeed = static_cast<uint32>(v[U"variationSeed"].getOr<int64>(0));

                Array<double> values;
                for (const auto& axis : v[U"origin"].arrayView())
                {
                    if (const auto value = axis.getOpt<double>())
                    {
                        values << *value;
                    }
                }
                if (values.size() >= 3)
                {
                    naturalObject.origin = Vec3{ values[0], values[1], values[2] };
                }

                document.natureObjects << naturalObject;
            }
        }

        return true;
    }
}
