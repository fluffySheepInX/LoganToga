#include "TitleUiEditorScene.h"

TitleUiEditorScene::TitleUiEditorScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_layout{ TitleUi::GetTitleUiLayout() }
{
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
	m_statusMessage = U"Reloaded title UI layout from disk";
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
		m_statusMessage = U"Saved title UI layout";
	}
	else
	{
		m_statusMessage = U"Failed to save title UI layout";
	}
}

void TitleUiEditorScene::applySelectedRectSizePreset(const Vec2& size, const String& label)
{
	if (RectF* rect = getSelectedRect())
	{
		rect->w = Max(8.0, size.x);
		rect->h = Max(8.0, size.y);
		markEdited(U"Applied size preset: " + label);
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

	markEdited(U"Reset selected element to default");
}

void TitleUiEditorScene::resetAllElements()
{
	m_layout = TitleUi::MakeDefaultTitleUiLayout();
	markEdited(U"Reset all title UI elements to default");
}

void TitleUiEditorScene::requestReturnToTitle()
{
	RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}
