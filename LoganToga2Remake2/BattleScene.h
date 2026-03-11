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
	Vec2 m_cameraCenter = Vec2::Zero();
	Optional<Vec2> m_cameraPanStartCursor;
	Vec2 m_cameraPanStartCenter = Vec2::Zero();
	bool m_isCameraPanning = false;
	BattleRenderer m_renderer;
	FixedStepClock m_clock;
	BattleInputController m_inputController;
	BattleConstructionController m_constructionController;

	[[nodiscard]] Vec2 screenToWorld(const Vec2& screenPosition) const;
	[[nodiscard]] Vec2 clampCameraCenter(const Vec2& desiredCenter) const;
	[[nodiscard]] bool updateCameraPan();
	void resetCameraPan();
	[[nodiscard]] static bool isProductionSlotTriggered(int32 slot);
	void handleProductionInput();
};
