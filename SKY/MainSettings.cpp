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

	[[nodiscard]] MainSupport::UnitAiRole ParseUnitAiRole(const StringView value)
	{
		if (value == U"AssaultBase")
		{
			return MainSupport::UnitAiRole::AssaultBase;
		}

		if (value == U"Support")
		{
			return MainSupport::UnitAiRole::Support;
		}

		return MainSupport::UnitAiRole::SecureResources;
	}

	[[nodiscard]] StringView ToTomlUnitAiRole(const MainSupport::UnitAiRole aiRole)
	{
		switch (aiRole)
		{
		case MainSupport::UnitAiRole::AssaultBase:
			return U"AssaultBase";

		case MainSupport::UnitAiRole::Support:
			return U"Support";

		case MainSupport::UnitAiRole::SecureResources:
		default:
			return U"SecureResources";
		}
	}

	void LoadUnitParameterGroup(const TOMLReader& toml, const StringView prefix, MainSupport::UnitParameters& parameters)
	{
        if (const auto movementType = toml[(String{ prefix } + U"MovementType")].getOpt<String>())
		{
			parameters.movementType = ((*movementType == U"Tank") ? MainSupport::MovementType::Tank : MainSupport::MovementType::Infantry);
		}

		if (const auto aiRole = toml[(String{ prefix } + U"AiRole")].getOpt<String>())
		{
			parameters.aiRole = ParseUnitAiRole(*aiRole);
		}

		if (const auto footprintType = toml[(String{ prefix } + U"FootprintType")].getOpt<String>())
		{
			parameters.footprintType = ((*footprintType == U"Capsule") ? MainSupport::UnitFootprintType::Capsule : MainSupport::UnitFootprintType::Circle);
		}

		LoadUnitParameterValue(toml, (String{ prefix } + U"MaxHitPoints"), parameters.maxHitPoints);
		LoadUnitParameterValue(toml, (String{ prefix } + U"MoveSpeed"), parameters.moveSpeed);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackRange"), parameters.attackRange);
        LoadUnitParameterValue(toml, (String{ prefix } + U"StopDistance"), parameters.stopDistance);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackDamage"), parameters.attackDamage);
		LoadUnitParameterValue(toml, (String{ prefix } + U"AttackInterval"), parameters.attackInterval);
		LoadUnitParameterValue(toml, (String{ prefix } + U"ManaCost"), parameters.manaCost);
       LoadUnitParameterValue(toml, (String{ prefix } + U"FootprintRadius"), parameters.footprintRadius);
		LoadUnitParameterValue(toml, (String{ prefix } + U"FootprintHalfLength"), parameters.footprintHalfLength);
	}

	void SaveUnitParameterGroup(TextWriter& writer, const StringView prefix, const MainSupport::UnitParameters& parameters)
	{
        writer.writeln(U"{}MovementType = \"{}\""_fmt(prefix, (parameters.movementType == MainSupport::MovementType::Tank) ? U"Tank" : U"Infantry"));
        writer.writeln(U"{}AiRole = \"{}\""_fmt(prefix, ToTomlUnitAiRole(parameters.aiRole)));
		writer.writeln(U"{}MaxHitPoints = {:.3f}"_fmt(prefix, parameters.maxHitPoints));
		writer.writeln(U"{}MoveSpeed = {:.3f}"_fmt(prefix, parameters.moveSpeed));
		writer.writeln(U"{}AttackRange = {:.3f}"_fmt(prefix, parameters.attackRange));
        writer.writeln(U"{}StopDistance = {:.3f}"_fmt(prefix, parameters.stopDistance));
		writer.writeln(U"{}AttackDamage = {:.3f}"_fmt(prefix, parameters.attackDamage));
		writer.writeln(U"{}AttackInterval = {:.3f}"_fmt(prefix, parameters.attackInterval));
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(prefix, parameters.manaCost));
       writer.writeln(U"{}FootprintType = \"{}\""_fmt(prefix, (parameters.footprintType == MainSupport::UnitFootprintType::Capsule) ? U"Capsule" : U"Circle"));
		writer.writeln(U"{}FootprintRadius = {:.3f}"_fmt(prefix, parameters.footprintRadius));
		writer.writeln(U"{}FootprintHalfLength = {:.3f}"_fmt(prefix, parameters.footprintHalfLength));
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

	UiLayoutSettings LoadUiLayoutSettings(const int32 sceneWidth, const int32 sceneHeight)
	{
		UiLayoutSettings settings{
			.miniMapPosition = SkyAppUiLayout::DefaultMiniMapPosition(sceneWidth),
			.resourcePanelPosition = SkyAppUiLayout::DefaultResourcePanelPosition(sceneWidth),
          .modelHeightPosition = SkyAppUiLayout::DefaultModelHeightPosition(sceneWidth, sceneHeight),
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
      settings.modelHeightPosition = ReadTomlPoint(toml, U"modelHeight", settings.modelHeightPosition);
		settings.unitEditorPosition = ReadTomlPoint(toml, U"unitEditor", settings.unitEditorPosition);
		settings.unitEditorListPosition = ReadTomlPoint(toml, U"unitEditorList", settings.unitEditorListPosition);

		const Rect miniMapRect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, settings.miniMapPosition, true);
		const Rect resourcePanelRect = SkyAppUiLayout::ResourcePanel(sceneWidth, sceneHeight, settings.resourcePanelPosition);
        const Rect modelHeightRect = SkyAppUiLayout::ModelHeight(sceneWidth, sceneHeight, settings.modelHeightPosition);
       const Rect unitEditorRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, settings.unitEditorPosition);
		const Rect unitEditorListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, settings.unitEditorListPosition);
		settings.miniMapPosition = Point{ miniMapRect.x, miniMapRect.y };
		settings.resourcePanelPosition = Point{ resourcePanelRect.x, resourcePanelRect.y };
      settings.modelHeightPosition = Point{ modelHeightRect.x, modelHeightRect.y };
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
     writer.writeln(U"modelHeight = [{}, {}]"_fmt(settings.modelHeightPosition.x, settings.modelHeightPosition.y));
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

       for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
		{
			for (const auto& unitDefinition : GetUnitDefinitions())
			{
				LoadUnitParameterGroup(toml,
					GetUnitSettingsGroupKey(team, unitDefinition.unitType),
					GetUnitParameters(settings, team, unitDefinition.unitType));
			}
		}

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

     bool needsBlankLine = false;
		for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
		{
			for (const auto& unitDefinition : GetUnitDefinitions())
			{
				if (needsBlankLine)
				{
					writer.writeln(U"");
				}

				SaveUnitParameterGroup(writer,
					GetUnitSettingsGroupKey(team, unitDefinition.unitType),
					GetUnitParameters(settings, team, unitDefinition.unitType));
				needsBlankLine = true;
			}
		}

		return true;
	}
}
