#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../TomlTextUtils.h"

namespace LT3
{
	namespace AiProfileToml
	{
		inline constexpr auto KeyProfiles = U"profiles";
		inline constexpr auto KeyTag = U"tag";
		inline constexpr auto KeyName = U"name";
		inline constexpr auto KeyDescription = U"description";
		inline constexpr auto KeyPresetType = U"preset_type";
		inline constexpr auto KeyOpeningDelaySec = U"opening_delay_sec";
		inline constexpr auto KeySpawnIntervalSec = U"spawn_interval_sec";
		inline constexpr auto KeyAttackWaveIntervalSec = U"attack_wave_interval_sec";
		inline constexpr auto KeyAggression = U"aggression";
		inline constexpr auto KeyEconomyFocus = U"economy_focus";
		inline constexpr auto KeyDefenseFocus = U"defense_focus";
		inline constexpr auto KeyTechFocus = U"tech_focus";
		inline constexpr auto KeyAttackGroupSize = U"attack_group_size";
		inline constexpr auto KeyMaxArmySize = U"max_army_size";
			inline constexpr auto KeyBattleTimeLimitSec = U"battle_time_limit_sec";
		inline constexpr auto KeyRetreatHpRatio = U"retreat_hp_ratio";
		inline constexpr auto KeyFreeSpawnEnabled = U"free_spawn_enabled";
		inline constexpr auto KeyResourceMultiplier = U"resource_multiplier";
		inline constexpr auto KeyContactBehavior = U"contact_behavior";
		inline constexpr auto KeyInitialUnits = U"initial_units";
		inline constexpr auto KeyUnitWeights = U"unit_weights";
		inline constexpr auto KeyUnit = U"unit";
		inline constexpr auto KeyWeight = U"weight";
		inline constexpr auto KeyBuildPriorities = U"build_priorities";
		inline constexpr auto KeyAction = U"action";
		inline constexpr auto KeyDesiredCount = U"desired_count";
		inline constexpr auto KeyTargetPriority = U"target_priority";
	}

	inline FilePath ResolveAiProfileTomlPath()
	{
		return ResolveFirstExistingPath({
			U"000_Warehouse/000_DefaultGame/070_Scenario/InfoAI/AiProfiles.toml",
			U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoAI/AiProfiles.toml",
		});
	}

	inline double ClampAiProfileRate(double value)
	{
		return Clamp(value, 0.0, 1.0);
	}

	inline AiProfileDef MakeDefaultAiProfile(StringView tag, StringView name, StringView presetType)
	{
		AiProfileDef def;
		def.tag = String{ tag };
		def.name = String{ name };
		def.description = U"Default AI profile";
		def.presetType = String{ presetType };
		def.unitWeights = {
			AiUnitWeightDef{ U"1", 1.0, 6 },
			AiUnitWeightDef{ U"2", 0.7, 4 },
			AiUnitWeightDef{ U"4", 0.25, 2 },
		};
		def.initialUnits = { U"1", U"2", U"4" };
		def.buildPriorities = {
			AiBuildPriorityDef{ U"establish_casque_base", 1.0, 1 },
			AiBuildPriorityDef{ U"assemble_engineer_warehouse", 0.95, 1 },
			AiBuildPriorityDef{ U"build_barbed_wire", 0.65, 2 },
		};
		def.targetPriority = { U"base", U"resource", U"unit" };
		return def;
	}

