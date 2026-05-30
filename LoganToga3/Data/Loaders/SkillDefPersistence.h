#pragma once
# include "SkillDefValueParsing.h"

namespace LT3
{
	inline SkillDef MakeDefaultSkillDefinition(
		const String& tag,
		const String& name,
		SkillKind kind,
		double range,
		double cooldownSec,
		double damage,
		double projectileSpeed,
		SkillProjectileMotion projectileMotion,
		int32 burstCount,
		double burstIntervalSec,
		double spreadDeg,
		double arcHeight,
		double orbitRadius,
		double orbitAngularSpeedDeg,
		double orbitDurationSec,
		const ColorF& color)
	{
		SkillDef def;
		def.tag = tag;
		def.name = name;
		def.kind = kind;
		def.range = range;
		def.rangeMin = 0.0;
		def.cooldownSec = cooldownSec;
		def.damage = damage;
		def.selfDamageOnHit = 0.0;
		def.projectileSpeed = projectileSpeed;
		def.projectileMotion = projectileMotion;
		def.burstCount = burstCount;
		def.burstIntervalSec = burstIntervalSec;
		def.burstFireMode = (burstIntervalSec > 0.0) ? SkillBurstFireMode::Staggered : SkillBurstFireMode::Simultaneous;
		def.burstOrderMode = SkillBurstOrderMode::Sequential;
		def.rayMode = SkillRayMode::None;
		def.rayLength = 1.0;
		def.rayLockToCaster = false;
		def.spreadDeg = spreadDeg;
		def.arcHeight = arcHeight;
		def.orbitRadius = orbitRadius;
		def.orbitAngularSpeedDeg = orbitAngularSpeedDeg;
		def.orbitDurationSec = orbitDurationSec;
		def.projectileWidth = orbitRadius;
		def.projectileHeight = arcHeight;
		def.swingRadius = 0.0;
		def.swingAngleDeg = 90.0;
		def.soundEffectVolume = 1.0;
		def.bom = false;
		def.bomRadius = 0.0;
		def.bomFriendlyFire = false;
		def.bomSelfDamageScale = 0.0;
		def.allfunc = false;
		def.color = color;
		return def;
	}

	inline Array<SkillDef> DefaultSkillDefinitions()
	{
		Array<SkillDef> skills;
		skills << MakeDefaultSkillDefinition(U"worker_hit", U"Tool Strike", SkillKind::Sword, 70.0, 0.75, 4, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Khaki);
		skills << MakeDefaultSkillDefinition(U"sword", U"Sword", SkillKind::Sword, 82.0, 0.65, 9, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Orange);
		skills << MakeDefaultSkillDefinition(U"arrow", U"Arrow", SkillKind::Missile, 210.0, 1.05, 7, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Skyblue);
		skills << MakeDefaultSkillDefinition(U"base_shot", U"Base Shot", SkillKind::Missile, 230.0, 1.35, 10, 340.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Violet);
		skills << MakeDefaultSkillDefinition(U"machine_gun", U"Machine Gun", SkillKind::Missile, 240.0, 1.40, 3, 520.0, SkillProjectileMotion::Direct, 5, 0.06, 8.0, 72.0, 54.0, 220.0, 1.2, Palette::Yellow);
		skills << MakeDefaultSkillDefinition(U"cannon", U"Cannon", SkillKind::Missile, 300.0, 2.20, 18, 260.0, SkillProjectileMotion::Parabola, 1, 0.0, 0.0, 110.0, 54.0, 220.0, 1.2, ColorF{ 1.0, 0.27, 0.0 });
		skills << MakeDefaultSkillDefinition(U"chakram", U"Chakram", SkillKind::Missile, 170.0, 1.80, 6, 380.0, SkillProjectileMotion::Orbit, 1, 0.0, 0.0, 72.0, 58.0, 360.0, 1.5, Palette::Aqua);
		SkillDef landmine = MakeDefaultSkillDefinition(U"landmine_blast", U"Landmine Blast", SkillKind::Missile, 34.0, 0.8, 40, 0.0, SkillProjectileMotion::Static, 1, 0.0, 0.0, 32.0, 34.0, 0.0, 0.2, ColorF{ 1.0, 0.35, 0.15 });
		landmine.selfDamageOnHit = 999.0;
		skills << landmine;
		for (SkillDef& skill : skills)
		{
			if (skill.tag == U"machine_gun")
			{
				skill.burstFireMode = SkillBurstFireMode::Staggered;
			}
		}
		return skills;
	}

	inline bool SaveSkillDefinitionsToml(const Array<SkillDef>& skills, String* statusText = nullptr)
	{
		const FilePath path = ResolveSkillTomlPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path };
		if (!writer)
		{
			if (statusText)
			{
				*statusText = U"Skill save failed: {}"_fmt(path);
			}
			return false;
		}

		writer << U"# LoganToga3 skill definitions\n";
		writer << U"# kind: missile / sword / heal / summon / charge / status\n";
		writer << U"# movetype: direct / static / arc / throw / drop / circle / swing\n\n";

