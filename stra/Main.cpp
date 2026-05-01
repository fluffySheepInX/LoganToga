# include <Siv3D.hpp> // Siv3D v0.6.16
#include "libs/AddonGaussian.h"

struct GameData {

};
using App = SceneManager<String, GameData>;

// 描画された最大のアルファ成分を保持するブレンドステートを作成する
BlendState MakeBlendState()
{
	BlendState blendState = BlendState::Default2D;
	blendState.srcAlpha = Blend::SrcAlpha;
	blendState.dstAlpha = Blend::DestAlpha;
	blendState.opAlpha = BlendOp::Max;
	return blendState;
}

namespace map {
	// 隣接するヘックスの取得（odd-r / odd-row offset レイアウト）
	// 使い方: auto n = GetHexNeighbors(index, GridSize);
	Array<Point> GetHexNeighbors(const Point& index, const Size& gridSize)
	{
		Array<Point> neighbors;
		const int32 x = index.x;
		const int32 y = index.y;
		const bool rowIsOdd = IsOdd(y);

		// even row (rowIsOdd == false)
		static constexpr Point deltasEven[6] = {
			Point{ 1, 0 }, Point{ 0, -1 }, Point{ -1, -1 },
			Point{ -1, 0 }, Point{ -1, 1 }, Point{ 0, 1 }
		};
		// odd row (rowIsOdd == true)
		static constexpr Point deltasOdd[6] = {
			Point{ 1, 0 }, Point{ 1, -1 }, Point{ 0, -1 },
			Point{ -1, 0 }, Point{ 0, 1 }, Point{ 1, 1 }
		};

		const Point* deltas = rowIsOdd ? deltasOdd : deltasEven;
		for (int i = 0; i < 6; ++i)
		{
			const Point n = Point{ x + deltas[i].x, y + deltas[i].y };
			if (n.x >= 0 && n.y >= 0 && n.x < gridSize.x && n.y < gridSize.y)
			{
				neighbors.push_back(n);
			}
		}
		return neighbors;
	}
	std::size_t getMaxColorValue(const Array<std::size_t>& colors) {
		if (colors.isEmpty()) {
			throw std::runtime_error("The array is empty.");
		}
		return *std::max_element(colors.begin(), colors.end());
	}
	inline double squaredDistance(double x1, double y1, double x2, double y2) {
		return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
	}
	//64ビット版メルセンヌ・ツイスタ
	std::mt19937_64 mt;
	bool randBool(const double probability_) {
		std::bernoulli_distribution uid(probability_);
		return uid(mt);
	}
	//ノイズを発生させる
	void noiseShoreBoolAfter(Image& vec_, const double rbool_) {
		for (std::size_t i{ 1 }; i < vec_.height(); ++i)
			for (std::size_t j{ 1 }; j < vec_.width(); ++j) {
				if (randBool(rbool_)) {
					if ((vec_[i][j] != vec_[i][j - 1]) || (vec_[i][j] != vec_[i - 1][j])) {
						if (vec_[i][j] == Color(U"#202d0a"))  // Assuming this is land
							vec_[i][j] = Color(U"#1e90ff");  // Change to water
						else if (vec_[i][j] == Color(U"#1e90ff"))  // Assuming this is water
							vec_[i][j] = Color(U"#202d0a");  // Change to land
					}
				}
			}
	}
	template<typename Point_>
	constexpr double distanceSqrd(const Point_& point_, double x_, double y_) {
		x_ -= point_.first;
		y_ -= point_.second;
		return (x_ * x_) + (y_ * y_);
	}
	template<typename STL_>
	constexpr void noiseShorePoints(STL_& points, const double magnitude, const double probability) {
		PerlinNoise perlin;
		for (auto& point : points) {
			if (RandomBool(probability)) {
				point.first += perlin.noise1D(magnitude);
				point.second += perlin.noise1D(magnitude);
			}
		}
	}
	// 川を刻む：Perlin ノイズに基づく縦方向の蛇行する経路を描画して水色で上書きする
	void carveRivers(Image& image, int riverCount = 2, double thickness = 3.0, double scale = 0.01)
	{
		//scaleを小さく（例 0.005）にすると蛇行が緩やかに、逆に大きくすると急に動く。球が出る原因がノイズの急変ならscaleを小さく
		//smoothAlphaを小さく（例 0.05〜0.2）にすると中心のジャンプをさらに抑えられる（ただし川が平坦になる
		//thicknessを大きくすると幅は広がる。radiusのwobbleの係数を下げると断面変化が滑らかに

		PerlinNoise perlin;
		const int w = image.width();
		const int h = image.height();

		// パラメータ: 平滑化係数 (0.0..1.0, 小さいほど滑らか)
		const double smoothAlpha = 0.20;
		// 半径の平滑化係数
		const double radiusSmoothAlpha = 0.35;

		for (int r = 0; r < riverCount; ++r) {
			const double seed = Random(0.0, 1000.0);

			double prevCx = -1.0;
			double prevRadius = Max(1.0, thickness);

			for (int y = 0; y < h; ++y) {
				// 中心 x を決める（ノイズ -> 0..1 -> 0..w-1）
				double n = perlin.noise1D(seed + y * scale);
				double nx = (n + 1.0) * 0.5;
				double cxDouble = nx * (double)(w - 1);

				// 中心を平滑化（EMA）
				double smoothedCx = (prevCx < 0.0) ? cxDouble : (smoothAlpha * cxDouble + (1.0 - smoothAlpha) * prevCx);

				// 半径（幅）のゆらぎを取り、平滑化
				double wobble = (perlin.noise1D(seed * 0.5 + y * scale * 0.5) + 1.0) * 0.5;
				double rawRadius = thickness + wobble * (thickness * 0.5); // 揺れ幅を控えめに
				double radius = (y == 0) ? rawRadius : (radiusSmoothAlpha * rawRadius + (1.0 - radiusSmoothAlpha) * prevRadius);

				// 中心間を補間して塗りつぶす（前の中心が無ければその行だけ塗る）
				if (prevCx < 0.0) {
					int cx = Clamp((int)std::lrint(smoothedCx), 0, w - 1);
					int rint = Max(1, (int)std::lrint(radius));
					int x0 = Max(0, cx - rint);
					int x1 = Min(w - 1, cx + rint);
					for (int xx = x0; xx <= x1; ++xx) {
						image[y][xx] = Color(U"#1e90ff");
					}
				}
				else {
					// 補間ステップ数は中心差に応じて決定（至少1）
					int steps = Max(1, (int)std::ceil(std::abs(smoothedCx - prevCx)));
					for (int s = 0; s <= steps; ++s) {
						double t = (double)s / (double)steps;
						double interCx = prevCx + (smoothedCx - prevCx) * t;
						double interR = prevRadius + (radius - prevRadius) * t;
						int center = Clamp((int)std::lrint(interCx), 0, w - 1);
						int rint = Max(1, (int)std::lrint(interR));

						// 横スパンを塗る（各補間点で水平に塗ることで球のスタンプ感を消す）
						int x0 = Max(0, center - rint);
						int x1 = Min(w - 1, center + rint);
						// 少し縦方向にも広げて連続性を向上
						for (int yy = y - 1; yy <= y + 1; ++yy) {
							if (yy < 0 || yy >= h) continue;
							for (int xx = x0; xx <= x1; ++xx) {
								image[yy][xx] = Color(U"#1e90ff");
							}
						}
					}
				}

				prevCx = smoothedCx;
				prevRadius = radius;
			}
		}
	}
	void forceOceanBorder(Image& image, int margin)
	{
		const int w = image.width();
		const int h = image.height();
		if (margin <= 0) return;
		// clamp margin
		margin = Clamp(margin, 0, Min(w, h) / 2);

		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				const int distToEdge = Min(Min(x, w - 1 - x), Min(y, h - 1 - y));
				if (distToEdge < margin)
				{
					// 強制的に海色へ上書き
					image[y][x] = Color(U"#1e90ff");
				}
			}
		}
	}
	template<typename Int>
	class SimpleVoronoiIsland {
	public:
		template<typename T_>
		constexpr SimpleVoronoiIsland(T_& t_, const std::size_t count_ = 100, const double rbool_ = 0.4, const Int land_ = 1, const Int sea_ = 0) {
			create(t_, count_, rbool_, land_, sea_);
		}
		Array<std::pair<double, double>> point;
		Array<std::size_t> color;
	private:
		template<typename T_>
		constexpr void create(T_& t_, const std::size_t count_ = 100, const double rbool_ = 0.4, const Int land_ = 1, const Int sea_ = 0) {
			for (std::size_t i{}; i < count_; ++i) {
				createPoint((t_.empty()) ? 0 : t_.width(), t_.height(), rbool_, land_, sea_);
			}
			createSites(t_, (t_.empty()) ? 0 : t_.width(), t_.height());
		}
		constexpr bool isMakeIsland(const std::size_t w_, const std::size_t h_, const std::size_t numerator_, const std::size_t denominator_) const {
			// 整数除算の副作用を避け、比率を浮動小数点で評価するように変更
			const double startX = w_ * (static_cast<double>(numerator_) / static_cast<double>(denominator_));
			const double endX = w_ * (static_cast<double>(denominator_ - numerator_) / static_cast<double>(denominator_));
			const double startY = h_ * (static_cast<double>(numerator_) / static_cast<double>(denominator_));
			const double endY = h_ * (static_cast<double>(denominator_ - numerator_) / static_cast<double>(denominator_));
			//return (point.back().first > (w_ * numerator_ / denominator_) && point.back().first < (w_ * (denominator_ - numerator_) / denominator_)) && (point.back().second > (h_ * numerator_ / denominator_) && point.back().second < (h_ * (denominator_ - numerator_) / denominator_));
			return (point.back().first > startX && point.back().first < endX)
				&& (point.back().second > startY && point.back().second < endY);
		}
		constexpr void createPoint(const std::size_t w_, const std::size_t h_, const double rbool_, const Int land_, const Int sea_) {
			double x = Random(0.0, (double)w_);
			double y = Random(0.0, (double)h_);
			point.emplace_back(x, y);

			//if (isMakeIsland(w_, h_, 1, 3) || (RandomBool(rbool_) && isMakeIsland(w_, h_, 1, 4)))
			//	color.emplace_back(land_);
			//else
			//	color.emplace_back(sea_);
						// 変更: 中央判定に加えてランダム確率で陸にすることで全体に陸を増やす
			// (中央領域はより高確率で陸に、外側も rbool_ によって陸になりやすい)
			if (isMakeIsland(w_, h_, 1, 3) || RandomBool(rbool_))
				color.emplace_back(land_);
			else
				color.emplace_back(sea_);
		}
		template<typename T_>
		constexpr void createSites(T_& t_, const std::size_t w_, const std::size_t h_) const {
			double ds{}, dist{};
			for (std::size_t hh{}, ind{}; hh < h_; ++hh)
				for (std::size_t ww{}; ww < w_; ++ww) {
					ind = std::numeric_limits<std::size_t>::max();
					dist = std::numeric_limits<double>::max();
					for (std::size_t it{}; it < point.size(); ++it) {
						const std::pair<double, double>& p{ point[it] };
						if ((ds = distanceSqrd(p, (double)ww, (double)hh)) >= dist) continue;
						dist = ds;
						ind = it;
					}
					if (ind != std::numeric_limits<std::size_t>::max()) t_[hh][ww] = (int32)color[ind];
				}
		}
	};
	void createVoronoiDiagram(const Array<std::pair<double, double>>& points, const Array<std::size_t>& colors, const std::size_t width, const std::size_t height) {
		Image image(width, height, ColorF(0.0, 0.0, 0.0, 0.0));

		// 最大値を取得
		std::size_t maxColorValue = 0;
		try {
			maxColorValue = getMaxColorValue(colors);
		}
		catch (const std::runtime_error& e) {
			Print << U"Error: " << Unicode::Widen(e.what());
			return;
		}

		const auto worker = [&](std::size_t startY, std::size_t endY) {
			for (std::size_t y = startY; y < endY; ++y) {
				for (std::size_t x = 0; x < width; ++x) {
					double minDistance = std::numeric_limits<double>::max();
					std::size_t closestPointIndex = 0;

					for (std::size_t i = 0; i < points.size(); ++i) {
						double distance = squaredDistance((double)x, (double)y, points[i].first, points[i].second);
						if (distance < minDistance) {
							minDistance = distance;
							closestPointIndex = i;
						}
					}

					size_t colorValue = colors[closestPointIndex];

					Color pixelColor;
					if (colorValue >= maxColorValue * 0.8) {
						pixelColor = Color(U"#202d0a");
					}
					else if (colorValue >= maxColorValue * 0.5) {
						pixelColor = Color(0, 0, 200);
					}
					else {
						pixelColor = Color(U"#1e90ff");
					}

					image[y][x] = pixelColor;
				}
			}
			};

		const std::size_t threadCount = std::thread::hardware_concurrency();
		std::vector<std::future<void>> futures;

		for (std::size_t i = 0; i < threadCount; ++i) {
			std::size_t startY = (height * i) / threadCount;
			std::size_t endY = (height * (i + 1)) / threadCount;
			futures.emplace_back(std::async(std::launch::async, worker, startY, endY));
		}

		for (auto& future : futures) {
			future.get();
		}

		// 外周を海にする（margin: 海岸から内側へ何ピクセルを海にするか）
		// 例: 画面幅に応じて 20〜80 を試す。ここでは 40 をデフォルトに。
		forceOceanBorder(image, 40);
		// 川を刻む（例: 3本、幅3.0、スケール 0.01）
		carveRivers(image, 3, 3.0, 0.01);
		// shore ノイズで陸が裏返らないように確率を下げる
		// 変更前: noiseShoreBoolAfter(image, 0.5);
		noiseShoreBoolAfter(image, 0.4);

		image.save(U"voronoi.png");
	}
}

