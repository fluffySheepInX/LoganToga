#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
	inline double MapEditorPerlinFade(double t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	inline uint32 MapEditorPerlinHash(int32 x, int32 y, uint32 seed)
	{
		uint32 h = static_cast<uint32>(x) * 374761393u + static_cast<uint32>(y) * 668265263u + seed * 1442695041u;
		h = (h ^ (h >> 13)) * 1274126177u;
		return h ^ (h >> 16);
	}

	inline double MapEditorPerlinCornerValue(int32 x, int32 y, uint32 seed)
	{
		return static_cast<double>(MapEditorPerlinHash(x, y, seed) & 0x00FFFFFFu) / static_cast<double>(0x00FFFFFFu);
	}

	inline double MapEditorPerlinValueNoise(double x, double y, uint32 seed)
	{
		const int32 x0 = static_cast<int32>(Math::Floor(x));
		const int32 y0 = static_cast<int32>(Math::Floor(y));
		const double tx = x - x0;
		const double ty = y - y0;
		const double sx = MapEditorPerlinFade(tx);
		const double sy = MapEditorPerlinFade(ty);

		const double v00 = MapEditorPerlinCornerValue(x0, y0, seed);
		const double v10 = MapEditorPerlinCornerValue(x0 + 1, y0, seed);
		const double v01 = MapEditorPerlinCornerValue(x0, y0 + 1, seed);
		const double v11 = MapEditorPerlinCornerValue(x0 + 1, y0 + 1, seed);
		return Math::Lerp(Math::Lerp(v00, v10, sx), Math::Lerp(v01, v11, sx), sy);
	}

	inline double MapEditorPerlinFbm(double x, double y, uint32 seed)
	{
		double amplitude = 0.55;
		double frequency = 1.0;
		double value = 0.0;
		double totalAmplitude = 0.0;
		for (int32 octave = 0; octave < 5; ++octave)
		{
			value += MapEditorPerlinValueNoise(x * frequency, y * frequency, seed + static_cast<uint32>(octave) * 1013u) * amplitude;
			totalAmplitude += amplitude;
			amplitude *= 0.52;
			frequency *= 2.03;
		}

		return (totalAmplitude <= 0.0) ? 0.0 : Clamp(value / totalAmplitude, 0.0, 1.0);
	}

	inline Optional<int32> FindMapEditorAssetIndexByPath(const MapEditorState& editor, const FilePath& path)
	{
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			if (editor.assets[i].path == path)
			{
				return i;
			}
		}

		return none;
	}

	inline Optional<int32> EnsurePerlinTerrainAsset(MapEditorState& editor, const FilePath& path)
	{
		if (!IsMapEditorImageFile(path))
		{
			editor.statusText = U"Perlin stack failed: not an image";
			return none;
		}

		if (const Optional<int32> byPath = FindMapEditorAssetIndexByPath(editor, path))
		{
			editor.assets[*byPath].kind = MapEditorAssetKind::Terrain;
			return byPath;
		}

		const String fileName = FileSystem::FileName(path);
		const int32 byName = FindMapEditorAssetIndexByFileName(editor, fileName);
		if (byName != InvalidMapEditorAsset)
		{
			editor.assets[byName].kind = MapEditorAssetKind::Terrain;
			return byName;
		}

		MapEditorAsset asset;
		asset.path = path;
		asset.fileName = fileName;
		asset.kind = MapEditorAssetKind::Terrain;
		if (!LoadMapEditorAssetVisual(asset))
		{
			editor.statusText = U"Perlin stack failed: cannot load {}"_fmt(fileName);
			return none;
		}

		editor.assets << asset;
		return static_cast<int32>(editor.assets.size() - 1);
	}

	inline bool StackPerlinTerrainFile(MapEditorState& editor, const FilePath& path)
	{
		const Optional<int32> assetIndex = EnsurePerlinTerrainAsset(editor, path);
		if (!assetIndex)
		{
			return false;
		}

		editor.perlinStack << MapEditorPerlinStackItem{ path, FileSystem::FileName(path), *assetIndex };
		editor.statusText = U"Perlin terrain stacked: {} ({} total)"_fmt(FileSystem::FileName(path), editor.perlinStack.size());
		return true;
	}

	inline bool ApplyPerlinTerrainMap(MapEditorState& editor)
	{
		Array<int32> terrainAssets;
		for (const auto& item : editor.perlinStack)
		{
			if ((0 <= item.assetIndex) && (item.assetIndex < static_cast<int32>(editor.assets.size())))
			{
				terrainAssets << item.assetIndex;
			}
		}

		if (terrainAssets.isEmpty())
		{
			editor.statusText = U"Perlin run skipped: stack is empty";
			return false;
		}

		ResizeMapEditorCells(editor, editor.perlinMapWidth, editor.perlinMapHeight);
		const double normalizer = static_cast<double>(Max(editor.mapWidth, editor.mapHeight));
		const double scale = Max(3.0, normalizer * 0.42);
		const uint32 seed = static_cast<uint32>(DateTime::Now().hash());

		for (int32 y = 0; y < editor.mapHeight; ++y)
		{
			for (int32 x = 0; x < editor.mapWidth; ++x)
			{
				const double nx = (static_cast<double>(x) + 0.37) / scale;
				const double ny = (static_cast<double>(y) + 0.61) / scale;
				const double noise = MapEditorPerlinFbm(nx, ny, seed);
				const int32 index = Clamp(static_cast<int32>(Math::Floor(noise * terrainAssets.size())), 0, static_cast<int32>(terrainAssets.size() - 1));
				editor.cells[MapEditorCellIndex(editor, x, y)].terrainAsset = terrainAssets[index];
			}
		}

		editor.selectedAsset = terrainAssets.front();
		editor.statusText = U"Perlin terrain generated: {} x {}, {} files"_fmt(editor.mapWidth, editor.mapHeight, terrainAssets.size());
		return true;
	}

	inline void DrawStarToolMenu(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.enabled || !editor.showStarToolMenu)
		{
			return;
		}

		const RectF menu = EditorStarToolMenuRect();
		menu.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		const Array<String> labels = { U"None", U"Resource Panels", U"Perlin Noise", U"Fog Panel", U"Z-order" };
		for (int32 i = 0; i < static_cast<int32>(labels.size()); ++i)
		{
			const RectF item = EditorStarToolMenuItemRect(i);
			const bool active = (i == 0) ? (!editor.showResourcePanels && !editor.showPerlinNoisePanel && !editor.showFogPanel && !editor.zOrderMode)
				: (i == 1) ? editor.showResourcePanels
				: (i == 2) ? editor.showPerlinNoisePanel
				: (i == 3) ? editor.showFogPanel
				: editor.zOrderMode;
			item.draw(active ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(1, item.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 });
			uiFont(labels[i]).drawAt(12, item.center(), active ? Palette::White : Palette::Lightgray);
		}
	}

	inline double FogColorChannelValue(const MapEditorState& editor, int32 channel)
	{
		if (channel == 0)
		{
			return editor.fogColor.r;
		}
		if (channel == 1)
		{
			return editor.fogColor.g;
		}
		return editor.fogColor.b;
	}

	inline void SetFogColorChannelValue(MapEditorState& editor, int32 channel, double value)
	{
		value = Math::Round(Clamp(value, 0.0, 1.0) * 100.0) / 100.0;
		if (channel == 0)
		{
			editor.fogColor.r = value;
		}
		else if (channel == 1)
		{
			editor.fogColor.g = value;
		}
		else
		{
			editor.fogColor.b = value;
		}
	}

	inline void DrawFogPanel(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.enabled || !editor.showFogPanel)
		{
			return;
		}

		const RectF panel = EditorFogPanelRect();
		const RectF closeRect = EditorFogCloseRect();
		const RectF toggleRect = EditorFogToggleRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Fog of War").draw(panel.x + 18.0, panel.y + 14.0, Palette::White);
		closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);

		toggleRect.draw(editor.fogEnabled ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, toggleRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(editor.fogEnabled ? U"Fog Enabled" : U"Fog Disabled").drawAt(12, toggleRect.center(), editor.fogEnabled ? Palette::White : Palette::Lightgray);

		const Array<String> channelLabels = { U"R", U"G", U"B" };
		for (int32 channel = 0; channel < 3; ++channel)
		{
			const RectF decRect = EditorFogColorDecRect(channel);
			const RectF incRect = EditorFogColorIncRect(channel);
			decRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, decRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			incRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, incRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"-").drawAt(18, decRect.center(), Palette::White);
			uiFont(U"+").drawAt(18, incRect.center(), Palette::White);
			uiFont(channelLabels[channel]).draw(12, panel.x + 72.0, decRect.y + 6.0, Palette::Gold);
			uiFont(U"{:.2f}"_fmt(FogColorChannelValue(editor, channel))).drawAt(15, Vec2{ panel.x + panel.w * 0.58, decRect.center().y }, Palette::White);
		}

		const RectF swatch{ panel.x + 18.0, panel.y + 196.0, panel.w - 36.0, 20.0 };
		swatch.draw(ColorF{ editor.fogColor.r, editor.fogColor.g, editor.fogColor.b, editor.fogOpacity }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

		const RectF opacityDecRect = EditorFogOpacityDecRect();
		const RectF opacityIncRect = EditorFogOpacityIncRect();
		opacityDecRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, opacityDecRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		opacityIncRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, opacityIncRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"-").drawAt(18, opacityDecRect.center(), Palette::White);
		uiFont(U"+").drawAt(18, opacityIncRect.center(), Palette::White);
		uiFont(U"Opacity").draw(12, panel.x + 72.0, opacityDecRect.y + 6.0, Palette::Gold);
		uiFont(U"{:.2f}"_fmt(editor.fogOpacity)).drawAt(15, Vec2{ panel.x + panel.w * 0.58, opacityDecRect.center().y }, Palette::White);

		const RectF previewRect = EditorFogPreviewToggleRect();
		previewRect.draw(editor.fogPreviewVision ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(1, previewRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(editor.fogPreviewVision ? U"Vision preview ON" : U"Vision preview OFF").drawAt(10, previewRect.center(), editor.fogPreviewVision ? Palette::White : Palette::Lightgray);
	}

	inline void DrawPerlinNoisePanel(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.enabled || !editor.showPerlinNoisePanel)
		{
			return;
		}

		const RectF panel = EditorPerlinNoisePanelRect(editor);
		const RectF closeRect = EditorPerlinNoiseCloseRect(editor);
		const RectF fileRect = EditorPerlinNoiseFileDialogRect(editor);
		const RectF runRect = EditorPerlinNoiseRunRect(editor);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Perlin Noise Terrain").draw(panel.x + 18.0, panel.y + 14.0, Palette::White);
		closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);
		if (editor.uiLayoutEditEnabled)
		{
			DrawUiLayoutDragHandleOnly(EditorPerlinNoiseDragHandleRect(editor), editor.uiLayoutDraggingPerlinNoisePanel, uiFont, 11);
		}

		fileRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, fileRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Add Image").drawAt(12, fileRect.center(), Palette::White);
		uiFont(U"Stack: {}"_fmt(editor.perlinStack.size())).draw(12, fileRect.x + fileRect.w + 16.0, fileRect.y + 7.0, Palette::Gold);

		for (int32 axis = 0; axis < 2; ++axis)
		{
			const bool widthAxis = (axis == 0);
			const RectF decRect = EditorPerlinNoiseSizeDecRect(editor, widthAxis);
			const RectF incRect = EditorPerlinNoiseSizeIncRect(editor, widthAxis);
			decRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, decRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			incRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, incRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"-").drawAt(22, decRect.center(), Palette::White);
			uiFont(U"+").drawAt(22, incRect.center(), Palette::White);
			uiFont(widthAxis ? U"Width" : U"Height").draw(12, decRect.x + 52.0, decRect.y + 6.0, Palette::Lightgray);
			uiFont(U"{}"_fmt(widthAxis ? editor.perlinMapWidth : editor.perlinMapHeight)).drawAt(18, Vec2{ decRect.x + 154.0, decRect.center().y }, Palette::White);
		}

		const RectF stackViewport = EditorPerlinNoiseStackViewportRect(editor);
		stackViewport.draw(ColorF{ 0, 0, 0, 0.12 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		if (editor.perlinStack.isEmpty())
		{
			uiFont(U"No images stacked").draw(11, stackViewport.x + 8.0, stackViewport.y + 12.0, Palette::Gray);
		}
		else
		{
			const int32 begin = Max(0, static_cast<int32>(editor.perlinStack.size()) - 2);
			for (int32 i = begin; i < static_cast<int32>(editor.perlinStack.size()); ++i)
			{
				uiFont(U"{}: {}"_fmt(i + 1, editor.perlinStack[i].fileName)).draw(10, stackViewport.x + 8.0, stackViewport.y + 5.0 + (i - begin) * 18.0, Palette::Lightgray);
			}
		}

		runRect.draw(editor.perlinStack.isEmpty() ? ColorF{ 0.10, 0.10, 0.10, 0.70 } : ColorF{ 0.10, 0.16, 0.08, 0.95 })
			.drawFrame(1, runRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Run").drawAt(13, runRect.center(), editor.perlinStack.isEmpty() ? Palette::Gray : Palette::White);
		uiFont(U"terrain only / fBm quantized by stack order").draw(10, panel.x + 18.0, panel.y + panel.h - 34.0, Palette::Gray);
	}

	inline bool ProcessStarToolMenuInput(MapEditorState& editor)
	{
		if (!editor.enabled)
		{
			return false;
		}

		if (EditorResourcePanelsToggleRect().leftClicked())
		{
			editor.showStarToolMenu = !editor.showStarToolMenu;
			return true;
		}

		if (!editor.showStarToolMenu)
		{
			return false;
		}

		if (EditorStarToolMenuItemRect(0).leftClicked())
		{
			editor.showResourcePanels = false;
			editor.showPerlinNoisePanel = false;
			editor.showFogPanel = false;
			editor.zOrderMode = false;
			editor.zOrderDragStartCell.reset();
			editor.zOrderSelectionRect.reset();
			editor.resourcePlacementDragKind.reset();
			editor.showStarToolMenu = false;
			editor.statusText = U"Panels hidden";
			return true;
		}

		if (EditorStarToolMenuItemRect(1).leftClicked())
		{
			editor.showResourcePanels = true;
			editor.showPerlinNoisePanel = false;
			editor.showFogPanel = false;
			editor.zOrderMode = false;
			editor.zOrderDragStartCell.reset();
			editor.zOrderSelectionRect.reset();
			editor.showStarToolMenu = false;
			editor.statusText = U"Resource panels ON";
			return true;
		}

		if (EditorStarToolMenuItemRect(2).leftClicked())
		{
			editor.showPerlinNoisePanel = true;
			editor.showResourcePanels = false;
			editor.showFogPanel = false;
			editor.zOrderMode = false;
			editor.zOrderDragStartCell.reset();
			editor.zOrderSelectionRect.reset();
			editor.resourcePlacementDragKind.reset();
			editor.showStarToolMenu = false;
			editor.statusText = U"Perlin noise panel ON";
			return true;
		}

		if (EditorStarToolMenuItemRect(3).leftClicked())
		{
			editor.showFogPanel = true;
			editor.showPerlinNoisePanel = false;
			editor.showResourcePanels = false;
			editor.zOrderMode = false;
			editor.zOrderDragStartCell.reset();
			editor.zOrderSelectionRect.reset();
			editor.resourcePlacementDragKind.reset();
			editor.showStarToolMenu = false;
			editor.statusText = U"Fog panel ON";
			return true;
		}

		if (EditorStarToolMenuItemRect(4).leftClicked())
		{
			editor.zOrderMode = true;
			editor.showResourcePanels = false;
			editor.showPerlinNoisePanel = false;
			editor.showFogPanel = false;
			editor.resourcePlacementDragKind.reset();
			editor.zOrderDragStartCell.reset();
			editor.zOrderSelectionRect.reset();
			editor.zOrderSelectedStackIndex = 0;
			editor.showStarToolMenu = false;
			editor.statusText = U"Z-order mode: drag cells";
			return true;
		}

		if (EditorStarToolMenuRect().mouseOver())
		{
			return true;
		}

		return false;
	}

	inline bool ProcessFogPanelInput(MapEditorState& editor)
	{
		if (!editor.enabled || !editor.showFogPanel)
		{
			return false;
		}

		bool consumed = false;
		if (EditorFogCloseRect().leftClicked())
		{
			editor.showFogPanel = false;
			editor.statusText = U"Fog panel OFF";
			return true;
		}
		if (EditorFogToggleRect().leftClicked())
		{
			editor.fogEnabled = !editor.fogEnabled;
			SaveMapEditorToml(editor, false);
			consumed = true;
		}
		for (int32 channel = 0; channel < 3; ++channel)
		{
			if (EditorFogColorDecRect(channel).leftClicked())
			{
				SetFogColorChannelValue(editor, channel, FogColorChannelValue(editor, channel) - 0.05);
				SaveMapEditorToml(editor, false);
				consumed = true;
			}
			if (EditorFogColorIncRect(channel).leftClicked())
			{
				SetFogColorChannelValue(editor, channel, FogColorChannelValue(editor, channel) + 0.05);
				SaveMapEditorToml(editor, false);
				consumed = true;
			}
		}
		if (EditorFogOpacityDecRect().leftClicked())
		{
			editor.fogOpacity = Math::Round(Clamp(editor.fogOpacity - 0.05, 0.0, 1.0) * 100.0) / 100.0;
			SaveMapEditorToml(editor, false);
			consumed = true;
		}
		if (EditorFogOpacityIncRect().leftClicked())
		{
			editor.fogOpacity = Math::Round(Clamp(editor.fogOpacity + 0.05, 0.0, 1.0) * 100.0) / 100.0;
			SaveMapEditorToml(editor, false);
			consumed = true;
		}
		if (EditorFogPreviewToggleRect().leftClicked())
		{
			editor.fogPreviewVision = !editor.fogPreviewVision;
			SaveMapEditorToml(editor, false);
			consumed = true;
		}
		if (EditorFogPanelRect().mouseOver())
		{
			consumed = true;
		}

		return consumed;
	}

	inline bool ProcessPerlinNoisePanelInput(MapEditorState& editor)
	{
		if (!editor.enabled || !editor.showPerlinNoisePanel)
		{
			return false;
		}

		bool consumed = false;
		if (EditorPerlinNoiseCloseRect(editor).leftClicked())
		{
			editor.showPerlinNoisePanel = false;
			editor.statusText = U"Perlin noise panel OFF";
			return true;
		}

		if (EditorPerlinNoiseFileDialogRect(editor).leftClicked())
		{
			if (const Optional<FilePath> path = Dialog::OpenFile({ FileFilter::AllFiles() }))
			{
				StackPerlinTerrainFile(editor, *path);
			}
			consumed = true;
		}

		if (EditorPerlinNoiseSizeDecRect(editor, true).leftClicked())
		{
			editor.perlinMapWidth = Clamp(editor.perlinMapWidth - 1, 4, 40);
			consumed = true;
		}
		if (EditorPerlinNoiseSizeIncRect(editor, true).leftClicked())
		{
			editor.perlinMapWidth = Clamp(editor.perlinMapWidth + 1, 4, 40);
			consumed = true;
		}
		if (EditorPerlinNoiseSizeDecRect(editor, false).leftClicked())
		{
			editor.perlinMapHeight = Clamp(editor.perlinMapHeight - 1, 4, 40);
			consumed = true;
		}
		if (EditorPerlinNoiseSizeIncRect(editor, false).leftClicked())
		{
			editor.perlinMapHeight = Clamp(editor.perlinMapHeight + 1, 4, 40);
			consumed = true;
		}

		if (EditorPerlinNoiseRunRect(editor).leftClicked())
		{
			ApplyPerlinTerrainMap(editor);
			consumed = true;
		}

		if (EditorPerlinNoisePanelRect(editor).mouseOver())
		{
			consumed = true;
		}

		return consumed;
	}
}
