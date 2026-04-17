# include "MainSettingsInternal.hpp"

namespace MainSupport
{
	ModelHeightSettings LoadModelHeightSettings()
	{
		const TOMLReader toml{ ModelHeightSettingsPath };

		if (not toml)
		{
			return{};
		}

		ModelHeightSettings settings;

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);
           const auto applyClipValue = [&](const String& clipKey, const UnitModelAnimationRole role)
				{
					if (const auto value = toml[clipKey].getOpt<int32>())
					{
                       GetModelAnimationClipIndex(settings, renderModel, role) = Max(*value, -1);
						return true;
					}

					return false;
				};

			if (const auto value = toml[(key + U"OffsetY")].getOpt<double>())
			{
				GetModelHeightOffset(settings, renderModel) = *value;
			}

			if (const auto value = toml[(key + U"Scale")].getOpt<double>())
			{
				GetModelScale(settings, renderModel) = Clamp(*value, ModelScaleMin, ModelScaleMax);
			}

          const bool loadedIdle = applyClipValue((key + U"IdleAnimationClip"), UnitModelAnimationRole::Idle);
			const bool loadedMove = applyClipValue((key + U"MoveAnimationClip"), UnitModelAnimationRole::Move);
			const bool loadedAttack = applyClipValue((key + U"AttackAnimationClip"), UnitModelAnimationRole::Attack);

			if ((not loadedIdle) && (not loadedMove) && (not loadedAttack))
			{
             if (const auto value = toml[(key + U"AnimationClip")].getOpt<int32>())
				{
                   const int32 legacyClipIndex = Max(*value, -1);
					GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Idle) = legacyClipIndex;
					GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Move) = legacyClipIndex;
					GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Attack) = legacyClipIndex;
				}
			}
		}

		for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
		{
			const String key = U"tireTrack{}YOffset"_fmt(GetTireTrackTextureSegmentLabel(segment));
			if (const auto value = toml[key].getOpt<double>())
			{
				GetTireTrackYOffset(settings, segment) = Clamp(*value, TireTrackYOffsetMin, TireTrackYOffsetMax);
			}

			if (const auto value = toml[U"tireTrack{}Opacity"_fmt(GetTireTrackTextureSegmentLabel(segment))].getOpt<double>())
			{
				GetTireTrackOpacity(settings, segment) = Clamp(*value, TireTrackOpacityMin, TireTrackOpacityMax);
			}

			if (const auto value = toml[U"tireTrack{}Softness"_fmt(GetTireTrackTextureSegmentLabel(segment))].getOpt<double>())
			{
				GetTireTrackSoftness(settings, segment) = Clamp(*value, TireTrackSoftnessMin, TireTrackSoftnessMax);
			}

			if (const auto value = toml[U"tireTrack{}Warmth"_fmt(GetTireTrackTextureSegmentLabel(segment))].getOpt<double>())
			{
				GetTireTrackWarmth(settings, segment) = Clamp(*value, TireTrackWarmthMin, TireTrackWarmthMax);
			}
		}

		return settings;
	}

	bool SaveModelHeightSettings(const ModelHeightSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ ModelHeightSettingsPath };

		if (not writer)
		{
			return false;
		}

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);
			writer.writeln(U"{}OffsetY = {:.3f}"_fmt(key, GetModelHeightOffset(settings, renderModel)));
			writer.writeln(U"{}Scale = {:.3f}"_fmt(key, Clamp(GetModelScale(settings, renderModel), ModelScaleMin, ModelScaleMax)));
         writer.writeln(U"{}IdleAnimationClip = {}"_fmt(key, Max(GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Idle), -1)));
			writer.writeln(U"{}MoveAnimationClip = {}"_fmt(key, Max(GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Move), -1)));
			writer.writeln(U"{}AttackAnimationClip = {}"_fmt(key, Max(GetModelAnimationClipIndex(settings, renderModel, UnitModelAnimationRole::Attack), -1)));
		}

		for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
		{
			writer.writeln(U"tireTrack{}YOffset = {:.3f}"_fmt(GetTireTrackTextureSegmentLabel(segment), Clamp(GetTireTrackYOffset(settings, segment), TireTrackYOffsetMin, TireTrackYOffsetMax)));
           writer.writeln(U"tireTrack{}Opacity = {:.3f}"_fmt(GetTireTrackTextureSegmentLabel(segment), Clamp(GetTireTrackOpacity(settings, segment), TireTrackOpacityMin, TireTrackOpacityMax)));
			writer.writeln(U"tireTrack{}Softness = {:.3f}"_fmt(GetTireTrackTextureSegmentLabel(segment), Clamp(GetTireTrackSoftness(settings, segment), TireTrackSoftnessMin, TireTrackSoftnessMax)));
			writer.writeln(U"tireTrack{}Warmth = {:.3f}"_fmt(GetTireTrackTextureSegmentLabel(segment), Clamp(GetTireTrackWarmth(settings, segment), TireTrackWarmthMin, TireTrackWarmthMax)));
		}
		return true;
	}
}
