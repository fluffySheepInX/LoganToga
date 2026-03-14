#pragma once

#include "BattleConfigLoader.h"
#include "BattleConfigPathResolver.h"
#include "ContinueRunSave.h"
#include "GameData.h"
#include "MenuButtonUi.h"

class MapEditScene : public SceneBase
{
private:
	enum class Tool
	{
		Select,
		AddUnit,
		AddObstacle,
		AddResource,
	};

	enum class SelectionKind
	{
		None,
		InitialUnit,
		Obstacle,
		ResourcePoint,
	};

	struct Selection
	{
		SelectionKind kind = SelectionKind::None;
		int32 index = -1;
	};

	struct EditableMapEntry
	{
		String mapConfigPath;
		String label;
	};

public:
	explicit MapEditScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	BattleConfigData m_config;
	String m_battleConfigPath;
	String m_mapConfigPath;
	Array<EditableMapEntry> m_editableMaps;
	int32 m_selectedMapIndex = 0;
	Tool m_tool = Tool::Select;
	Selection m_selection;
	Optional<Vec2> m_dragOffset;
	Owner m_unitPlacementOwner = Owner::Player;
	UnitArchetype m_unitPlacementArchetype = UnitArchetype::Soldier;
	Owner m_resourcePlacementOwner = Owner::Neutral;
	String m_statusMessage = U"Ready";

	[[nodiscard]] static String resolveMapConfigPath(const String& battleConfigPath);
	[[nodiscard]] static Array<EditableMapEntry> loadEditableMaps(const String& battleConfigPath);
	void reloadConfig();
	void saveMap();
	void startTestPlay();
	void selectEditableMap(const int32 mapIndex);
	[[nodiscard]] String getCurrentMapLabel() const;

	[[nodiscard]] static bool isButtonClicked(const RectF& rect)
	{
		return IsMenuButtonClicked(rect);
	}

	[[nodiscard]] static RectF getLeftPanelRect()
	{
		const RectF sceneRect = Scene::Rect();
		return RectF{ sceneRect.x + 12, sceneRect.y + 12, 220, sceneRect.h - 24 };
	}

	[[nodiscard]] static RectF getRightPanelRect()
	{
		const RectF sceneRect = Scene::Rect();
		return RectF{ sceneRect.x + sceneRect.w - 272, sceneRect.y + 12, 260, sceneRect.h - 24 };
	}

	[[nodiscard]] RectF getCanvasRect() const
	{
		const RectF leftPanel = getLeftPanelRect();
		const RectF rightPanel = getRightPanelRect();
		const double x = leftPanel.x + leftPanel.w + 12;
		const double width = rightPanel.x - x - 12;
		return RectF{ x, 12, width, Scene::Rect().h - 24 };
	}

	[[nodiscard]] double getCanvasScale() const
	{
		const RectF canvasRect = getCanvasRect().stretched(-12);
		return Min(canvasRect.w / m_config.world.width, canvasRect.h / m_config.world.height);
	}

	[[nodiscard]] Vec2 getCanvasOrigin() const
	{
		const RectF canvasRect = getCanvasRect().stretched(-12);
		const double scale = getCanvasScale();
		const double drawWidth = m_config.world.width * scale;
		const double drawHeight = m_config.world.height * scale;
		return Vec2{
			canvasRect.x + ((canvasRect.w - drawWidth) * 0.5),
			canvasRect.y + ((canvasRect.h - drawHeight) * 0.5)
		};
	}

	[[nodiscard]] Vec2 worldToScreen(const Vec2& worldPosition) const
	{
		return getCanvasOrigin() + (worldPosition * getCanvasScale());
	}

	[[nodiscard]] Vec2 screenToWorld(const Vec2& screenPosition) const
	{
		return (screenPosition - getCanvasOrigin()) / getCanvasScale();
	}

	[[nodiscard]] RectF worldToScreen(const RectF& worldRect) const
	{
		const Vec2 topLeft = worldToScreen(worldRect.pos);
		const double scale = getCanvasScale();
		return RectF{ topLeft.x, topLeft.y, worldRect.w * scale, worldRect.h * scale };
	}

	void drawCanvas(const RectF& canvasRect) const;
	void drawLeftPanel(const RectF& panelRect) const;
	void drawRightPanel(const RectF& panelRect) const;

	[[nodiscard]] static RectF getPanelButtonRect(const RectF& panelRect, const int32 index)
	{
		return RectF{ panelRect.x + 16, panelRect.y + 140 + (index * 42), panelRect.w - 32, 32 };
	}

	static void drawButton(const RectF& rect, const String& label, const Font& font, const bool selected = false);

	[[nodiscard]] bool handleLeftPanelInput();
	[[nodiscard]] bool handleRightPanelInput();
	void handleCanvasInput();
	void beginDragging(const Vec2& cursorWorld);
	void updateDragging(const Vec2& cursorWorld);
	[[nodiscard]] Selection hitTest(const Vec2& cursorWorld) const;
	void deleteSelection();

	template <class T>
	[[nodiscard]] static bool isValidIndex(const Array<T>& values, const int32 index)
	{
		return (0 <= index) && (static_cast<size_t>(index) < values.size());
	}

	[[nodiscard]] InitialUnitPlacement* getSelectedUnit();
	[[nodiscard]] const InitialUnitPlacement* getSelectedUnit() const;
	[[nodiscard]] ObstacleConfig* getSelectedObstacle();
	[[nodiscard]] const ObstacleConfig* getSelectedObstacle() const;
	[[nodiscard]] ResourcePointConfig* getSelectedResourcePoint();
	[[nodiscard]] const ResourcePointConfig* getSelectedResourcePoint() const;

	[[nodiscard]] Vec2 clampWorldPosition(const Vec2& worldPosition) const
	{
		return Vec2{
			Clamp(worldPosition.x, 0.0, m_config.world.width),
			Clamp(worldPosition.y, 0.0, m_config.world.height)
		};
	}

	void clampObstacle(ObstacleConfig& obstacle) const;

	[[nodiscard]] static Owner cycleUnitPlacementOwner(const Owner owner);
	[[nodiscard]] static Owner cycleResourceOwner(const Owner owner);
	[[nodiscard]] static UnitArchetype cycleUnitArchetype(const UnitArchetype archetype);
	[[nodiscard]] static ColorF getOwnerColor(const Owner owner);
	[[nodiscard]] static String toOwnerDisplayString(const Owner owner);
	[[nodiscard]] static String toOwnerTomlString(const Owner owner);
	[[nodiscard]] static String toUnitArchetypeDisplayString(const UnitArchetype archetype);
	[[nodiscard]] static String toUnitArchetypeTomlString(const UnitArchetype archetype);
	[[nodiscard]] static String toUnitArchetypeShortString(const UnitArchetype archetype);
};
