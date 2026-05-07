# pragma once
# include <Siv3D.hpp>

namespace procedural
{
    enum class GeneratedNatureType
    {
        Tree,
        Mushroom,
    };

    struct GeneratedStair
    {
        Vec3 origin;
        int32 steps;
        double height;
        double width;
        double depth;
        double rotation01 = 0.0;
        ColorF color{ 0.72, 0.68, 0.60 };
        bool useDullNoise = false;
        bool useColorVariation = false;
        double dullNoiseAmount = 0.35;
        double colorVariationAmount = 0.20;
    };

    struct GeneratedNatureObject
    {
        Vec3 origin;
        GeneratedNatureType type = GeneratedNatureType::Tree;
        uint32 serial = 0;
        uint32 variationSeed = 0;
    };
}
