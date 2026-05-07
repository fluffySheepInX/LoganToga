# pragma once
# include <Siv3D.hpp>

struct GroundLayer
{
    String id;
    String label;
    FilePath texturePath;
    int32 categoryIndex = 0; // 0=Grass 1=Dirt 2=Plaza 3=Brick 4=Stone 5=Decal
    Vec2 position{ 0.0, 0.0 }; // world X, Z
    Vec2 size{ 10.0, 10.0 };
    double rotation = 0.0; // radians
    double yOffset = 0.014;
    ColorF tint{ 1.0 };
    double tilingScale = 4.0;
    double edgeSoftness = 0.12;
    double edgeNoiseAmount = 0.04;
    double edgeNoiseFrequency = 3.0;
    uint64 edgeNoiseSeed = 42;
    bool visible = true;
};
