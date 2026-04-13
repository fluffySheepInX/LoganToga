# include "SkyAppCampaignEditorSceneInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
		using namespace CampaignEditorDetail;

		class SkyMissionDialogueEditorScene : public App::Scene
		{
		public:
			explicit SkyMissionDialogueEditorScene(const InitData& init)
				: App::Scene{ init }
				, m_titleFont{ 30, Typeface::Heavy }
				, m_labelFont{ 18, Typeface::Medium }
              , m_textFont{ 18 }
				, m_buttonFont{ 22, Typeface::Medium }
				, m_missionIndex{ getData().selectedEditorMissionIndex.value_or(0) }
				, m_originalPre{ getData().missionPreDialogueStates[m_missionIndex] }
				, m_originalPost{ getData().missionPostDialogueStates[m_missionIndex] }
               , m_preText{ getData().missionPreDialogueStates[m_missionIndex].text }
				, m_postText{ getData().missionPostDialogueStates[m_missionIndex].text }
				, m_preCursorIndex{ m_preText.size() }
				, m_postCursorIndex{ m_postText.size() }
			{
			}

			void update() override
			{
				auto& data = getData();
				ClampEditorMissionSelection(data);
				Scene::Rect().draw(ColorF{ 0.05, 0.09, 0.15 });
				const RectF panel{ 104, 70, 1072, 580 };
               const RectF preEditorRect{ panel.x + 30, panel.y + 176, 930, 118 };
				const RectF postEditorRect{ panel.x + 30, panel.y + 364, 930, 118 };
				panel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				panel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });

               const Array<WrappedEditorLine> preWrappedLines = BuildWrappedEditorLines(m_preText, m_textFont, (preEditorRect.w - 40.0));
				const Array<WrappedEditorLine> postWrappedLines = BuildWrappedEditorLines(m_postText, m_textFont, (postEditorRect.w - 40.0));
				const double lineHeight = 28.0;
				const int32 preVisibleLineCount = Max(1, static_cast<int32>((preEditorRect.h - 24.0) / lineHeight));
				const int32 postVisibleLineCount = Max(1, static_cast<int32>((postEditorRect.h - 24.0) / lineHeight));
				const int32 preMaxScrollLine = Max(0, static_cast<int32>(preWrappedLines.size()) - preVisibleLineCount);
				const int32 postMaxScrollLine = Max(0, static_cast<int32>(postWrappedLines.size()) - postVisibleLineCount);

                m_titleFont(U"Mission Dialogue").draw(panel.pos.movedBy(28, 24), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
				m_labelFont(U"{}"_fmt(data.missionNameStates[m_missionIndex].text.isEmpty() ? U"Mission" : data.missionNameStates[m_missionIndex].text)).draw(panel.pos.movedBy(30, 68), SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor());
				m_labelFont(U"Custom multiline editor / Enter: newline / Wheel: scroll").draw(panel.pos.movedBy(30, 96), SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor());

                m_labelFont(U"Pre Dialogue").draw(panel.pos.movedBy(30, 148), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
             HandleEditorFocus(preEditorRect, preWrappedLines, lineHeight, true);
				if (preEditorRect.mouseOver())
				{
					m_preScrollLine = Clamp((m_preScrollLine - static_cast<int32>(Mouse::Wheel())), 0, preMaxScrollLine);
				}
				DrawTextEditor(preEditorRect, preWrappedLines, lineHeight, preVisibleLineCount, m_preScrollLine, m_preCursorIndex, (m_focusedField == DialogueEditorField::Pre), m_preText);
                m_labelFont(U"Preview: {}"_fmt(MakeDialogueSummary(m_preText))).draw(panel.pos.movedBy(30, 302), SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor());

             m_labelFont(U"Post Dialogue").draw(panel.pos.movedBy(30, 336), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
				HandleEditorFocus(postEditorRect, postWrappedLines, lineHeight, false);
				if (postEditorRect.mouseOver())
				{
					m_postScrollLine = Clamp((m_postScrollLine - static_cast<int32>(Mouse::Wheel())), 0, postMaxScrollLine);
				}
				DrawTextEditor(postEditorRect, postWrappedLines, lineHeight, postVisibleLineCount, m_postScrollLine, m_postCursorIndex, (m_focusedField == DialogueEditorField::Post), m_postText);
                m_labelFont(U"Preview: {}"_fmt(MakeDialogueSummary(m_postText))).draw(panel.pos.movedBy(30, 490), SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor());

				if (m_focusedField == DialogueEditorField::Pre)
				{
					m_preCursorIndex = TextInput::UpdateText(m_preText, m_preCursorIndex, TextInputMode::AllowEnterBackSpaceDelete);
					HandleCursorMoveKeys(m_preText, m_preCursorIndex);
					KeepCursorVisible(preWrappedLines, m_preCursorIndex, preVisibleLineCount, m_preScrollLine, preMaxScrollLine);
				}
				else if (m_focusedField == DialogueEditorField::Post)
				{
					m_postCursorIndex = TextInput::UpdateText(m_postText, m_postCursorIndex, TextInputMode::AllowEnterBackSpaceDelete);
					HandleCursorMoveKeys(m_postText, m_postCursorIndex);
					KeepCursorVisible(postWrappedLines, m_postCursorIndex, postVisibleLineCount, m_postScrollLine, postMaxScrollLine);
				}

				DrawEditorActionButton(GetApplyButton(), m_buttonFont, U"Apply", EditorActionButtonStyle::Primary);
				DrawEditorActionButton(GetCancelButton(), m_labelFont, U"Cancel", EditorActionButtonStyle::Back);

				if (IsRectButtonClicked(GetApplyButton()))
				{
                  data.missionPreDialogueStates[m_missionIndex].text = m_preText;
					data.missionPostDialogueStates[m_missionIndex].text = m_postText;
					changeScene(U"CampaignEditor", 0);
					return;
				}

				if (IsRectButtonClicked(GetCancelButton()) || KeyEscape.down())
				{
					data.missionPreDialogueStates[m_missionIndex] = m_originalPre;
					data.missionPostDialogueStates[m_missionIndex] = m_originalPost;
					changeScene(U"CampaignEditor", 0);
					return;
				}
			}

			void draw() const override
			{
			}

		private:
           enum class DialogueEditorField
			{
				None,
				Pre,
				Post,
			};

			Font m_titleFont;
			Font m_labelFont;
          Font m_textFont;
			Font m_buttonFont;
			size_t m_missionIndex;
			TextEditState m_originalPre;
			TextEditState m_originalPost;
			String m_preText;
			String m_postText;
			size_t m_preCursorIndex = 0;
			size_t m_postCursorIndex = 0;
			int32 m_preScrollLine = 0;
			int32 m_postScrollLine = 0;
			DialogueEditorField m_focusedField = DialogueEditorField::Pre;

			void HandleEditorFocus(const RectF& rect, const Array<WrappedEditorLine>& lines, const double lineHeight, const bool isPreEditor)
			{
				if (rect.leftClicked())
				{
					m_focusedField = (isPreEditor ? DialogueEditorField::Pre : DialogueEditorField::Post);
					if (isPreEditor)
					{
						m_preCursorIndex = FindCursorIndexFromPoint(lines, rect, lineHeight, Cursor::PosF(), m_preText, m_preScrollLine);
					}
					else
					{
						m_postCursorIndex = FindCursorIndexFromPoint(lines, rect, lineHeight, Cursor::PosF(), m_postText, m_postScrollLine);
					}
				}
			}

			void DrawTextEditor(const RectF& rect, const Array<WrappedEditorLine>& wrappedLines, const double lineHeight, const int32 visibleLineCount, const int32 scrollLine, const size_t cursorIndex, const bool focused, const String& text) const
			{
				rect.rounded(14).draw(ColorF{ 0.07, 0.11, 0.18, 0.96 });
				rect.rounded(14).drawFrame(2, 0, focused ? ColorF{ 0.82, 0.90, 1.0, 0.94 } : ColorF{ 0.42, 0.52, 0.64, 0.82 });

				{
					const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
					const Rect previousScissor = Graphics2D::GetScissorRect();
					Graphics2D::SetScissorRect(rect.stretched(-8).asRect());

					for (int32 i = 0; i < visibleLineCount; ++i)
					{
						const int32 lineIndex = (scrollLine + i);
						if (wrappedLines.size() <= static_cast<size_t>(lineIndex))
						{
							break;
						}

                        m_textFont(wrappedLines[lineIndex].text).draw(rect.pos.movedBy(16, 12 + i * lineHeight), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
					}

					if (focused)
					{
						const Vec2 caretPos = GetCursorDrawPosition(wrappedLines, rect, lineHeight, cursorIndex, scrollLine, text);
						if ((rect.y + 8) <= caretPos.y && caretPos.y <= (rect.bottomY() - 24))
						{
                          Line{ caretPos, caretPos.movedBy(0, 22) }.draw(2.0, ColorF{ SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().r, SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().g, SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().b, (0.55 + 0.45 * Periodic::Sine0_1(1.0s)) });
						}
					}

					Graphics2D::SetScissorRect(previousScissor);
				}

				const int32 maxScrollLine = Max(0, static_cast<int32>(wrappedLines.size()) - visibleLineCount);
				if (0 < maxScrollLine)
				{
					const RectF scrollTrack{ rect.rightX() - 12, rect.y + 10, 6, (rect.h - 20) };
					scrollTrack.rounded(3).draw(ColorF{ 0.16, 0.20, 0.28, 0.88 });
					const double thumbHeight = Max(32.0, (scrollTrack.h * visibleLineCount / wrappedLines.size()));
					const double thumbY = scrollTrack.y + ((scrollTrack.h - thumbHeight) * scrollLine / maxScrollLine);
					RectF{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight }.rounded(3).draw(ColorF{ 0.74, 0.84, 0.96, 0.88 });
				}
			}

			void HandleCursorMoveKeys(const String& text, size_t& cursorIndex)
			{
				if (KeyLeft.down() && (0 < cursorIndex))
				{
					--cursorIndex;
				}
				if (KeyRight.down() && (cursorIndex < text.size()))
				{
					++cursorIndex;
				}
				if (KeyHome.down())
				{
					cursorIndex = 0;
				}
				if (KeyEnd.down())
				{
					cursorIndex = text.size();
				}
			}

			void KeepCursorVisible(const Array<WrappedEditorLine>& lines, const size_t cursorIndex, const int32 visibleLineCount, int32& scrollLine, const int32 maxScrollLine) const
			{
				const int32 cursorLineIndex = FindCursorLineIndex(lines, cursorIndex);
				if (cursorLineIndex < scrollLine)
				{
					scrollLine = cursorLineIndex;
				}
				else if ((scrollLine + visibleLineCount) <= cursorLineIndex)
				{
					scrollLine = Max(0, (cursorLineIndex - visibleLineCount + 1));
				}

				scrollLine = Clamp(scrollLine, 0, maxScrollLine);
			}

			[[nodiscard]] static int32 FindCursorLineIndex(const Array<WrappedEditorLine>& lines, const size_t cursorIndex)
			{
				for (size_t i = 0; i < lines.size(); ++i)
				{
					const size_t nextStart = ((i + 1) < lines.size() ? lines[i + 1].startIndex : (lines[i].endIndex + 1));
					if ((lines[i].startIndex <= cursorIndex) && (cursorIndex < nextStart))
					{
						return static_cast<int32>(i);
					}
				}

				return static_cast<int32>(lines.size() - 1);
			}

			[[nodiscard]] size_t FindCursorIndexFromPoint(const Array<WrappedEditorLine>& lines, const RectF& rect, const double lineHeight, const Vec2& cursorPos, const String& text, const int32 scrollLine) const
			{
				const int32 lineIndex = Clamp((scrollLine + static_cast<int32>((cursorPos.y - rect.y - 12) / lineHeight)), 0, static_cast<int32>(lines.size()) - 1);
				const WrappedEditorLine& line = lines[lineIndex];
				double bestDistance = Math::Inf;
				size_t bestIndex = line.startIndex;

				for (size_t i = line.startIndex; i <= line.endIndex; ++i)
				{
					const String prefix = text.substr(line.startIndex, (i - line.startIndex));
					const double x = (rect.x + 16 + m_textFont(prefix).region().w);
					const double distance = Abs(cursorPos.x - x);
					if (distance < bestDistance)
					{
						bestDistance = distance;
						bestIndex = i;
					}
				}

				return bestIndex;
			}

			[[nodiscard]] Vec2 GetCursorDrawPosition(const Array<WrappedEditorLine>& lines, const RectF& rect, const double lineHeight, const size_t cursorIndex, const int32 scrollLine, const String& text) const
			{
				const int32 lineIndex = FindCursorLineIndex(lines, cursorIndex);
				const WrappedEditorLine& line = lines[lineIndex];
				const size_t clampedIndex = Clamp(cursorIndex, line.startIndex, line.endIndex);
				const String prefix = text.substr(line.startIndex, (clampedIndex - line.startIndex));
				return Vec2{ rect.x + 16 + m_textFont(prefix).region().w, rect.y + 12 + (lineIndex - scrollLine) * lineHeight };
			}

			[[nodiscard]] RectF GetApplyButton() const
			{
				return RectF{ 848, 584, 180, 46 };
			}

			[[nodiscard]] RectF GetCancelButton() const
			{
				return RectF{ 1040, 584, 120, 46 };
			}
		};
	}

	void AddMissionDialogueEditorScene(App& manager)
	{
		manager.add<SkyMissionDialogueEditorScene>(U"MissionDialogueEditor");
	}
}
