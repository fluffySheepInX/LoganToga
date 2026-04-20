# include "MainSettingsInternal.hpp"

namespace MainSupport
{
   namespace
	{
		struct AnimationRoleEntry
		{
			UnitModelAnimationRole role;
			StringView suffix;
		};

		inline constexpr std::array<AnimationRoleEntry, 3> AnimationRoles{ {
			{ UnitModelAnimationRole::Idle,   U"IdleAnimationClip" },
			{ UnitModelAnimationRole::Move,   U"MoveAnimationClip" },
			{ UnitModelAnimationRole::Attack, U"AttackAnimationClip" },
		} };

		struct TireTrackParam
		{
			StringView suffix;
			double minValue;
			double maxValue;
			double& (*ref)(ModelHeightSettings&, TireTrackTextureSegment);
			double  (*get)(const ModelHeightSettings&, TireTrackTextureSegment);
		};

		inline const std::array<TireTrackParam, 4> TireTrackParams{ {
			{ U"YOffset",  TireTrackYOffsetMin,  TireTrackYOffsetMax,
				static_cast<double& (*)(ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackYOffset),
				static_cast<double  (*)(const ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackYOffset) },
			{ U"Opacity",  TireTrackOpacityMin,  TireTrackOpacityMax,
				static_cast<double& (*)(ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackOpacity),
				static_cast<double  (*)(const ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackOpacity) },
			{ U"Softness", TireTrackSoftnessMin, TireTrackSoftnessMax,
				static_cast<double& (*)(ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackSoftness),
				static_cast<double  (*)(const ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackSoftness) },
			{ U"Warmth",   TireTrackWarmthMin,   TireTrackWarmthMax,
				static_cast<double& (*)(ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackWarmth),
				static_cast<double  (*)(const ModelHeightSettings&, TireTrackTextureSegment)>(&GetTireTrackWarmth) },
		} };

		[[nodiscard]] String EscapeTomlKey(StringView value)
		{
			return String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
		}

