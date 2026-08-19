#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	struct ModContext
	{
		String id;
		String displayName;
		String version;
		FilePath rootPath;
		bool inheritDefaultGame = false;
		String scenariosRoot = U"Scenarios";
	};

	struct BattleRequest
	{
		ModContext mod;
		String scenarioId;
		String battleId;
		String displayName;
		FilePath mapPath;
		FilePath resourceNodePath;
		String aiProfileTag;
		bool valid = false;
	};

	// ASCII 安定 ID を小文字へ正規化します。
	inline Optional<String> NormalizeContentId(StringView value)
	{
		if (value.isEmpty())
		{
			return none;
		}

		String normalized;
		for (const char32 ch : value)
		{
			if ((U'A' <= ch) && (ch <= U'Z'))
			{
				normalized += (ch - U'A' + U'a');
			}
			else if (((U'a' <= ch) && (ch <= U'z')) || ((U'0' <= ch) && (ch <= U'9') || (ch == U'-')))
			{
				normalized += ch;
			}
			else
			{
				return none;
			}
		}

		return normalized;
	}

	// manifest からの相対パスが mod root の外部を参照しないことを検証します。
	inline bool IsSafeModRelativePath(StringView path)
	{
		return !path.isEmpty()
			&& !String{ path }.includes(U"..")
			&& !String{ path }.starts_with(U"/")
			&& !String{ path }.includes(U":");
	}

	// 指定された mod ID の manifest を Warehouse から読み込みます。
	inline bool TryLoadModContext(StringView requestedId, ModContext& context, String& statusText)
	{
		const Optional<String> normalizedRequestedId = NormalizeContentId(requestedId);
		if (!normalizedRequestedId)
		{
			statusText = U"Invalid mod ID: {}"_fmt(requestedId);
			return false;
		}

		const Array<FilePath> warehouseRoots = {
			U"000_Warehouse/",
			U"App/000_Warehouse/",
		};
		for (const FilePath& warehouseRoot : warehouseRoots)
		{
			if (!FileSystem::IsDirectory(warehouseRoot))
			{
				continue;
			}

			for (const FilePath& candidateRoot : FileSystem::DirectoryContents(warehouseRoot, Recursive::No))
			{
				if (!FileSystem::IsDirectory(candidateRoot))
				{
					continue;
				}

				const FilePath manifestPath = candidateRoot + U"/mod.toml";
				const TOMLReader toml{ manifestPath };
				if (!toml)
				{
					continue;
				}

				const Optional<String> manifestId = NormalizeContentId(toml[U"id"].getOr<String>(U""));
				if (!manifestId || (*manifestId != *normalizedRequestedId))
				{
					continue;
				}

				const int32 schemaVersion = toml[U"schema_version"].getOr<int32>(0);
				if (schemaVersion != 1)
				{
					statusText = U"Unsupported mod manifest version: {}"_fmt(manifestPath);
					return false;
				}

				context.id = *manifestId;
				context.displayName = toml[U"display_name"].getOr<String>(context.id);
				context.version = toml[U"version"].getOr<String>(U"");
				context.rootPath = candidateRoot + U"/";
				context.inheritDefaultGame = toml[U"inherit_default_game"].getOr<bool>(false);
				context.scenariosRoot = toml[U"scenarios_root"].getOr<String>(U"Scenarios");
				if (!IsSafeModRelativePath(context.scenariosRoot))
				{
					statusText = U"Invalid scenarios root: {}"_fmt(manifestPath);
					return false;
				}

				return true;
			}
		}

		statusText = U"Mod not found: {}"_fmt(*normalizedRequestedId);
		return false;
	}

	// スカーミッシュ battle manifest を解決済みの戦闘要求へ変換します。
	inline bool TryLoadSkirmishBattleRequest(const ModContext& mod, StringView requestedBattleId, BattleRequest& request, String& statusText)
	{
		const Optional<String> battleId = NormalizeContentId(requestedBattleId);
		if (!battleId)
		{
			statusText = U"Invalid skirmish ID: {}"_fmt(requestedBattleId);
			return false;
		}

		const FilePath battlePath = mod.rootPath + U"Skirmishes/" + *battleId + U".toml";
		const TOMLReader toml{ battlePath };
		if (!toml)
		{
			statusText = U"Skirmish not found: {}"_fmt(*battleId);
			return false;
		}

		if (toml[U"schema_version"].getOr<int32>(0) != 1)
		{
			statusText = U"Unsupported skirmish manifest version: {}"_fmt(battlePath);
			return false;
		}

		const Optional<String> manifestId = NormalizeContentId(toml[U"id"].getOr<String>(U""));
		if (!manifestId || (*manifestId != *battleId))
		{
			statusText = U"Skirmish ID does not match manifest: {}"_fmt(battlePath);
			return false;
		}

		const String mapRelativePath = toml[U"map"].getOr<String>(U"");
		const String resourceRelativePath = toml[U"resource_nodes"].getOr<String>(U"");
		if (!IsSafeModRelativePath(mapRelativePath) || !IsSafeModRelativePath(resourceRelativePath))
		{
			statusText = U"Invalid skirmish asset path: {}"_fmt(battlePath);
			return false;
		}

		const FilePath mapPath = mod.rootPath + mapRelativePath;
		const FilePath resourcePath = mod.rootPath + resourceRelativePath;
		if (!FileSystem::Exists(mapPath) || !FileSystem::Exists(resourcePath))
		{
			statusText = U"Skirmish references missing data: {}"_fmt(battlePath);
			return false;
		}

		request.mod = mod;
		request.scenarioId = U"";
		request.battleId = *battleId;
		request.displayName = toml[U"display_name"].getOr<String>(*battleId);
		request.mapPath = mapPath;
		request.resourceNodePath = resourcePath;
		request.aiProfileTag = toml[U"ai_profile"].getOr<String>(U"").lowercased();
		request.valid = true;
		return true;
	}
}
