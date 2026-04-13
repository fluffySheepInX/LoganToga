# include "TerrainSurface.hpp"

# include <array>
# include <bit>

namespace
{
	constexpr uint64 HashSeed = 1469598103934665603ull;
	constexpr uint64 HashPrime = 1099511628211ull;

	[[nodiscard]] int64 ToTerrainCellKey(const Point& cell)
	{
		return (static_cast<int64>(cell.x) << 32)
			^ static_cast<uint32>(cell.y);
	}

	void HashCombine(uint64& hash, const uint64 value)
	{
		hash ^= value;
		hash *= HashPrime;
	}

	[[nodiscard]] uint64 HashDouble(const double value)
	{
		return std::bit_cast<uint64>(value);
	}

	[[nodiscard]] size_t ToTerrainTypeIndex(const TerrainCellType type)
	{
		switch (type)
		{
		case TerrainCellType::Grass:
			return 0;

		case TerrainCellType::Dirt:
			return 1;

		case TerrainCellType::Sand:
			return 2;

		case TerrainCellType::Rock:
		default:
			return 3;
		}
	}

	[[nodiscard]] TerrainCellType ToTerrainType(const size_t index)
	{
		switch (index)
		{
		case 0:
			return TerrainCellType::Grass;

		case 1:
			return TerrainCellType::Dirt;

		case 2:
			return TerrainCellType::Sand;

		case 3:
		default:
			return TerrainCellType::Rock;
		}
	}

	struct TerrainSurfaceAccumulator
	{
		Point cell{ 0, 0 };
		std::array<double, 4> materialWeights{};
		double wear = 0.0;
		double ambientOcclusion = 0.0;
		ColorF tint{ 1.0, 1.0, 1.0, 1.0 };
		double coverage = 0.0;
		bool painted = false;
	};

	[[nodiscard]] TerrainSurfaceAccumulator& EnsureAccumulator(HashTable<int64, TerrainSurfaceAccumulator>& accumulators, const Point& cell)
	{
		const int64 key = ToTerrainCellKey(cell);
		if (const auto it = accumulators.find(key); it != accumulators.end())
		{
			return it->second;
		}

		TerrainSurfaceAccumulator accumulator;
		accumulator.cell = cell;
		return accumulators.emplace(key, accumulator).first->second;
	}

	void EnsureBaseDirtWeight(TerrainSurfaceAccumulator& accumulator)
	{
		const double totalWeight = (accumulator.materialWeights[0]
			+ accumulator.materialWeights[1]
			+ accumulator.materialWeights[2]
			+ accumulator.materialWeights[3]);
		if (totalWeight < 0.001)
		{
			accumulator.materialWeights[ToTerrainTypeIndex(TerrainCellType::Dirt)] = 1.0;
		}
	}

	[[nodiscard]] double SampleTerrainNoise(const Vec2& worldPosition, const double scale)
	{
		const double frequency = Max(0.0001, scale);
		const double coarse = (0.5 + 0.5 * Math::Sin(worldPosition.x * frequency * 0.83 + Math::Cos(worldPosition.y * frequency * 0.57) * 1.9));
		const double detail = (0.5 + 0.5 * Math::Cos((worldPosition.x + worldPosition.y) * frequency * 1.74 + 0.85));
		const double diagonal = (0.5 + 0.5 * Math::Sin((worldPosition.x * 0.62 - worldPosition.y * 1.28) * frequency * 1.37));
		return Clamp((coarse * 0.54 + detail * 0.28 + diagonal * 0.18), 0.0, 1.0);
	}

