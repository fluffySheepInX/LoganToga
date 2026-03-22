#include "TitleUiEditorScene.h"

#include "Localization.h"

TitleUiEditorScene::TitleUiEditorScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_layout{ TitleUi::GetTitleUiLayout() }
{
   m_statusMessage = Localization::GetText(U"title_ui_editor.status.ready");
}

void TitleUiEditorScene::update()
{
	auto& data = getData();
	if (UpdateSceneTransition(data, [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	handleTopButtonInput();
	handleSelectionInput();
	handleInfoPanelInput();
	handlePreviewToggleInput();
	handleEditorShortcuts();
	handleDragInput();
}

void TitleUiEditorScene::draw() const
{
	drawPreview();
	drawSelectionHighlight();
	drawPanels();
	DrawSceneTransitionOverlay(getData());
}

void TitleUiEditorScene::reloadLayout()
{
	m_layout = TitleUi::ReloadTitleUiLayout();
	m_hasUnsavedChanges = false;
    m_statusMessage = Localization::GetText(U"title_ui_editor.status.reloaded");
}

void TitleUiEditorScene::saveLayout()
{
	if (const auto validationIssue = findValidationIssue())
	{
		m_selectedElementIndex = validationIssue->elementIndex;
		ensureSelectedElementVisible();
		m_statusMessage = validationIssue->message;
		return;
	}

	if (TitleUi::SaveTitleUiLayout(m_layout))
	{
		m_hasUnsavedChanges = false;
     m_statusMessage = Localization::GetText(U"title_ui_editor.status.saved");
	}
	else
	{
        m_statusMessage = Localization::GetText(U"title_ui_editor.status.failed_save");
	}
}

void TitleUiEditorScene::applySelectedRectSizePreset(const Vec2& size, const String& label)
{
	if (RectF* rect = getSelectedRect())
	{
		rect->w = Max(8.0, size.x);
		rect->h = Max(8.0, size.y);
       markEdited(Localization::FormatText(U"title_ui_editor.status.applied_size_preset", label));
	}
}

void TitleUiEditorScene::resetSelectedElement()
{
	const TitleUiLayout defaults = TitleUi::MakeDefaultTitleUiLayout();
	const EditableElement& element = getSelectedElement();
	if (element.rectMember)
	{
		m_layout.*(element.rectMember) = defaults.*(element.rectMember);
	}
	else if (element.pointMember)
	{
		m_layout.*(element.pointMember) = defaults.*(element.pointMember);
	}

   markEdited(Localization::GetText(U"title_ui_editor.status.reset_selected"));
}

void TitleUiEditorScene::resetAllElements()
{
	m_layout = TitleUi::MakeDefaultTitleUiLayout();
  markEdited(Localization::GetText(U"title_ui_editor.status.reset_all"));
}

void TitleUiEditorScene::requestReturnToTitle()
{
	RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}
