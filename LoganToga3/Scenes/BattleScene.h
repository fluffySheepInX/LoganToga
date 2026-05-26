#pragma once
# include <Siv3D.hpp>
# include "../App/AppRuntimeState.h"
# include "../App/AppUiState.h"
# include "../App/AppSceneSharedData.h"
# include "../App/AppUpdate.h"
# include "../App/AppRender.h"
# include "../Data/MusicManager.h"
# include "../Data/MusicPreview.h"

namespace LT3
{
	class BattleScene : public AppSceneManager::Scene
	{
	public:
		explicit BattleScene(const InitData& init)
			: AppSceneManager::Scene(init)
		{
			auto& data = getData();
			InitializeAppUiState(m_ui);
			InitializeAppRuntimeState(m_runtime, data.definitions);
			SyncBattleWorldMapFromEditor(m_ui.mapEditor, m_runtime.world, data.definitions.defs);
			StopMusicPreview(data.musicEditor);
			PlaySceneMusic(data, MusicSceneId::Battle);
		}

		void update() override
		{
			auto& data = getData();
			if (KeyEscape.down())
			{
				StopMusicPreview(data.musicEditor);
				changeScene(AppSceneState::Title, 0.4s);
				return;
			}

			UpdateAppUiState(m_ui);
			UpdateAppRuntimeState(m_runtime, data.definitions, m_ui);
		}

		void draw() const override
		{
			auto& data = getData();
			DrawAppRuntime(m_runtime, data.definitions, m_ui, data.uiFont, data.titleFont);
			DrawAppUi(data.definitions, m_ui, data.uiFont);
		}

		AppRuntimeState& runtime()
		{
			return m_runtime;
		}

		AppUiState& ui()
		{
			return m_ui;
		}

	private:
		AppRuntimeState m_runtime;
		mutable AppUiState m_ui;
	};
}
