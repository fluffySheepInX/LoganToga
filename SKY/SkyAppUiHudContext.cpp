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
		DrawMiniMap(state.miniMapExpanded,
			frame.panels,
			state.uiEditMode,
           not frame.isEditorMode,
			state.camera,
			state.mapData,
           state.fogOfWar,
			state.spawnedSappers,
			state.enemySappers,
			state.resourceAreaStates,
			state.selectedSapperIndices);

		if (frame.isEditorMode)
		{
			DrawMapEditorPanel(state.mapEditor, state.mapData, state.currentMapPath, frame.panels.mapEditor);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && state.showBlacksmithMenu)
		{
            DrawBattleCommandMenu(frame.panels,
				state.spawnedSappers,
				state.mapData,
				state.mapData.playerBasePosition,
				state.mapData.sapperRallyPoint,
				state.playerResources,
               state.battleCommandSelectedSlotIndex,
				state.battleCommandUnlockedSlotCount,
				state.unitEditorSettings,
				state.modelHeightSettings,
				state.blacksmithMenuMessage);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
		{
			const size_t selectedIndex = state.selectedSapperIndices.front();

			const SapperMenuAction menuAction = DrawSapperMenu(frame.panels,
				state.spawnedSappers,
				state.playerResources,
              state.unitEditorSettings,
				selectedIndex,
				state.blacksmithMenuMessage);
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
				state.uiEditMode,
				state.unitEditorSettings,
				state.unitEditorSelection,
				state.unitEditorPage,
				state.spawnedSappers,
				state.enemySappers,
				state.unitEditorMessage);
		}

		if (frame.showMillStatusEditor && state.selectedMillIndex)
		{
			DrawMillStatusEditor(frame.panels, state.mapData, *state.selectedMillIndex, MapDataPath, state.mapDataMessage);
		}
	}
}
