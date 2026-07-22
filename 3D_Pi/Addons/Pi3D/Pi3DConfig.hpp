# pragma once
# include <Siv3D.hpp>

# ifndef PI3D_ENABLE_EDITOR_UI
# define PI3D_ENABLE_EDITOR_UI 1
# endif

static_assert((PI3D_ENABLE_EDITOR_UI == 0) || (PI3D_ENABLE_EDITOR_UI == 1),
	"PI3D_ENABLE_EDITOR_UI must be 0 or 1");

namespace Pi3D
{
	inline constexpr bool EditorUICompiled = (PI3D_ENABLE_EDITOR_UI != 0);

	struct Config
	{
		FilePath shaderDirectory = U"../Addons/Pi3D/Resources/shader";
		FilePath textureDirectory = U"../Addons/Pi3D/Resources/texture";
		FilePath effectPresetsPath = U"../Addons/Pi3D/Resources/toml/effect_presets.toml";
		FilePath lightingPresetsPath = U"../Addons/Pi3D/Resources/toml/lighting_presets.toml";
		FilePath settingsPath = U"save/pi3d_settings.toml";
		bool editorUIEnabled = EditorUICompiled;
		bool persistenceEnabled = true;
	};

	namespace detail
	{
		// Pi3D 全体で共有する構成を保持する。
		[[nodiscard]] inline Config& MutableConfig()
		{
			static Config config;
			return config;
		}

		// System 構築後の構成変更を防ぐロック状態を保持する。
		[[nodiscard]] inline bool& ConfigLocked()
		{
			static bool locked = false;
			return locked;
		}
	}

	// 現在の Pi3D 構成を返す。
	[[nodiscard]] inline const Config& GetConfig()
	{
		return detail::MutableConfig();
	}

	// System 構築前に Pi3D 構成を設定する。
	inline bool SetConfig(const Config& config)
	{
		if (detail::ConfigLocked())
		{
			return false;
		}

		detail::MutableConfig() = config;
		return true;
	}

	// Pi3D 構成を固定し、以後の変更を拒否する。
	inline void LockConfig()
	{
		detail::ConfigLocked() = true;
	}

	// ディレクトリと相対パスを結合する。
	[[nodiscard]] inline FilePath JoinPath(const FilePath& directory, const StringView relativePath)
	{
		if (directory.isEmpty())
		{
			return FilePath{ relativePath };
		}

		FilePath result = directory;
		if ((result.back() != U'/') && (result.back() != U'\\'))
		{
			result.push_back(U'/');
		}
		result.append(relativePath);
		return result;
	}

	// Editor 用画像の構成済みパスを返す。
	[[nodiscard]] inline FilePath ResolveTexturePath(const StringView fileName)
	{
		return JoinPath(GetConfig().textureDirectory, fileName);
	}

	// プリセット TOML のパスを解決する。明示パスが空の場合のみ既存プロジェクト互換の候補を探索する。
	[[nodiscard]] inline FilePath ResolvePresetPath(const FilePath& configuredPath, const StringView fileName)
	{
		if (not configuredPath.isEmpty())
		{
			return configuredPath;
		}

		const FilePath relative = JoinPath(U"Addons/Pi3D/Resources/toml", fileName);
		const Array<FilePath> candidates = {
			relative,
			(U"../" + relative),
			(U"3D_Pi/" + relative),
		};
		for (const auto& p : candidates)
		{
			if (FileSystem::Exists(p))
			{
				return p;
			}
		}
		return candidates.front();
	}
}
