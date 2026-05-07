# pragma once
# include <Siv3D.hpp>
# include "GroundLayerDocument.hpp"

namespace ground
{
    inline void SaveGroundLayerDocument(const FilePath& savePath, const GroundLayerDocument& document)
    {
        const FilePath dir = FileSystem::ParentPath(savePath);
        if (not dir.isEmpty())
        {
            FileSystem::CreateDirectories(dir);
        }

        TextWriter writer{ savePath };
        if (not writer)
        {
            return;
        }

        writer.writeln(U"[settings]");
        writer.writeln(U"autoYOffset = {}"_fmt(document.autoYOffset ? U"true" : U"false"));
        writer.writeln(U"autoYOffsetStep = {:.5f}"_fmt(document.autoYOffsetStep));
        writer.writeln(U"baseYOffset = {:.5f}"_fmt(document.baseYOffset));
        writer.writeln(U"");

        for (const auto& layer : document.layers)
        {
            writer.writeln(U"[[layers]]");
            writer.writeln(U"id = \"{}\""_fmt(layer.id));
            writer.writeln(U"label = \"{}\""_fmt(layer.label));
            writer.writeln(U"texturePath = \"{}\""_fmt(layer.texturePath));
            writer.writeln(U"categoryIndex = {}"_fmt(layer.categoryIndex));
            writer.writeln(U"positionX = {:.3f}"_fmt(layer.position.x));
            writer.writeln(U"positionZ = {:.3f}"_fmt(layer.position.y));
            writer.writeln(U"sizeW = {:.3f}"_fmt(layer.size.x));
            writer.writeln(U"sizeH = {:.3f}"_fmt(layer.size.y));
            writer.writeln(U"rotation = {:.5f}"_fmt(layer.rotation));
            writer.writeln(U"yOffset = {:.5f}"_fmt(layer.yOffset));
            writer.writeln(U"tintR = {:.4f}"_fmt(layer.tint.r));
            writer.writeln(U"tintG = {:.4f}"_fmt(layer.tint.g));
            writer.writeln(U"tintB = {:.4f}"_fmt(layer.tint.b));
            writer.writeln(U"tintA = {:.4f}"_fmt(layer.tint.a));
            writer.writeln(U"tilingScale = {:.3f}"_fmt(layer.tilingScale));
            writer.writeln(U"edgeSoftness = {:.5f}"_fmt(layer.edgeSoftness));
            writer.writeln(U"edgeNoiseAmount = {:.5f}"_fmt(layer.edgeNoiseAmount));
            writer.writeln(U"edgeNoiseFrequency = {:.4f}"_fmt(layer.edgeNoiseFrequency));
            writer.writeln(U"edgeNoiseSeed = {}"_fmt(layer.edgeNoiseSeed));
            writer.writeln(U"visible = {}"_fmt(layer.visible ? U"true" : U"false"));
            writer.writeln(U"");
        }
    }

    inline bool LoadGroundLayerDocument(const FilePath& savePath, GroundLayerDocument& document)
    {
        document = GroundLayerDocument{};

        if (not FileSystem::Exists(savePath))
        {
            return false;
        }

        const TOMLReader toml{ savePath };
        if (not toml)
        {
            return false;
        }

        const auto settings = toml[U"settings"];
        document.autoYOffset = settings[U"autoYOffset"].getOr<bool>(true);
        document.autoYOffsetStep = settings[U"autoYOffsetStep"].getOr<double>(0.003);
        document.baseYOffset = settings[U"baseYOffset"].getOr<double>(0.002);

        for (const auto& v : toml[U"layers"].tableArrayView())
        {
            GroundLayer layer;
            layer.id = v[U"id"].getOr<String>(U"");
            layer.label = v[U"label"].getOr<String>(U"Layer");
            layer.texturePath = v[U"texturePath"].getOr<String>(U"");
            layer.categoryIndex = v[U"categoryIndex"].getOr<int32>(0);
            layer.position.x = v[U"positionX"].getOr<double>(0.0);
            layer.position.y = v[U"positionZ"].getOr<double>(0.0);
            layer.size.x = v[U"sizeW"].getOr<double>(10.0);
            layer.size.y = v[U"sizeH"].getOr<double>(10.0);
            layer.rotation = v[U"rotation"].getOr<double>(0.0);
            layer.yOffset = v[U"yOffset"].getOr<double>(0.014);
            layer.tint.r = v[U"tintR"].getOr<double>(1.0);
            layer.tint.g = v[U"tintG"].getOr<double>(1.0);
            layer.tint.b = v[U"tintB"].getOr<double>(1.0);
            layer.tint.a = v[U"tintA"].getOr<double>(1.0);
            layer.tilingScale = v[U"tilingScale"].getOr<double>(4.0);
            layer.edgeSoftness = v[U"edgeSoftness"].getOr<double>(0.12);
            layer.edgeNoiseAmount = v[U"edgeNoiseAmount"].getOr<double>(0.04);
            layer.edgeNoiseFrequency = v[U"edgeNoiseFrequency"].getOr<double>(3.0);
            layer.edgeNoiseSeed = static_cast<uint64>(v[U"edgeNoiseSeed"].getOr<int64>(42));
            layer.visible = v[U"visible"].getOr<bool>(true);
            document.layers << layer;
        }

        return true;
    }

    inline GroundLayerDocument BuildDefaultGroundLayerDocument()
    {
        GroundLayerDocument document;

        {
            GroundLayer g;
            g.id = U"grass_base"; g.label = U"Grass Base";
            g.texturePath = U"example/texture/grass.jpg";
            g.categoryIndex = 0;
            g.position = Vec2{ 0.0, 0.0 }; g.size = Vec2{ 36.0, 30.0 };
            g.rotation = 0.0; g.yOffset = 0.004;
            g.tint = ColorF{ 0.60, 0.68, 0.56, 0.95 };
            g.tilingScale = 6.0;
            g.edgeSoftness = 0.14; g.edgeNoiseAmount = 0.05;
            g.edgeNoiseFrequency = 2.5; g.edgeNoiseSeed = 10;
            document.layers << g;
        }
        {
            GroundLayer p;
            p.id = U"plaza_center"; p.label = U"Plaza Center";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ -1.0, 0.4 }; p.size = Vec2{ 12.5, 10.5 };
            p.rotation = Math::ToRadians(-4.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.74, 0.66, 0.60, 0.96 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.04;
            p.edgeNoiseFrequency = 3.0; p.edgeNoiseSeed = 7;
            document.layers << p;
        }
        {
            GroundLayer p;
            p.id = U"plaza_east"; p.label = U"Plaza East";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ 5.5, 3.2 }; p.size = Vec2{ 8.0, 6.0 };
            p.rotation = Math::ToRadians(12.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.76, 0.68, 0.61, 0.92 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.04;
            p.edgeNoiseFrequency = 3.5; p.edgeNoiseSeed = 22;
            document.layers << p;
        }
        {
            GroundLayer p;
            p.id = U"plaza_west"; p.label = U"Plaza West";
            p.texturePath = U"example/texture/rock.jpg";
            p.categoryIndex = 2;
            p.position = Vec2{ -8.3, 3.8 }; p.size = Vec2{ 7.5, 5.2 };
            p.rotation = Math::ToRadians(-10.0); p.yOffset = 0.014;
            p.tint = ColorF{ 0.72, 0.65, 0.59, 0.90 };
            p.tilingScale = 4.0;
            p.edgeSoftness = 0.10; p.edgeNoiseAmount = 0.035;
            p.edgeNoiseFrequency = 3.0; p.edgeNoiseSeed = 35;
            document.layers << p;
        }

        return document;
    }
}
