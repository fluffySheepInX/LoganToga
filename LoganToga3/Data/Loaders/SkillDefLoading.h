#pragma once
# include "SkillDefPersistence.h"

namespace LT3
{
	inline SkillDef ReadSkillDefFromToml(const TOMLValue& skillValue)
	{
		SkillDef def;
		def.tag = skillValue[U"tag"].getOr<String>(U"").lowercased();
		def.name = skillValue[U"name"].getOr<String>(def.tag);
		def.description = skillValue[U"description"].getOr<String>(U"");
		def.icon = skillValue[U"icon"].getOr<String>(U"");
		def.projectileImage = skillValue[U"image"].getOr<String>(U"");
		def.projectileDiagonalImage = skillValue[U"image_diagonal"].getOr<String>(U"");
		def.iconLayers = NormalizeSkillIconLayerOrder(ReadTomlStringArrayValue(skillValue[U"icons"]));
		if (def.iconLayers.isEmpty() && !def.icon.isEmpty())
		{
			def.iconLayers << def.icon;
		}
		def.iconLayers = NormalizeSkillIconLayerOrder(def.iconLayers);
		if (def.icon.isEmpty() && !def.iconLayers.isEmpty())
		{
			def.icon = def.iconLayers.front();
		}
		if (def.projectileImage.isEmpty())
		{
			def.projectileImage = def.icon;
		}
		def.kind = ParseSkillKind(skillValue[U"kind"].getOr<String>(U"missile"));
		def.range = Max(0.0, skillValue[U"range"].getOr<double>(def.range));
		def.cooldownSec = Max(0.05, skillValue[U"cooldown_sec"].getOr<double>(def.cooldownSec));
		def.mpCost = Max(0, skillValue[U"mp_cost"].getOr<int32>(def.mpCost));
		def.damage = Clamp(skillValue[U"damage"].getOr<double>(def.damage), 0.01, 100.0);
		def.projectileSpeed = skillValue[U"speed"].getOr<double>(skillValue[U"projectile_speed"].getOr<double>(def.projectileSpeed));
		const bool hasMoveType = skillValue[U"movetype"].getOpt<String>().has_value();
		def.projectileMotion = ParseSkillProjectileMotion(hasMoveType ? skillValue[U"movetype"].getOr<String>(U"direct") : skillValue[U"projectile_motion"].getOr<String>(U"direct"));
		if (!hasMoveType && def.kind == SkillKind::Missile && def.projectileMotion == SkillProjectileMotion::Direct && Abs(def.projectileSpeed) <= 0.0001)
		{
			def.projectileMotion = SkillProjectileMotion::Static;
		}
		def.projectileCenter = ParseSkillProjectileCenter(skillValue[U"center"].getOr<String>(U"off"));
		def.projectileHoming = ReadSkillBoolSwitch(skillValue[U"homing"], def.projectileHoming);
		def.projectileD360 = ReadSkillBoolSwitch(skillValue[U"d360"], def.projectileD360);
		def.projectileStartDegree = skillValue[U"start_degree"].getOr<double>(def.projectileStartDegree);
		def.projectileStartDegreeType = skillValue[U"start_degree_type"].getOr<int32>(def.projectileStartDegreeType);
		def.burstCount = Clamp(skillValue[U"burst_count"].getOr<int32>(def.burstCount), 1, 32);
		def.burstIntervalSec = Max(0.0, skillValue[U"burst_interval_sec"].getOr<double>(def.burstIntervalSec));
		def.spreadDeg = Clamp(skillValue[U"spread_deg"].getOr<double>(def.spreadDeg), 0.0, 180.0);
		def.arcHeight = Max(0.0, skillValue[U"arc_height"].getOr<double>(def.arcHeight));
		def.orbitRadius = Max(1.0, skillValue[U"orbit_radius"].getOr<double>(def.orbitRadius));
		def.orbitAngularSpeedDeg = skillValue[U"orbit_angular_speed_deg"].getOr<double>(def.orbitAngularSpeedDeg);
		def.orbitDurationSec = Max(0.05, skillValue[U"orbit_duration_sec"].getOr<double>(def.orbitDurationSec));
		def.projectileWidth = Max(1.0, skillValue[U"projectile_width"].getOr<double>(def.orbitRadius));
		def.projectileHeight = Max(1.0, skillValue[U"projectile_height"].getOr<double>(def.arcHeight));
		def.swingRadius = Max(0.0, skillValue[U"swing_radius"].getOr<double>(def.projectileMotion == SkillProjectileMotion::Swing ? 0.0 : def.orbitRadius));
		def.swingAngleDeg = skillValue[U"swing_angle_deg"].getOr<double>(def.projectileMotion == SkillProjectileMotion::Swing ? def.range : 90.0);
		def.color = ReadSkillColor(skillValue, def.color);
		return def;
	}

	inline void LoadSkillDefinitions(DefinitionStores& defs)
	{
		defs.skills.clear();
		defs.skillIconWarningsByTag.clear();
		defs.skillByTag.clear();

		EnsureSkillTomlExists();
		const FilePath skillPath = ResolveSkillTomlPath();
		const TOMLReader toml{ skillPath };
		if (!toml)
		{
			for (const SkillDef& skill : DefaultSkillDefinitions())
			{
				defs.addSkill(skill);
			}
			return;
		}

		HashSet<String> loadedTags;
		for (const auto skillValue : toml[SkillToml::KeySkills].tableArrayView())
		{
			SkillDef skill = ReadSkillDefFromToml(skillValue);
			if (skill.tag.isEmpty() || loadedTags.contains(skill.tag))
			{
				continue;
			}

			defs.addSkill(skill);
			const Array<String> warnings = ValidateSkillIconLayers(skill);
			if (!warnings.isEmpty())
			{
				defs.skillIconWarningsByTag[skill.tag] = warnings;
			}
			loadedTags.insert(skill.tag);
		}

		if (defs.skills.isEmpty())
		{
			for (const SkillDef& skill : DefaultSkillDefinitions())
			{
				defs.addSkill(skill);
				const Array<String> warnings = ValidateSkillIconLayers(skill);
				if (!warnings.isEmpty())
				{
					defs.skillIconWarningsByTag[skill.tag] = warnings;
				}
			}
		}
	}
}
