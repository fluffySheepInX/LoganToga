# pragma once
# include "SkyAppUiInternal.hpp"

namespace SkyAppSupport
{
	namespace UiParameterEditorDetail
	{
		struct MillParameterEditorSpec
		{
			StringView label;
			StringView suffix;
			double minValue;
			double maxValue;
			double smallStep;
			double mediumStep;
			double largeStep;
			double sliderRoundStep;
			int32 decimals;
		};

		[[nodiscard]] String FormatMillParameterValue(double value, int32 decimals, StringView suffix);
		[[nodiscard]] StringView ToMovementTypeLabel(MainSupport::MovementType movementType);
       [[nodiscard]] StringView ToUnitFootprintTypeLabel(MainSupport::UnitFootprintType footprintType);
		[[nodiscard]] double RoundMillParameterValue(double value, double roundStep);
		void ApplyMillParameterDelta(double& value, const MillParameterEditorSpec& spec, double delta);
		[[nodiscard]] bool DrawMillStepButton(const Rect& rect, StringView label);
		void DrawMillDragSlider(const RectF& trackRect, int32 sliderId, double& value, const MillParameterEditorSpec& spec);
		void DrawMillParameterEditorRow(const Rect& panel, double top, int32 sliderId, double& value, const MillParameterEditorSpec& spec);
       [[nodiscard]] bool DrawMillParameterEditorCard(const Rect& rect, int32 sliderId, double& value, const MillParameterEditorSpec& spec);
      [[nodiscard]] StringView ToUnitEditorSectionLabel(MainSupport::UnitTeam team, MainSupport::SapperUnitType unitType);
       [[nodiscard]] StringView ToUnitEditorPageLabel(MainSupport::UnitEditorPage page);
       [[nodiscard]] int32 ToUnitEditorSliderBase(MainSupport::UnitTeam team, MainSupport::SapperUnitType unitType);
		void ClampUnitParameters(MainSupport::UnitParameters& parameters);
		void DrawMovementTypeSelector(const Rect& panel, double top, MainSupport::MovementType& movementType);
       void DrawFootprintTypeSelector(const Rect& panel, double top, MainSupport::UnitParameters& parameters);
		void ApplyUnitParametersToSpawned(Array<MainSupport::SpawnedSapper>& sappers,
			MainSupport::UnitTeam team,
			MainSupport::SapperUnitType unitType,
			const MainSupport::UnitParameters& parameters);
     void DrawUnitParameterRows(const Rect& panel, int32 sliderBase, MainSupport::UnitParameters& parameters, MainSupport::UnitEditorPage page, int32 top, String& hoveredDescription, Optional<Rect>& hoveredRect);
	}
}
