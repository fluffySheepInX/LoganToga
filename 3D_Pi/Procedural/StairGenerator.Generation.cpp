# include "../stdafx.h"
# include "StairGenerator.hpp"

namespace procedural
{
    uint32 StairGenerator::makeNatureVariationSeed(const uint32 serial) const{
            uint32 seed = static_cast<uint32>(Math::Round(Clamp(m_natureSeed, 0.0, 9999.0)));
            seed ^= (serial * 0x9E3779B9u);
            seed ^= (seed << 13u);
            seed ^= (seed >> 17u);
            seed ^= (seed << 5u);
            return seed;
        }



void StairGenerator::regenerateNatureObjects(){
            for (auto& naturalObject : m_naturalObjects)
            {
                naturalObject.variationSeed = makeNatureVariationSeed(naturalObject.serial);
            }
        }



void StairGenerator::addNatureObject(const GeneratedNatureType type){
            if (not m_generatePosition)
            {
                return;
            }

            GeneratedNatureObject naturalObject;
            naturalObject.origin = *m_generatePosition;
            naturalObject.type = type;
            naturalObject.serial = m_nextNatureSerial++;
            naturalObject.variationSeed = makeNatureVariationSeed(naturalObject.serial);
            m_naturalObjects << naturalObject;
        }



GeneratedStair* StairGenerator::getSelectedStair(){
            if (m_selectedIndex && (*m_selectedIndex < m_stairs.size()))
            {
                return &m_stairs[*m_selectedIndex];
            }

            return nullptr;
        }



ColorF StairGenerator::getMaterializedColor(const GeneratedStair& stair, const int32 stepIndex){
            ColorF result = stair.color;
            const auto fract = [](const double value) { return value - Math::Floor(value); };
            const double noise = fract(Math::Sin((stair.origin.x * 12.9898) + (stair.origin.z * 78.233) + (stepIndex * 37.719)) * 43758.5453);

            if (stair.useDullNoise)
            {
                const double amount = Clamp(stair.dullNoiseAmount, 0.0, 1.0);
                const double gray = (result.r + result.g + result.b) / 3.0;
                const double saturationMix = amount * (0.35 + noise * 0.35);
                const double darken = 1.0 - amount * (0.08 + noise * 0.22);
                result.r = Math::Lerp(result.r, gray, saturationMix) * darken;
                result.g = Math::Lerp(result.g, gray, saturationMix) * darken;
                result.b = Math::Lerp(result.b, gray, saturationMix) * darken;
            }

            if (stair.useColorVariation)
            {
                const double amount = Clamp(stair.colorVariationAmount, 0.0, 1.0);
                const double rNoise = fract(noise * 17.13);
                const double gNoise = fract(noise * 29.71);
                const double bNoise = fract(noise * 43.37);
                result.r *= 1.0 + (rNoise - 0.5) * amount;
                result.g *= 1.0 + (gNoise - 0.5) * amount;
                result.b *= 1.0 + (bNoise - 0.5) * amount;
            }

            result.r = Clamp(result.r, 0.0, 1.0);
            result.g = Clamp(result.g, 0.0, 1.0);
            result.b = Clamp(result.b, 0.0, 1.0);
            return result.removeSRGBCurve();
        }
}
