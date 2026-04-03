# include "MainSettings.hpp"

namespace
{
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 3)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}
}

namespace MainSupport
{
	CameraSettings LoadCameraSettings()
	{
		const TOMLReader toml{ CameraSettingsPath };

		if (not toml)
		{
			return{};
		}

		return{
			.eye = ReadTomlVec3(toml, U"eye", DefaultCameraEye),
			.focus = ReadTomlVec3(toml, U"focus", DefaultCameraFocus),
		};
	}

	bool SaveCameraSettings(const CameraSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ CameraSettingsPath };

		if (not writer)
		{
			return false;
		}

		writer.writeln(U"eye = [{:.3f}, {:.3f}, {:.3f}]"_fmt(settings.eye.x, settings.eye.y, settings.eye.z));
		writer.writeln(U"focus = [{:.3f}, {:.3f}, {:.3f}]"_fmt(settings.focus.x, settings.focus.y, settings.focus.z));
		return true;
	}

	ModelHeightSettings LoadModelHeightSettings()
	{
		const TOMLReader toml{ ModelHeightSettingsPath };

		if (not toml)
		{
			return{};
		}

		ModelHeightSettings settings;

		if (const auto value = toml[U"birdOffsetY"].getOpt<double>())
		{
			settings.birdOffsetY = *value;
		}

		if (const auto value = toml[U"ashigaruOffsetY"].getOpt<double>())
		{
			settings.ashigaruOffsetY = *value;
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

		writer.writeln(U"birdOffsetY = {:.3f}"_fmt(settings.birdOffsetY));
		writer.writeln(U"ashigaruOffsetY = {:.3f}"_fmt(settings.ashigaruOffsetY));
		return true;
	}
}
