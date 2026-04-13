# include "SkyAppCampaignEditorSceneInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
		using namespace CampaignEditorDetail;

		class SkyCampaignDescriptionEditorScene : public App::Scene
		{
		public:
			explicit SkyCampaignDescriptionEditorScene(const InitData& init)
				: App::Scene{ init }
				, m_titleFont{ 30, Typeface::Heavy }
				, m_labelFont{ 18, Typeface::Medium }
				, m_textFont{ 18 }
				, m_buttonFont{ 22, Typeface::Medium }
				, m_originalState{ getData().campaignDescriptionState }
				, m_workingText{ getData().campaignDescriptionState.text }
				, m_cursorIndex{ getData().campaignDescriptionState.text.size() }
			{
			}

			void update() override
			{
				auto& data = getData();
				Scene::Rect().draw(ColorF{ 0.05, 0.09, 0.15 });
				const RectF panel{ 120, 86, 1040, 548 };
				const RectF editorRect{ panel.x + 28, panel.y + 146, 884, 288 };
				const RectF hintRect{ panel.x + 28, panel.y + 446, 884, 46 };
				panel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				panel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });

				const Array<WrappedEditorLine> wrappedLines = BuildWrappedEditorLines(m_workingText, m_textFont, (editorRect.w - 40.0));
				const double lineHeight = 28.0;
				const int32 visibleLineCount = Max(1, static_cast<int32>((editorRect.h - 24.0) / lineHeight));
				const int32 maxScrollLine = Max(0, static_cast<int32>(wrappedLines.size()) - visibleLineCount);

                m_titleFont(U"Campaign Description").draw(panel.pos.movedBy(26, 22), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
				m_labelFont(U"Campaign 全体説明を編集します").draw(panel.pos.movedBy(28, 66), SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor());
				m_labelFont(U"Description").draw(panel.pos.movedBy(28, 118), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());

				if (editorRect.leftClicked())
				{
					m_isFocused = true;
					m_cursorIndex = FindCursorIndexFromPoint(wrappedLines, editorRect, lineHeight, Cursor::PosF());
				}
				else if (MouseL.down() && (not editorRect.mouseOver()) && (not GetApplyButton().mouseOver()) && (not GetCancelButton().mouseOver()))
				{
					m_isFocused = false;
				}

				if (editorRect.mouseOver())
				{
					m_scrollLine = Clamp((m_scrollLine - static_cast<int32>(Mouse::Wheel())), 0, maxScrollLine);
				}

				if (m_isFocused)
				{
					m_cursorIndex = TextInput::UpdateText(m_workingText, m_cursorIndex, TextInputMode::AllowEnterBackSpaceDelete);
					HandleCursorMoveKeys();
				}

				const int32 cursorLineIndex = FindCursorLineIndex(wrappedLines, m_cursorIndex);
				if (cursorLineIndex < m_scrollLine)
				{
					m_scrollLine = cursorLineIndex;
				}
				else if ((m_scrollLine + visibleLineCount) <= cursorLineIndex)
				{
					m_scrollLine = Max(0, (cursorLineIndex - visibleLineCount + 1));
				}
				m_scrollLine = Clamp(m_scrollLine, 0, maxScrollLine);

				editorRect.rounded(14).draw(ColorF{ 0.07, 0.11, 0.18, 0.96 });
				editorRect.rounded(14).drawFrame(2, 0, m_isFocused ? ColorF{ 0.82, 0.90, 1.0, 0.94 } : ColorF{ 0.42, 0.52, 0.64, 0.82 });

				{
					const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
					const Rect previousScissor = Graphics2D::GetScissorRect();
					Graphics2D::SetScissorRect(editorRect.stretched(-8).asRect());

					for (int32 i = 0; i < visibleLineCount; ++i)
					{
						const int32 lineIndex = (m_scrollLine + i);
						if (wrappedLines.size() <= static_cast<size_t>(lineIndex))
						{
							break;
						}

                        m_textFont(wrappedLines[lineIndex].text).draw(editorRect.pos.movedBy(16, 12 + i * lineHeight), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
					}

					if (m_isFocused && Cursor::Pos().x >= 0)
					{
						const Vec2 caretPos = GetCursorDrawPosition(wrappedLines, editorRect, lineHeight, m_cursorIndex, m_scrollLine);
						if ((editorRect.y + 8) <= caretPos.y && caretPos.y <= (editorRect.bottomY() - 24))
						{
                          Line{ caretPos, caretPos.movedBy(0, 22) }.draw(2.0, ColorF{ SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().r, SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().g, SkyAppSupport::UiInternal::EditorTextOnDarkAccentColor().b, (0.55 + 0.45 * Periodic::Sine0_1(1.0s)) });
						}
					}

					Graphics2D::SetScissorRect(previousScissor);
				}

				if (0 < maxScrollLine)
				{
					const RectF scrollTrack{ editorRect.rightX() - 12, editorRect.y + 10, 6, (editorRect.h - 20) };
					scrollTrack.rounded(3).draw(ColorF{ 0.16, 0.20, 0.28, 0.88 });
					const double thumbHeight = Max(32.0, (scrollTrack.h * visibleLineCount / wrappedLines.size()));
					const double thumbY = scrollTrack.y + ((scrollTrack.h - thumbHeight) * m_scrollLine / maxScrollLine);
					RectF{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight }.rounded(3).draw(ColorF{ 0.74, 0.84, 0.96, 0.88 });
				}

				hintRect.rounded(12).draw(ColorF{ 0.09, 0.13, 0.20, 0.84 });
				hintRect.rounded(12).drawFrame(1, 0, ColorF{ 0.34, 0.46, 0.62, 0.70 });
                m_labelFont(U"Enter: newline / Wheel: scroll / Apply で反映 / Cancel で元に戻す").draw(hintRect.pos.movedBy(14, 10), SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor());

				DrawEditorActionButton(GetApplyButton(), m_buttonFont, U"Apply", EditorActionButtonStyle::Primary);
				DrawEditorActionButton(GetCancelButton(), m_labelFont, U"Cancel", EditorActionButtonStyle::Back);

				if (IsRectButtonClicked(GetApplyButton()))
				{
					data.campaignDescriptionState.text = m_workingText;
					changeScene(U"CampaignEditor", 0);
					return;
				}

				if (IsRectButtonClicked(GetCancelButton()) || KeyEscape.down())
				{
					data.campaignDescriptionState = m_originalState;
					changeScene(U"CampaignEditor", 0);
					return;
				}
			}

			void draw() const override
			{
			}

		private:
			Font m_titleFont;
			Font m_labelFont;
			Font m_textFont;
			Font m_buttonFont;
			TextEditState m_originalState;
			String m_workingText;
			size_t m_cursorIndex = 0;
			int32 m_scrollLine = 0;
			bool m_isFocused = true;

			void HandleCursorMoveKeys()
			{
				if (KeyLeft.down() && (0 < m_cursorIndex))
				{
					--m_cursorIndex;
				}
				if (KeyRight.down() && (m_cursorIndex < m_workingText.size()))
				{
					++m_cursorIndex;
				}
				if (KeyHome.down())
				{
					m_cursorIndex = 0;
				}
				if (KeyEnd.down())
				{
					m_cursorIndex = m_workingText.size();
				}
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

			[[nodiscard]] size_t FindCursorIndexFromPoint(const Array<WrappedEditorLine>& lines, const RectF& rect, const double lineHeight, const Vec2& cursorPos) const
			{
				const int32 lineIndex = Clamp((m_scrollLine + static_cast<int32>((cursorPos.y - rect.y - 12) / lineHeight)), 0, static_cast<int32>(lines.size()) - 1);
				const WrappedEditorLine& line = lines[lineIndex];
				double bestDistance = Math::Inf;
				size_t bestIndex = line.startIndex;

				for (size_t i = line.startIndex; i <= line.endIndex; ++i)
				{
					const String prefix = m_workingText.substr(line.startIndex, (i - line.startIndex));
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

			[[nodiscard]] Vec2 GetCursorDrawPosition(const Array<WrappedEditorLine>& lines, const RectF& rect, const double lineHeight, const size_t cursorIndex, const int32 scrollLine) const
			{
				const int32 lineIndex = FindCursorLineIndex(lines, cursorIndex);
				const WrappedEditorLine& line = lines[lineIndex];
				const size_t clampedIndex = Clamp(cursorIndex, line.startIndex, line.endIndex);
				const String prefix = m_workingText.substr(line.startIndex, (clampedIndex - line.startIndex));
				return Vec2{ rect.x + 16 + m_textFont(prefix).region().w, rect.y + 12 + (lineIndex - scrollLine) * lineHeight };
			}

			[[nodiscard]] RectF GetApplyButton() const
			{
				return RectF{ 812, 528, 180, 46 };
			}

			[[nodiscard]] RectF GetCancelButton() const
			{
				return RectF{ 1004, 528, 120, 46 };
			}
		};
	}

	void AddCampaignDescriptionEditorScene(App& manager)
	{
		manager.add<SkyCampaignDescriptionEditorScene>(U"CampaignDescriptionEditor");
	}
}
