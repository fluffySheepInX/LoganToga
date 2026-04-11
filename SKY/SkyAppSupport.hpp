# pragma once
# include <array>
# include "MainContext.hpp"
# include "MapData.hpp"
# include "SkyAppUiLayout.hpp"

class UnitModel;

namespace SkyAppSupport
{
 struct UnitRenderModelRegistryView
	{
       std::array<const UnitModel*, MainSupport::UnitRenderModelCount> models{};
	};

	struct TimedMessage
	{
		String text;
		double until = 0.0;

      void show(StringView message, double durationSeconds = 2.0);
		[[nodiscard]] bool isVisible() const;
	};

	struct SkyAppPanels
	{
      Rect miniMap;
		Rect skySettings;
		Rect cameraSettings;
		Rect mapEditor;
		Rect blacksmithMenu;
		Rect sapperMenu;
		Rect millStatusEditor;
		Rect modelHeight;
     Rect unitEditor;
     Rect unitEditorList;
		Rect resourcePanel;
      Rect escMenu;
		Rect uiToggle;
		Rect mapModeToggle;
		Rect modelHeightModeToggle;
        Rect unitEditorModeToggle;
        Rect skySettingsToggle;
		Rect cameraSettingsToggle;
        Rect uiEditModeToggle;
        Rect resourceAdjustToggle;
        Rect enemyPlanToggle;
		Rect timeSlider;

      SkyAppPanels(const MainSupport::UiLayoutSettings& uiLayoutSettings = MainSupport::UiLayoutSettings{}, bool skySettingsExpanded = true, bool cameraSettingsExpanded = true, bool miniMapExpanded = true, bool resourceAdjustExpanded = false);

       [[nodiscard]] bool isHoveringUi(bool showUI, bool showSkySettings, bool showCameraSettings, bool isEditorMode, bool showBlacksmithMenu, bool showSapperMenu, bool showMillStatusEditor, bool modelHeightEditMode, bool showUnitEditor) const;
	};

	[[nodiscard]] bool IsSapperRetreatOrdered(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] bool IsSapperRetreatedHidden(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] bool IsSpawnedSapperSelectable(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] bool IsSpawnedSapperCombatActive(const MainSupport::SpawnedSapper& sapper);
	void OrderSapperRetreat(MainSupport::SpawnedSapper& sapper, const Vec3& rallyPoint);

	[[nodiscard]] Vec3 GetSpawnedSapperBasePosition(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetSpawnedSapperBounceOffset(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] Vec3 GetSpawnedSapperRenderPosition(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] double GetSpawnedSapperYaw(const MainSupport::SpawnedSapper& sapper);
   [[nodiscard]] bool IsSapperSuppressed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperMoveSpeed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackDamage(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackInterval(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] ColorF GetSpawnedSapperTint(const MainSupport::SpawnedSapper& sapper, const ColorF& baseColor);
    void ApplyUnitParameters(MainSupport::SpawnedSapper& sapper, const MainSupport::UnitParameters& parameters);
    void ApplySapperSuppression(MainSupport::SpawnedSapper& sapper, double durationSeconds, double moveSpeedMultiplier, double attackDamageMultiplier, double attackIntervalMultiplier);
	void UpdateSpawnedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers, const MapData& mapData, const MainSupport::ModelHeightSettings& modelHeightSettings);
    void ResolveSapperSpacingAgainstUnits(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Array<MainSupport::SpawnedSapper>& enemySappers, const MainSupport::ModelHeightSettings& modelHeightSettings);
	void ResolveSapperSpacingAgainstBase(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& enemyBasePosition, const MainSupport::ModelHeightSettings& modelHeightSettings);
    void ResolveSapperSpacingAgainstObstacles(Array<MainSupport::SpawnedSapper>& spawnedSappers, const MapData& mapData, const MainSupport::ModelHeightSettings& modelHeightSettings);
	void SetSpawnedSapperTarget(MainSupport::SpawnedSapper& sapper, const Vec3& targetPosition, const MapData& mapData, const MainSupport::ModelHeightSettings& modelHeightSettings);
	void UpdateAutoCombat(Array<MainSupport::SpawnedSapper>& attackers, Array<MainSupport::SpawnedSapper>& defenders, const MainSupport::ModelHeightSettings& modelHeightSettings);
	void RemoveDefeatedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers);
 [[nodiscard]] Optional<size_t> HitTestSpawnedSapper(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const MainSupport::AppCamera3D& camera, const MainSupport::ModelHeightSettings& modelHeightSettings);
  void DrawSelectedSapperAttackRange(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
  void DrawSelectedSapperFootprint(const MainSupport::SpawnedSapper& sapper, const ColorF& color = ColorF{ 0.96, 0.98, 1.0, 0.78 });
	void DrawUnitFootprintPreview(const Vec3& position, double yaw, const MainSupport::UnitParameters& parameters, const ColorF& color = ColorF{ 0.90, 0.96, 1.0, 0.72 });
	void DrawSelectedSapperRing(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
	void DrawSelectedSapperIcon(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
    void DrawSapperHealthBars(const MainSupport::AppCamera3D& camera, const Array<MainSupport::SpawnedSapper>& spawnedSappers, const ColorF& fillColor);
      void UpdateCameraWheelZoom(MainSupport::AppCamera3D& camera, MainSupport::CameraSettings& cameraSettings, const Vec3& playerBasePosition);
		   void DrawSpawnedSappers(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const UnitRenderModelRegistryView& renderModels, const MainSupport::ModelHeightSettings& modelHeightSettings, const ColorF& color);
	void UpdateSkyFromTime(Sky& sky, double skyTime);
  void SpawnSapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, const MapData& mapData, MainSupport::SapperUnitType unitType = MainSupport::SapperUnitType::Infantry);
  void SpawnEnemySapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& position, double facingYaw = MainSupport::BirdDisplayYaw, MainSupport::SapperUnitType unitType = MainSupport::SapperUnitType::Infantry);
}
