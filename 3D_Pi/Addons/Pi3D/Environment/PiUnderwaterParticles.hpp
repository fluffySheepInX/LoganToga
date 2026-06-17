# pragma once
# include <Siv3D.hpp>

namespace Pi3D
{
	struct PiUnderwaterParticleLayerSettings
	{
		int32 count = 80;
		double speedX = -8.0;
		double speedY = -2.0;
		double size = 1.6;
		double alpha = 0.18;
		double sway = 8.0;
		ColorF color{ 0.76, 0.92, 1.0, 1.0 };

		[[nodiscard]] bool operator ==(const PiUnderwaterParticleLayerSettings& other) const = default;
	};

	class PiUnderwaterParticles
	{
	public:
		static constexpr size_t LayerCount = 3;

		// 水中浮遊物のレイヤー設定を更新
		void setLayerSettings(const size_t index, const PiUnderwaterParticleLayerSettings& settings)
		{
			if (LayerCount <= index)
			{
				return;
			}

			m_layers[index].settings = sanitizeSettings(settings);
			rebuildLayerIfNeeded(index);
		}

		// 水中浮遊物のレイヤー設定を取得
		[[nodiscard]] PiUnderwaterParticleLayerSettings getLayerSettings(const size_t index) const
		{
			if (LayerCount <= index)
			{
				return {};
			}

			return m_layers[index].settings;
		}

		// 水中浮遊物の位置を更新
		void update(const double deltaTime, const Size& sceneSize)
		{
			const Vec2 safeSize{ Max(1, sceneSize.x), Max(1, sceneSize.y) };
			m_time += deltaTime;

			for (size_t layerIndex = 0; layerIndex < LayerCount; ++layerIndex)
			{
				rebuildLayerIfNeeded(layerIndex);
				auto& layer = m_layers[layerIndex];
				const auto& settings = layer.settings;

				for (auto& particle : layer.particles)
				{
					particle.position.x += (settings.speedX * particle.speedScale * deltaTime);
					particle.position.y += (settings.speedY * particle.speedScale * deltaTime);

					if (particle.position.x < -24.0)
					{
						particle.position.x += safeSize.x + 48.0;
						particle.position.y = Random(-24.0, safeSize.y + 24.0);
					}
					else if ((safeSize.x + 24.0) < particle.position.x)
					{
						particle.position.x -= safeSize.x + 48.0;
						particle.position.y = Random(-24.0, safeSize.y + 24.0);
					}

					if (particle.position.y < -24.0)
					{
						particle.position.y += safeSize.y + 48.0;
						particle.position.x = Random(-24.0, safeSize.x + 24.0);
					}
					else if ((safeSize.y + 24.0) < particle.position.y)
					{
						particle.position.y -= safeSize.y + 48.0;
						particle.position.x = Random(-24.0, safeSize.x + 24.0);
					}
				}
			}
		}

		// 水中浮遊物を 2D オーバーレイとして描画
		void draw2D(const double globalAmount = 1.0) const
		{
			const double amount = Clamp(globalAmount, 0.0, 1.0);
			if (amount <= 0.0)
			{
				return;
			}

			const ScopedRenderStates2D blend{ BlendState::Additive };
			for (size_t layerIndex = 0; layerIndex < LayerCount; ++layerIndex)
			{
				const auto& layer = m_layers[layerIndex];
				const auto& settings = layer.settings;
				const double layerAlpha = Clamp(settings.alpha * amount, 0.0, 1.0);
				if (layerAlpha <= 0.0)
				{
					continue;
				}

				for (const auto& particle : layer.particles)
				{
					const double sway = Math::Sin(m_time * (0.55 + particle.speedScale * 0.35) + particle.phase) * settings.sway;
					const Vec2 pos = particle.position.movedBy(sway, 0.0);
					const double radius = Max(0.2, settings.size * particle.radiusScale);
					const ColorF color{ settings.color.r, settings.color.g, settings.color.b, layerAlpha * particle.alphaScale };
					Circle{ pos, radius }.draw(color.removeSRGBCurve());
				}
			}
		}

	private:
		struct Particle
		{
			Vec2 position;
			double phase = 0.0;
			double radiusScale = 1.0;
			double alphaScale = 1.0;
			double speedScale = 1.0;
		};

		struct Layer
		{
			PiUnderwaterParticleLayerSettings settings;
			Array<Particle> particles;
			int32 builtCount = -1;
		};

		// レイヤー設定値を安全な範囲へ丸める
		[[nodiscard]] static PiUnderwaterParticleLayerSettings sanitizeSettings(PiUnderwaterParticleLayerSettings settings)
		{
			settings.count = Clamp(settings.count, 0, 2000);
			settings.size = Clamp(settings.size, 0.2, 12.0);
			settings.alpha = Clamp(settings.alpha, 0.0, 1.0);
			settings.sway = Clamp(settings.sway, 0.0, 80.0);
			settings.color.r = Clamp(settings.color.r, 0.0, 1.0);
			settings.color.g = Clamp(settings.color.g, 0.0, 1.0);
			settings.color.b = Clamp(settings.color.b, 0.0, 1.0);
			settings.color.a = 1.0;
			return settings;
		}

		// 必要な場合だけレイヤーの粒子配列を作り直す
		void rebuildLayerIfNeeded(const size_t index)
		{
			auto& layer = m_layers[index];
			const int32 count = Max(0, layer.settings.count);
			if (layer.builtCount == count)
			{
				return;
			}

			layer.particles.clear();
			layer.particles.reserve(count);
			const Vec2 sceneSize = Scene::Size();
			for (int32 i = 0; i < count; ++i)
			{
				layer.particles << Particle{
					.position = Vec2{ Random(-24.0, sceneSize.x + 24.0), Random(-24.0, sceneSize.y + 24.0) },
					.phase = Random(0.0, Math::TwoPi),
					.radiusScale = Random(0.65, 1.45),
					.alphaScale = Random(0.45, 1.0),
					.speedScale = Random(0.65, 1.35),
				};
			}
			layer.builtCount = count;
		}

		std::array<Layer, LayerCount> m_layers = {
			Layer{ PiUnderwaterParticleLayerSettings{ .count = 90, .speedX = -4.0, .speedY = -1.2, .size = 2.4, .alpha = 0.13, .sway = 12.0, .color = ColorF{ 0.68, 0.90, 1.0, 1.0 } } },
			Layer{ PiUnderwaterParticleLayerSettings{ .count = 150, .speedX = -8.0, .speedY = -2.2, .size = 1.4, .alpha = 0.16, .sway = 8.0, .color = ColorF{ 0.75, 0.95, 1.0, 1.0 } } },
			Layer{ PiUnderwaterParticleLayerSettings{ .count = 260, .speedX = -13.0, .speedY = -3.0, .size = 0.75, .alpha = 0.10, .sway = 5.0, .color = ColorF{ 0.82, 0.98, 1.0, 1.0 } } },
		};
		double m_time = 0.0;
	};
}