class GameSceneBase : public App::Scene
{
public:
	GameSceneBase(const InitData& init) : IScene(init)
	{
		Image image(U"voronoi.png");

		int32 gridSizeWidth = (int32)(GaussianFSAddon::GetWindowSize().x - (1 * 2) - 16);
		int32 gridSizeHeight = GaussianFSAddon::GetWindowSize().y - 30 - (1 * 2) - 16;// -30は下のメニュー分、-1*2は枠線分

		if (image.isEmpty())
		{
			Grid<int32> grid(gridSizeWidth, gridSizeHeight);
			map::SimpleVoronoiIsland<int32> diagram(grid, 400, 0.4);
			map::noiseShorePoints(diagram.point, 20, 0.8);
			map::createVoronoiDiagram(diagram.point, diagram.color, grid.width(), grid.height());
			image = Image(U"voronoi.png");
		}

		Texture mapTexture(image);
		renderTextureMap = RenderTexture{ Size{ gridSizeWidth, gridSizeHeight }, ColorF{ 0.5, 0.0 } };
		{
			// レンダーターゲットを renderTextureMap に変更する
			const ScopedRenderTarget2D target{ renderTextureMap };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			mapTexture.draw(0, 0);
		}

	}

	void update() override
	{
	}

	void draw() const override
	{
		const Transformer2D screenScaling{
			Mat3x2::Scale(GaussianFSAddon::GetSCALE()).translated(GaussianFSAddon::GetOFFSET()),
			TransformCursor::Yes
		};

		renderTextureMap
			.drawAt(
				(GaussianFSAddon::GetWindowSize().x - 2) / 2,
				(GaussianFSAddon::GetWindowSize().y - 30 - 2) / 2
			);
	}

private:
	RenderTexture renderTextureMap;

};


void Main()
{
#pragma region Addon
	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,900 });
	GaussianFSAddon::SetLangSet({
		{ U"Japan",     U"日本語" },
		{ U"English",   U"English" },
		{ U"Deutsch",   U"Deutsch" },
		{ U"Test",      U"TestLang" },
		});
	GaussianFSAddon::SetLang(U"Japan");
	GaussianFSAddon::SetSceneSet({
		{ U"1600*900", U"1600",U"900"},
		{ U"1200*675", U"1200",U"675"},
		});
	GaussianFSAddon::SetScene(U"1600*900");
	//GaussianFSAddon::SetSceneName(U"SelectLang");
#pragma endregion

	App manager;
	manager.add<GameSceneBase>(U"GameSceneBase");

	while (System::Update())
	{
		if (not manager.update())break;

#pragma region Addon
		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion
	}
}
