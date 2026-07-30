# pragma once
# include <Siv3D.hpp>

namespace app
{
	// 潜れる海プロトタイプ用の地形 + 水面。
	// 高さ場 h(x, z) を解析関数として持ち、描画メッシュと CPU クエリが常に一致する。
	class SeaTerrain
	{
	public:
		struct Settings
		{
			double terrainSize = 240.0;      // 地形一辺の長さ
			Size terrainResolution{ 240, 240 };
			double shoreX = 20.0;            // これより +X 側が海
			double beachWidth = 34.0;        // 海岸から海盆までの遷移幅
			double seaBasinDepth = 9.0;      // 海盆の深さ
			double landHillHeight = 0.55;    // 陸側のゆるい起伏
			double seaLevel = -0.25;         // 水面の高さ
			double fadeDepth = 5.0;          // 浅瀬→深海のグラデーション深度
			double waterBaseAlpha = 0.42;    // 水面の基礎不透明度
			double foamStrength = 0.85;      // 海岸線フォームの強度
			ColorF shallowColor{ 0.30, 0.78, 0.72, 1.0 }; // 浅瀬色
			ColorF deepColor{ 0.05, 0.19, 0.38, 1.0 };    // 深海色
		};

		SeaTerrain()
			: SeaTerrain{ Settings{} } {}

		explicit SeaTerrain(const Settings& settings)
			: m_settings{ settings }
			, m_terrainMesh{ makeTerrainMeshData(settings) }
			, m_waterMesh{ MeshData::TwoSidedPlane(Float2{ settings.terrainSize, settings.terrainSize }) }
			, m_waterSurfacePS{ HLSL{ U"example/shader/hlsl/water_surface.hlsl", U"PS" } } {}

		// 指定した XZ 座標の地形高さを返す
		[[nodiscard]] double getHeightAt(const double x, const double z) const noexcept
		{
			return HeightAt(m_settings, x, z);
		}

		// 指定した XZ 座標の水深を返す（陸地では 0）
		[[nodiscard]] double getDepthAt(const double x, const double z) const noexcept
		{
			return Max(0.0, (m_settings.seaLevel - getHeightAt(x, z)));
		}

		// 指定位置が水中かを返す
		[[nodiscard]] bool isUnderwater(const Vec3& position) const noexcept
		{
			return (position.y < m_settings.seaLevel) && (0.0 < getDepthAt(position.x, position.z));
		}

		// 水面の高さを返す
		[[nodiscard]] double seaLevel() const noexcept
		{
			return m_settings.seaLevel;
		}

		// 地形メッシュを返す
		[[nodiscard]] const Mesh& terrainMesh() const noexcept
		{
			return m_terrainMesh;
		}

		// 水面を深度着色シェーダーで描画する
		void drawWaterSurface() const
		{
			if (not m_waterSurfacePS)
			{
				drawWaterSurfaceFallback();
				return;
			}

			m_waterBuffer->terrain = Float4{
				static_cast<float>(m_settings.seaLevel),
				static_cast<float>(m_settings.shoreX),
				static_cast<float>(m_settings.beachWidth),
				static_cast<float>(m_settings.seaBasinDepth) };
			m_waterBuffer->params = Float4{
				static_cast<float>(m_settings.landHillHeight),
				static_cast<float>(m_settings.fadeDepth),
				static_cast<float>(Scene::Time()),
				static_cast<float>(m_settings.waterBaseAlpha) };
			const ColorF shallowLinear = m_settings.shallowColor.removeSRGBCurve();
			const ColorF deepLinear = m_settings.deepColor.removeSRGBCurve();
			m_waterBuffer->shallowColor = Float4{
				static_cast<float>(shallowLinear.r),
				static_cast<float>(shallowLinear.g),
				static_cast<float>(shallowLinear.b),
				static_cast<float>(m_settings.foamStrength) };
			m_waterBuffer->deepColor = Float4{
				static_cast<float>(deepLinear.r),
				static_cast<float>(deepLinear.g),
				static_cast<float>(deepLinear.b),
				0.0f };

			Graphics3D::SetPSConstantBuffer(2, m_waterBuffer);

			const ScopedRenderStates3D states{ BlendState::Default3D, RasterizerState::SolidCullNone };
			const ScopedCustomShader3D shader{ m_waterSurfacePS };
			const Transformer3D transform{ Mat4x4::Translate(0.0, m_settings.seaLevel, 0.0) };
			m_waterMesh.draw();
		}

	private:
		struct WaterShaderConstants
		{
			Float4 terrain;      // x: seaLevel, y: shoreX, z: beachWidth, w: seaBasinDepth
			Float4 params;       // x: landHillHeight, y: fadeDepth, z: time, w: baseAlpha
			Float4 shallowColor; // rgb: 浅瀬色(linear), a: フォーム強度
			Float4 deepColor;    // rgb: 深海色(linear), a: 予備
		};

		Settings m_settings;
		Mesh m_terrainMesh;
		Mesh m_waterMesh;
		PixelShader m_waterSurfacePS;
		mutable ConstantBuffer<WaterShaderConstants> m_waterBuffer;

		// シェーダーが読めない場合の予備描画
		void drawWaterSurfaceFallback() const
		{
			const ScopedRenderStates3D states{ BlendState::Default3D, RasterizerState::SolidCullNone };
			const Transformer3D transform{ Mat4x4::Translate(0.0, m_settings.seaLevel, 0.0) };
			m_waterMesh.draw(ColorF{ 0.16, 0.45, 0.58, 0.55 }.removeSRGBCurve());
		}

		// 解析的高さ場（描画と CPU クエリで共有）
		[[nodiscard]] static double HeightAt(const Settings& s, const double x, const double z) noexcept
		{
			// 陸側のゆるい起伏
			const double hills = s.landHillHeight * Math::Sin(x * 0.055) * Math::Cos(z * 0.047);

			// 海岸 → 海盆の滑らかな遷移
			const double basin = Math::Smoothstep(s.shoreX, (s.shoreX + s.beachWidth), x);

			// 海底のうねり
			const double seabedRipple = 0.35 * Math::Sin(x * 0.11 + z * 0.07) * basin;

			return (hills * (1.0 - basin)) - (s.seaBasinDepth * basin) + seabedRipple;
		}

		// 高さ場で変位させた地形メッシュを生成する
		[[nodiscard]] static MeshData makeTerrainMeshData(const Settings& s)
		{
			MeshData meshData = MeshData::OneSidedPlane(s.terrainSize, s.terrainResolution);

			for (auto& vertex : meshData.vertices)
			{
				vertex.pos.y = static_cast<float>(HeightAt(s, vertex.pos.x, vertex.pos.z));
			}

			meshData.computeNormals();
			return meshData;
		}
	};
}