	inline Array<AiProfileDef> MakeFallbackAiProfiles()
	{
		AiProfileDef balanced = MakeDefaultAiProfile(U"balanced", U"Balanced", U"balanced");
		balanced.description = U"標準的な攻撃・防衛バランスの AI";
		balanced.openingDelaySec = 10.0;
		balanced.spawnIntervalSec = 8.0;
		balanced.attackWaveIntervalSec = 35.0;
		balanced.aggression = 0.55;
		balanced.economyFocus = 0.50;
		balanced.defenseFocus = 0.45;
		balanced.techFocus = 0.35;
		balanced.attackGroupSize = 4;
		balanced.maxArmySize = 24;
		balanced.battleTimeLimitSec = 25.0 * 60.0;

		AiProfileDef longWar = MakeDefaultAiProfile(U"long_war", U"Long War", U"long_war");
		longWar.description = U"序盤は控えめに守り、中盤以降に大きな攻撃波を作る長期戦 AI";
		longWar.openingDelaySec = 25.0;
		longWar.spawnIntervalSec = 11.0;
		longWar.attackWaveIntervalSec = 55.0;
		longWar.aggression = 0.45;
		longWar.economyFocus = 0.80;
		longWar.defenseFocus = 0.65;
		longWar.techFocus = 0.55;
		longWar.attackGroupSize = 7;
		longWar.maxArmySize = 36;
		longWar.battleTimeLimitSec = 25.0 * 60.0;
		longWar.retreatHpRatio = 0.22;

		AiProfileDef blitz = MakeDefaultAiProfile(U"blitz", U"Blitz", U"blitz");
		blitz.description = U"序盤から小規模部隊を頻繁に送り込む電撃戦 AI";
		blitz.openingDelaySec = 4.0;
		blitz.spawnIntervalSec = 5.0;
		blitz.attackWaveIntervalSec = 18.0;
		blitz.aggression = 0.90;
		blitz.economyFocus = 0.25;
		blitz.defenseFocus = 0.20;
		blitz.techFocus = 0.20;
		blitz.attackGroupSize = 3;
		blitz.maxArmySize = 18;
		blitz.battleTimeLimitSec = 25.0 * 60.0;
		blitz.retreatHpRatio = 0.0;

		return { balanced, longWar, blitz };
	}

	inline Array<AiUnitWeightDef> ReadAiUnitWeights(const TOMLValue& value)
	{
		Array<AiUnitWeightDef> weights;
		if (!value.isTableArray())
		{
			return weights;
		}

		for (const auto weightValue : value.tableArrayView())
		{
			const String unitTag = weightValue[AiProfileToml::KeyUnit].getOr<String>(U"").lowercased();
			if (unitTag.isEmpty())
			{
				continue;
			}

			weights << AiUnitWeightDef{
				unitTag,
				Max(0.0, weightValue[AiProfileToml::KeyWeight].getOr<double>(1.0)),
				Max(0, weightValue[AiProfileToml::KeyDesiredCount].getOr<int32>(0)),
			};
		}

		return weights;
	}

	inline Array<AiBuildPriorityDef> ReadAiBuildPriorities(const TOMLValue& value)
	{
		Array<AiBuildPriorityDef> priorities;
		if (!value.isTableArray())
		{
			return priorities;
		}

		for (const auto priorityValue : value.tableArrayView())
		{
			const String actionTag = priorityValue[AiProfileToml::KeyAction].getOr<String>(U"").lowercased();
			if (actionTag.isEmpty())
			{
				continue;
			}

			priorities << AiBuildPriorityDef{
				actionTag,
				Max(0.0, priorityValue[AiProfileToml::KeyWeight].getOr<double>(1.0)),
				Max(0, priorityValue[AiProfileToml::KeyDesiredCount].getOr<int32>(1)),
			};
		}

		return priorities;
	}

	inline void NormalizeAiProfile(AiProfileDef& def)
	{
		def.tag = def.tag.lowercased();
		def.contactBehavior = def.contactBehavior.lowercased();
		if (def.contactBehavior != U"engage")
		{
			def.contactBehavior = U"ignore";
		}
		def.openingDelaySec = Max(0.0, def.openingDelaySec);
		def.spawnIntervalSec = Max(0.25, def.spawnIntervalSec);
		def.attackWaveIntervalSec = Max(1.0, def.attackWaveIntervalSec);
		def.aggression = ClampAiProfileRate(def.aggression);
		def.economyFocus = ClampAiProfileRate(def.economyFocus);
		def.defenseFocus = ClampAiProfileRate(def.defenseFocus);
		def.techFocus = ClampAiProfileRate(def.techFocus);
		def.attackGroupSize = Max(1, def.attackGroupSize);
		def.maxArmySize = Max(def.attackGroupSize, def.maxArmySize);
		def.battleTimeLimitSec = Max(60.0, def.battleTimeLimitSec);
		def.retreatHpRatio = ClampAiProfileRate(def.retreatHpRatio);
		def.resourceMultiplier = Max(0.0, def.resourceMultiplier);
		for (auto& unitTag : def.initialUnits)
		{
			unitTag = unitTag.lowercased();
		}
		for (auto& unitWeight : def.unitWeights)
		{
			unitWeight.unitTag = unitWeight.unitTag.lowercased();
			unitWeight.weight = Max(0.0, unitWeight.weight);
			unitWeight.desiredCount = Max(0, unitWeight.desiredCount);
		}
		for (auto& buildPriority : def.buildPriorities)
		{
			buildPriority.actionTag = buildPriority.actionTag.lowercased();
			buildPriority.weight = Max(0.0, buildPriority.weight);
			buildPriority.desiredCount = Max(0, buildPriority.desiredCount);
		}
	}

