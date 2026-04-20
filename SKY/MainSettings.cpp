# include "MainSettingsInternal.hpp"
# include "SettingsRegistry.hpp"
# include "SettingsSchemas.hpp"
# include "UnitParameterSchemas.hpp"

namespace MainSupport::SettingsDetail
{
	Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
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

	ColorF ReadTomlColorF(const TOMLReader& toml, const String& key, const ColorF& fallback)
	{
		try
		{
			const TOMLValue tomlValue = toml[key];

			if (tomlValue.isEmpty() || (not tomlValue.isArray()))
			{
				return fallback;
			}

			Array<double> values;
			values.reserve(4);

			for (const auto& value : tomlValue.arrayView())
			{
				values << value.get<double>();
				if (values.size() >= 4)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return ColorF{ values[0], values[1], values[2], ((values.size() >= 4) ? values[3] : fallback.a) };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	void WriteTomlColorF(TextWriter& writer, StringView key, const ColorF& color)
	{
		writer.writeln(U"{} = [{:.3f}, {:.3f}, {:.3f}, {:.3f}]"_fmt(key, color.r, color.g, color.b, color.a));
	}

	Point ReadTomlPoint(const TOMLReader& toml, const String& key, const Point& fallback)
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

	void LoadUnitParameterValue(const TOMLReader& toml, StringView key, double& value)
	{
		if (const auto loaded = toml[String{ key }].getOpt<double>())
		{
			value = *loaded;
		}
	}
    namespace
    {
        // Generic adapter: walks a per-struct schema (declared in
        // UnitParameterSchemas.hpp) with either a Load or Save visitor,
        // producing the same TOML output as the previous hand-written
        // Load*Group / Save*Group helpers.
        template <class Schema, class Params>
        void LoadGroup(const TOMLReader& toml, StringView prefix, Params& params, Schema schema)
        {
            schema(MainSupport::TomlSchema::LoadVisitor{ toml, prefix }, params);
        }

        template <class Schema, class Params>
        void SaveGroup(TextWriter& writer, StringView prefix, const Params& params, Schema schema)
        {
            schema(MainSupport::TomlSchema::SaveVisitor{ writer, prefix }, params);
        }
    }

	UnitAiRole ParseUnitAiRole(StringView value)
	{
		if (value == U"AssaultBase")
		{
			return UnitAiRole::AssaultBase;
		}

		if (value == U"Support")
		{
			return UnitAiRole::Support;
		}

		return UnitAiRole::SecureResources;
	}

	StringView ToTomlUnitAiRole(UnitAiRole aiRole)
	{
		switch (aiRole)
		{
		case UnitAiRole::AssaultBase:
			return U"AssaultBase";

		case UnitAiRole::Support:
			return U"Support";

		case UnitAiRole::SecureResources:
		default:
			return U"SecureResources";
		}
	}

	void LoadUnitParameterGroup(const TOMLReader& toml, StringView prefix, UnitParameters& parameters)
	{
		LoadGroup(toml, prefix, parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitUnitParameters(v, p);
		});
	}

	void SaveUnitParameterGroup(TextWriter& writer, StringView prefix, const UnitParameters& parameters)
	{
		SaveGroup(writer, prefix, parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitUnitParameters(v, p);
		});
	}

	void LoadExplosionSkillParameterGroup(const TOMLReader& toml, StringView prefix, ExplosionSkillParameters& parameters)
	{
		LoadGroup(toml, (String{ prefix } + U"Explosion"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitExplosionSkillParameters(v, p);
		});
	}

	void SaveExplosionSkillParameterGroup(TextWriter& writer, StringView prefix, const ExplosionSkillParameters& parameters)
	{
		SaveGroup(writer, (String{ prefix } + U"Explosion"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitExplosionSkillParameters(v, p);
		});
	}

	void LoadBuildMillSkillParameterGroup(const TOMLReader& toml, StringView prefix, BuildMillSkillParameters& parameters)
	{
		LoadGroup(toml, (String{ prefix } + U"BuildMill"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitBuildMillSkillParameters(v, p);
		});
	}

	void SaveBuildMillSkillParameterGroup(TextWriter& writer, StringView prefix, const BuildMillSkillParameters& parameters)
	{
		SaveGroup(writer, (String{ prefix } + U"BuildMill"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitBuildMillSkillParameters(v, p);
		});
	}

	void LoadHealSkillParameterGroup(const TOMLReader& toml, StringView prefix, HealSkillParameters& parameters)
	{
		LoadGroup(toml, (String{ prefix } + U"Heal"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitHealSkillParameters(v, p);
		});
	}

	void SaveHealSkillParameterGroup(TextWriter& writer, StringView prefix, const HealSkillParameters& parameters)
	{
		SaveGroup(writer, (String{ prefix } + U"Heal"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitHealSkillParameters(v, p);
		});
	}

	void LoadScoutSkillParameterGroup(const TOMLReader& toml, StringView prefix, ScoutSkillParameters& parameters)
	{
		LoadGroup(toml, (String{ prefix } + U"Scout"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitScoutSkillParameters(v, p);
		});
	}

	void SaveScoutSkillParameterGroup(TextWriter& writer, StringView prefix, const ScoutSkillParameters& parameters)
	{
		SaveGroup(writer, (String{ prefix } + U"Scout"), parameters, [](auto&& v, auto&& p) {
			MainSupport::UnitParameterSchemas::VisitScoutSkillParameters(v, p);
		});
	}
}

namespace MainSupport
{
	namespace
	{
		constexpr SettingDescriptor<CameraSettings> CameraSettingsDescriptor{
			.name = U"CameraSettings",
			.path = CameraSettingsPath,
			.loadFn = [](const TOMLReader& toml, CameraSettings& value)
			{
				SettingsSchemas::VisitCameraSettings(TomlSchema::LoadVisitor{ toml, U"" }, value);
			},
			.saveFn = [](TextWriter& writer, const CameraSettings& value)
			{
				SettingsSchemas::VisitCameraSettings(TomlSchema::SaveVisitor{ writer, U"" }, value);
			},
			.makeDefault = []() -> CameraSettings
			{
				return CameraSettings{ .eye = DefaultCameraEye, .focus = DefaultCameraFocus };
			},
		};

