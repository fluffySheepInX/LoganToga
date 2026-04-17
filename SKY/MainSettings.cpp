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
        LoadUnitParameterValue(toml, (String{ prefix } + U"VisionRange"), parameters.visionRange);
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
        writer.writeln(U"{}VisionRange = {:.3f}"_fmt(prefix, parameters.visionRange));
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(prefix, parameters.manaCost));
		writer.writeln(U"{}FootprintType = \"{}\""_fmt(prefix, (parameters.footprintType == UnitFootprintType::Capsule) ? U"Capsule" : U"Circle"));
		writer.writeln(U"{}FootprintRadius = {:.3f}"_fmt(prefix, parameters.footprintRadius));
		writer.writeln(U"{}FootprintHalfLength = {:.3f}"_fmt(prefix, parameters.footprintHalfLength));
	}

	void LoadExplosionSkillParameterGroup(const TOMLReader& toml, StringView prefix, ExplosionSkillParameters& parameters)
	{
		const String explosionPrefix = (String{ prefix } + U"Explosion");
		LoadUnitParameterValue(toml, (explosionPrefix + U"Radius"), parameters.radius);
		LoadUnitParameterValue(toml, (explosionPrefix + U"UnitDamage"), parameters.unitDamage);
		LoadUnitParameterValue(toml, (explosionPrefix + U"BaseDamage"), parameters.baseDamage);
		LoadUnitParameterValue(toml, (explosionPrefix + U"CooldownSeconds"), parameters.cooldownSeconds);
		LoadUnitParameterValue(toml, (explosionPrefix + U"GunpowderCost"), parameters.gunpowderCost);
		LoadUnitParameterValue(toml, (explosionPrefix + U"EffectLifetime"), parameters.effectLifetime);
		LoadUnitParameterValue(toml, (explosionPrefix + U"EffectThickness"), parameters.effectThickness);
		LoadUnitParameterValue(toml, (explosionPrefix + U"EffectOffsetY"), parameters.effectOffsetY);
		parameters.effectColor = ReadTomlColorF(toml, (explosionPrefix + U"EffectColor"), parameters.effectColor);
	}

	void SaveExplosionSkillParameterGroup(TextWriter& writer, StringView prefix, const ExplosionSkillParameters& parameters)
	{
		const String explosionPrefix = (String{ prefix } + U"Explosion");
		writer.writeln(U"{}Radius = {:.3f}"_fmt(explosionPrefix, parameters.radius));
		writer.writeln(U"{}UnitDamage = {:.3f}"_fmt(explosionPrefix, parameters.unitDamage));
		writer.writeln(U"{}BaseDamage = {:.3f}"_fmt(explosionPrefix, parameters.baseDamage));
		writer.writeln(U"{}CooldownSeconds = {:.3f}"_fmt(explosionPrefix, parameters.cooldownSeconds));
		writer.writeln(U"{}GunpowderCost = {:.3f}"_fmt(explosionPrefix, parameters.gunpowderCost));
		writer.writeln(U"{}EffectLifetime = {:.3f}"_fmt(explosionPrefix, parameters.effectLifetime));
		writer.writeln(U"{}EffectThickness = {:.3f}"_fmt(explosionPrefix, parameters.effectThickness));
		writer.writeln(U"{}EffectOffsetY = {:.3f}"_fmt(explosionPrefix, parameters.effectOffsetY));
		WriteTomlColorF(writer, (explosionPrefix + U"EffectColor"), parameters.effectColor);
	}

	void LoadBuildMillSkillParameterGroup(const TOMLReader& toml, StringView prefix, BuildMillSkillParameters& parameters)
	{
		const String buildPrefix = (String{ prefix } + U"BuildMill");
		LoadUnitParameterValue(toml, (buildPrefix + U"ManaCost"), parameters.manaCost);
		LoadUnitParameterValue(toml, (buildPrefix + U"GunpowderCost"), parameters.gunpowderCost);
		LoadUnitParameterValue(toml, (buildPrefix + U"ForwardOffset"), parameters.forwardOffset);
	}

	void SaveBuildMillSkillParameterGroup(TextWriter& writer, StringView prefix, const BuildMillSkillParameters& parameters)
	{
		const String buildPrefix = (String{ prefix } + U"BuildMill");
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(buildPrefix, parameters.manaCost));
		writer.writeln(U"{}GunpowderCost = {:.3f}"_fmt(buildPrefix, parameters.gunpowderCost));
		writer.writeln(U"{}ForwardOffset = {:.3f}"_fmt(buildPrefix, parameters.forwardOffset));
	}

	void LoadHealSkillParameterGroup(const TOMLReader& toml, StringView prefix, HealSkillParameters& parameters)
	{
		const String healPrefix = (String{ prefix } + U"Heal");
		LoadUnitParameterValue(toml, (healPrefix + U"ManaCost"), parameters.manaCost);
		LoadUnitParameterValue(toml, (healPrefix + U"Radius"), parameters.radius);
		LoadUnitParameterValue(toml, (healPrefix + U"Amount"), parameters.amount);
	}

	void SaveHealSkillParameterGroup(TextWriter& writer, StringView prefix, const HealSkillParameters& parameters)
	{
		const String healPrefix = (String{ prefix } + U"Heal");
		writer.writeln(U"{}ManaCost = {:.3f}"_fmt(healPrefix, parameters.manaCost));
		writer.writeln(U"{}Radius = {:.3f}"_fmt(healPrefix, parameters.radius));
		writer.writeln(U"{}Amount = {:.3f}"_fmt(healPrefix, parameters.amount));
	}

	void LoadScoutSkillParameterGroup(const TOMLReader& toml, StringView prefix, ScoutSkillParameters& parameters)
	{
		const String scoutPrefix = (String{ prefix } + U"Scout");
		LoadUnitParameterValue(toml, (scoutPrefix + U"GunpowderCost"), parameters.gunpowderCost);
		LoadUnitParameterValue(toml, (scoutPrefix + U"DurationSeconds"), parameters.durationSeconds);
		LoadUnitParameterValue(toml, (scoutPrefix + U"VisionMultiplier"), parameters.visionMultiplier);
	}

	void SaveScoutSkillParameterGroup(TextWriter& writer, StringView prefix, const ScoutSkillParameters& parameters)
	{
		const String scoutPrefix = (String{ prefix } + U"Scout");
		writer.writeln(U"{}GunpowderCost = {:.3f}"_fmt(scoutPrefix, parameters.gunpowderCost));
		writer.writeln(U"{}DurationSeconds = {:.3f}"_fmt(scoutPrefix, parameters.durationSeconds));
		writer.writeln(U"{}VisionMultiplier = {:.3f}"_fmt(scoutPrefix, parameters.visionMultiplier));
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

	ResourceStock LoadInitialPlayerResources()
	{
		ResourceStock resources{ .budget = StartingResources };
		const TOMLReader toml{ ResourceAdjustSettingsPath };

		if (not toml)
		{
			return resources;
		}

		if (const auto loadedBudget = toml[U"budget"].getOpt<double>())
		{
			resources.budget = Max(0.0, *loadedBudget);
		}

		if (const auto loadedGunpowder = toml[U"gunpowder"].getOpt<double>())
		{
			resources.gunpowder = Max(0.0, *loadedGunpowder);
		}

		if (const auto loadedMana = toml[U"mana"].getOpt<double>())
		{
			resources.mana = Max(0.0, *loadedMana);
		}

		return resources;
	}

	bool SaveInitialPlayerResources(const ResourceStock& resources)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ ResourceAdjustSettingsPath };

		if (not writer)
		{
			return false;
		}

		writer.writeln(U"budget = {:.3f}"_fmt(Max(0.0, resources.budget)));
		writer.writeln(U"gunpowder = {:.3f}"_fmt(Max(0.0, resources.gunpowder)));
		writer.writeln(U"mana = {:.3f}"_fmt(Max(0.0, resources.mana)));
		return true;
	}
}
