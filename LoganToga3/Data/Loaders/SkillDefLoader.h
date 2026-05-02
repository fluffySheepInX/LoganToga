#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"

namespace LT3
{
    inline void LoadSkillDefinitions(DefinitionStores& defs)
    {
        defs.skills.clear();
        defs.skillByTag.clear();

        defs.addSkill({ U"worker_hit", U"Tool Strike", 70.0, 0.75, 4, Palette::Khaki });
        defs.addSkill({ U"sword", U"Sword", 82.0, 0.65, 9, Palette::Orange });
        defs.addSkill({ U"arrow", U"Arrow", 210.0, 1.05, 7, Palette::Skyblue });
        defs.addSkill({ U"base_shot", U"Base Shot", 230.0, 1.35, 10, Palette::Violet });
    }
}