	inline void AddFallbackAiProfiles(DefinitionStores& defs)
	{
		for (auto profile : MakeFallbackAiProfiles())
		{
			NormalizeAiProfile(profile);
			defs.addAiProfile(profile);
		}
	}

	inline void LoadAiProfileDefinitions(DefinitionStores& defs)
	{
		defs.aiProfiles.clear();
		defs.aiProfileByTag.clear();

		const FilePath aiProfilePath = ResolveAiProfileTomlPath();
		const TOMLReader toml{ aiProfilePath };
		if (!toml)
		{
			AddFallbackAiProfiles(defs);
			return;
		}

		HashSet<String> loadedTags;
		for (const auto profileValue : toml[AiProfileToml::KeyProfiles].tableArrayView())
		{
			AiProfileDef def;
			def.tag = profileValue[AiProfileToml::KeyTag].getOr<String>(U"").lowercased();
			if (def.tag.isEmpty() || loadedTags.contains(def.tag))
			{
				continue;
			}

			def.name = profileValue[AiProfileToml::KeyName].getOr<String>(def.tag);
			def.description = profileValue[AiProfileToml::KeyDescription].getOr<String>(U"");
			def.presetType = profileValue[AiProfileToml::KeyPresetType].getOr<String>(def.tag).lowercased();
			def.openingDelaySec = profileValue[AiProfileToml::KeyOpeningDelaySec].getOr<double>(def.openingDelaySec);
			def.spawnIntervalSec = profileValue[AiProfileToml::KeySpawnIntervalSec].getOr<double>(def.spawnIntervalSec);
			def.attackWaveIntervalSec = profileValue[AiProfileToml::KeyAttackWaveIntervalSec].getOr<double>(def.attackWaveIntervalSec);
			def.aggression = profileValue[AiProfileToml::KeyAggression].getOr<double>(def.aggression);
			def.economyFocus = profileValue[AiProfileToml::KeyEconomyFocus].getOr<double>(def.economyFocus);
			def.defenseFocus = profileValue[AiProfileToml::KeyDefenseFocus].getOr<double>(def.defenseFocus);
			def.techFocus = profileValue[AiProfileToml::KeyTechFocus].getOr<double>(def.techFocus);
			def.attackGroupSize = profileValue[AiProfileToml::KeyAttackGroupSize].getOr<int32>(def.attackGroupSize);
			def.maxArmySize = profileValue[AiProfileToml::KeyMaxArmySize].getOr<int32>(def.maxArmySize);
			def.battleTimeLimitSec = profileValue[AiProfileToml::KeyBattleTimeLimitSec].getOr<double>(def.battleTimeLimitSec);
			def.retreatHpRatio = profileValue[AiProfileToml::KeyRetreatHpRatio].getOr<double>(def.retreatHpRatio);
			def.freeSpawnEnabled = profileValue[AiProfileToml::KeyFreeSpawnEnabled].getOr<bool>(def.freeSpawnEnabled);
			def.resourceMultiplier = profileValue[AiProfileToml::KeyResourceMultiplier].getOr<double>(def.resourceMultiplier);
			def.contactBehavior = profileValue[AiProfileToml::KeyContactBehavior].getOr<String>(def.contactBehavior);
			def.initialUnits = ReadTomlStringArrayValue(profileValue[AiProfileToml::KeyInitialUnits]);
			def.unitWeights = ReadAiUnitWeights(profileValue[AiProfileToml::KeyUnitWeights]);
			def.buildPriorities = ReadAiBuildPriorities(profileValue[AiProfileToml::KeyBuildPriorities]);
			def.targetPriority = ReadTomlStringArrayValue(profileValue[AiProfileToml::KeyTargetPriority]);
			NormalizeAiProfile(def);

			defs.addAiProfile(def);
			loadedTags.insert(def.tag);
		}

		if (defs.aiProfiles.isEmpty())
		{
			AddFallbackAiProfiles(defs);
		}
	}

