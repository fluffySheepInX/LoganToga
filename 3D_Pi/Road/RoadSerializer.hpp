# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"
# include "RoadMaterialBuilder.hpp"
# include "RoadMeshBuilder.hpp"

namespace road
{
    inline void SaveRoadData(const FilePath& savePath, const RoadMaterialSettings& materialSettings, const Array<RoadPath>& roads)
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

        writer.writeln(U"[material]");
        writer.writeln(U"baseBrightness = {:.3f}"_fmt(materialSettings.baseBrightness));
        writer.writeln(U"baseWarmth = {:.3f}"_fmt(materialSettings.baseWarmth));
        writer.writeln(U"macroVariation = {:.3f}"_fmt(materialSettings.macroVariation));
        writer.writeln(U"detailVariation = {:.3f}"_fmt(materialSettings.detailVariation));
        writer.writeln(U"trackStrength = {:.3f}"_fmt(materialSettings.trackStrength));
        writer.writeln(U"trackWidth = {:.3f}"_fmt(materialSettings.trackWidth));
        writer.writeln(U"edgeMudStrength = {:.3f}"_fmt(materialSettings.edgeMudStrength));
        writer.writeln(U"pebbleStrength = {:.3f}"_fmt(materialSettings.pebbleStrength));
        writer.writeln(U"sootStrength = {:.3f}"_fmt(materialSettings.sootStrength));
        writer.writeln(U"shoulderWidthExpand = {:.3f}"_fmt(materialSettings.shoulderWidthExpand));
        writer.writeln(U"shoulderOpacity = {:.3f}"_fmt(materialSettings.shoulderOpacity));
        writer.writeln(U"shoulderBrightness = {:.3f}"_fmt(materialSettings.shoulderBrightness));
        writer.writeln(U"shoulderOuterFade = {:.3f}"_fmt(materialSettings.shoulderOuterFade));
        writer.writeln(U"shoulderUseColorFade = {}"_fmt(materialSettings.shoulderUseColorFade ? U"true" : U"false"));
        writer.writeln(U"");

        for (const auto& road : roads)
        {
            writer.writeln(U"[[roads]]");
            writer.writeln(U"width = {:.3f}"_fmt(road.width));
            writer.writeln(U"textureRepeat = {:.3f}"_fmt(road.textureRepeat));
            writer.write(U"points = [");
            for (size_t i = 0; i < road.points.size(); ++i)
            {
                const auto& point = road.points[i];
                writer.write(U"[{:.3f}, {:.3f}, {:.3f}]"_fmt(point.x, point.y, point.z));
                if ((i + 1) < road.points.size())
                {
                    writer.write(U", ");
                }
            }
            writer.writeln(U"]");
            writer.writeln(U"");
        }
    }

    inline bool LoadRoadData(const FilePath& savePath, RoadMaterialSettings& materialSettings, Array<RoadPath>& roads, String& statusMessage)
    {
        roads.clear();
        materialSettings = DefaultRoadMaterialSettings();

        if (not FileSystem::Exists(savePath))
        {
            statusMessage = U"Road data file not found";
            return false;
        }

        const TOMLReader toml{ savePath };
        if (not toml)
        {
            statusMessage = U"Road data load failed";
            return false;
        }

        const auto material = toml[U"material"];
        materialSettings.baseBrightness = material[U"baseBrightness"].getOr<double>(materialSettings.baseBrightness);
        materialSettings.baseWarmth = material[U"baseWarmth"].getOr<double>(materialSettings.baseWarmth);
        materialSettings.macroVariation = material[U"macroVariation"].getOr<double>(materialSettings.macroVariation);
        materialSettings.detailVariation = material[U"detailVariation"].getOr<double>(materialSettings.detailVariation);
        materialSettings.trackStrength = material[U"trackStrength"].getOr<double>(materialSettings.trackStrength);
        materialSettings.trackWidth = material[U"trackWidth"].getOr<double>(materialSettings.trackWidth);
        materialSettings.edgeMudStrength = material[U"edgeMudStrength"].getOr<double>(materialSettings.edgeMudStrength);
        materialSettings.pebbleStrength = material[U"pebbleStrength"].getOr<double>(materialSettings.pebbleStrength);
        materialSettings.sootStrength = material[U"sootStrength"].getOr<double>(materialSettings.sootStrength);
        materialSettings.shoulderWidthExpand = material[U"shoulderWidthExpand"].getOr<double>(materialSettings.shoulderWidthExpand);
        materialSettings.shoulderOpacity = material[U"shoulderOpacity"].getOr<double>(materialSettings.shoulderOpacity);
        materialSettings.shoulderBrightness = material[U"shoulderBrightness"].getOr<double>(materialSettings.shoulderBrightness);
        materialSettings.shoulderOuterFade = material[U"shoulderOuterFade"].getOr<double>(materialSettings.shoulderOuterFade);
        materialSettings.shoulderUseColorFade = material[U"shoulderUseColorFade"].getOr<bool>(materialSettings.shoulderUseColorFade);
        ClampRoadMaterialSettings(materialSettings);

        for (const auto& roadValue : toml[U"roads"].tableArrayView())
        {
            RoadPath road;
            road.width = Max(roadValue[U"width"].getOr<double>(3.0), MinRoadWidth);
            road.textureRepeat = Max(roadValue[U"textureRepeat"].getOr<double>(4.0), MinRoadTextureRepeat);

            for (const auto& pointValue : roadValue[U"points"].arrayView())
            {
                Array<double> values;
                for (const auto& axis : pointValue.arrayView())
                {
                    if (const auto value = axis.getOpt<double>())
                    {
                        values << *value;
                    }
                }

                if (values.size() >= 3)
                {
                    road.points << Vec3{ values[0], values[1], values[2] };
                }
            }

            if (road.points.size() >= 2)
            {
                roads << std::move(road);
            }
        }

        statusMessage = U"Road data loaded";
        return true;
    }
}
