# pragma once
# include <Siv3D.hpp>
# include "PiSettingsTypes.hpp"

namespace Pi3D
{
	[[nodiscard]] inline String EscapeTomlString(String value)
	{
		value.replace(U"\\", U"\\\\");
		value.replace(U"\"", U"\\\"");
		return value;
	}

	[[nodiscard]] inline String QuoteTomlString(const String& value)
	{
		return (U'"' + EscapeTomlString(value) + U'"');
	}

	[[nodiscard]] inline FilePath GetSettingsPath()
	{
		return U"save/pi3d_settings.toml";
	}

	[[nodiscard]] inline PersistentSettings LoadSettings()
	{
		PersistentSettings settings;
		const TOMLReader toml{ GetSettingsPath() };
		if (not toml)
		{
			return settings;
		}

		try
		{
			if (const auto schemaVersion = toml[U"schemaVersion"].getOpt<int32>())
			{
				settings.schemaVersion = *schemaVersion;
			}
			if (const auto presetIndex = toml[U"lightingPresetIndex"].getOpt<int32>())
			{
				settings.lighting.presetIndex = static_cast<size_t>(Max(0, *presetIndex));
			}
			if (const auto sunIntensityScale = toml[U"sunIntensityScale"].getOpt<double>())
			{
				settings.lighting.sunIntensityScale = *sunIntensityScale;
			}
			if (const auto ambientIntensityScale = toml[U"ambientIntensityScale"].getOpt<double>())
			{
				settings.lighting.ambientIntensityScale = *ambientIntensityScale;
			}
			if (const auto sunDirectionOverride = toml[U"sunDirectionOverride"].getOpt<int32>())
			{
				if (0 <= *sunDirectionOverride)
				{
					settings.lighting.sunDirectionOverride = static_cast<size_t>(*sunDirectionOverride % 8);
				}
			}
			if (const auto kickerEnabled = toml[U"kickerEnabled"].getOpt<bool>())
			{
				settings.lighting.kickerEnabled = *kickerEnabled;
			}
			if (const auto kickerRightSide = toml[U"kickerRightSide"].getOpt<bool>())
			{
				settings.lighting.kickerRightSide = *kickerRightSide;
			}
			if (const auto kickerIntensity = toml[U"kickerIntensity"].getOpt<double>())
			{
				settings.lighting.kickerIntensity = Clamp(*kickerIntensity, 0.0, 2.0);
			}
			if (const auto kickerYawDegrees = toml[U"kickerYawDegrees"].getOpt<double>())
			{
				settings.lighting.kickerYawDegrees = Clamp(*kickerYawDegrees, 45.0, 170.0);
			}
			if (const auto kickerHeight = toml[U"kickerHeight"].getOpt<double>())
			{
				settings.lighting.kickerHeight = Clamp(*kickerHeight, -0.3, 0.8);
			}
			if (const auto kickerColorNode = toml[U"kickerColor"]; kickerColorNode.isArray())
			{
				Array<double> colorValues;
				for (const auto& item : kickerColorNode.arrayView())
				{
					if (const auto value = item.getOpt<double>())
					{
						colorValues << Clamp(*value, 0.0, 1.0);
					}
					else if (const auto intValue = item.getOpt<int32>())
					{
						colorValues << Clamp(static_cast<double>(*intValue) / 255.0, 0.0, 1.0);
					}
					if (colorValues.size() >= 4)
					{
						break;
					}
				}
				if (colorValues.size() >= 3)
				{
					settings.lighting.kickerColor = ColorF{ colorValues[0], colorValues[1], colorValues[2], (colorValues.size() >= 4 ? colorValues[3] : 1.0) };
				}
			}
			if (const auto groundMode = toml[U"groundMode"].getOpt<String>())
			{
				settings.environment.groundMode = *groundMode;
			}
			if (const auto rainEnabled = toml[U"rainEnabled"].getOpt<bool>())
			{
				settings.environment.rain.enabled = *rainEnabled;
			}
			if (const auto rainDropCount = toml[U"rainDropCount"].getOpt<int32>())
			{
				settings.environment.rain.dropCount = Max(0, *rainDropCount);
			}
			if (const auto rainFallSpeed = toml[U"rainFallSpeed"].getOpt<double>())
			{
				settings.environment.rain.fallSpeed = rainFallSpeed.value_or(settings.environment.rain.fallSpeed);
			}
			if (const auto rainWindStrength = toml[U"rainWindStrength"].getOpt<double>())
			{
				settings.environment.rain.windStrength = *rainWindStrength;
			}
			if (const auto rainStreakLength = toml[U"rainStreakLength"].getOpt<double>())
			{
				settings.environment.rain.streakLength = *rainStreakLength;
			}
			if (const auto rainAlpha = toml[U"rainAlpha"].getOpt<double>())
			{
				settings.environment.rain.alpha = *rainAlpha;
			}
			if (const auto fogEnabled = toml[U"fogEnabled"].getOpt<bool>())
			{
				settings.environment.fog.enabled = *fogEnabled;
			}
			if (const auto fogStartDistance = toml[U"fogStartDistance"].getOpt<double>())
			{
				settings.environment.fog.startDistance = Max(0.0, *fogStartDistance);
			}
			if (const auto fogEndDistance = toml[U"fogEndDistance"].getOpt<double>())
			{
				settings.environment.fog.endDistance = Max(0.0, *fogEndDistance);
			}
			if (const auto fogDensity = toml[U"fogDensity"].getOpt<double>())
			{
				settings.environment.fog.density = Clamp(*fogDensity, 0.0, 1.0);
			}
			if (const auto fogColorNode = toml[U"fogColor"]; fogColorNode.isArray())
			{
				Array<double> colorValues;
				for (const auto& item : fogColorNode.arrayView())
				{
					if (const auto value = item.getOpt<double>())
					{
						colorValues << Clamp(*value, 0.0, 1.0);
					}
					else if (const auto intValue = item.getOpt<int32>())
					{
						colorValues << Clamp(static_cast<double>(*intValue) / 255.0, 0.0, 1.0);
					}
					if (colorValues.size() >= 4)
					{
						break;
					}
				}
				if (colorValues.size() >= 3)
				{
					settings.environment.fog.color = ColorF{ colorValues[0], colorValues[1], colorValues[2], (colorValues.size() >= 4 ? colorValues[3] : 1.0) };
				}
			}
			if (const auto underwaterEnabled = toml[U"underwaterEnabled"].getOpt<bool>())
			{
				settings.environment.underwater.enabled = *underwaterEnabled;
			}
			if (const auto underwaterFogStartDistance = toml[U"underwaterFogStartDistance"].getOpt<double>())
			{
				settings.environment.underwater.fogStartDistance = Max(0.0, *underwaterFogStartDistance);
			}
			if (const auto underwaterFogEndDistance = toml[U"underwaterFogEndDistance"].getOpt<double>())
			{
				settings.environment.underwater.fogEndDistance = Max(0.0, *underwaterFogEndDistance);
			}
			if (const auto underwaterFogDensity = toml[U"underwaterFogDensity"].getOpt<double>())
			{
				settings.environment.underwater.fogDensity = Clamp(*underwaterFogDensity, 0.0, 1.0);
			}
			if (const auto underwaterFogColorNode = toml[U"underwaterFogColor"]; underwaterFogColorNode.isArray())
			{
				Array<double> colorValues;
				for (const auto& item : underwaterFogColorNode.arrayView())
				{
					if (const auto doubleValue = item.getOpt<double>())
					{
						colorValues << Clamp(*doubleValue, 0.0, 1.0);
					}
					else if (const auto intValue = item.getOpt<int32>())
					{
						colorValues << Clamp(static_cast<double>(*intValue) / 255.0, 0.0, 1.0);
					}
					if (colorValues.size() >= 4)
					{
						break;
					}
				}
				if (colorValues.size() >= 3)
				{
					settings.environment.underwater.fogColor = ColorF{ colorValues[0], colorValues[1], colorValues[2], (colorValues.size() >= 4 ? colorValues[3] : 1.0) };
				}
			}
			if (const auto underwaterDistortionStrength = toml[U"underwaterDistortionStrength"].getOpt<double>())
			{
				settings.environment.underwater.distortionStrength = Clamp(*underwaterDistortionStrength, 0.0, 0.05);
			}
			if (const auto underwaterDistortionSpeed = toml[U"underwaterDistortionSpeed"].getOpt<double>())
			{
				settings.environment.underwater.distortionSpeed = Clamp(*underwaterDistortionSpeed, 0.0, 5.0);
			}
			if (const auto underwaterDistortionScale = toml[U"underwaterDistortionScale"].getOpt<double>())
			{
				settings.environment.underwater.distortionScale = Clamp(*underwaterDistortionScale, 1.0, 80.0);
			}
			if (const auto underwaterParticleAmount = toml[U"underwaterParticleAmount"].getOpt<double>())
			{
				settings.environment.underwater.particleAmount = Clamp(*underwaterParticleAmount, 0.0, 1.0);
			}
			const auto loadParticleLayer = [&](StringView key, PiUnderwaterParticleLayerSettings& layer)
			{
				const auto node = toml[String{ key }];
				if (not node.isArray())
				{
					return;
				}

				Array<double> values;
				for (const auto& item : node.arrayView())
				{
					if (const auto value = item.getOpt<double>())
					{
						values << *value;
					}
					else if (const auto intValue = item.getOpt<int32>())
					{
						values << static_cast<double>(*intValue);
					}
				}

				if (values.size() < 9)
				{
					return;
				}

				layer.count = Clamp(static_cast<int32>(std::round(values[0])), 0, 2000);
				layer.speedX = values[1];
				layer.speedY = values[2];
				layer.size = Clamp(values[3], 0.2, 12.0);
				layer.alpha = Clamp(values[4], 0.0, 1.0);
				layer.sway = Clamp(values[5], 0.0, 80.0);
				layer.color = ColorF{ Clamp(values[6], 0.0, 1.0), Clamp(values[7], 0.0, 1.0), Clamp(values[8], 0.0, 1.0), 1.0 };
			};
			loadParticleLayer(U"underwaterNearParticles", settings.environment.underwater.nearParticles);
			loadParticleLayer(U"underwaterMidParticles", settings.environment.underwater.midParticles);
			loadParticleLayer(U"underwaterFarParticles", settings.environment.underwater.farParticles);
			Array<String> chainEffectNames;
			const auto effectChainNode = toml[U"effectChain"];
			if (effectChainNode.isArray())
			{
				for (const auto& effectNameNode : effectChainNode.arrayView())
				{
					if (const auto effectName = effectNameNode.getOpt<String>())
					{
						chainEffectNames << *effectName;
					}
				}
			}
			if (not chainEffectNames.isEmpty())
			{
				settings.effects.chainEffectNames = chainEffectNames;
				settings.effects.chainEnabled.assign(chainEffectNames.size(), true);
			}

			Array<bool> chainEnabled;
			const auto effectChainEnabledNode = toml[U"effectChainEnabled"];
			if (effectChainEnabledNode.isArray())
			{
				for (const auto& enabledNode : effectChainEnabledNode.arrayView())
				{
					chainEnabled << enabledNode.getOpt<bool>().value_or(true);
				}
			}
			if (not chainEnabled.isEmpty())
			{
				settings.effects.chainEnabled = chainEnabled;
				if (settings.effects.chainEnabled.size() < settings.effects.chainEffectNames.size())
				{
					settings.effects.chainEnabled.resize(settings.effects.chainEffectNames.size(), true);
				}
			}
			if (const auto panelCollapsed = toml[U"panelCollapsed"].getOpt<bool>())
			{
				settings.panelCollapsed = *panelCollapsed;
			}
			if (const auto vSyncEnabled = toml[U"vSyncEnabled"].getOpt<bool>())
			{
				settings.performance.vSyncEnabled = *vSyncEnabled;
			}
			if (const auto maxFPS = toml[U"maxFPS"].getOpt<int32>())
			{
				settings.performance.maxFPS = Clamp(*maxFPS, 15, 240);
			}
			if (const auto powerSavingMode = toml[U"powerSavingMode"].getOpt<bool>())
			{
				settings.performance.powerSavingMode = *powerSavingMode;
			}
			if (const auto idleFPS = toml[U"idleFPS"].getOpt<int32>())
			{
				settings.performance.idleFPS = Clamp(*idleFPS, 5, 120);
			}
			if (const auto backgroundFPS = toml[U"backgroundFPS"].getOpt<int32>())
			{
				settings.performance.backgroundFPS = Clamp(*backgroundFPS, 1, 60);
			}
			if (const auto panelPosX = toml[U"panelPosX"].getOpt<double>())
			{
				settings.panelPosX = *panelPosX;
			}
			if (const auto panelPosY = toml[U"panelPosY"].getOpt<double>())
			{
				settings.panelPosY = *panelPosY;
			}
		}
		catch (const std::exception&)
		{
		}

		return settings;
	}

