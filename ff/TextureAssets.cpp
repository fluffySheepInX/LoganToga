# include "TextureAssets.h"

namespace ff
{
	ColorF BlendColor(const ColorF& a, const ColorF& b, const double t)
	{
		const double rate = Clamp(t, 0.0, 1.0);
		return{
			(a.r + ((b.r - a.r) * rate)),
			(a.g + ((b.g - a.g) * rate)),
			(a.b + ((b.b - a.b) * rate)),
			(a.a + ((b.a - a.a) * rate))
		};
	}

	Texture MakeTileTexture(const ColorF& baseColor, const ColorF& accentColor)
	{
		Image image{ Size{ TileWidth, TileHeight }, Color{ 0, 0, 0, 0 } };
		const Vec2 center{ (TileWidth / 2.0), (TileHeight / 2.0) };

		for (int32 y = 0; y < TileHeight; ++y)
		{
			for (int32 x = 0; x < TileWidth; ++x)
			{
				const double dx = (Abs((x + 0.5) - center.x) / TileHalfSize.x);
				const double dy = (Abs((y + 0.5) - center.y) / TileHalfSize.y);
				const double diamond = (dx + dy);

				if (diamond > 1.0)
				{
					continue;
				}

				const double edge = Clamp((diamond - 0.82) / 0.18, 0.0, 1.0);
				const double vertical = Clamp(1.0 - (dy * 0.85), 0.0, 1.0);
				ColorF color = BlendColor(baseColor, accentColor, (1.0 - vertical));
				color = BlendColor(color, ColorF{ 1.0, 1.0, 1.0 }, (0.08 * (1.0 - edge)));
				color = BlendColor(color, ColorF{ 0.1, 0.12, 0.15 }, (0.24 * edge));

				if ((((x / 6) + (y / 5)) % 7) == 0)
				{
					color = BlendColor(color, accentColor, 0.14);
				}

				image[Point{ x, y }] = color.toColor();
			}
		}

		return Texture{ image };
	}

	namespace
	{
		[[nodiscard]]
		Texture LoadPremultipliedTexture(FilePathView path)
		{
			Image image{ path };
			Color* p = image.data();
			const Color* const pEnd = (p + image.num_pixels());

			while (p != pEnd)
			{
				p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
				p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
				p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
				++p;
			}

			return Texture{ image };
		}

		Texture LoadTerrainTexture(FilePathView fileName)
		{
			const FilePath path = (U"png/" + FilePath{ fileName });

			if (not FileSystem::Exists(path))
			{
				throw Error{ U"Texture not found: {}"_fmt(path) };
			}

			return LoadPremultipliedTexture(path);
		}
	}

	Array<Texture> LoadTerrainTextures()
	{
		return{
			LoadTerrainTexture(U"grassWhole.png"),
			LoadTerrainTexture(U"dirt.png"),
			LoadTerrainTexture(U"beach.png"),
			LoadTerrainTexture(U"water.png")
		};
	}
}
