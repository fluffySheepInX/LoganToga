# include "SkyAppInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
		class SkyDialogueScene : public App::Scene
		{
		public:
			explicit SkyDialogueScene(const InitData& init)
				: App::Scene{ init }
				, m_titleFont{ 30, Typeface::Heavy }
				, m_textFont{ 24, Typeface::Medium }
				, m_infoFont{ 16 }
			{
			}

			void update() override
			{
				auto& data = getData();

				if (data.dialogueSceneLines.isEmpty())
				{
					FinalizeDialogue(data);
					return;
				}

				if (KeyEnter.down() || KeySpace.down() || MouseL.down())
				{
					if ((data.dialogueSceneLineIndex + 1) < data.dialogueSceneLines.size())
					{
						++data.dialogueSceneLineIndex;
					}
					else
					{
						FinalizeDialogue(data);
					}
				}
			}

			void draw() const override
			{
				const auto& data = getData();
				Scene::Rect().draw(ColorF{ 0.05, 0.08, 0.14 });
				const RectF panel{ 96, 96, 1088, 520 };
				panel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				panel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });

				m_titleFont(data.dialogueSceneTitle.isEmpty() ? U"Intermission" : data.dialogueSceneTitle).draw(panel.pos.movedBy(30, 28), Palette::White);
				m_infoFont(U"{} / {}"_fmt(Min(data.dialogueSceneLineIndex + 1, Max<size_t>(1, data.dialogueSceneLines.size())), Max<size_t>(1, data.dialogueSceneLines.size()))).draw(panel.pos.movedBy(30, 78), ColorF{ 0.82, 0.89, 0.98, 0.92 });

				const String line = data.dialogueSceneLines.isEmpty()
					? U""
					: data.dialogueSceneLines[Min(data.dialogueSceneLineIndex, (data.dialogueSceneLines.size() - 1))];
				m_textFont(line).draw(RectF{ 138, 220, 1004, 180 }, ColorF{ 0.96, 0.98, 1.0 });
				m_infoFont(U"Enter / Space / Click to continue").draw(panel.pos.movedBy(30, 460), ColorF{ 1.0, 0.94, 0.72, 0.96 });
			}

		private:
			Font m_titleFont;
			Font m_textFont;
			Font m_infoFont;

			void FinalizeDialogue(SkyAppData& data)
			{
				const String nextScene = data.dialogueNextScene;
				data.dialogueSceneTitle.clear();
				data.dialogueSceneLines.clear();
				data.dialogueSceneLineIndex = 0;
				data.dialogueNextScene = U"Title";

				if (nextScene != U"Battle")
				{
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
				}

				changeScene(nextScene, 0);
			}
		};
	}

	void AddDialogueScene(App& manager)
	{
		manager.add<SkyDialogueScene>(U"Dialogue");
	}
}