		for (const SkillDef& skill : skills)
		{
			writer << U"[[skills]]\n";
			writer << U"tag = \"" << EscapeTomlBasicString(skill.tag) << U"\"\n";
			writer << U"name = \"" << EscapeTomlBasicString(skill.name) << U"\"\n";
			writer << U"description = \"" << EscapeTomlBasicString(skill.description) << U"\"\n";
			writer << U"icon = \"" << EscapeTomlBasicString(skill.icon) << U"\"\n";
			writer << U"icons = " << BuildTomlStringArrayValue(skill.iconLayers) << U"\n";
			writer << U"kind = \"" << SkillKindToTag(skill.kind) << U"\"\n";
			writer << U"range = " << skill.range << U"\n";
			writer << U"range_min = " << skill.rangeMin << U"\n";
			writer << U"cooldown_sec = " << skill.cooldownSec << U"\n";
			writer << U"mp_cost = " << skill.mpCost << U"\n";
			writer << U"damage = " << skill.damage << U"\n";
			writer << U"self_damage_on_hit = " << skill.selfDamageOnHit << U"\n";
			writer << U"se = \"" << EscapeTomlBasicString(skill.soundEffect) << U"\"\n";
			writer << U"se_volume = " << skill.soundEffectVolume << U"\n";
			writer << U"image = \"" << EscapeTomlBasicString(skill.projectileImage) << U"\"\n";
			writer << U"image_diagonal = \"" << EscapeTomlBasicString(skill.projectileDiagonalImage) << U"\"\n";
			writer << U"projectile_speed = " << skill.projectileSpeed << U"\n";
			writer << U"speed = " << skill.projectileSpeed << U"\n";
			writer << U"movetype = \"" << SkillProjectileMotionToTag(skill.projectileMotion) << U"\"\n";
			writer << U"projectile_motion = \"" << SkillProjectileMotionToTag(skill.projectileMotion) << U"\"\n";
			writer << U"bom = \"" << (skill.bom ? U"on" : U"off") << U"\"\n";
			writer << U"bom_radius = " << skill.bomRadius << U"\n";
			writer << U"bom_friendly_fire = \"" << (skill.bomFriendlyFire ? U"on" : U"off") << U"\"\n";
			writer << U"bom_self_damage_scale = " << skill.bomSelfDamageScale << U"\n";
			writer << U"allfunc = \"" << (skill.allfunc ? U"on" : U"off") << U"\"\n";
			writer << U"center = \"" << SkillProjectileCenterToTag(skill.projectileCenter) << U"\"\n";
			writer << U"homing = \"" << (skill.projectileHoming ? U"on" : U"off") << U"\"\n";
			writer << U"d360 = \"" << (skill.projectileD360 ? U"on" : U"off") << U"\"\n";
			writer << U"start_degree = " << skill.projectileStartDegree << U"\n";
			writer << U"start_degree_type = " << skill.projectileStartDegreeType << U"\n";
			writer << U"burst_count = " << skill.burstCount << U"\n";
			writer << U"burst_interval_sec = " << skill.burstIntervalSec << U"\n";
			writer << U"burst_fire_mode = \"" << SkillBurstFireModeToTag(skill.burstFireMode) << U"\"\n";
			writer << U"burst_order_mode = \"" << SkillBurstOrderModeToTag(skill.burstOrderMode) << U"\"\n";
			writer << U"ray = \"" << SkillRayModeToTag(skill.rayMode) << U"\"\n";
			writer << U"ray_length = " << skill.rayLength << U"\n";
			writer << U"ray_lock_to_caster = \"" << (skill.rayLockToCaster ? U"on" : U"off") << U"\"\n";
			writer << U"spread_deg = " << skill.spreadDeg << U"\n";
			writer << U"arc_height = " << skill.arcHeight << U"\n";
			writer << U"orbit_radius = " << skill.orbitRadius << U"\n";
			writer << U"orbit_angular_speed_deg = " << skill.orbitAngularSpeedDeg << U"\n";
			writer << U"orbit_duration_sec = " << skill.orbitDurationSec << U"\n";
			writer << U"projectile_width = " << skill.projectileWidth << U"\n";
			writer << U"projectile_height = " << skill.projectileHeight << U"\n";
			writer << U"swing_radius = " << skill.swingRadius << U"\n";
			writer << U"swing_angle_deg = " << skill.swingAngleDeg << U"\n";
			for (const auto& cost : skill.resourceCosts)
			{
				if (cost.resourceTag.isEmpty())
				{
					continue;
				}
				writer << U"[[skills.resource_costs]]\n";
				writer << U"tag = \"" << EscapeTomlBasicString(cost.resourceTag) << U"\"\n";
				writer << U"amount = " << Max(1, cost.amount) << U"\n";
			}
			writer << U"color_r = " << skill.color.r << U"\n";
			writer << U"color_g = " << skill.color.g << U"\n";
			writer << U"color_b = " << skill.color.b << U"\n";
			writer << U"color_a = " << skill.color.a << U"\n\n";
		}

		if (statusText)
		{
			*statusText = U"Skill saved: {}"_fmt(path);
		}
		return true;
	}

	inline void EnsureSkillTomlExists()
	{
		const FilePath path = ResolveSkillTomlPath();
		if (!FileSystem::Exists(path))
		{
			SaveSkillDefinitionsToml(DefaultSkillDefinitions());
		}
	}
}
