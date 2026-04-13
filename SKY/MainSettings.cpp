# include "MainSettingsInternal.hpp"

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
		if (const auto movementType = toml[(String{ prefix } + U"MovementType")].getOpt<String>())
		{
			parameters.movementType = ((*movementType == U"Tank") ? MovementType::Tank : MovementType::Infantry);
		}

		if (const auto aiRole = toml[(String{ prefix } + U"AiRole")].getOpt<String>())
		{
			parameters.aiRole = ParseUnitAiRole(*aiRole);
		}

		if (const auto footprintType = toml[(String{ prefix } + U"FootprintType")].getOpt<String>())
		{
			parameters.footprintType = ((*footprintType == U"Capsule") ? UnitFootprintType::Capsule : UnitFootprintType::Circle);
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

	void SaveUnitParameterGroup(TextWriter& writer, StringView prefix, const UnitParameters& parameters)
	{
		writer.writeln(U"{}MovementType = \"{}\""_fmt(prefix, (parameters.movementType == MovementType::Tank) ? U"Tank" : U"Infantry"));
		writer.writeln(U"{}AiRole = \"{}\""_fmt(prefix, ToTomlUnitAiRole(parameters.aiRole)));
		writer.writeln(U"{}MaxHitPoints = {:.3f}"_fmt(prefix, parameters.maxHitPoints));
		writer.writeln(U"{}MoveSpeed = {:.3f}"_fmt(prefix, parameters.moveSpeed));
		writer.writeln(U"{}AttackRange = {:.3f}"_fmt(prefix, parameters.attackRange));
		writer.writeln(U"{}StopDistance = {:.3f}"_fmt(prefix, parameters.stopDistance));
		writer.writeln(U"{}AttackDamage = {:.3f}"_fmt(prefix, parameters.attackDamage));
		writer.writeln(U"{}AttackInterval = {:.3f}"_fmt(prefix, parameters.attackInterval));
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(prefix, parameters.manaCost));
		writer.writeln(U"{}FootprintType = \"{}\""_fmt(prefix, (parameters.footprintType == UnitFootprintType::Capsule) ? U"Capsule" : U"Circle"));
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
			.eye = SettingsDetail::ReadTomlVec3(toml, U"eye", DefaultCameraEye),
			.focus = SettingsDetail::ReadTomlVec3(toml, U"focus", DefaultCameraFocus),
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
}
