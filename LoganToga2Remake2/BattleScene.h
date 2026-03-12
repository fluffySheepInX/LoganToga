#pragma once

#include "BattleConstructionController.h"
#include "BattleInputController.h"
#include "BattleRenderer.h"
#include "BattleSession.h"
#include "FixedStepClock.h"
#include "GameData.h"

class BattleScene : public SceneBase
{
public:
	explicit BattleScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	BattleSession m_session;
	Camera2D m_camera{ Vec2::Zero(), 1.0, (CameraControl::WASDKeys | CameraControl::UpDownKeys | CameraControl::Wheel) };
	Optional<Vec2> m_cameraPanStartCursor;
	Vec2 m_cameraPanStartCenter = Vec2::Zero();
	bool m_isCameraPanning = false;
	BattleRenderer m_renderer;
	FixedStepClock m_clock;
	BattleInputController m_inputController;
	BattleConstructionController m_constructionController;
	bool m_isPaused = false;
	int32 m_pauseMenuIndex = 0;

	[[nodiscard]] Vec2 screenToWorld(const Vec2& screenPosition) const;
	[[nodiscard]] Vec2 clampCameraCenter(const Vec2& desiredCenter) const;
	[[nodiscard]] bool updateCameraPan();
	void resetCameraPan();
	void clearTransientInputState();
	void togglePause();
	void updatePauseMenu();
	void drawPauseMenu() const;
	[[nodiscard]] static bool isCommandSlotTriggered(int32 slot);
	void handleProductionInput();
};
