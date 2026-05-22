#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../TomlTextUtils.h"

namespace LT3
{
    namespace SkillToml
    {
        inline constexpr auto KeySkills = U"skills";
    }

    inline FilePath ResolveSkillTomlPath()
    {
        return ResolveFirstExistingPath({
            U"000_Warehouse/000_DefaultGame/070_Scenario/InfoSkill/skill.toml",
            U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoSkill/skill.toml",
        });
    }

    inline String SkillKindToTag(SkillKind kind)
    {
        switch (kind)
        {
        case SkillKind::Sword:
            return U"sword";
        case SkillKind::Heal:
            return U"heal";
        case SkillKind::Summon:
            return U"summon";
        case SkillKind::Charge:
            return U"charge";
        case SkillKind::Status:
            return U"status";
        default:
            return U"missile";
        }
    }

    inline SkillKind ParseSkillKind(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"sword")
        {
            return SkillKind::Sword;
        }
        if (lowered == U"heal")
        {
            return SkillKind::Heal;
        }
        if (lowered == U"summon")
        {
            return SkillKind::Summon;
        }
        if (lowered == U"charge")
        {
            return SkillKind::Charge;
        }
        if (lowered == U"status")
        {
            return SkillKind::Status;
        }
        return SkillKind::Missile;
    }

    inline String SkillProjectileMotionToTag(SkillProjectileMotion motion)
    {
        switch (motion)
        {
        case SkillProjectileMotion::Parabola:
            return U"parabola";
        case SkillProjectileMotion::Orbit:
            return U"orbit";
        default:
            return U"direct";
        }
    }

    inline SkillProjectileMotion ParseSkillProjectileMotion(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"parabola" || lowered == U"arc")
        {
            return SkillProjectileMotion::Parabola;
        }
        if (lowered == U"orbit" || lowered == U"satellite")
        {
            return SkillProjectileMotion::Orbit;
        }
        return SkillProjectileMotion::Direct;
    }

    inline ColorF ReadSkillColor(const TOMLValue& skillValue, const ColorF& fallback)
    {
        return ColorF{
            Clamp(skillValue[U"color_r"].getOr<double>(fallback.r), 0.0, 1.0),
            Clamp(skillValue[U"color_g"].getOr<double>(fallback.g), 0.0, 1.0),
            Clamp(skillValue[U"color_b"].getOr<double>(fallback.b), 0.0, 1.0),
            Clamp(skillValue[U"color_a"].getOr<double>(fallback.a), 0.0, 1.0),
        };
    }

    inline Array<SkillDef> DefaultSkillDefinitions()
    {
        return {
            { U"worker_hit", U"Tool Strike", U"", U"", {}, SkillKind::Sword, 70.0, 0.75, 0, 4, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Khaki },
            { U"sword", U"Sword", U"", U"", {}, SkillKind::Sword, 82.0, 0.65, 0, 9, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Orange },
            { U"arrow", U"Arrow", U"", U"", {}, SkillKind::Missile, 210.0, 1.05, 0, 7, 380.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Skyblue },
            { U"base_shot", U"Base Shot", U"", U"", {}, SkillKind::Missile, 230.0, 1.35, 0, 10, 340.0, SkillProjectileMotion::Direct, 1, 0.0, 0.0, 72.0, 54.0, 220.0, 1.2, Palette::Violet },
            { U"machine_gun", U"Machine Gun", U"", U"", {}, SkillKind::Missile, 240.0, 1.40, 0, 3, 520.0, SkillProjectileMotion::Direct, 5, 0.06, 8.0, 72.0, 54.0, 220.0, 1.2, Palette::Yellow },
            { U"cannon", U"Cannon", U"", U"", {}, SkillKind::Missile, 300.0, 2.20, 0, 18, 260.0, SkillProjectileMotion::Parabola, 1, 0.0, 0.0, 110.0, 54.0, 220.0, 1.2, ColorF{ 1.0, 0.27, 0.0 } },
            { U"chakram", U"Chakram", U"", U"", {}, SkillKind::Missile, 170.0, 1.80, 0, 6, 380.0, SkillProjectileMotion::Orbit, 1, 0.0, 0.0, 72.0, 58.0, 360.0, 1.5, Palette::Aqua },
        };
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
        writer << U"# projectile_motion: direct / parabola / orbit\n\n";

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
            writer << U"cooldown_sec = " << skill.cooldownSec << U"\n";
            writer << U"mp_cost = " << skill.mpCost << U"\n";
            writer << U"damage = " << skill.damage << U"\n";
            writer << U"projectile_speed = " << skill.projectileSpeed << U"\n";
            writer << U"projectile_motion = \"" << SkillProjectileMotionToTag(skill.projectileMotion) << U"\"\n";
            writer << U"burst_count = " << skill.burstCount << U"\n";
            writer << U"burst_interval_sec = " << skill.burstIntervalSec << U"\n";
            writer << U"spread_deg = " << skill.spreadDeg << U"\n";
            writer << U"arc_height = " << skill.arcHeight << U"\n";
            writer << U"orbit_radius = " << skill.orbitRadius << U"\n";
            writer << U"orbit_angular_speed_deg = " << skill.orbitAngularSpeedDeg << U"\n";
            writer << U"orbit_duration_sec = " << skill.orbitDurationSec << U"\n";
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

    inline SkillDef ReadSkillDefFromToml(const TOMLValue& skillValue)
    {
        SkillDef def;
        def.tag = skillValue[U"tag"].getOr<String>(U"").lowercased();
        def.name = skillValue[U"name"].getOr<String>(def.tag);
        def.description = skillValue[U"description"].getOr<String>(U"");
        def.icon = skillValue[U"icon"].getOr<String>(U"");
        def.iconLayers = ReadTomlStringArrayValue(skillValue[U"icons"]);
        if (def.iconLayers.isEmpty() && !def.icon.isEmpty())
        {
            def.iconLayers << def.icon;
        }
        if (def.icon.isEmpty() && !def.iconLayers.isEmpty())
        {
            def.icon = def.iconLayers.front();
        }
        def.kind = ParseSkillKind(skillValue[U"kind"].getOr<String>(U"missile"));
        def.range = Max(0.0, skillValue[U"range"].getOr<double>(def.range));
        def.cooldownSec = Max(0.05, skillValue[U"cooldown_sec"].getOr<double>(def.cooldownSec));
        def.mpCost = Max(0, skillValue[U"mp_cost"].getOr<int32>(def.mpCost));
        def.damage = skillValue[U"damage"].getOr<int32>(def.damage);
        def.projectileSpeed = Max(1.0, skillValue[U"projectile_speed"].getOr<double>(def.projectileSpeed));
        def.projectileMotion = ParseSkillProjectileMotion(skillValue[U"projectile_motion"].getOr<String>(U"direct"));
        def.burstCount = Clamp(skillValue[U"burst_count"].getOr<int32>(def.burstCount), 1, 32);
        def.burstIntervalSec = Max(0.0, skillValue[U"burst_interval_sec"].getOr<double>(def.burstIntervalSec));
        def.spreadDeg = Clamp(skillValue[U"spread_deg"].getOr<double>(def.spreadDeg), 0.0, 180.0);
        def.arcHeight = Max(0.0, skillValue[U"arc_height"].getOr<double>(def.arcHeight));
        def.orbitRadius = Max(1.0, skillValue[U"orbit_radius"].getOr<double>(def.orbitRadius));
        def.orbitAngularSpeedDeg = skillValue[U"orbit_angular_speed_deg"].getOr<double>(def.orbitAngularSpeedDeg);
        def.orbitDurationSec = Max(0.05, skillValue[U"orbit_duration_sec"].getOr<double>(def.orbitDurationSec));
        def.color = ReadSkillColor(skillValue, def.color);
        return def;
    }

    inline void LoadSkillDefinitions(DefinitionStores& defs)
    {
        defs.skills.clear();
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
            loadedTags.insert(skill.tag);
        }

        if (defs.skills.isEmpty())
        {
            for (const SkillDef& skill : DefaultSkillDefinitions())
            {
                defs.addSkill(skill);
            }
        }
    }
}