	inline bool SaveSettings(const PersistentSettings& settings)
	{
		const FilePath path = GetSettingsPath();
		const String directory = FileSystem::ParentPath(path);
		if (not directory.isEmpty())
		{
			FileSystem::CreateDirectories(directory);
		}

		TextWriter writer{ path };
		if (not writer)
		{
			return false;
		}

		writer.writeln(U"schemaVersion = {}"_fmt(settings.schemaVersion));
		writer.writeln(U"lightingPresetIndex = {}"_fmt(settings.lighting.presetIndex));
		writer.writeln(U"sunIntensityScale = {:.6f}"_fmt(settings.lighting.sunIntensityScale));
		writer.writeln(U"ambientIntensityScale = {:.6f}"_fmt(settings.lighting.ambientIntensityScale));
		writer.writeln(U"sunDirectionOverride = {}"_fmt(settings.lighting.sunDirectionOverride ? Format(*settings.lighting.sunDirectionOverride) : U"-1"));
		writer.writeln(U"kickerEnabled = {}"_fmt(settings.lighting.kickerEnabled ? U"true" : U"false"));
		writer.writeln(U"kickerRightSide = {}"_fmt(settings.lighting.kickerRightSide ? U"true" : U"false"));
		writer.writeln(U"kickerIntensity = {:.6f}"_fmt(settings.lighting.kickerIntensity));
		writer.writeln(U"kickerYawDegrees = {:.6f}"_fmt(settings.lighting.kickerYawDegrees));
		writer.writeln(U"kickerHeight = {:.6f}"_fmt(settings.lighting.kickerHeight));
		writer.writeln(U"kickerColor = [{:.6f}, {:.6f}, {:.6f}, {:.6f}]"_fmt(
			settings.lighting.kickerColor.r,
			settings.lighting.kickerColor.g,
			settings.lighting.kickerColor.b,
			settings.lighting.kickerColor.a));
		writer.writeln(U"groundMode = {}"_fmt(QuoteTomlString(settings.environment.groundMode)));
		writer.writeln(U"rainEnabled = {}"_fmt(settings.environment.rain.enabled ? U"true" : U"false"));
		writer.writeln(U"rainDropCount = {}"_fmt(settings.environment.rain.dropCount));
		writer.writeln(U"rainFallSpeed = {:.6f}"_fmt(settings.environment.rain.fallSpeed));
		writer.writeln(U"rainWindStrength = {:.6f}"_fmt(settings.environment.rain.windStrength));
		writer.writeln(U"rainStreakLength = {:.6f}"_fmt(settings.environment.rain.streakLength));
		writer.writeln(U"rainAlpha = {:.6f}"_fmt(settings.environment.rain.alpha));
		writer.writeln(U"fogEnabled = {}"_fmt(settings.environment.fog.enabled ? U"true" : U"false"));
		writer.writeln(U"fogStartDistance = {:.6f}"_fmt(settings.environment.fog.startDistance));
		writer.writeln(U"fogEndDistance = {:.6f}"_fmt(settings.environment.fog.endDistance));
		writer.writeln(U"fogDensity = {:.6f}"_fmt(settings.environment.fog.density));
		writer.writeln(U"fogColor = [{:.6f}, {:.6f}, {:.6f}, {:.6f}]"_fmt(
			settings.environment.fog.color.r,
			settings.environment.fog.color.g,
			settings.environment.fog.color.b,
			settings.environment.fog.color.a));
		writer.writeln(U"underwaterEnabled = {}"_fmt(settings.environment.underwater.enabled ? U"true" : U"false"));
		writer.writeln(U"underwaterFogStartDistance = {:.6f}"_fmt(settings.environment.underwater.fogStartDistance));
		writer.writeln(U"underwaterFogEndDistance = {:.6f}"_fmt(settings.environment.underwater.fogEndDistance));
		writer.writeln(U"underwaterFogDensity = {:.6f}"_fmt(settings.environment.underwater.fogDensity));
		writer.writeln(U"underwaterFogColor = [{:.6f}, {:.6f}, {:.6f}, {:.6f}]"_fmt(
			settings.environment.underwater.fogColor.r,
			settings.environment.underwater.fogColor.g,
			settings.environment.underwater.fogColor.b,
			settings.environment.underwater.fogColor.a));
		writer.writeln(U"underwaterDistortionStrength = {:.6f}"_fmt(settings.environment.underwater.distortionStrength));
		writer.writeln(U"underwaterDistortionSpeed = {:.6f}"_fmt(settings.environment.underwater.distortionSpeed));
		writer.writeln(U"underwaterDistortionScale = {:.6f}"_fmt(settings.environment.underwater.distortionScale));
		writer.writeln(U"underwaterParticleAmount = {:.6f}"_fmt(settings.environment.underwater.particleAmount));

		const auto writeParticleLayer = [&](StringView key, const PiUnderwaterParticleLayerSettings& layer)
		{
			writer.writeln(U"{} = [{}, {:.6f}, {:.6f}, {:.6f}, {:.6f}, {:.6f}, {:.6f}, {:.6f}, {:.6f}]"_fmt(
				key,
				layer.count,
				layer.speedX,
				layer.speedY,
				layer.size,
				layer.alpha,
				layer.sway,
				layer.color.r,
				layer.color.g,
				layer.color.b));
		};
		writeParticleLayer(U"underwaterNearParticles", settings.environment.underwater.nearParticles);
		writeParticleLayer(U"underwaterMidParticles", settings.environment.underwater.midParticles);
		writeParticleLayer(U"underwaterFarParticles", settings.environment.underwater.farParticles);

		String chainLine = U"effectChain = [";
		for (size_t i = 0; i < settings.effects.chainEffectNames.size(); ++i)
		{
			if (i != 0)
			{
				chainLine += U", ";
			}
			chainLine += QuoteTomlString(settings.effects.chainEffectNames[i]);
		}
		chainLine += U"]";
		writer.writeln(chainLine);

		String enabledLine = U"effectChainEnabled = [";
		for (size_t i = 0; i < settings.effects.chainEnabled.size(); ++i)
		{
			if (i != 0)
			{
				enabledLine += U", ";
			}
			enabledLine += (settings.effects.chainEnabled[i] ? U"true" : U"false");
		}
		enabledLine += U"]";
		writer.writeln(enabledLine);
		writer.writeln(U"panelCollapsed = {}"_fmt(settings.panelCollapsed ? U"true" : U"false"));
		writer.writeln(U"vSyncEnabled = {}"_fmt(settings.performance.vSyncEnabled ? U"true" : U"false"));
		writer.writeln(U"maxFPS = {}"_fmt(settings.performance.maxFPS));
		writer.writeln(U"powerSavingMode = {}"_fmt(settings.performance.powerSavingMode ? U"true" : U"false"));
		writer.writeln(U"idleFPS = {}"_fmt(settings.performance.idleFPS));
		writer.writeln(U"backgroundFPS = {}"_fmt(settings.performance.backgroundFPS));
		writer.writeln(U"panelPosX = {:.2f}"_fmt(settings.panelPosX));
		writer.writeln(U"panelPosY = {:.2f}"_fmt(settings.panelPosY));
		return true;
	}
}