	void StampCircularInfluence(HashTable<int64, TerrainSurfaceAccumulator>& accumulators,
		const Vec3& center,
		const double radius,
		const double dirtBoost,
		const double grassReduction,
		const double wearBoost,
		const double aoBoost,
		const double coverageBoost,
      const double falloffPower,
		const double imprintStrength)
	{
       if (imprintStrength <= 0.0)
		{
			return;
		}

		const Point minCell = ToTerrainCell(center.movedBy(-radius, 0, -radius));
		const Point maxCell = ToTerrainCell(center.movedBy(radius, 0, radius));

		for (int32 y = minCell.y; y <= maxCell.y; ++y)
		{
			for (int32 x = minCell.x; x <= maxCell.x; ++x)
			{
				const Point cell{ x, y };
				const Vec3 cellCenter = ToTerrainCellCenter(cell);
				const double distance = Vec2{ (cellCenter.x - center.x), (cellCenter.z - center.z) }.length();
				if (radius < distance)
				{
					continue;
				}

				const double falloff = Math::Pow(Math::Saturate(1.0 - distance / Max(0.001, radius)), falloffPower);
              const double scaledFalloff = (falloff * imprintStrength);
				auto& accumulator = EnsureAccumulator(accumulators, cell);
				EnsureBaseDirtWeight(accumulator);
                accumulator.materialWeights[ToTerrainTypeIndex(TerrainCellType::Dirt)] += (dirtBoost * scaledFalloff);
				accumulator.materialWeights[ToTerrainTypeIndex(TerrainCellType::Grass)] *= Max(0.0, (1.0 - grassReduction * scaledFalloff));
				accumulator.wear = Max(accumulator.wear, (wearBoost * scaledFalloff));
				accumulator.ambientOcclusion = Max(accumulator.ambientOcclusion, (aoBoost * scaledFalloff));
				accumulator.coverage = Max(accumulator.coverage, (coverageBoost * scaledFalloff));
			}
		}
	}

  void StampRoadInfluence(HashTable<int64, TerrainSurfaceAccumulator>& accumulators, const PlacedModel& placedModel, const double imprintStrength)
	{
     if (imprintStrength <= 0.0)
		{
			return;
		}

		const double roadLength = Clamp(placedModel.roadLength, 2.0, 80.0);
		const double roadWidth = Clamp(placedModel.roadWidth, 2.0, 80.0);
		const double influenceHalfWidth = (roadWidth * 0.5 + 1.2);
		const double influenceHalfLength = (roadLength * 0.5 + 1.0);
		const Vec3 minPosition = placedModel.position.movedBy(-(influenceHalfWidth + 0.8), 0, -(influenceHalfLength + 0.8));
		const Vec3 maxPosition = placedModel.position.movedBy((influenceHalfWidth + 0.8), 0, (influenceHalfLength + 0.8));
		const Point minCell = ToTerrainCell(minPosition);
		const Point maxCell = ToTerrainCell(maxPosition);
		const double cosYaw = Math::Cos(placedModel.yaw);
		const double sinYaw = Math::Sin(placedModel.yaw);

		for (int32 y = minCell.y; y <= maxCell.y; ++y)
		{
			for (int32 x = minCell.x; x <= maxCell.x; ++x)
			{
				const Point cell{ x, y };
				const Vec3 cellCenter = ToTerrainCellCenter(cell);
				const Vec2 delta{ (cellCenter.x - placedModel.position.x), (cellCenter.z - placedModel.position.z) };
				const double localX = (delta.x * cosYaw - delta.y * sinYaw);
				const double localZ = (delta.x * sinYaw + delta.y * cosYaw);
				if ((influenceHalfWidth < Abs(localX)) || (influenceHalfLength < Abs(localZ)))
				{
					continue;
				}

				const double alongFactor = Math::Saturate(1.0 - (Abs(localZ) / influenceHalfLength));
				const double bodyFactor = Math::Saturate(1.0 - (Abs(localX) / Max(0.35, roadWidth * 0.5)));
				const double shoulderFactor = Math::Saturate(1.0 - (Abs(Abs(localX) - roadWidth * 0.5) / 0.9));
             const double roadFactor = (alongFactor * Max(bodyFactor, (shoulderFactor * 0.72)));
				if (roadFactor <= 0.001)
				{
					continue;
				}

				const double scaledRoadFactor = (roadFactor * imprintStrength);

				auto& accumulator = EnsureAccumulator(accumulators, cell);
				EnsureBaseDirtWeight(accumulator);
              accumulator.materialWeights[ToTerrainTypeIndex(TerrainCellType::Dirt)] += (0.95 * scaledRoadFactor);
				accumulator.materialWeights[ToTerrainTypeIndex(TerrainCellType::Grass)] *= Max(0.0, (1.0 - 1.20 * scaledRoadFactor));
				accumulator.wear = Max(accumulator.wear, Min(1.0, scaledRoadFactor * (0.72 + bodyFactor * 0.18)));
				accumulator.ambientOcclusion = Max(accumulator.ambientOcclusion, Min(1.0, shoulderFactor * alongFactor * 0.24 * imprintStrength));
				accumulator.coverage = Max(accumulator.coverage, Min(0.62, scaledRoadFactor * (0.44 + shoulderFactor * 0.12)));
			}
		}
	}

