# pragma once
# include "SkyAppUi.hpp"
# include "SkyAppUiParameterEditorInternal.hpp"

namespace SkyAppSupport
{
    namespace UnitEditorDetail
    {
        void DrawUnitEditorSetupSection(const Rect& detailPanel,
            MainSupport::UnitEditorSettings& unitEditorSettings,
            MainSupport::UnitTeam team,
            MainSupport::SapperUnitType unitType,
            MainSupport::UnitEditorPage activePage,
            MainSupport::UnitParameters& parameters,
            String& hoveredDescription,
            Optional<Rect>& hoveredRect,
            TimedMessage& unitEditorMessage);

        void DrawUnitEditorFooter(const Rect& detailPanel,
            MainSupport::UnitEditorSettings& unitEditorSettings,
            MainSupport::UnitTeam team,
            MainSupport::SapperUnitType unitType,
            MainSupport::UnitEditorPage activePage,
            MainSupport::UnitParameters& parameters,
            MainSupport::ExplosionSkillParameters& explosionSkillParameters,
            MainSupport::BuildMillSkillParameters& buildMillSkillParameters,
            MainSupport::HealSkillParameters& healSkillParameters,
            MainSupport::ScoutSkillParameters& scoutSkillParameters,
            Array<MainSupport::SpawnedSapper>& spawnedSappers,
            Array<MainSupport::SpawnedSapper>& enemySappers,
            String& hoveredDescription,
            Optional<Rect>& hoveredRect,
            TimedMessage& unitEditorMessage);
    }
}
