# pragma once
# include <Siv3D.hpp>
# include "RoadSceneSnapshot.hpp"
# include "RoadSerializer.hpp"

namespace road
{
    inline bool SavePreset(const FilePath& presetsDir, const RoadSceneSnapshot& snapshot)
    {
        FileSystem::CreateDirectories(presetsDir);

        const String safeId = snapshot.name.replaced(U" ", U"_");
        const FilePath path = presetsDir + safeId + U".toml";

        TextWriter writer{ path };
        if (not writer)
        {
            return false;
        }

        writer.writeln(U"[preset]");
        writer.writeln(U"name = \"{}\""_fmt(snapshot.name));
        writer.writeln(U"");

        const auto& m = snapshot.material;
        writer.writeln(U"[material]");
        writer.writeln(U"baseBrightness = {:.3f}"_fmt(m.baseBrightness));
        writer.writeln(U"baseWarmth = {:.3f}"_fmt(m.baseWarmth));
        writer.writeln(U"macroVariation = {:.3f}"_fmt(m.macroVariation));
        writer.writeln(U"detailVariation = {:.3f}"_fmt(m.detailVariation));
        writer.writeln(U"trackStrength = {:.3f}"_fmt(m.trackStrength));
        writer.writeln(U"trackWidth = {:.3f}"_fmt(m.trackWidth));
        writer.writeln(U"edgeMudStrength = {:.3f}"_fmt(m.edgeMudStrength));
        writer.writeln(U"pebbleStrength = {:.3f}"_fmt(m.pebbleStrength));
        writer.writeln(U"sootStrength = {:.3f}"_fmt(m.sootStrength));
        writer.writeln(U"shoulderWidthExpand = {:.3f}"_fmt(m.shoulderWidthExpand));
        writer.writeln(U"shoulderOpacity = {:.3f}"_fmt(m.shoulderOpacity));
        writer.writeln(U"shoulderBrightness = {:.3f}"_fmt(m.shoulderBrightness));
        writer.writeln(U"shoulderOuterFade = {:.3f}"_fmt(m.shoulderOuterFade));
        writer.writeln(U"shoulderUseColorFade = {}"_fmt(m.shoulderUseColorFade ? U"true" : U"false"));
        writer.writeln(U"");

        for (const auto& road : snapshot.roads)
        {
            writer.writeln(U"[[roads]]");
            writer.writeln(U"width = {:.3f}"_fmt(road.width));
            writer.writeln(U"textureRepeat = {:.3f}"_fmt(road.textureRepeat));
            writer.write(U"points = [");
            for (size_t i = 0; i < road.points.size(); ++i)
            {
                const auto& p = road.points[i];
                writer.write(U"[{:.3f}, {:.3f}, {:.3f}]"_fmt(p.x, p.y, p.z));
                if ((i + 1) < road.points.size())
                {
                    writer.write(U", ");
                }
            }
            writer.writeln(U"]");
            writer.writeln(U"");
        }

        return true;
    }

    [[nodiscard]] inline Array<RoadSceneSnapshot> LoadAllPresets(const FilePath& presetsDir)
    {
        Array<RoadSceneSnapshot> result;

        if (not FileSystem::IsDirectory(presetsDir))
        {
            return result;
        }

        for (const auto& path : FileSystem::DirectoryContents(presetsDir))
        {
            if (FileSystem::Extension(path) != U"toml")
            {
                continue;
            }

            const TOMLReader toml{ path };
            if (not toml)
            {
                continue;
            }

            RoadSceneSnapshot snapshot;
            snapshot.name = toml[U"preset"][U"name"].getOr<String>(FileSystem::BaseName(path));

            const auto mat = toml[U"material"];
            auto& ms = snapshot.material;
            ms.baseBrightness        = mat[U"baseBrightness"].getOr<double>(ms.baseBrightness);
            ms.baseWarmth            = mat[U"baseWarmth"].getOr<double>(ms.baseWarmth);
            ms.macroVariation        = mat[U"macroVariation"].getOr<double>(ms.macroVariation);
            ms.detailVariation       = mat[U"detailVariation"].getOr<double>(ms.detailVariation);
            ms.trackStrength         = mat[U"trackStrength"].getOr<double>(ms.trackStrength);
            ms.trackWidth            = mat[U"trackWidth"].getOr<double>(ms.trackWidth);
            ms.edgeMudStrength       = mat[U"edgeMudStrength"].getOr<double>(ms.edgeMudStrength);
            ms.pebbleStrength        = mat[U"pebbleStrength"].getOr<double>(ms.pebbleStrength);
            ms.sootStrength          = mat[U"sootStrength"].getOr<double>(ms.sootStrength);
            ms.shoulderWidthExpand   = mat[U"shoulderWidthExpand"].getOr<double>(ms.shoulderWidthExpand);
            ms.shoulderOpacity       = mat[U"shoulderOpacity"].getOr<double>(ms.shoulderOpacity);
            ms.shoulderBrightness    = mat[U"shoulderBrightness"].getOr<double>(ms.shoulderBrightness);
            ms.shoulderOuterFade     = mat[U"shoulderOuterFade"].getOr<double>(ms.shoulderOuterFade);
            ms.shoulderUseColorFade  = mat[U"shoulderUseColorFade"].getOr<bool>(ms.shoulderUseColorFade);
            ClampRoadMaterialSettings(ms);

            for (const auto& rv : toml[U"roads"].tableArrayView())
            {
                RoadPath road;
                road.width         = Max(rv[U"width"].getOr<double>(3.0), MinRoadWidth);
                road.textureRepeat = Max(rv[U"textureRepeat"].getOr<double>(4.0), MinRoadTextureRepeat);

                for (const auto& pv : rv[U"points"].arrayView())
                {
                    Array<double> vals;
                    for (const auto& ax : pv.arrayView())
                    {
                        if (const auto v = ax.getOpt<double>())
                        {
                            vals << *v;
                        }
                    }
                    if (vals.size() >= 3)
                    {
                        road.points << Vec3{ vals[0], vals[1], vals[2] };
                    }
                }

                if (road.points.size() >= 2)
                {
                    snapshot.roads << std::move(road);
                }
            }

            result << std::move(snapshot);
        }

        return result;
    }

    [[nodiscard]] inline bool DeletePreset(const FilePath& presetsDir, const String& presetName)
    {
        const String safeId = presetName.replaced(U" ", U"_");
        const FilePath path = presetsDir + safeId + U".toml";
        return FileSystem::Remove(path);
    }
}