		void TryLoadQuotedModelHeightSettingLine(ModelHeightSettings& settings, const String& rawLine)
		{
			const String line = rawLine.trimmed();
			if (line.isEmpty() || (not line.starts_with(U'"'))) { return; }

			const Array<String> parts = line.split(U'=');
			if (parts.size() < 2) { return; }

			String key = parts[0].trimmed();
			if ((key.size() < 2) || (key.front() != U'"') || (key.back() != U'"')) { return; }

			key = key.substr(1, (key.size() - 2));
			if (not key.starts_with(U"Model:")) { return; }

			const size_t suffixSeparatorIndex = key.lastIndexOf(U':');
			if ((suffixSeparatorIndex == 0) || (suffixSeparatorIndex == String::npos)) { return; }

			const String modelKey = key.substr(6, (suffixSeparatorIndex - 6));
			const String suffix = key.substr(suffixSeparatorIndex + 1);
			const String valueText = parts[1].trimmed();
			auto& fileSettings = settings.fileSettings[modelKey];

			try
			{
				if (suffix == U"OffsetY")
				{
					fileSettings.offsetY = Clamp(Parse<double>(valueText), ModelHeightOffsetMin, ModelHeightOffsetMax);
					return;
				}
				if (suffix == U"Scale")
				{
					fileSettings.scale = Clamp(Parse<double>(valueText), ModelScaleMin, ModelScaleMax);
					return;
				}
				for (const auto& entry : AnimationRoles)
				{
					if (suffix == entry.suffix)
					{
						fileSettings.animationClip[GetUnitModelAnimationRoleIndex(entry.role)] = Max(Parse<int32>(valueText), -1);
						return;
					}
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

		if (not toml) { return{}; }

		ModelHeightSettings settings;

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);

			if (const auto value = toml[(key + U"OffsetY")].getOpt<double>())
			{
				GetModelHeightOffset(settings, renderModel) = *value;
			}

			if (const auto value = toml[(key + U"Scale")].getOpt<double>())
			{
				GetModelScale(settings, renderModel) = Clamp(*value, ModelScaleMin, ModelScaleMax);
			}

			bool loadedAny = false;
			for (const auto& entry : AnimationRoles)
			{
				if (const auto value = toml[(key + entry.suffix)].getOpt<int32>())
				{
					GetModelAnimationClipIndex(settings, renderModel, entry.role) = Max(*value, -1);
					loadedAny = true;
				}
			}

			if (not loadedAny)
			{
				if (const auto value = toml[(key + U"AnimationClip")].getOpt<int32>())
				{
					const int32 legacyClipIndex = Max(*value, -1);
					for (const auto& entry : AnimationRoles)
					{
						GetModelAnimationClipIndex(settings, renderModel, entry.role) = legacyClipIndex;
					}
				}
			}
		}

		for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
		{
			const StringView segmentLabel = GetTireTrackTextureSegmentLabel(segment);
			for (const auto& param : TireTrackParams)
			{
				if (const auto value = toml[U"tireTrack{}{}"_fmt(segmentLabel, param.suffix)].getOpt<double>())
				{
					param.ref(settings, segment) = Clamp(*value, param.minValue, param.maxValue);
				}
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

			for (const auto& entry : AnimationRoles)
			{
				if (const auto value = toml[BuildModelHeightTomlKey(modelPath, entry.suffix)].getOpt<int32>())
				{
					GetModelAnimationClipIndex(settings, modelPath, entry.role) = Max(*value, -1);
				}
			}
		}

		LoadQuotedModelHeightSettingsFromText(settings);

		return settings;
	}

	bool SaveModelHeightSettings(const ModelHeightSettings& settings)
	{
		FileSystem::CreateDirectories(FileSystem::ParentPath(ModelHeightSettingsPath));

		TextWriter writer{ ModelHeightSettingsPath };

		if (not writer) { return false; }

		for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
			const StringView key = GetUnitRenderModelLabel(renderModel);
			writer.writeln(U"{}OffsetY = {:.3f}"_fmt(key, GetModelHeightOffset(settings, renderModel)));
			writer.writeln(U"{}Scale = {:.3f}"_fmt(key, Clamp(GetModelScale(settings, renderModel), ModelScaleMin, ModelScaleMax)));
			for (const auto& entry : AnimationRoles)
			{
				writer.writeln(U"{}{} = {}"_fmt(key, entry.suffix, Max(GetModelAnimationClipIndex(settings, renderModel, entry.role), -1)));
			}
		}

		for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
		{
			const StringView segmentLabel = GetTireTrackTextureSegmentLabel(segment);
			for (const auto& param : TireTrackParams)
			{
				writer.writeln(U"tireTrack{}{} = {:.3f}"_fmt(segmentLabel, param.suffix, Clamp(param.get(settings, segment), param.minValue, param.maxValue)));
			}
		}

		for (const auto& [modelKey, fileSettings] : settings.fileSettings)
		{
			writer.writeln(U"\"{}\" = {:.3f}"_fmt(EscapeTomlKey(U"Model:{}:OffsetY"_fmt(modelKey)), Clamp(fileSettings.offsetY, ModelHeightOffsetMin, ModelHeightOffsetMax)));
			writer.writeln(U"\"{}\" = {:.3f}"_fmt(EscapeTomlKey(U"Model:{}:Scale"_fmt(modelKey)), Clamp(fileSettings.scale, ModelScaleMin, ModelScaleMax)));
			for (const auto& entry : AnimationRoles)
			{
				writer.writeln(U"\"{}\" = {}"_fmt(EscapeTomlKey(U"Model:{}:{}"_fmt(modelKey, entry.suffix)), Max(fileSettings.animationClip[GetUnitModelAnimationRoleIndex(entry.role)], -1)));
			}
		}
		return true;
	}
}
