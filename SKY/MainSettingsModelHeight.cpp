# include "MainSettingsInternal.hpp"

namespace MainSupport
{
   namespace
	{
		[[nodiscard]] String EscapeTomlKey(StringView value)
		{
			return String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
		}

			void TryLoadQuotedModelHeightSettingLine(ModelHeightSettings& settings, const String& rawLine)
			{
				const String line = rawLine.trimmed();
				if (line.isEmpty() || (not line.starts_with(U'"')))
				{
					return;
				}

               const Array<String> parts = line.split(U'=');
				if (parts.size() < 2)
				{
					return;
				}

				String key = parts[0].trimmed();
				if ((key.size() < 2) || (key.front() != U'"') || (key.back() != U'"'))
				{
					return;
				}

				key = key.substr(1, (key.size() - 2));
				if (not key.starts_with(U"Model:"))
				{
					return;
				}

				const size_t suffixSeparatorIndex = key.lastIndexOf(U':');
				if ((suffixSeparatorIndex == 0) || (suffixSeparatorIndex == String::npos))
				{
					return;
				}

				const String modelKey = key.substr(6, (suffixSeparatorIndex - 6));
				const String suffix = key.substr(suffixSeparatorIndex + 1);
				const String valueText = parts[1].trimmed();
				auto& fileSettings = settings.fileSettings[modelKey];

				try
				{
					if (suffix == U"OffsetY")
					{
						fileSettings.offsetY = Clamp(Parse<double>(valueText), ModelHeightOffsetMin, ModelHeightOffsetMax);
					}
					else if (suffix == U"Scale")
					{
						fileSettings.scale = Clamp(Parse<double>(valueText), ModelScaleMin, ModelScaleMax);
					}
					else if (suffix == U"IdleAnimationClip")
					{
						fileSettings.idleAnimationClip = Max(Parse<int32>(valueText), -1);
					}
					else if (suffix == U"MoveAnimationClip")
					{
						fileSettings.moveAnimationClip = Max(Parse<int32>(valueText), -1);
					}
					else if (suffix == U"AttackAnimationClip")
					{
						fileSettings.attackAnimationClip = Max(Parse<int32>(valueText), -1);
					}
				}
				catch (const std::exception&)
				{
				}
			}

			void LoadQuotedModelHeightSettingsFromText(ModelHeightSettings& settings)
			{
				TextReader reader{ ModelHeightSettingsPath };
				if (not reader)
				{
					return;
				}

				String line;
				while (reader.readLine(line))
				{
					TryLoadQuotedModelHeightSettingLine(settings, line);
				}
			}

		void AppendUniqueModelHeightSettingTargetPath(Array<FilePath>& paths, const FilePath& path)
		{
			if (path.isEmpty())
			{
				return;
			}

			const FilePath normalizedPath = FileSystem::FullPath(path);
			if (std::find(paths.begin(), paths.end(), normalizedPath) == paths.end())
			{
				paths << normalizedPath;
			}
		}

		[[nodiscard]] Array<FilePath> DiscoverModelHeightSettingTargetPaths()
		{
			Array<FilePath> paths;
			for (const FilePath& directory : { FilePath{ U"model" }, FilePath{ U"App/model" } })
			{
				if (not FileSystem::IsDirectory(directory))
				{
					continue;
				}

				for (const auto& path : FileSystem::DirectoryContents(directory))
				{
					if (FileSystem::IsFile(path) && FileSystem::FileName(path).lowercased().ends_with(U".glb"))
					{
                       AppendUniqueModelHeightSettingTargetPath(paths, path);
					}
				}
			}

			const UnitEditorSettings unitEditorSettings = LoadUnitEditorSettings();
			for (const auto& modelPath : unitEditorSettings.modelPaths)
			{
				AppendUniqueModelHeightSettingTargetPath(paths, modelPath);
			}

			std::sort(paths.begin(), paths.end(), [](const FilePath& a, const FilePath& b)
				{
					return FileSystem::FileName(a) < FileSystem::FileName(b);
				});

			return paths;
		}

		[[nodiscard]] String BuildModelHeightTomlKey(FilePathView modelPath, StringView suffix)
		{
			return U"Model:{}:{}"_fmt(MakeModelHeightFileKey(modelPath), suffix);
		}
	}

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

		for (const auto& modelPath : DiscoverModelHeightSettingTargetPaths())
		{
			if (const auto value = toml[BuildModelHeightTomlKey(modelPath, U"OffsetY")].getOpt<double>())
			{
				GetModelHeightOffset(settings, modelPath) = *value;
			}

			if (const auto value = toml[BuildModelHeightTomlKey(modelPath, U"Scale")].getOpt<double>())
			{
				GetModelScale(settings, modelPath) = Clamp(*value, ModelScaleMin, ModelScaleMax);
			}

			if (const auto value = toml[BuildModelHeightTomlKey(modelPath, U"IdleAnimationClip")].getOpt<int32>())
			{
				GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Idle) = Max(*value, -1);
			}

			if (const auto value = toml[BuildModelHeightTomlKey(modelPath, U"MoveAnimationClip")].getOpt<int32>())
			{
				GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Move) = Max(*value, -1);
			}

			if (const auto value = toml[BuildModelHeightTomlKey(modelPath, U"AttackAnimationClip")].getOpt<int32>())
			{
				GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Attack) = Max(*value, -1);
			}
		}

			LoadQuotedModelHeightSettingsFromText(settings);

		return settings;
	}

	bool SaveModelHeightSettings(const ModelHeightSettings& settings)
	{
     FileSystem::CreateDirectories(FileSystem::ParentPath(ModelHeightSettingsPath));

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

		for (const auto& [modelKey, fileSettings] : settings.fileSettings)
		{
			writer.writeln(U"\"{}\" = {:.3f}"_fmt(EscapeTomlKey(U"Model:{}:OffsetY"_fmt(modelKey)), Clamp(fileSettings.offsetY, ModelHeightOffsetMin, ModelHeightOffsetMax)));
			writer.writeln(U"\"{}\" = {:.3f}"_fmt(EscapeTomlKey(U"Model:{}:Scale"_fmt(modelKey)), Clamp(fileSettings.scale, ModelScaleMin, ModelScaleMax)));
			writer.writeln(U"\"{}\" = {}"_fmt(EscapeTomlKey(U"Model:{}:IdleAnimationClip"_fmt(modelKey)), Max(fileSettings.idleAnimationClip, -1)));
			writer.writeln(U"\"{}\" = {}"_fmt(EscapeTomlKey(U"Model:{}:MoveAnimationClip"_fmt(modelKey)), Max(fileSettings.moveAnimationClip, -1)));
			writer.writeln(U"\"{}\" = {}"_fmt(EscapeTomlKey(U"Model:{}:AttackAnimationClip"_fmt(modelKey)), Max(fileSettings.attackAnimationClip, -1)));
		}
		return true;
	}
}
