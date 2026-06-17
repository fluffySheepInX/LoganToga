namespace Pi3D
{
	// 環境アニメーションを更新
	inline void PiEnvironment::update(const double deltaTime)
	{
		m_rain.update(deltaTime);
		if (m_underwaterEnabled)
		{
			m_underwaterParticles.update(deltaTime, Scene::Size());
		}
	}

	// 地面を描画
	inline void PiEnvironment::drawGround(const Mesh& groundPlane, const Texture& groundTexture) const
	{
		if (m_groundMode == PiGroundMode::None)
		{
			return;
		}

		if (m_groundMode == PiGroundMode::Gray)
		{
			groundPlane.draw(ColorF{ 0.45, 0.45, 0.45 }.removeSRGBCurve());
		}
		else if (m_groundMode == PiGroundMode::White)
		{
			groundPlane.draw(ColorF{ 0.92, 0.92, 0.92 }.removeSRGBCurve());
		}
		else if (m_groundMode == PiGroundMode::Grid)
		{
			groundPlane.draw(ColorF{ 0.22, 0.24, 0.28 }.removeSRGBCurve());

			const ScopedRenderStates3D wireframe{ BlendState::Opaque, RasterizerState::WireframeCullNone };
			groundPlane.draw(ColorF{ 0.78, 0.82, 0.88, 0.95 }.removeSRGBCurve());
		}
		else
		{
			groundPlane.draw(groundTexture);
		}
	}

	// 3D 環境要素を描画
	inline void PiEnvironment::draw3D() const
	{
		m_rain.draw3D();
	}

	// シーン深度が必要かを返す
	inline bool PiEnvironment::needsSceneDepth() const
	{
		return (m_fogEnabled || m_underwaterEnabled);
	}

	// 大気合成パスが必要かを返す
	inline bool PiEnvironment::hasAtmospherePass() const
	{
		return (m_fogEnabled || m_underwaterEnabled);
	}

	// 水中後処理が必要かを返す
	inline bool PiEnvironment::hasUnderwaterPostProcess() const
	{
		return m_underwaterEnabled;
	}

	// 状態に応じて通常フォグまたは水中フォグを適用
	inline void PiEnvironment::applyAtmosphere(const Texture& source, const Texture& depthTexture) const
	{
		if (m_underwaterEnabled)
		{
			applyUnderwaterFog(source, depthTexture);
			return;
		}

		applyFog(source, depthTexture);
	}

	// 通常フォグを適用
	inline void PiEnvironment::applyFog(const Texture& source, const Texture& depthTexture) const
	{
		if (not m_fogEnabled)
		{
			source.draw();
			return;
		}

		m_fogParams = Float4{
			static_cast<float>(m_fogStartDistance),
			static_cast<float>(Max(m_fogStartDistance + 0.01, m_fogEndDistance)),
			static_cast<float>(m_fogDensity),
			0.0f };
		m_fogColorBuffer = Float4{
			static_cast<float>(m_fogColor.r),
			static_cast<float>(m_fogColor.g),
			static_cast<float>(m_fogColor.b),
			static_cast<float>(m_fogColor.a) };

		Graphics2D::SetPSConstantBuffer(1, m_fogParams);
		Graphics2D::SetPSConstantBuffer(2, m_fogColorBuffer);
		Graphics2D::SetPSTexture(1, depthTexture);

		{
			const ScopedCustomShader2D shader{ m_fogPS };
			source.draw();
		}
		Graphics2D::Flush();
		Graphics2D::SetPSTexture(1, none);
	}

	// 水中フォグを適用
	inline void PiEnvironment::applyUnderwaterFog(const Texture& source, const Texture& depthTexture) const
	{
		if (not m_underwaterEnabled)
		{
			source.draw();
			return;
		}

		m_underwaterFogParams = Float4{
			static_cast<float>(m_underwaterFogStartDistance),
			static_cast<float>(Max(m_underwaterFogStartDistance + 0.01, m_underwaterFogEndDistance)),
			static_cast<float>(m_underwaterFogDensity),
			static_cast<float>(Scene::Time()) };
		m_underwaterFogColorBuffer = Float4{
			static_cast<float>(m_underwaterFogColor.r),
			static_cast<float>(m_underwaterFogColor.g),
			static_cast<float>(m_underwaterFogColor.b),
			static_cast<float>(m_underwaterFogColor.a) };

		Graphics2D::SetPSConstantBuffer(1, m_underwaterFogParams);
		Graphics2D::SetPSConstantBuffer(2, m_underwaterFogColorBuffer);
		Graphics2D::SetPSTexture(1, depthTexture);

		{
			const ScopedCustomShader2D shader{ m_underwaterFogPS };
			source.draw();
		}
		Graphics2D::Flush();
		Graphics2D::SetPSTexture(1, none);
	}

	// 水中の屈折を適用
	inline void PiEnvironment::applyUnderwaterDistortion(const Texture& source, const Texture& depthTexture) const
	{
		if ((not m_underwaterEnabled) || (m_underwaterDistortionStrength <= 0.0))
		{
			source.draw();
			return;
		}

		m_underwaterDistortionParams = Float4{
			static_cast<float>(m_underwaterDistortionStrength),
			static_cast<float>(Scene::Time()),
			static_cast<float>(m_underwaterDistortionSpeed),
			static_cast<float>(m_underwaterDistortionScale) };

		Graphics2D::SetPSConstantBuffer(1, m_underwaterDistortionParams);
		Graphics2D::SetPSTexture(1, depthTexture);

		{
			const ScopedCustomShader2D shader{ m_underwaterDistortionPS };
			source.draw();
		}
		Graphics2D::Flush();
		Graphics2D::SetPSTexture(1, none);
	}

	// 水中浮遊物を 2D オーバーレイ描画
	inline void PiEnvironment::drawUnderwaterParticles() const
	{
		if (not m_underwaterEnabled)
		{
			return;
		}

		m_underwaterParticles.draw2D(m_underwaterParticleAmount);
	}

	// 現在の環境設定を取得
	inline Pi3D::EnvironmentSettings PiEnvironment::getSettings() const
	{
		Pi3D::EnvironmentSettings settings;
		settings.groundMode = groundModeToString(m_groundMode);

		const auto rain = m_rain.getSettings();
		settings.rain.enabled = rain.enabled;
		settings.rain.dropCount = rain.dropCount;
		settings.rain.fallSpeed = rain.fallSpeed;
		settings.rain.windStrength = rain.windStrength;
		settings.rain.streakLength = rain.streakLength;
		settings.rain.alpha = rain.alpha;

		settings.fog.enabled = m_fogEnabled;
		settings.fog.startDistance = m_fogStartDistance;
		settings.fog.endDistance = m_fogEndDistance;
		settings.fog.density = m_fogDensity;
		settings.fog.color = m_fogColor;
		settings.underwater.enabled = m_underwaterEnabled;
		settings.underwater.fogStartDistance = m_underwaterFogStartDistance;
		settings.underwater.fogEndDistance = m_underwaterFogEndDistance;
		settings.underwater.fogDensity = m_underwaterFogDensity;
		settings.underwater.fogColor = m_underwaterFogColor;
		settings.underwater.distortionStrength = m_underwaterDistortionStrength;
		settings.underwater.distortionSpeed = m_underwaterDistortionSpeed;
		settings.underwater.distortionScale = m_underwaterDistortionScale;
		settings.underwater.particleAmount = m_underwaterParticleAmount;
		settings.underwater.nearParticles = m_underwaterParticles.getLayerSettings(0);
		settings.underwater.midParticles = m_underwaterParticles.getLayerSettings(1);
		settings.underwater.farParticles = m_underwaterParticles.getLayerSettings(2);
		return settings;
	}

	// 保存済み環境設定を適用
	inline void PiEnvironment::applySettings(const Pi3D::EnvironmentSettings& settings)
	{
		m_groundMode = stringToGroundMode(settings.groundMode);

		PiRain::Settings rainSettings;
		rainSettings.enabled = settings.rain.enabled;
		rainSettings.dropCount = settings.rain.dropCount;
		rainSettings.fallSpeed = settings.rain.fallSpeed;
		rainSettings.windStrength = settings.rain.windStrength;
		rainSettings.streakLength = settings.rain.streakLength;
		rainSettings.alpha = settings.rain.alpha;
		m_rain.setSettings(rainSettings);

		m_fogEnabled = settings.fog.enabled;
		m_fogStartDistance = Max(0.0, settings.fog.startDistance);
		m_fogEndDistance = Max(m_fogStartDistance + 1.0, settings.fog.endDistance);
		m_fogDensity = Clamp(settings.fog.density, 0.0, 1.0);
		m_fogColor = settings.fog.color;

		m_underwaterEnabled = settings.underwater.enabled;
		m_underwaterFogStartDistance = Max(0.0, settings.underwater.fogStartDistance);
		m_underwaterFogEndDistance = Max(m_underwaterFogStartDistance + 1.0, settings.underwater.fogEndDistance);
		m_underwaterFogDensity = Clamp(settings.underwater.fogDensity, 0.0, 1.0);
		m_underwaterFogColor = settings.underwater.fogColor;
		m_underwaterDistortionStrength = Clamp(settings.underwater.distortionStrength, 0.0, 0.05);
		m_underwaterDistortionSpeed = Clamp(settings.underwater.distortionSpeed, 0.0, 5.0);
		m_underwaterDistortionScale = Clamp(settings.underwater.distortionScale, 1.0, 80.0);
		m_underwaterParticleAmount = Clamp(settings.underwater.particleAmount, 0.0, 1.0);
		m_underwaterParticles.setLayerSettings(0, settings.underwater.nearParticles);
		m_underwaterParticles.setLayerSettings(1, settings.underwater.midParticles);
		m_underwaterParticles.setLayerSettings(2, settings.underwater.farParticles);
	}
}
