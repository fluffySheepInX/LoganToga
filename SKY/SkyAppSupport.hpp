# pragma once
# include "MainContext.hpp"
# include "MapData.hpp"
# include "SkyAppUiLayout.hpp"

class BirdModel;

namespace SkyAppSupport
{
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
		Rect resourcePanel;
      Rect escMenu;
		Rect uiToggle;
		Rect mapModeToggle;
		Rect modelHeightModeToggle;
		Rect timeSlider;

     SkyAppPanels(bool skySettingsExpanded = true, bool cameraSettingsExpanded = true, bool miniMapExpanded = true);

      [[nodiscard]] bool isHoveringUi(bool showUI, bool isEditorMode, bool showBlacksmithMenu, bool showSapperMenu, bool showMillStatusEditor, bool modelHeightEditMode) const;
	};

	[[nodiscard]] Vec3 GetSpawnedSapperBasePosition(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetSpawnedSapperBounceOffset(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] Vec3 GetSpawnedSapperRenderPosition(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] double GetSpawnedSapperYaw(const MainSupport::SpawnedSapper& sapper);
   [[nodiscard]] bool IsSapperSuppressed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperMoveSpeed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackDamage(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackInterval(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] ColorF GetSpawnedSapperTint(const MainSupport::SpawnedSapper& sapper, const ColorF& baseColor);
	void ApplySapperSuppression(MainSupport::SpawnedSapper& sapper, double durationSeconds, double moveSpeedMultiplier, double attackDamageMultiplier, double attackIntervalMultiplier);
   void UpdateSpawnedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers, const MapData& mapData);
    void ResolveSapperSpacingAgainstUnits(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Array<MainSupport::SpawnedSapper>& enemySappers);
	void ResolveSapperSpacingAgainstBase(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& enemyBasePosition);
   void ResolveSapperSpacingAgainstObstacles(Array<MainSupport::SpawnedSapper>& spawnedSappers, const MapData& mapData);
	void SetSpawnedSapperTarget(MainSupport::SpawnedSapper& sapper, const Vec3& targetPosition, const MapData& mapData);
  void UpdateAutoCombat(Array<MainSupport::SpawnedSapper>& attackers, Array<MainSupport::SpawnedSapper>& defenders);
	void RemoveDefeatedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers);
  [[nodiscard]] Optional<size_t> HitTestSpawnedSapper(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const MainSupport::AppCamera3D& camera);
	void DrawSelectedSapperRing(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
	void DrawSelectedSapperIcon(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
	 void DrawSapperHealthBars(const MainSupport::AppCamera3D& camera, const Array<MainSupport::SpawnedSapper>& spawnedSappers, const ColorF& fillColor);
	 void UpdateCameraWheelZoom(MainSupport::AppCamera3D& camera, MainSupport::CameraSettings& cameraSettings, const Vec3& playerBasePosition);
 void DrawSpawnedSappers(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const BirdModel& sapperModel, const ColorF& color);
	void UpdateSkyFromTime(Sky& sky, double skyTime);
  void SpawnSapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, const MapData& mapData, MainSupport::SapperUnitType unitType = MainSupport::SapperUnitType::Infantry);
   void SpawnEnemySapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& position, double facingYaw = MainSupport::BirdDisplayYaw);
}
