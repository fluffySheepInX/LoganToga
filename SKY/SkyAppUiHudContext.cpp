# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	void DrawContextHud(SkyAppState& state, const SkyAppFrameState& frame)
	{
		DrawMiniMap(state.hud.miniMapExpanded,
			frame.panels,
			state.hud.uiEditMode,
           not frame.isEditorMode,
			state.env.camera,
			state.world.mapData,
           state.env.fogOfWar,
			state.battle.spawnedSappers,
			state.battle.enemySappers,
			state.battle.resourceAreaStates,
			state.battle.selectedSapperIndices);

		if (frame.isEditorMode)
		{
			DrawMapEditorPanel(state.editor.mapEditor, state.world.mapData, state.world.currentMapPath, frame.panels.mapEditor);
		}

		if ((not frame.isEditorMode) && (not state.battle.playerWon) && state.hud.showBlacksmithMenu)
		{
            DrawBattleCommandMenu(frame.panels,
				state.battle.spawnedSappers,
				state.world.mapData,
				state.world.mapData.playerBasePosition,
				state.world.mapData.sapperRallyPoint,
				state.battle.playerResources,
               state.battle.battleCommandSelectedSlotIndex,
				state.battle.battleCommandUnlockedSlotCount,
				state.editor.unitEditorSettings,
				state.editor.modelHeightSettings,
				state.messages[SkyAppSupport::MessageChannel::BlacksmithMenu]);
		}

		if ((not frame.isEditorMode) && (not state.battle.playerWon) && (state.battle.selectedSapperIndices.size() == 1))
		{
			const size_t selectedIndex = state.battle.selectedSapperIndices.front();

			const SapperMenuAction menuAction = DrawSapperMenu(frame.panels,
				state.battle.spawnedSappers,
				state.battle.playerResources,
              state.editor.unitEditorSettings,
				selectedIndex,
				state.messages[SkyAppSupport::MessageChannel::BlacksmithMenu]);
         if (menuAction == SapperMenuAction::UseUniqueSkill)
			{
              TryUsePlayerSapperUniqueSkill(state, selectedIndex);
			}
			else if (menuAction == SapperMenuAction::UseExplosionSkill)
			{
				TryUsePlayerSapperExplosionSkill(state, selectedIndex);
			}
			else if (menuAction == SapperMenuAction::Retreat)
			{
				TryOrderPlayerSapperRetreat(state, selectedIndex);
			}
		}

		if (frame.showUnitEditor)
		{
			DrawUnitEditor(frame.panels,
				state.hud.uiEditMode,
				state.editor.unitEditorSettings,
				state.editor.unitEditorSelection,
				state.editor.unitEditorPage,
				state.battle.spawnedSappers,
				state.battle.enemySappers,
				state.messages[SkyAppSupport::MessageChannel::UnitEditor]);
		}

		if (frame.showMillStatusEditor && state.battle.selectedMillIndex)
		{
			DrawMillStatusEditor(frame.panels, state.world.mapData, MapDataPath, state.messages[SkyAppSupport::MessageChannel::MapData]);
		}
	}
}
