# include <Siv3D.hpp> // Siv3D v0.6.16
#include "libs/AddonGaussian.h"
#include <queue>

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
	struct RiverNetwork {
		int32 width = 0;
		int32 height = 0;
		std::vector<uint8> land;
		std::vector<int32> parent;
		std::vector<int32> outlet;
		std::vector<double> accumulation;
		std::vector<int32> selectedOutlets;
	};

	struct RiverMouth {
		int32 index = -1;
		double drainageArea = 0.0;
	};

	// 外海へ必ず到達する排水木と集水量を構築する
	RiverNetwork buildRiverNetwork(Image& image, const int32 riverCount)
	{
		const Color landColor{ U"#202d0a" };
		const int32 width = static_cast<int32>(image.width());
		const int32 height = static_cast<int32>(image.height());
		const int32 cellCount = (width * height);
		const double infinity = std::numeric_limits<double>::max();
		static constexpr Point neighborOffsets[8] = {
			Point{ -1, -1 }, Point{ 0, -1 }, Point{ 1, -1 }, Point{ -1, 0 },
			Point{ 1, 0 }, Point{ -1, 1 }, Point{ 0, 1 }, Point{ 1, 1 }
		};

		RiverNetwork network;
		network.width = width;
		network.height = height;
		network.land.resize(cellCount, 0);
		network.parent.resize(cellCount, -1);
		network.outlet.resize(cellCount, -1);
		network.accumulation.resize(cellCount, 0.0);

		for (int32 y = 0; y < height; ++y) {
			for (int32 x = 0; x < width; ++x) {
				network.land[y * width + x] = (image[y][x] == landColor);
			}
		}

		std::vector<uint8> ocean(cellCount, 0);
		std::queue<int32> oceanQueue;
		const auto enqueueOcean = [&](const int32 x, const int32 y) {
			const int32 index = (y * width + x);
			if ((network.land[index] == 0) && (ocean[index] == 0)) {
				ocean[index] = 1;
				oceanQueue.push(index);
			}
		};

		for (int32 x = 0; x < width; ++x) {
			enqueueOcean(x, 0);
			enqueueOcean(x, height - 1);
		}
		for (int32 y = 1; y < (height - 1); ++y) {
			enqueueOcean(0, y);
			enqueueOcean(width - 1, y);
		}

		while (not oceanQueue.empty()) {
			const int32 index = oceanQueue.front();
			oceanQueue.pop();
			const int32 x = (index % width);
			const int32 y = (index / width);

			for (const Point& offset : neighborOffsets) {
				const int32 nx = (x + offset.x);
				const int32 ny = (y + offset.y);
				if ((nx < 0) || (ny < 0) || (nx >= width) || (ny >= height)) {
					continue;
				}

				const int32 neighbor = (ny * width + nx);
				if ((network.land[neighbor] == 0) && (ocean[neighbor] == 0)) {
					ocean[neighbor] = 1;
					oceanQueue.push(neighbor);
				}
			}
		}

		for (int32 index = 0; index < cellCount; ++index) {
			if ((network.land[index] == 0) && (ocean[index] == 0)) {
				network.land[index] = 1;
				image[index / width][index % width] = landColor;
			}
		}

		std::vector<double> distanceToWater(cellCount, infinity);
		for (int32 index = 0; index < cellCount; ++index) {
			if (network.land[index] == 0) {
				distanceToWater[index] = 0.0;
			}
		}

		const double diagonalDistance = std::sqrt(2.0);
		for (int32 y = 0; y < height; ++y) {
			for (int32 x = 0; x < width; ++x) {
				const int32 index = (y * width + x);
				if (x > 0) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index - 1] + 1.0);
				if (y > 0) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index - width] + 1.0);
				if ((x > 0) && (y > 0)) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index - width - 1] + diagonalDistance);
				if (((x + 1) < width) && (y > 0)) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index - width + 1] + diagonalDistance);
			}
		}
		for (int32 y = (height - 1); y >= 0; --y) {
			for (int32 x = (width - 1); x >= 0; --x) {
				const int32 index = (y * width + x);
				if ((x + 1) < width) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index + 1] + 1.0);
				if ((y + 1) < height) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index + width] + 1.0);
				if (((x + 1) < width) && ((y + 1) < height)) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index + width + 1] + diagonalDistance);
				if ((x > 0) && ((y + 1) < height)) distanceToWater[index] = Min(distanceToWater[index], distanceToWater[index + width - 1] + diagonalDistance);
			}
		}

		PerlinNoise perlin;
		const double seedX = Random(0.0, 1000.0);
		const double seedY = Random(0.0, 1000.0);
		std::vector<double> rawElevation(cellCount, 0.0);
		for (int32 y = 0; y < height; ++y) {
			for (int32 x = 0; x < width; ++x) {
				const int32 index = (y * width + x);
				if (network.land[index] == 0) {
					continue;
				}

				double noise = 0.0;
				double amplitude = 1.0;
				double frequency = 0.004;
				double amplitudeSum = 0.0;
				for (int32 octave = 0; octave < 4; ++octave) {
					noise += perlin.noise2D(seedX + x * frequency, seedY + y * frequency) * amplitude;
					amplitudeSum += amplitude;
					amplitude *= 0.5;
					frequency *= 2.0;
				}
				noise /= amplitudeSum;
				rawElevation[index] = Max(0.01, distanceToWater[index] + noise * 10.0);
			}
		}

		using QueueEntry = std::pair<double, int32>;
		std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> floodQueue;
		std::vector<uint8> discovered(cellCount, 0);
		std::vector<double> filledElevation(cellCount, infinity);
		std::vector<int32> floodOrder;
		floodOrder.reserve(cellCount);

		for (int32 index = 0; index < cellCount; ++index) {
			if (ocean[index] != 0) {
				discovered[index] = 1;
				filledElevation[index] = 0.0;
				floodQueue.emplace(0.0, index);
			}
		}

		while (not floodQueue.empty()) {
			const auto [elevation, index] = floodQueue.top();
			floodQueue.pop();
			floodOrder.push_back(index);
			const int32 x = (index % width);
			const int32 y = (index / width);

			for (const Point& offset : neighborOffsets) {
				const int32 nx = (x + offset.x);
				const int32 ny = (y + offset.y);
				if ((nx < 0) || (ny < 0) || (nx >= width) || (ny >= height)) {
					continue;
				}

				const int32 neighbor = (ny * width + nx);
				if (discovered[neighbor] != 0) {
					continue;
				}

				discovered[neighbor] = 1;
				network.parent[neighbor] = index;
				filledElevation[neighbor] = Max(rawElevation[neighbor], elevation + 0.0001);
				floodQueue.emplace(filledElevation[neighbor], neighbor);
			}
		}

		for (int32 index = 0; index < cellCount; ++index) {
			if (network.land[index] != 0) {
				network.accumulation[index] = 1.0;
			}
		}
		for (auto it = floodOrder.rbegin(); it != floodOrder.rend(); ++it) {
			const int32 index = *it;
			const int32 parent = network.parent[index];
			if (parent >= 0) {
				network.accumulation[parent] += network.accumulation[index];
			}
		}

		std::vector<RiverMouth> mouths;
		for (const int32 index : floodOrder) {
			if (network.land[index] == 0) {
				continue;
			}

			const int32 parent = network.parent[index];
			if ((parent >= 0) && (ocean[parent] != 0)) {
				network.outlet[index] = index;
				mouths.push_back(RiverMouth{ index, network.accumulation[index] });
			}
			else if (parent >= 0) {
				network.outlet[index] = network.outlet[parent];
			}
		}

		std::sort(mouths.begin(), mouths.end(), [](const RiverMouth& a, const RiverMouth& b) {
			return (a.drainageArea > b.drainageArea);
		});

		const double minimumMouthDistance = Max(32.0, Min(width, height) * 0.12);
		const double minimumMouthDistanceSq = (minimumMouthDistance * minimumMouthDistance);
		for (const RiverMouth& mouth : mouths) {
			const Point position{ mouth.index % width, mouth.index / width };
			bool sufficientlySeparated = true;
			for (const int32 selected : network.selectedOutlets) {
				const Point selectedPosition{ selected % width, selected / width };
				if (position.distanceFromSq(selectedPosition) < minimumMouthDistanceSq) {
					sufficientlySeparated = false;
					break;
				}
			}

			if (sufficientlySeparated) {
				network.selectedOutlets.push_back(mouth.index);
				if (network.selectedOutlets.size() >= static_cast<size_t>(Max(0, riverCount))) {
					break;
				}
			}
		}
		for (const RiverMouth& mouth : mouths) {
			if (network.selectedOutlets.size() >= static_cast<size_t>(Max(0, riverCount))) {
				break;
			}
			if (std::find(network.selectedOutlets.begin(), network.selectedOutlets.end(), mouth.index) == network.selectedOutlets.end()) {
				network.selectedOutlets.push_back(mouth.index);
			}
		}

		return network;
	}

	// 親子画素間を集水量に応じた連続幅で塗りつぶす
	void rasterizeRiverSegment(Image& image, const RiverNetwork& network, const int32 from, const int32 to,
		const double fromRadius, const double toRadius)
	{
		const Vec2 start{ static_cast<double>(from % network.width), static_cast<double>(from / network.width) };
		const Vec2 end{ static_cast<double>(to % network.width), static_cast<double>(to / network.width) };
		const Vec2 segment = (end - start);
		const double lengthSq = segment.lengthSq();
		const double boundingRadius = Max(fromRadius, toRadius);
		const int32 minX = Clamp(static_cast<int32>(std::floor(Min(start.x, end.x) - boundingRadius)), 0, network.width - 1);
		const int32 maxX = Clamp(static_cast<int32>(std::ceil(Max(start.x, end.x) + boundingRadius)), 0, network.width - 1);
		const int32 minY = Clamp(static_cast<int32>(std::floor(Min(start.y, end.y) - boundingRadius)), 0, network.height - 1);
		const int32 maxY = Clamp(static_cast<int32>(std::ceil(Max(start.y, end.y) + boundingRadius)), 0, network.height - 1);
		const Color riverColor{ U"#1e90ff" };

		for (int32 y = minY; y <= maxY; ++y) {
			for (int32 x = minX; x <= maxX; ++x) {
				const int32 index = (y * network.width + x);
				if (network.land[index] == 0) {
					continue;
				}

				const Vec2 point{ static_cast<double>(x), static_cast<double>(y) };
				const double t = (lengthSq > 0.0) ? Clamp((point - start).dot(segment) / lengthSq, 0.0, 1.0) : 0.0;
				const Vec2 closest = (start + segment * t);
				const double radius = Math::Lerp(fromRadius, toRadius, t);
				if (point.distanceFromSq(closest) <= (radius * radius)) {
					image[y][x] = riverColor;
				}
			}
		}
	}

	// 擬似標高と集水量から主要流域の河川網を刻む
	void carveRivers(Image& image, const int32 riverCount = 3, const double minimumRadius = 1.2, const double maximumRadius = 5.0)
	{
		const RiverNetwork network = buildRiverNetwork(image, riverCount);
		const int32 cellCount = (network.width * network.height);
		std::vector<double> riverThreshold(cellCount, std::numeric_limits<double>::max());
		std::vector<double> outletArea(cellCount, 0.0);

		for (const int32 outlet : network.selectedOutlets) {
			const double area = network.accumulation[outlet];
			outletArea[outlet] = area;
			riverThreshold[outlet] = Max(40.0, area * 0.015);
		}

		for (int32 index = 0; index < cellCount; ++index) {
			if (network.land[index] == 0) {
				continue;
			}

			const int32 outlet = network.outlet[index];
			if ((outlet < 0) || (outletArea[outlet] <= 0.0) || (network.accumulation[index] < riverThreshold[outlet])) {
				continue;
			}

			const int32 parent = network.parent[index];
			if (parent < 0) {
				continue;
			}

			const double areaRatio = Clamp(network.accumulation[index] / outletArea[outlet], 0.0, 1.0);
			const double fromRadius = Math::Lerp(minimumRadius, maximumRadius, std::pow(areaRatio, 0.4));
			double toRadius = maximumRadius;
			if ((network.land[parent] != 0) && (network.outlet[parent] == outlet)) {
				const double parentAreaRatio = Clamp(network.accumulation[parent] / outletArea[outlet], 0.0, 1.0);
				toRadius = Math::Lerp(minimumRadius, maximumRadius, std::pow(parentAreaRatio, 0.4));
			}

			rasterizeRiverSegment(image, network, index, parent, fromRadius, toRadius);
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

		// shore ノイズで陸が裏返らないように確率を下げる
		// 変更前: noiseShoreBoolAfter(image, 0.5);
		noiseShoreBoolAfter(image, 0.4);
		// 海岸確定後も外周から外海へ接続できるようにする
		forceOceanBorder(image, 40);
		// 擬似標高と集水量に基づく主要河川網を刻む
		carveRivers(image, 3, 1.2, 5.0);

		image.save(U"voronoi_hydrology_v1.png");
	}
}

class GameSceneBase : public App::Scene
{
public:
	GameSceneBase(const InitData& init) : IScene(init)
	{
		Image image(U"voronoi_hydrology_v1.png");

		int32 gridSizeWidth = (int32)(GaussianFSAddon::GetWindowSize().x - (1 * 2) - 16);
		int32 gridSizeHeight = GaussianFSAddon::GetWindowSize().y - 30 - (1 * 2) - 16;// -30は下のメニュー分、-1*2は枠線分

		if (image.isEmpty())
		{
			Grid<int32> grid(gridSizeWidth, gridSizeHeight);
			map::SimpleVoronoiIsland<int32> diagram(grid, 400, 0.4);
			map::noiseShorePoints(diagram.point, 20, 0.8);
			map::createVoronoiDiagram(diagram.point, diagram.color, grid.width(), grid.height());
			image = Image(U"voronoi_hydrology_v1.png");
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
