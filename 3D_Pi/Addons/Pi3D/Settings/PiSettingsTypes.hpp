# pragma once
# include <Siv3D.hpp>
# include "../Environment/PiUnderwaterParticles.hpp"

namespace Pi3D
{
	struct EffectChainSettings
	{
		Array<String> chainEffectNames{ U"なし" };
		Array<bool> chainEnabled{ true };

		[[nodiscard]] bool operator ==(const EffectChainSettings& other) const = default;
	};

	struct LightingSettings
	{
		size_t presetIndex = 1;
		double sunIntensityScale = 1.0;
		double ambientIntensityScale = 1.0;
		Optional<size_t> sunDirectionOverride;
		bool kickerEnabled = false;
		bool kickerRightSide = true;
		double kickerIntensity = 0.25;
		double kickerYawDegrees = 120.0;
		double kickerHeight = 0.2;
		ColorF kickerColor{ 1.0, 0.95, 0.9, 1.0 };

		[[nodiscard]] bool operator ==(const LightingSettings& other) const = default;
	};

	struct RainSettings
	{
		bool enabled = false;
		int32 dropCount = 420;
		double fallSpeed = 24.0;
		double windStrength = 0.12;
		double streakLength = 1.8;
		double alpha = 0.55;

		[[nodiscard]] bool operator ==(const RainSettings& other) const = default;
	};

	struct FogSettings
	{
		bool enabled = false;
		double startDistance = 22.0;
		double endDistance = 90.0;
		double density = 0.65;
		ColorF color{ 0.58, 0.66, 0.76, 1.0 };

		[[nodiscard]] bool operator ==(const FogSettings& other) const = default;
	};

	struct UnderwaterSettings
	{
		bool enabled = false;
		double fogStartDistance = 4.0;
		double fogEndDistance = 70.0;
		double fogDensity = 0.82;
		ColorF fogColor{ 0.08, 0.32, 0.43, 1.0 };
		double distortionStrength = 0.006;
		double distortionSpeed = 0.65;
		double distortionScale = 18.0;
		double particleAmount = 0.75;
		PiUnderwaterParticleLayerSettings nearParticles{ .count = 90, .speedX = -4.0, .speedY = -1.2, .size = 2.4, .alpha = 0.13, .sway = 12.0, .color = ColorF{ 0.68, 0.90, 1.0, 1.0 } };
		PiUnderwaterParticleLayerSettings midParticles{ .count = 150, .speedX = -8.0, .speedY = -2.2, .size = 1.4, .alpha = 0.16, .sway = 8.0, .color = ColorF{ 0.75, 0.95, 1.0, 1.0 } };
		PiUnderwaterParticleLayerSettings farParticles{ .count = 260, .speedX = -13.0, .speedY = -3.0, .size = 0.75, .alpha = 0.10, .sway = 5.0, .color = ColorF{ 0.82, 0.98, 1.0, 1.0 } };

		[[nodiscard]] bool operator ==(const UnderwaterSettings& other) const = default;
	};

	struct EnvironmentSettings
	{
		String groundMode = U"Texture";
		RainSettings rain;
		FogSettings fog;
		UnderwaterSettings underwater;

		[[nodiscard]] bool operator ==(const EnvironmentSettings& other) const = default;
	};

	struct PerformanceSettings
	{
		bool vSyncEnabled = true;
		int32 maxFPS = 60;
		bool powerSavingMode = true;
		int32 idleFPS = 30;
		int32 backgroundFPS = 10;

		[[nodiscard]] bool operator ==(const PerformanceSettings& other) const = default;
	};

	struct PersistentSettings
	{
		int32 schemaVersion = 1;
		LightingSettings lighting;
		EnvironmentSettings environment;
		PerformanceSettings performance;
		EffectChainSettings effects;
		bool panelCollapsed = false;
		double panelPosX = 16.0;
		double panelPosY = 96.0;

		[[nodiscard]] bool operator ==(const PersistentSettings& other) const = default;
	};
}
