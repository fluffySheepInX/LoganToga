# include "MainSettings.hpp"
# include "SkyAppUiLayout.hpp"

namespace
{
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
	{
		try
		{
           const TOMLValue tomlValue = toml[key];

			if (tomlValue.isEmpty() || (not tomlValue.isArray()))
			{
				return fallback;
			}

			Array<double> values;
			values.reserve(3);

         for (const auto& value : tomlValue.arrayView())
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

	[[nodiscard]] Point ReadTomlPoint(const TOMLReader& toml, const String& key, const Point& fallback)
	{
		try
		{
           const TOMLValue tomlValue = toml[key];

			if (tomlValue.isEmpty() || (not tomlValue.isArray()))
			{
				return fallback;
			}

			Array<double> values;
			values.reserve(2);

         for (const auto& value : tomlValue.arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 2)
				{
					break;
				}
			}

			if (values.size() < 2)
			{
				return fallback;
			}

			return Point{ static_cast<int32>(Math::Round(values[0])), static_cast<int32>(Math::Round(values[1])) };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	void LoadUnitParameterValue(const TOMLReader& toml, const StringView key, double& value)
	{
		if (const auto loaded = toml[String{ key }].getOpt<double>())
		{
			value = *loaded;
		}
	}

	void LoadUnitParameterGroup(const TOMLReader& toml, const StringView prefix, MainSupport::UnitParameters& parameters)
	{
        if (const auto movementType = toml[(String{ prefix } + U"MovementType")].getOpt<String>())
		{
			parameters.movementType = ((*movementType == U"Tank") ? MainSupport::MovementType::Tank : MainSupport::MovementType::Infantry);
		}

		LoadUnitParameterValue(toml, (String{ prefix } + U"MaxHitPoints"), parameters.maxHitPoints);
		LoadUnitParameterValue(toml, (String{ prefix } + U"MoveSpeed"), parameters.moveSpeed);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackRange"), parameters.attackRange);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackDamage"), parameters.attackDamage);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackInterval"), parameters.attackInterval);
		LoadUnitParameterValue(toml, (String{ prefix } + U"ManaCost"), parameters.manaCost);
	}

	void SaveUnitParameterGroup(TextWriter& writer, const StringView prefix, const MainSupport::UnitParameters& parameters)
	{
        writer.writeln(U"{}MovementType = \"{}\""_fmt(prefix, (parameters.movementType == MainSupport::MovementType::Tank) ? U"Tank" : U"Infantry"));
		writer.writeln(U"{}MaxHitPoints = {:.3f}"_fmt(prefix, parameters.maxHitPoints));
		writer.writeln(U"{}MoveSpeed = {:.3f}"_fmt(prefix, parameters.moveSpeed));
		writer.writeln(U"{}AttackRange = {:.3f}"_fmt(prefix, parameters.attackRange));
		writer.writeln(U"{}AttackDamage = {:.3f}"_fmt(prefix, parameters.attackDamage));
		writer.writeln(U"{}AttackInterval = {:.3f}"_fmt(prefix, parameters.attackInterval));
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(prefix, parameters.manaCost));
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

	UiLayoutSettings LoadUiLayoutSettings(const int32 sceneWidth, const int32 sceneHeight)
	{
		UiLayoutSettings settings{
			.miniMapPosition = SkyAppUiLayout::DefaultMiniMapPosition(sceneWidth),
			.resourcePanelPosition = SkyAppUiLayout::DefaultResourcePanelPosition(sceneWidth),
          .unitEditorPosition = SkyAppUiLayout::DefaultUnitEditorPosition(sceneWidth),
			.unitEditorListPosition = SkyAppUiLayout::DefaultUnitEditorListPosition(),
		};

		const TOMLReader toml{ UiLayoutSettingsPath };

		if (not toml)
		{
			return settings;
		}

		settings.miniMapPosition = ReadTomlPoint(toml, U"miniMap", settings.miniMapPosition);
		settings.resourcePanelPosition = ReadTomlPoint(toml, U"resourcePanel", settings.resourcePanelPosition);
		settings.unitEditorPosition = ReadTomlPoint(toml, U"unitEditor", settings.unitEditorPosition);
		settings.unitEditorListPosition = ReadTomlPoint(toml, U"unitEditorList", settings.unitEditorListPosition);

		const Rect miniMapRect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, settings.miniMapPosition, true);
		const Rect resourcePanelRect = SkyAppUiLayout::ResourcePanel(sceneWidth, sceneHeight, settings.resourcePanelPosition);
       const Rect unitEditorRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, settings.unitEditorPosition);
		const Rect unitEditorListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, settings.unitEditorListPosition);
		settings.miniMapPosition = Point{ miniMapRect.x, miniMapRect.y };
		settings.resourcePanelPosition = Point{ resourcePanelRect.x, resourcePanelRect.y };
        settings.unitEditorPosition = Point{ unitEditorRect.x, unitEditorRect.y };
		settings.unitEditorListPosition = Point{ unitEditorListRect.x, unitEditorListRect.y };
		return settings;
	}

	bool SaveUiLayoutSettings(const UiLayoutSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ UiLayoutSettingsPath };

		if (not writer)
		{
			return false;
		}

		writer.writeln(U"miniMap = [{}, {}]"_fmt(settings.miniMapPosition.x, settings.miniMapPosition.y));
		writer.writeln(U"resourcePanel = [{}, {}]"_fmt(settings.resourcePanelPosition.x, settings.resourcePanelPosition.y));
        writer.writeln(U"unitEditor = [{}, {}]"_fmt(settings.unitEditorPosition.x, settings.unitEditorPosition.y));
		writer.writeln(U"unitEditorList = [{}, {}]"_fmt(settings.unitEditorListPosition.x, settings.unitEditorListPosition.y));
		return true;
	}

	UnitEditorSettings LoadUnitEditorSettings()
	{
		const TOMLReader toml{ UnitSettingsPath };
		UnitEditorSettings settings;

		if (not toml)
		{
			return settings;
		}

		LoadUnitParameterGroup(toml, U"playerInfantry", settings.playerInfantry);
		LoadUnitParameterGroup(toml, U"playerArcaneInfantry", settings.playerArcaneInfantry);
		LoadUnitParameterGroup(toml, U"enemyInfantry", settings.enemyInfantry);
		LoadUnitParameterGroup(toml, U"enemyArcaneInfantry", settings.enemyArcaneInfantry);
		return settings;
	}

	bool SaveUnitEditorSettings(const UnitEditorSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ UnitSettingsPath };

		if (not writer)
		{
			return false;
		}

		SaveUnitParameterGroup(writer, U"playerInfantry", settings.playerInfantry);
		writer.writeln(U"");
		SaveUnitParameterGroup(writer, U"playerArcaneInfantry", settings.playerArcaneInfantry);
		writer.writeln(U"");
		SaveUnitParameterGroup(writer, U"enemyInfantry", settings.enemyInfantry);
		writer.writeln(U"");
		SaveUnitParameterGroup(writer, U"enemyArcaneInfantry", settings.enemyArcaneInfantry);
		return true;
	}
}
