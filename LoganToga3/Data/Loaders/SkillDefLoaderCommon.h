#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../TomlTextUtils.h"
# include "../BattleAssetPaths.h"

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
}