   void ApplyPlacedModelInfluence(HashTable<int64, TerrainSurfaceAccumulator>& accumulators, const MapData& mapData, const MainSupport::TerrainVisualSettings& settings)
	{
        const double imprintStrength = Clamp(settings.placementImprintStrength, 0.0, 1.5);
		if (imprintStrength <= 0.0)
		{
			return;
		}

		for (const auto& placedModel : mapData.placedModels)
		{
			switch (placedModel.type)
			{
			case PlaceableModelType::Mill:
             StampCircularInfluence(accumulators, placedModel.position, 4.8, 0.95, 1.10, 0.48, 0.32, 0.30, 1.8, imprintStrength);
				break;

			case PlaceableModelType::Tree:
             StampCircularInfluence(accumulators, placedModel.position, 1.6, 0.36, 0.55, 0.18, 0.20, 0.14, 1.7, imprintStrength);
				break;

			case PlaceableModelType::Pine:
             StampCircularInfluence(accumulators, placedModel.position, 1.5, 0.34, 0.50, 0.16, 0.22, 0.14, 1.7, imprintStrength);
				break;

			case PlaceableModelType::GrassPatch:
               StampCircularInfluence(accumulators, placedModel.position, 1.8, 0.0, -0.38, 0.0, 0.0, 0.18, 1.5, imprintStrength);
				break;

			case PlaceableModelType::Rock:
             StampCircularInfluence(accumulators, placedModel.position, 2.4, 0.28, 0.40, 0.18, 0.36, 0.22, 1.6, imprintStrength);
				break;

			case PlaceableModelType::Wall:
              StampCircularInfluence(accumulators, placedModel.position, Clamp(placedModel.wallLength * 0.18, 2.0, 8.0), 0.42, 0.55, 0.24, 0.18, 0.22, 1.8, imprintStrength);
				break;

			case PlaceableModelType::Road:
              StampRoadInfluence(accumulators, placedModel, imprintStrength);
				break;

			default:
				break;
			}
		}
	}

   void ApplyTerrainNeighborhoodBlend(HashTable<int64, TerrainSurfaceAccumulator>& accumulators, const MainSupport::TerrainVisualSettings& settings)
	{
      const double blendStrength = Clamp(settings.materialBlendStrength, 0.0, 1.0);
		const double centerWeight = Max(0.0, (1.0 - blendStrength));
		const double neighborWeight = (blendStrength * 0.25);
		const HashTable<int64, TerrainSurfaceAccumulator> original = accumulators;
		for (auto& [key, accumulator] : accumulators)
		{
			std::array<double, 4> blendedWeights{};
			for (size_t i = 0; i < blendedWeights.size(); ++i)
			{
                blendedWeights[i] = (accumulator.materialWeights[i] * centerWeight);
			}

			double neighborCoverage = 0.0;
			for (const Point direction : { Point{ 1, 0 }, Point{ -1, 0 }, Point{ 0, 1 }, Point{ 0, -1 } })
			{
				const auto it = original.find(ToTerrainCellKey(accumulator.cell + direction));
				if (it == original.end())
				{
					continue;
				}

				for (size_t i = 0; i < blendedWeights.size(); ++i)
				{
                    blendedWeights[i] += (it->second.materialWeights[i] * neighborWeight);
				}
				neighborCoverage = Max(neighborCoverage, it->second.coverage * 0.92);
			}

			accumulator.materialWeights = blendedWeights;
			accumulator.coverage = Max(accumulator.coverage, neighborCoverage);
			EnsureBaseDirtWeight(accumulator);
		}
	}

