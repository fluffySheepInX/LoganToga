#include "stdafx.h"
#include "MapTile.h"
#include "Common.h"

MapTile::MapTile()
{
	columnQuads = MakeColumnQuads(N);
	rowQuads = MakeRowQuads(N);
}

Optional<Point> MapTile::ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) const
{
	int32 x = -1, y = -1;

	// タイルの列インデックスを調べる
	for (int32 i = 0; i < columnQuads.size(); ++i)
	{
		if (columnQuads[i].intersects(pos))
		{
			x = i;
			break;
		}
	}

	// タイルの行インデックスを調べる
	for (int32 i = 0; i < rowQuads.size(); ++i)
	{
		if (rowQuads[i].intersects(pos))
		{
			y = i;
			break;
		}
	}

	// インデックスが -1 の場合、タイル上にはない
	if ((x == -1) || (y == -1))
	{
		return none;
	}

	return Point{ x, y };
}

Vec2 MapTile::ToIso(double x, double y) const
{
	return Vec2((x - y), (x + y) / 2.0);
}

Quad MapTile::ToTile(const Point& index, const int32 N) const
{
	const Vec2 bottomCenter = ToTileBottomCenter(index, N);

	return Quad{
		bottomCenter.movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		bottomCenter.movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		bottomCenter.movedBy(0, -TileThickness),
		bottomCenter.movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
	};
}

Color MapTile::GetDominantColor(const String imageName, HashTable<String, Color>& data)
{
	// dataに指定された画像名が存在する場合はその色を返す
	if (data.contains(imageName))
	{
		return data[imageName];
	}

	const Image image{ PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/" + imageName };

	// 色の頻度を記録するハッシュマップ
	HashTable<Color, int32> colorCount;

	for (const auto& pixel : image)
	{
		// 透明ピクセルを除外したい場合は以下の if を有効に
		if (pixel.a == 0) continue;

		++colorCount[pixel];
	}

	// 最も頻度の高い色を見つける
	Color dominantColor = Palette::Black;
	int32 maxCount = 0;

	for (const auto& [color, count] : colorCount)
	{
		if (count > maxCount)
		{
			dominantColor = color;
			maxCount = count;
		}
	}

	//dataに追加
	data[imageName] = dominantColor;

	return dominantColor;
}

void MapTile::DrawMiniMap(const int32 N, const RectF& cameraRect) const
{
	//何故このサイズだとちょうどいいのかよくわかっていない
	//pngファイルは100*65
	//ミニマップ描写方法と通常マップ描写方法は異なるので、無理に合わせなくて良い？
	const Vec2 tileSize = Vec2(50, 25 + 25);

	// --- ミニマップに表示する位置・サイズ
	const Vec2 miniMapTopLeft = miniMapPosition;
	const SizeF miniMapDrawArea = miniMapSize;

	// --- マップの4隅をアイソメ変換 → 範囲を取得
	const Vec2 isoA = ToIso(0, 0) * tileSize;
	const Vec2 isoB = ToIso(N, 0) * tileSize;
	const Vec2 isoC = ToIso(0, N) * tileSize;
	const Vec2 isoD = ToIso(N, N) * tileSize;

	const double minX = Min({ isoA.x, isoB.x, isoC.x, isoD.x });
	const double minY = Min({ isoA.y, isoB.y, isoC.y, isoD.y });
	const double maxX = Max({ isoA.x, isoB.x, isoC.x, isoD.x });
	const double maxY = Max({ isoA.y, isoB.y, isoC.y, isoD.y });

	const SizeF isoMapSize = Vec2(maxX - minX, maxY - minY);

	// --- ミニマップに収めるためのスケーリング
	const double scale = Min(miniMapDrawArea.x / isoMapSize.x, miniMapDrawArea.y / isoMapSize.y);

	// --- スケーリング後のマップサイズ
	const SizeF scaledMapSize = isoMapSize * scale;

	// --- 中央に表示するためのオフセット（ミニマップ内で中央揃え）
	const Vec2 offset = miniMapTopLeft + (miniMapDrawArea - scaledMapSize) / 2.0 - Vec2(minX, minY) * scale;

	// --- 背景（薄い色で下地）を描画
	RectF(miniMapTopLeft, miniMapDrawArea).draw(ColorF(0.1, 0.1, 0.1, 0.5));

	// --- タイルの描画
	for (int32 y = 0; y < N; ++y)
	{
		for (int32 x = 0; x < N; ++x)
		{
			const Vec2 iso = ToIso(x, y) * tileSize;
			const Vec2 miniPos = iso * scale + offset;
			const SizeF tileSizeScaled = tileSize * scale;

			// デフォルト色
			ColorF tileColor = Palette::White;
			if (auto it = minimapCols.find(Point(x, y)); it != minimapCols.end())
			{
				tileColor = it->second;
			}

			// 描画
			RectF(miniPos, tileSizeScaled).draw(tileColor);
		}
	}

	// --- カメラ範囲を表示（白い枠）
	{
		const Vec2 camTopLeft = cameraRect.pos * Vec2(1.0, 1.0);
		const Vec2 camBottomRight = camTopLeft + cameraRect.size;
		RectF(camTopLeft * scale + offset, cameraRect.size * scale).drawFrame(1.5, ColorF(1.0));
	}
}

Vec2 MapTile::ToTileBottomCenter(const Point& index, const int32 N) const
{
	const int32 i = index.manhattanLength();
	const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	const int32 yi = (i < (N - 1)) ? i : (N - 1);
	const int32 k = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
	const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	const double posY = (i * TileOffset.y);
	return{ (posX + TileOffset.x * 2 * k), posY };
}

Quad MapTile::ToColumnQuad(const int32 x, const int32 N) const
{
	return{
		ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(0, 0),
		ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
	};
}

Quad MapTile::ToRowQuad(const int32 y, const int32 N) const
{
	return{
		ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(0, 0)
	};
}

Array<Quad> MapTile::MakeColumnQuads(const int32 N) const
{
	Array<Quad> quads;

	for (int32 x = 0; x < N; ++x)
	{
		quads << ToColumnQuad(x, N);
	}

	return quads;
}

Array<Quad> MapTile::MakeRowQuads(const int32 N) const
{
	Array<Quad> quads;

	for (int32 y = 0; y < N; ++y)
	{
		quads << ToRowQuad(y, N);
	}

	return quads;
}
