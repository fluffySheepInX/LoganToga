# pragma once
# include "UnitEditorContext.hpp"
# include "UnitParameterSchemas.hpp"

namespace MainSupport
{
    // Compile-time descriptor for one skill kind. Each entry binds:
    //   * the TOML key suffix (e.g. U"Explosion")
    //   * the Array<TParams> member inside UnitEditorSettings
    //   * the per-team/per-unit default factory
    //   * load / save callbacks (delegate to the matching schema)
    //
    // Adding a new skill is now a single descriptor addition (plus the
    // matching Array<> member + default factory + Visit*SkillParameters
    // schema), instead of editing every Load / Save / init site.
    template <class TParams>
    struct SkillDescriptor
    {
        StringView keySuffix;
        Array<TParams> UnitEditorSettings::* arrayMember;
        TParams (*makeDefault)(UnitTeam, SapperUnitType);
        void (*load)(const TOMLReader&, const String&, TParams&);
        void (*save)(TextWriter&, const String&, const TParams&);
    };

    // The full skill registry. To add a new skill: append a new
    // SkillDescriptor here.
    inline constexpr auto SkillRegistry = std::tuple{
        SkillDescriptor<ExplosionSkillParameters>{
            U"Explosion",
            &UnitEditorSettings::explosionSkillParameters,
            &MakeDefaultExplosionSkillParameters,
            +[](const TOMLReader& toml, const String& prefix, ExplosionSkillParameters& out)
            {
                UnitParameterSchemas::VisitExplosionSkillParameters(TomlSchema::LoadVisitor{ toml, prefix }, out);
            },
            +[](TextWriter& writer, const String& prefix, const ExplosionSkillParameters& v)
            {
                UnitParameterSchemas::VisitExplosionSkillParameters(TomlSchema::SaveVisitor{ writer, prefix }, v);
            },
        },
        SkillDescriptor<BuildMillSkillParameters>{
            U"BuildMill",
            &UnitEditorSettings::buildMillSkillParameters,
            &MakeDefaultBuildMillSkillParameters,
            +[](const TOMLReader& toml, const String& prefix, BuildMillSkillParameters& out)
            {
                UnitParameterSchemas::VisitBuildMillSkillParameters(TomlSchema::LoadVisitor{ toml, prefix }, out);
            },
            +[](TextWriter& writer, const String& prefix, const BuildMillSkillParameters& v)
            {
                UnitParameterSchemas::VisitBuildMillSkillParameters(TomlSchema::SaveVisitor{ writer, prefix }, v);
            },
        },
        SkillDescriptor<HealSkillParameters>{
            U"Heal",
            &UnitEditorSettings::healSkillParameters,
            &MakeDefaultHealSkillParameters,
            +[](const TOMLReader& toml, const String& prefix, HealSkillParameters& out)
            {
                UnitParameterSchemas::VisitHealSkillParameters(TomlSchema::LoadVisitor{ toml, prefix }, out);
            },
            +[](TextWriter& writer, const String& prefix, const HealSkillParameters& v)
            {
                UnitParameterSchemas::VisitHealSkillParameters(TomlSchema::SaveVisitor{ writer, prefix }, v);
            },
        },
        SkillDescriptor<ScoutSkillParameters>{
            U"Scout",
            &UnitEditorSettings::scoutSkillParameters,
            &MakeDefaultScoutSkillParameters,
            +[](const TOMLReader& toml, const String& prefix, ScoutSkillParameters& out)
            {
                UnitParameterSchemas::VisitScoutSkillParameters(TomlSchema::LoadVisitor{ toml, prefix }, out);
            },
            +[](TextWriter& writer, const String& prefix, const ScoutSkillParameters& v)
            {
                UnitParameterSchemas::VisitScoutSkillParameters(TomlSchema::SaveVisitor{ writer, prefix }, v);
            },
        },
    };

    // Iterate over every descriptor, invoking f(descriptor).
    template <class F>
    inline void ForEachSkill(F&& f)
    {
        std::apply([&](auto&&... d) { (f(d), ...); }, SkillRegistry);
    }
}