	[[nodiscard]] ColorF ComputeTerrainColor(const TerrainSurfaceAccumulator& accumulator, const MainSupport::TerrainVisualSettings& settings)
	{
		std::array<double, 4> normalizedWeights = accumulator.materialWeights;
		double totalWeight = 0.0;
		for (const double weight : normalizedWeights)
		{
			totalWeight += weight;
		}

		if (totalWeight < 0.001)
		{
			normalizedWeights[ToTerrainTypeIndex(TerrainCellType::Dirt)] = 1.0;
			totalWeight = 1.0;
		}

		for (double& weight : normalizedWeights)
		{
			weight /= totalWeight;
		}

		ColorF color{ 0.0, 0.0, 0.0, 1.0 };
		for (size_t i = 0; i < normalizedWeights.size(); ++i)
		{
			color += (GetTerrainCellBaseColor(ToTerrainType(i)) * normalizedWeights[i]);
		}

		if (accumulator.painted)
		{
			color = ColorF{
				Clamp(color.r * accumulator.tint.r, 0.0, 1.0),
				Clamp(color.g * accumulator.tint.g, 0.0, 1.0),
				Clamp(color.b * accumulator.tint.b, 0.0, 1.0),
				1.0,
			};
		}

		const ColorF wearColor = ColorF{ 0.31, 0.25, 0.18 };
       const double wearStrength = Clamp(settings.wearStrength, 0.0, 2.0);
		color = color.lerp(wearColor, Clamp(accumulator.wear * 0.42 * wearStrength, 0.0, 0.72));

		const Vec3 center = ToTerrainCellCenter(accumulator.cell);
		const Vec2 worldPosition{ center.x, center.z };
		if (settings.noiseEnabled)
		{
            const double detailStrength = Clamp(settings.noiseStrength, 0.0, 1.0);
			const double detailScale = Clamp(settings.noiseScale, 0.04, 0.60);
           const double broadStrength = Clamp(settings.macroNoiseStrength, 0.0, 1.0);
			const double broadScale = Clamp(settings.macroNoiseScale, 0.01, 0.20);
			const double broadNoise = SampleTerrainNoise(worldPosition, broadScale);
			const double detailNoise = SampleTerrainNoise((worldPosition + Vec2{ 13.7, -7.2 }), (detailScale * 1.85));
			const double broadVariation = ((broadNoise - 0.5) * 2.0);
			const double detailVariation = ((detailNoise - 0.5) * 2.0);
          const double brightness = Clamp((1.0 + broadVariation * 0.14 * broadStrength + detailVariation * 0.06 * detailStrength), 0.72, 1.18);
			const double dryness = (broadVariation * 0.08 * broadStrength);
			color.r = Clamp(color.r * brightness * (1.0 + dryness * 0.16), 0.0, 1.0);
			color.g = Clamp(color.g * brightness * (1.0 - dryness * 0.10), 0.0, 1.0);
			color.b = Clamp(color.b * brightness * (1.0 - dryness * 0.18), 0.0, 1.0);
		}

     const double aoStrength = Clamp(settings.ambientOcclusionStrength, 0.0, 2.0);
		const double aoShade = Clamp(1.0 - accumulator.ambientOcclusion * 0.26 * aoStrength, 0.56, 1.0);
		color.r *= aoShade;
		color.g *= aoShade;
		color.b *= aoShade;
        color.a = Clamp(accumulator.coverage + accumulator.wear * 0.10 * wearStrength + (accumulator.painted ? 0.08 : 0.0), 0.0, 0.92);
		return color;
	}
}

