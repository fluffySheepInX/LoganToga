#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	inline constexpr double TitleUnderBarHeight = 30.0;
	inline constexpr double TitleEditorHotspotSize = 96.0;
	inline constexpr double TitleEditorPanelX = 6.0;
	inline constexpr double TitleEditorPanelY = 6.0;
	inline constexpr double TitleOuterFrameWidth = 96.0;
	inline constexpr double TitleUiDefaultOffset = 24.0;
	inline constexpr double TitleUiEditorMinButtonWidth = 120.0;

	struct TitleUiLayout
	{
		int32 gridSize = 24;
		RectF skirmishButtonRect{
			Scene::Width() - TitleOuterFrameWidth - 220.0 - TitleUiDefaultOffset,
			120.0,
			220.0,
			44.0,
		};
		RectF musicEditorToggleRect{
			Scene::Width() - TitleOuterFrameWidth - 220.0 - TitleUiDefaultOffset,
			120.0 + 44.0 + TitleUiDefaultOffset,
			180.0,
			36.0,
		};
	};

	inline FilePath ResolveTitleUiLayoutTomlPath()
	{
		const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoUI/TitleUiLayout.toml";
		if (FileSystem::Exists(fromApp))
		{
			return fromApp;
		}

		const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoUI/TitleUiLayout.toml";
		if (FileSystem::Exists(fromRepo))
		{
			return fromRepo;
		}

		return fromApp;
	}

	inline TitleUiLayout CreateDefaultTitleUiLayout()
	{
		return {};
	}

	inline Vec2 SnapTitleUiPosition(const Vec2& pos, int32 gridSize)
	{
		const int32 safeGrid = Max(8, gridSize);
		return Vec2{
			Math::Round(pos.x / safeGrid) * safeGrid,
			Math::Round(pos.y / safeGrid) * safeGrid,
		};
	}

	inline double SnapTitleUiScalar(const double value, int32 gridSize)
	{
		const int32 safeGrid = Max(8, gridSize);
		return Math::Round(value / safeGrid) * safeGrid;
	}

	inline void RepairTitleUiLayout(TitleUiLayout& layout)
	{
		const TitleUiLayout defaults = CreateDefaultTitleUiLayout();
		layout.gridSize = Clamp(layout.gridSize, 8, 160);

		layout.skirmishButtonRect.w = Max(TitleUiEditorMinButtonWidth, layout.skirmishButtonRect.w);
		layout.skirmishButtonRect.h = defaults.skirmishButtonRect.h;
		layout.musicEditorToggleRect.w = Max(TitleUiEditorMinButtonWidth, layout.musicEditorToggleRect.w);
		layout.musicEditorToggleRect.h = defaults.musicEditorToggleRect.h;

		layout.skirmishButtonRect.x = Clamp(layout.skirmishButtonRect.x, 0.0, Max(0.0, Scene::Width() - layout.skirmishButtonRect.w));
		layout.skirmishButtonRect.y = Clamp(layout.skirmishButtonRect.y, 0.0, Max(0.0, Scene::Height() - TitleUnderBarHeight - layout.skirmishButtonRect.h));
		layout.musicEditorToggleRect.x = Clamp(layout.musicEditorToggleRect.x, 0.0, Max(0.0, Scene::Width() - layout.musicEditorToggleRect.w));
		layout.musicEditorToggleRect.y = Clamp(layout.musicEditorToggleRect.y, 0.0, Max(0.0, Scene::Height() - TitleUnderBarHeight - layout.musicEditorToggleRect.h));
	}

	inline bool SaveTitleUiLayoutToml(const TitleUiLayout& sourceLayout)
	{
		TitleUiLayout layout = sourceLayout;
		RepairTitleUiLayout(layout);
		const FilePath path = ResolveTitleUiLayoutTomlPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path };
		if (!writer)
		{
			return false;
		}

		String tomlText;
		tomlText += U"[layout]\n";
		tomlText += U"grid = {}\n\n"_fmt(layout.gridSize);
		tomlText += U"[skirmish_button]\n";
		tomlText += U"x = {}\n"_fmt(layout.skirmishButtonRect.x);
		tomlText += U"y = {}\n"_fmt(layout.skirmishButtonRect.y);
		tomlText += U"w = {}\n"_fmt(layout.skirmishButtonRect.w);
		tomlText += U"h = {}\n\n"_fmt(layout.skirmishButtonRect.h);
		tomlText += U"[music_editor_toggle]\n";
		tomlText += U"x = {}\n"_fmt(layout.musicEditorToggleRect.x);
		tomlText += U"y = {}\n"_fmt(layout.musicEditorToggleRect.y);
		tomlText += U"w = {}\n"_fmt(layout.musicEditorToggleRect.w);
		tomlText += U"h = {}\n"_fmt(layout.musicEditorToggleRect.h);
		writer.write(tomlText);
		return true;
	}

	inline bool LoadTitleUiLayoutToml(TitleUiLayout& layout)
	{
		layout = CreateDefaultTitleUiLayout();
		const TOMLReader toml{ ResolveTitleUiLayoutTomlPath() };
		if (!toml)
		{
			RepairTitleUiLayout(layout);
			return false;
		}

		layout.gridSize = Clamp(toml[U"layout.grid"].getOr<int32>(layout.gridSize), 8, 160);
		layout.skirmishButtonRect.x = toml[U"skirmish_button.x"].getOr<double>(layout.skirmishButtonRect.x);
		layout.skirmishButtonRect.y = toml[U"skirmish_button.y"].getOr<double>(layout.skirmishButtonRect.y);
		layout.skirmishButtonRect.w = toml[U"skirmish_button.w"].getOr<double>(layout.skirmishButtonRect.w);
		layout.skirmishButtonRect.h = toml[U"skirmish_button.h"].getOr<double>(layout.skirmishButtonRect.h);
		layout.musicEditorToggleRect.x = toml[U"music_editor_toggle.x"].getOr<double>(layout.musicEditorToggleRect.x);
		layout.musicEditorToggleRect.y = toml[U"music_editor_toggle.y"].getOr<double>(layout.musicEditorToggleRect.y);
		layout.musicEditorToggleRect.w = toml[U"music_editor_toggle.w"].getOr<double>(layout.musicEditorToggleRect.w);
		layout.musicEditorToggleRect.h = toml[U"music_editor_toggle.h"].getOr<double>(layout.musicEditorToggleRect.h);
		RepairTitleUiLayout(layout);
		return true;
	}
}
