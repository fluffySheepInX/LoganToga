# pragma once
# include <Siv3D.hpp>

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

    struct EnvironmentSettings
    {
        String groundMode = U"Texture";
        RainSettings rain;
        FogSettings fog;

        [[nodiscard]] bool operator ==(const EnvironmentSettings& other) const = default;
    };

    struct PersistentSettings
    {
        int32 schemaVersion = 1;
        LightingSettings lighting;
        EnvironmentSettings environment;
        EffectChainSettings effects;
        bool panelCollapsed = false;
        double panelPosX = 16.0;
        double panelPosY = 96.0;

        [[nodiscard]] bool operator ==(const PersistentSettings& other) const = default;
    };

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
                    else if (const auto value = item.getOpt<int32>())
                    {
                        colorValues << Clamp(static_cast<double>(*value) / 255.0, 0.0, 1.0);
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
        writer.writeln(U"panelPosX = {:.2f}"_fmt(settings.panelPosX));
        writer.writeln(U"panelPosY = {:.2f}"_fmt(settings.panelPosY));
        return true;
    }
}