	inline bool SaveAiProfileDefinitions(const DefinitionStores& defs, String& statusText)
	{
		const FilePath aiProfilePath = ResolveAiProfileTomlPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(aiProfilePath));
		TextWriter writer{ aiProfilePath };
		if (!writer)
		{
			statusText = U"AI profile save failed: {}"_fmt(aiProfilePath);
			return false;
		}

		String tomlText;
		bool firstProfile = true;
		for (auto profile : defs.aiProfiles)
		{
			NormalizeAiProfile(profile);
			if (profile.tag.isEmpty())
			{
				continue;
			}

			if (!firstProfile)
			{
				tomlText += U"\n";
			}
			firstProfile = false;

			tomlText += U"[[profiles]]\n";
			tomlText += U"tag = \"" + EscapeTomlBasicString(profile.tag) + U"\"\n";
			tomlText += U"name = \"" + EscapeTomlBasicString(profile.name) + U"\"\n";
			tomlText += U"description = \"" + EscapeTomlBasicString(profile.description) + U"\"\n";
			tomlText += U"preset_type = \"" + EscapeTomlBasicString(profile.presetType) + U"\"\n";
			tomlText += U"opening_delay_sec = {}\n"_fmt(profile.openingDelaySec);
			tomlText += U"spawn_interval_sec = {}\n"_fmt(profile.spawnIntervalSec);
			tomlText += U"attack_wave_interval_sec = {}\n"_fmt(profile.attackWaveIntervalSec);
			tomlText += U"aggression = {}\n"_fmt(profile.aggression);
			tomlText += U"economy_focus = {}\n"_fmt(profile.economyFocus);
			tomlText += U"defense_focus = {}\n"_fmt(profile.defenseFocus);
			tomlText += U"tech_focus = {}\n"_fmt(profile.techFocus);
			tomlText += U"attack_group_size = {}\n"_fmt(profile.attackGroupSize);
			tomlText += U"max_army_size = {}\n"_fmt(profile.maxArmySize);
			tomlText += U"battle_time_limit_sec = {}\n"_fmt(profile.battleTimeLimitSec);
			tomlText += U"retreat_hp_ratio = {}\n"_fmt(profile.retreatHpRatio);
			tomlText += U"free_spawn_enabled = {}\n"_fmt(profile.freeSpawnEnabled ? U"true" : U"false");
			tomlText += U"resource_multiplier = {}\n"_fmt(profile.resourceMultiplier);
			tomlText += U"contact_behavior = \"" + EscapeTomlBasicString(profile.contactBehavior) + U"\"\n";
			tomlText += U"initial_units = " + BuildTomlStringArrayValue(profile.initialUnits) + U"\n";
			tomlText += U"target_priority = " + BuildTomlStringArrayValue(profile.targetPriority) + U"\n";
			for (const auto& buildPriority : profile.buildPriorities)
			{
				if (buildPriority.actionTag.isEmpty())
				{
					continue;
				}

				tomlText += U"[[profiles.build_priorities]]\n";
				tomlText += U"action = \"" + EscapeTomlBasicString(buildPriority.actionTag) + U"\"\n";
				tomlText += U"weight = {}\n"_fmt(Max(0.0, buildPriority.weight));
				tomlText += U"desired_count = {}\n"_fmt(Max(0, buildPriority.desiredCount));
			}
			for (const auto& unitWeight : profile.unitWeights)
			{
				if (unitWeight.unitTag.isEmpty())
				{
					continue;
				}

				tomlText += U"[[profiles.unit_weights]]\n";
				tomlText += U"unit = \"" + EscapeTomlBasicString(unitWeight.unitTag) + U"\"\n";
				tomlText += U"weight = {}\n"_fmt(Max(0.0, unitWeight.weight));
				tomlText += U"desired_count = {}\n"_fmt(Max(0, unitWeight.desiredCount));
			}
		}

		writer.write(tomlText);
		statusText = U"Saved AI profiles: {}"_fmt(aiProfilePath);
		return true;
	}
}