		constexpr SettingDescriptor<ResourceStock> InitialPlayerResourcesDescriptor{
			.name = U"InitialPlayerResources",
			.path = ResourceAdjustSettingsPath,
			.loadFn = [](const TOMLReader& toml, ResourceStock& value)
			{
				SettingsSchemas::VisitInitialPlayerResources(TomlSchema::LoadVisitor{ toml, U"" }, value);
				value.budget    = Max(0.0, value.budget);
				value.gunpowder = Max(0.0, value.gunpowder);
				value.mana      = Max(0.0, value.mana);
			},
			.saveFn = [](TextWriter& writer, const ResourceStock& value)
			{
				const ResourceStock clamped{
					.budget    = Max(0.0, value.budget),
					.gunpowder = Max(0.0, value.gunpowder),
					.mana      = Max(0.0, value.mana),
				};
				SettingsSchemas::VisitInitialPlayerResources(TomlSchema::SaveVisitor{ writer, U"" }, clamped);
			},
			.makeDefault = []() -> ResourceStock
			{
				return ResourceStock{ .budget = StartingResources };
			},
		};
	}

	CameraSettings LoadCameraSettings()
	{
		return LoadSetting(CameraSettingsDescriptor);
	}

	bool SaveCameraSettings(const CameraSettings& settings)
	{
		return SaveSetting(CameraSettingsDescriptor, settings);
	}

	ResourceStock LoadInitialPlayerResources()
	{
		return LoadSetting(InitialPlayerResourcesDescriptor);
	}

	bool SaveInitialPlayerResources(const ResourceStock& resources)
	{
		return SaveSetting(InitialPlayerResourcesDescriptor, resources);
	}
}
