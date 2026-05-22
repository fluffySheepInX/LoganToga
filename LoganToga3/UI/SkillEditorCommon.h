#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "RectUiHelpers.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/Loaders/SkillDefLoader.h"

namespace LT3
{
	inline RectF SkillEditorPanelRect()
	{
		return RectF{ 760.0, 84.0, 800.0, 722.0 };
	}

	inline RectF SkillEditorCloseRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF SkillEditorListViewportRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 106.0, panel.y + 54.0, 188.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorUnitViewportRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 54.0, 76.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorDetailRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 310.0, panel.y + 54.0, panel.w - 328.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorDetailViewportRect()
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0, detail.y + 8.0, detail.w - 16.0, detail.h - 52.0 };
	}

	inline RectF SkillEditorSaveRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + panel.w - 138.0, panel.y + panel.h - 38.0, 118.0, 26.0 };
	}

	inline RectF SkillEditorSkillRowRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorListViewportRect();
		return RectF{ viewport.x + 4.0, viewport.y + 4.0 + visibleIndex * 48.0, viewport.w - 8.0, 42.0 };
	}

	inline RectF SkillEditorContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 156.0, 30.0 };
	}

	inline RectF SkillEditorContextMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectF{ pos.x + 4.0, pos.y + 4.0 + index * 28.0, 148.0, 22.0 };
	}

	inline RectF SkillEditorUnitIconRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorUnitViewportRect();
		return RectF{ viewport.x + 10.0, viewport.y + 8.0 + visibleIndex * 58.0, 52.0, 52.0 };
	}

	inline RectF SkillEditorIconPreviewRect(double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 10.0, detail.y + 58.0 - scroll, 54.0, 54.0 };
	}

	inline RectF SkillEditorIconBrowseRect(double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 72.0, detail.y + 74.0 - scroll, 82.0, 24.0 };
	}

	inline RectF SkillEditorKindButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0 + (index % 3) * 86.0, detail.y + 150.0 + (index / 3) * 30.0 - scroll, 78.0, 24.0 };
	}

	inline RectF SkillEditorMotionButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0 + index * 86.0, detail.y + 228.0 - scroll, 78.0, 24.0 };
	}

	inline RectF SkillEditorValueButtonRect(int32 row, int32 buttonIndex, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 128.0 + buttonIndex * 42.0, detail.y + 290.0 + row * 38.0 - scroll, 36.0, 24.0 };
	}

	inline double SkillEditorDetailContentHeight()
	{
		return 710.0;
	}

	inline FilePath ResolveSkillIconPath(const String& iconName)
	{
		if (iconName.isEmpty())
		{
			return FilePath{};
		}

		const Array<FilePath> candidates = {
			ResolveBuildIconPath(iconName),
			ResolveSystemImagePath(iconName),
			ResolveUnitChipPath(iconName),
			ResolveBuildingChipPath(iconName),
		};
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}
		return FilePath{};
	}

	inline bool UnitHasSkill(const UnitCatalogEntry& entry, const String& skillTag)
	{
		return entry.skills.any([&](const String& tag) { return tag == skillTag; });
	}

	inline Array<FilePath> SkillEditorIconPaths(const SkillDef& skill)
	{
		Array<FilePath> paths;
		if (!skill.iconLayers.isEmpty())
		{
			for (const auto& icon : skill.iconLayers)
			{
				const FilePath path = ResolveSkillIconPath(icon);
				if (!path.isEmpty())
				{
					paths << path;
				}
			}
		}
		else if (!skill.icon.isEmpty())
		{
			const FilePath path = ResolveSkillIconPath(skill.icon);
			if (!path.isEmpty())
			{
				paths << path;
			}
		}
		return paths;
	}

	inline void DrawSkillEditorLayeredIcon(const SkillDef& skill, const RectF& iconRect)
	{
		for (const auto& iconPath : SkillEditorIconPaths(skill))
		{
			if (!FileSystem::Exists(iconPath))
			{
				continue;
			}

			auto& cache = BuildingEditorTextureCache();
			if (!cache.contains(iconPath))
			{
				cache.emplace(iconPath, Texture{ iconPath });
			}

			const Texture& texture = cache.at(iconPath);
			const double fitScale = Min((iconRect.w - 4.0) / Max(1.0, static_cast<double>(texture.width())), (iconRect.h - 4.0) / Max(1.0, static_cast<double>(texture.height())));
			texture.scaled(Min(1.0, fitScale)).drawAt(iconRect.center());
		}
	}

	inline const Array<String>& SkillKindLabels()
	{
		static const Array<String> labels = { U"missile", U"sword", U"heal", U"summon", U"charge", U"status" };
		return labels;
	}

	inline const Array<String>& SkillMotionLabels()
	{
		static const Array<String> labels = { U"direct", U"parabola", U"orbit" };
		return labels;
	}

	inline int32 SkillKindIndex(SkillKind kind)
	{
		return static_cast<int32>(kind);
	}

	inline int32 SkillMotionIndex(SkillProjectileMotion motion)
	{
		return static_cast<int32>(motion);
	}

	inline bool HasSelectedSkill(const MapEditorState& editor, const DefinitionStores& defs)
	{
		return 0 <= editor.selectedSkillIndex && editor.selectedSkillIndex < static_cast<int32>(defs.skills.size());
	}

	inline void SaveSkillEditorDefinitions(MapEditorState& editor, const DefinitionStores& defs)
	{
		String status;
		if (SaveSkillDefinitionsToml(defs.skills, &status))
		{
			editor.skillDefsDirty = true;
		}
		editor.statusText = status;
	}

	inline void ChangeSkillValue(SkillDef& skill, int32 row, double delta)
	{
		switch (row)
		{
		case 0:
			skill.range = Max(0.0, skill.range + delta);
			break;
		case 1:
			skill.cooldownSec = Max(0.05, skill.cooldownSec + delta);
			break;
		case 2:
			skill.damage = Clamp(skill.damage + static_cast<int32>(delta), -999, 999);
			break;
		case 3:
			skill.projectileSpeed = Max(1.0, skill.projectileSpeed + delta);
			break;
		case 4:
			skill.burstCount = Clamp(skill.burstCount + static_cast<int32>(delta), 1, 32);
			break;
		case 5:
			skill.spreadDeg = Clamp(skill.spreadDeg + delta, 0.0, 180.0);
			break;
		case 6:
			skill.arcHeight = Max(0.0, skill.arcHeight + delta);
			break;
		case 7:
			skill.orbitRadius = Max(1.0, skill.orbitRadius + delta);
			break;
		case 8:
			skill.orbitAngularSpeedDeg += delta;
			break;
		case 9:
			skill.orbitDurationSec = Max(0.05, skill.orbitDurationSec + delta);
			break;
		default:
			break;
		}
	}
}
