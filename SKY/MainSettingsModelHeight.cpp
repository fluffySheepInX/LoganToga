# include "MainSettingsInternal.hpp"

namespace MainSupport
{
	ModelHeightSettings LoadModelHeightSettings()
	{
		const TOMLReader toml{ ModelHeightSettingsPath };

		if (not toml)
		{
			return{};
		}

		ModelHeightSettings settings;

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);
			if (const auto value = toml[(key + U"OffsetY")].getOpt<double>())
			{
				GetModelHeightOffset(settings, renderModel) = *value;
			}

			if (const auto value = toml[(key + U"Scale")].getOpt<double>())
			{
				GetModelScale(settings, renderModel) = Clamp(*value, ModelScaleMin, ModelScaleMax);
			}
		}

		return settings;
	}

	bool SaveModelHeightSettings(const ModelHeightSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ ModelHeightSettingsPath };

		if (not writer)
		{
			return false;
		}

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);
			writer.writeln(U"{}OffsetY = {:.3f}"_fmt(key, GetModelHeightOffset(settings, renderModel)));
			writer.writeln(U"{}Scale = {:.3f}"_fmt(key, Clamp(GetModelScale(settings, renderModel), ModelScaleMin, ModelScaleMax)));
		}
		return true;
	}
}
