#pragma once
#include "stdafx.h"
#include "Common.h"

class MapTile
{
public:
	/// @brief マップの一辺のタイル数
	int32 N = 64;
	/// @brief タイルの一辺の長さ（ピクセル）
	Vec2 TileOffset{ 50, 25 };
	/// @brief タイルの厚み（ピクセル）
	int32 TileThickness = 15;
	/// @brief タイルの種類
	Grid<int32> grid;
	/// @brief
	HashTable<Point, ColorF> minimapCols;
	/// @brief
	HashTable<String, Color> colData;
	/// @brief
	const Size miniMapSize = Size(200, 200);
	/// @brief
	const Vec2 miniMapPosition =
		Scene::Size()
		- miniMapSize
		- Vec2(20, 20); // 右下から20pxオフセット
	/// @brief
	Array<Quad> columnQuads;
	/// @brief
	Array<Quad> rowQuads;

	MapTile();

	/// @brief 指定した位置がどのタイルに属するかを調べ、そのインデックスを返します。
	/// @param pos 調べる位置（座標）。
	/// @param columnQuads 列ごとのタイル領域を表すQuadの配列。
	/// @param rowQuads 行ごとのタイル領域を表すQuadの配列。
	/// @return 位置がタイル上にある場合はそのインデックス（Point型）、タイル上にない場合はnone（Optional<Point>型）。
	Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) const;

	/// @brief 2D座標を等角投影座標に変換します。
	/// @param x 変換する元のX座標。
	/// @param y 変換する元のY座標。
	/// @return 等角投影後の座標を表すVec2オブジェクト。
	Vec2 ToIso(double x, double y) const;

	/// @brief タイルのインデックスから、タイルの四角形を計算します。
	/// @param index タイルのインデックス
	/// @param N マップの一辺のタイル数
	/// @return タイルの四角形
	Quad ToTile(const Point& index, const int32 N) const;

	/// @brief 指定した画像の支配的な色（最も頻度の高い色）を取得します。
	/// @param imageName 色を取得する画像のファイル名。
	/// @param data 画像名とその支配色を格納するハッシュテーブル。参照渡しで、結果がキャッシュされます。
	/// @return 画像内で最も頻度の高い色（支配色）。
	Color GetDominantColor(const String imageName, HashTable<String, Color>& data);

	/// @brief ミニマップを描画します。マップデータとカメラの表示範囲をもとに、ミニマップ上にタイルとカメラ範囲を表示します。
	/// @param map ミニマップとして描画するマップデータ（int32型のグリッド）。
	/// @param cameraRect カメラの表示範囲を示す矩形（RectF型）。
	void DrawMiniMap(const int32 N, const RectF& cameraRect) const;

	/// @brief タイルのインデックスから、タイルの底辺中央の座標を計算します。
	/// @param index タイルのインデックス
	/// @param N マップの一辺のタイル数
	/// @return タイルの底辺中央の座標
	Vec2 ToTileBottomCenter(const Point& index, const int32 N) const;

	/// @brief 指定した列のタイルによって構成される四角形を計算します。
	/// @param x 列インデックス
	/// @param N マップの一辺のタイル数
	/// @return 指定した列のタイルによって構成される四角形
	Quad ToColumnQuad(const int32 x, const int32 N) const;

	/// @brief 指定した行のタイルによって構成される四角形を計算します。
	/// @param y 行インデックス
	/// @param N マップの一辺のタイル数
	/// @return 指定した行のタイルによって構成される四角形
	Quad ToRowQuad(const int32 y, const int32 N) const;

	/// @brief 各列のタイルによって構成される四角形の配列を作成します。
	/// @param N マップの一辺のタイル数
	/// @return 各列のタイルによって構成される四角形の配列
	Array<Quad> MakeColumnQuads(const int32 N) const;

	/// @brief 各行のタイルによって構成される四角形の配列を作成します。
	/// @param N マップの一辺のタイル数
	/// @return 各行のタイルによって構成される四角形の配列
	Array<Quad> MakeRowQuads(const int32 N) const;
};