uint64 ComputeTerrainSurfaceRevision(const MapData& mapData, const MainSupport::TerrainVisualSettings& settings)
{
	uint64 revision = HashSeed;
	HashCombine(revision, static_cast<uint64>(mapData.terrainCells.size()));
	HashCombine(revision, static_cast<uint64>(mapData.placedModels.size()));
	HashCombine(revision, settings.noiseEnabled ? 1ull : 0ull);
  HashCombine(revision, HashDouble(settings.materialBlendStrength));
   HashCombine(revision, HashDouble(settings.placementImprintStrength));
	HashCombine(revision, HashDouble(settings.wearStrength));
	HashCombine(revision, HashDouble(settings.ambientOcclusionStrength));
	HashCombine(revision, HashDouble(settings.macroNoiseStrength));
	HashCombine(revision, HashDouble(settings.macroNoiseScale));
	HashCombine(revision, HashDouble(settings.noiseStrength));
	HashCombine(revision, HashDouble(settings.noiseScale));

	for (const auto& terrainCell : mapData.terrainCells)
	{
		HashCombine(revision, std::bit_cast<uint64>(static_cast<int64>(terrainCell.cell.x)));
		HashCombine(revision, std::bit_cast<uint64>(static_cast<int64>(terrainCell.cell.y)));
		HashCombine(revision, static_cast<uint64>(ToTerrainTypeIndex(terrainCell.type)));
		HashCombine(revision, terrainCell.color.r);
		HashCombine(revision, terrainCell.color.g);
		HashCombine(revision, terrainCell.color.b);
		HashCombine(revision, terrainCell.color.a);
	}

	for (const auto& placedModel : mapData.placedModels)
	{
		HashCombine(revision, static_cast<uint64>(placedModel.type));
		HashCombine(revision, HashDouble(placedModel.position.x));
		HashCombine(revision, HashDouble(placedModel.position.y));
		HashCombine(revision, HashDouble(placedModel.position.z));
		HashCombine(revision, HashDouble(placedModel.yaw));
		HashCombine(revision, HashDouble(placedModel.wallLength));
		HashCombine(revision, HashDouble(placedModel.roadLength));
		HashCombine(revision, HashDouble(placedModel.roadWidth));
	}

	return revision;
}

TerrainSurfaceData BuildTerrainSurface(const MapData& mapData, const MainSupport::TerrainVisualSettings& settings)
{
	HashTable<int64, TerrainSurfaceAccumulator> accumulators;
	accumulators.reserve(mapData.terrainCells.size() + mapData.placedModels.size() * 16);

	for (const auto& terrainCell : mapData.terrainCells)
	{
		auto& accumulator = EnsureAccumulator(accumulators, terrainCell.cell);
		accumulator.materialWeights.fill(0.0);
		accumulator.materialWeights[ToTerrainTypeIndex(terrainCell.type)] = 1.0;
		accumulator.tint = ColorF{ terrainCell.color };
		accumulator.coverage = Max(accumulator.coverage, 0.74);
		accumulator.painted = true;
	}

    ApplyPlacedModelInfluence(accumulators, mapData, settings);
	ApplyTerrainNeighborhoodBlend(accumulators, settings);

	TerrainSurfaceData surface;
		surface.cells.reserve(accumulators.size());
	for (const auto& [key, accumulator] : accumulators)
	{
		(void)key;
		const ColorF finalColor = ComputeTerrainColor(accumulator, settings);
		if (finalColor.a <= 0.01)
		{
			continue;
		}

		size_t dominantTypeIndex = 0;
		double dominantTypeWeight = accumulator.materialWeights[0];
		for (size_t i = 1; i < accumulator.materialWeights.size(); ++i)
		{
			if (dominantTypeWeight < accumulator.materialWeights[i])
			{
				dominantTypeWeight = accumulator.materialWeights[i];
				dominantTypeIndex = i;
			}
		}

		surface.cells << TerrainSurfaceCell{
			.cell = accumulator.cell,
			.dominantType = ToTerrainType(dominantTypeIndex),
			.wear = accumulator.wear,
			.ambientOcclusion = accumulator.ambientOcclusion,
			.finalColor = finalColor,
		};
	}

	return surface;
}
