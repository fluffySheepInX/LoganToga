# pragma once
# include "MainSettings.hpp"

namespace MainSupport::SettingsDetail
{
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback);
	[[nodiscard]] ColorF ReadTomlColorF(const TOMLReader& toml, const String& key, const ColorF& fallback);
	void WriteTomlColorF(TextWriter& writer, StringView key, const ColorF& color);
	[[nodiscard]] Point ReadTomlPoint(const TOMLReader& toml, const String& key, const Point& fallback);
	void LoadUnitParameterValue(const TOMLReader& toml, StringView key, double& value);
	[[nodiscard]] UnitAiRole ParseUnitAiRole(StringView value);
	[[nodiscard]] StringView ToTomlUnitAiRole(UnitAiRole aiRole);
	void LoadUnitParameterGroup(const TOMLReader& toml, StringView prefix, UnitParameters& parameters);
   void LoadExplosionSkillParameterGroup(const TOMLReader& toml, StringView prefix, ExplosionSkillParameters& parameters);
	void SaveUnitParameterGroup(TextWriter& writer, StringView prefix, const UnitParameters& parameters);
   void SaveExplosionSkillParameterGroup(TextWriter& writer, StringView prefix, const ExplosionSkillParameters& parameters);
   void LoadBuildMillSkillParameterGroup(const TOMLReader& toml, StringView prefix, BuildMillSkillParameters& parameters);
	void SaveBuildMillSkillParameterGroup(TextWriter& writer, StringView prefix, const BuildMillSkillParameters& parameters);
	void LoadHealSkillParameterGroup(const TOMLReader& toml, StringView prefix, HealSkillParameters& parameters);
	void SaveHealSkillParameterGroup(TextWriter& writer, StringView prefix, const HealSkillParameters& parameters);
	void LoadScoutSkillParameterGroup(const TOMLReader& toml, StringView prefix, ScoutSkillParameters& parameters);
	void SaveScoutSkillParameterGroup(TextWriter& writer, StringView prefix, const ScoutSkillParameters& parameters);
	EditorTextColorSettings& CachedEditorTextColorSettings();
}
