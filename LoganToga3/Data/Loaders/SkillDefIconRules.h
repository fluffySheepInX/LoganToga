#pragma once
# include "SkillDefLoaderCommon.h"

namespace LT3
{
	struct SkillIconLayerOrderKey
	{
		bool isFrame = false;
		int32 frameOrder = 99;
		String normalizedName;
	};

	inline SkillIconLayerOrderKey ParseSkillIconLayerOrderKey(StringView iconName)
	{
		SkillIconLayerOrderKey key;
		key.normalizedName = String{ iconName }.lowercased();
		if (key.normalizedName.starts_with(U"n_"))
		{
			return key;
		}

		if (key.normalizedName.starts_with(U"w_"))
		{
			key.isFrame = true;
			const Array<String> parts = key.normalizedName.split(U'_');
			if (parts.size() >= 2)
			{
				if (const Optional<int32> order = ParseOpt<int32>(parts[1]))
				{
					key.frameOrder = Clamp(*order, 0, 99);
				}
			}
		}

		return key;
	}

	inline Array<String> NormalizeSkillIconLayerOrder(const Array<String>& source)
	{
		struct IndexedLayer
		{
			String name;
			SkillIconLayerOrderKey key;
			int32 sourceIndex = 0;
		};

		Array<IndexedLayer> indexed;
		indexed.reserve(source.size());
		for (int32 i = 0; i < static_cast<int32>(source.size()); ++i)
		{
			if (source[i].isEmpty())
			{
				continue;
			}

			indexed << IndexedLayer{ source[i], ParseSkillIconLayerOrderKey(source[i]), i };
		}

		std::stable_sort(indexed.begin(), indexed.end(), [](const IndexedLayer& a, const IndexedLayer& b)
		{
			if (a.key.isFrame != b.key.isFrame)
			{
				return !a.key.isFrame;
			}

			if (a.key.isFrame && b.key.isFrame && a.key.frameOrder != b.key.frameOrder)
			{
				return a.key.frameOrder < b.key.frameOrder;
			}

			return a.sourceIndex < b.sourceIndex;
		});

		Array<String> normalized;
		normalized.reserve(indexed.size());
		for (const auto& item : indexed)
		{
			normalized << item.name;
		}

		return normalized;
	}

	inline FilePath ResolveSkillValidationIconPath(const String& iconName)
	{
		if (iconName.isEmpty())
		{
			return FilePath{};
		}

		const Array<FilePath> candidates = {
			ResolveBuildIconPath(iconName),
			ResolveSystemImagePath(iconName),
			ResolveUnitChipPath(iconName),
			ResolveBuildingChipPath(iconName),
		};
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}

		return FilePath{};
	}

	inline Array<String> ValidateSkillIconLayers(const SkillDef& skill)
	{
		Array<String> warnings;
		Optional<Size> referenceSize;
		String referenceName;

		for (const auto& iconName : skill.iconLayers)
		{
			const SkillIconLayerOrderKey orderKey = ParseSkillIconLayerOrderKey(iconName);
			const String normalizedName = String{ iconName }.lowercased();
			if (!(normalizedName.starts_with(U"n_") || normalizedName.starts_with(U"w_")))
			{
				warnings << U"{} does not follow SkillIcon naming rule (use n_name or w_xx_name)"_fmt(iconName);
			}
			if (orderKey.isFrame)
			{
				const Array<String> parts = normalizedName.split(U'_');
				const bool allDigits = (parts.size() >= 2)
					&& std::all_of(parts[1].begin(), parts[1].end(), [](const char32 ch)
				{
					return IsDigit(ch);
				});
				if (parts.size() < 3 || parts[1].isEmpty() || !allDigits || 2 < parts[1].size())
				{
					warnings << U"{} frame order must use w_xx_name with 1-2 digit xx"_fmt(iconName);
				}
			}

			const FilePath path = ResolveSkillValidationIconPath(iconName);
			if (path.isEmpty())
			{
				warnings << U"Missing icon file: {}"_fmt(iconName);
				continue;
			}

			const Image image{ path };
			if (!image)
			{
				warnings << U"Icon load failed: {}"_fmt(iconName);
				continue;
			}

			const Size size = image.size();
			if ((size.x % 32) != 0 || (size.y % 32) != 0)
			{
				warnings << U"{} size {}x{} is not a multiple of 32"_fmt(iconName, size.x, size.y);
			}

			if (!referenceSize)
			{
				referenceSize = size;
				referenceName = iconName;
			}
			else if (*referenceSize != size)
			{
				warnings << U"{} size {}x{} differs from {} size {}x{}"_fmt(iconName, size.x, size.y, referenceName, referenceSize->x, referenceSize->y);
			}
		}

		return warnings;
	}
}
