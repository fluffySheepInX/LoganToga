#pragma once
# include <Siv3D.hpp>
# include <filesystem>
# include "BattleOutcome.h"

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
		BattleOutcomeRules outcomeRules;
		bool valid = false;
	};

	struct ModManifestCandidate
	{
		String id;
		FilePath rootPath;
		FilePath manifestPath;
	};

	// schema v1 の従来勝敗規則を互換用の実行時ルールへ変換する。
	inline BattleOutcomeRules MakeLegacySkirmishOutcomeRules()
	{
		BattleOutcomeRules rules;
		rules.victoryCondition = BattleEndCondition::DestroyEnemyBases;
		rules.defeatCondition = BattleEndCondition::DestroyEnemyBases;
		rules.timeLimitSec = -1.0;
		rules.timeoutOutcome = BattleOutcome::Defeat;
		return rules;
	}

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

	// 正規化済みの候補パスが正規化済み root の厳密な配下かを判定します。
	inline bool IsStrictlyWithinCanonicalRoot(const std::filesystem::path& canonicalRoot, const std::filesystem::path& canonicalCandidate)
	{
		auto rootPart = canonicalRoot.begin();
		auto candidatePart = canonicalCandidate.begin();
		for (; rootPart != canonicalRoot.end(); ++rootPart, ++candidatePart)
		{
			if (candidatePart == canonicalCandidate.end() || *rootPart != *candidatePart)
			{
				return false;
			}
		}

		return candidatePart != canonicalCandidate.end();
	}

	// 存在しない将来用ディレクトリを含め、相対パスの字句上の root 逸脱を検証します。
	inline bool IsLexicallySafeModRelativePath(StringView relativePath)
	{
		if (relativePath.isEmpty())
		{
			return false;
		}

		const std::filesystem::path inputPath{ String{ relativePath }.toWstr() };
		if (!inputPath.is_relative() || inputPath.has_root_name() || inputPath.has_root_directory())
		{
			return false;
		}

		for (const auto& part : inputPath.lexically_normal())
		{
			if (part == L"..")
			{
				return false;
			}
		}

		return true;
	}

	// manifest からの相対パスを解決し、実体が root の外部を参照しない場合だけ返します。
	inline Optional<FilePath> ResolveCanonicalModRelativePath(StringView rootPath, StringView relativePath)
	{
		if (rootPath.isEmpty() || relativePath.isEmpty())
		{
			return none;
		}

		const std::filesystem::path inputPath{ String{ relativePath }.toWstr() };
		if (!inputPath.is_relative() || inputPath.has_root_name() || inputPath.has_root_directory())
		{
			return none;
		}

		std::error_code error;
		const std::filesystem::path canonicalRoot = std::filesystem::canonical(std::filesystem::path{ String{ rootPath }.toWstr() }, error);
		if (error || !std::filesystem::is_directory(canonicalRoot, error) || error)
		{
			return none;
		}

		const std::filesystem::path canonicalCandidate = std::filesystem::canonical(canonicalRoot / inputPath, error);
		if (error || !IsStrictlyWithinCanonicalRoot(canonicalRoot, canonicalCandidate))
		{
			return none;
		}

		return Unicode::FromWstring(canonicalCandidate.wstring());
	}

	// Warehouse 配下の有効な mod manifest を、実体ディレクトリごとに一度だけ索引化します。
	inline Array<ModManifestCandidate> IndexWarehouseModManifests()
	{
		Array<ModManifestCandidate> candidates;
		HashSet<String> indexedRoots;
		const Array<FilePath> warehouseRoots = {
			U"000_Warehouse/",
			U"App/000_Warehouse/",
		};
		for (const FilePath& warehouseRoot : warehouseRoots)
		{
			std::error_code warehouseError;
			const std::filesystem::path canonicalWarehouse = std::filesystem::canonical(std::filesystem::path{ warehouseRoot.toWstr() }, warehouseError);
			if (warehouseError || !std::filesystem::is_directory(canonicalWarehouse, warehouseError) || warehouseError)
			{
				continue;
			}

			for (const FilePath& candidateRoot : FileSystem::DirectoryContents(warehouseRoot, Recursive::No))
			{
				if (!FileSystem::IsDirectory(candidateRoot))
				{
					continue;
				}

				std::error_code rootError;
				const std::filesystem::path canonicalRoot = std::filesystem::canonical(std::filesystem::path{ candidateRoot.toWstr() }, rootError);
				if (rootError || !std::filesystem::is_directory(canonicalRoot, rootError) || rootError
					|| !IsStrictlyWithinCanonicalRoot(canonicalWarehouse, canonicalRoot))
				{
					continue;
				}

				const FilePath rootPath = Unicode::FromWstring(canonicalRoot.wstring()) + U"/";
				if (indexedRoots.contains(rootPath))
				{
					continue;
				}

				const Optional<FilePath> manifestPath = ResolveCanonicalModRelativePath(rootPath, U"mod.toml");
				if (!manifestPath)
				{
					continue;
				}

				const TOMLReader toml{ *manifestPath };
				const Optional<String> manifestId = toml ? NormalizeContentId(toml[U"id"].getOr<String>(U"")) : none;
				if (!manifestId || toml[U"schema_version"].getOr<int32>(0) != 1)
				{
					continue;
				}

				indexedRoots.insert(rootPath);
				candidates << ModManifestCandidate{ *manifestId, rootPath, *manifestPath };
			}
		}

		candidates.sort_by([](const ModManifestCandidate& a, const ModManifestCandidate& b) { return a.manifestPath < b.manifestPath; });
		return candidates;
	}

	// 同一の正規化 mod ID を持つ manifest の競合診断を生成します。
	inline String DescribeDuplicateModId(StringView id, const Array<ModManifestCandidate>& candidates)
	{
		String message = U"Duplicate mod ID '{}':"_fmt(id);
		for (const ModManifestCandidate& candidate : candidates)
		{
			message += U" {}"_fmt(candidate.manifestPath);
		}
		return message;
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

		Array<ModManifestCandidate> matches;
		for (const ModManifestCandidate& candidate : IndexWarehouseModManifests())
		{
			if (candidate.id == *normalizedRequestedId)
			{
				matches << candidate;
			}
		}

		if (matches.isEmpty())
		{
			statusText = U"Mod not found: {}"_fmt(*normalizedRequestedId);
			return false;
		}
		if (matches.size() != 1)
		{
			statusText = DescribeDuplicateModId(*normalizedRequestedId, matches);
			return false;
		}

		const ModManifestCandidate& selected = matches.front();
		const TOMLReader toml{ selected.manifestPath };
		if (!toml)
		{
			statusText = U"Unable to read mod manifest: {}"_fmt(selected.manifestPath);
			return false;
		}

		context.id = selected.id;
		context.displayName = toml[U"display_name"].getOr<String>(context.id);
		context.version = toml[U"version"].getOr<String>(U"");
		context.rootPath = selected.rootPath;
		context.inheritDefaultGame = toml[U"inherit_default_game"].getOr<bool>(false);
		context.scenariosRoot = toml[U"scenarios_root"].getOr<String>(U"Scenarios");
		if (!IsLexicallySafeModRelativePath(context.scenariosRoot))
		{
			statusText = U"Invalid scenarios root: {}"_fmt(selected.manifestPath);
			return false;
		}

		return true;
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

		const int32 schemaVersion = toml[U"schema_version"].getOr<int32>(0);
		if (schemaVersion != 1 && schemaVersion != 2)
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
		const Optional<FilePath> mapPath = ResolveCanonicalModRelativePath(mod.rootPath, mapRelativePath);
		const Optional<FilePath> resourcePath = ResolveCanonicalModRelativePath(mod.rootPath, resourceRelativePath);
		if (!mapPath || !resourcePath)
		{
			statusText = U"Invalid skirmish asset path: {}"_fmt(battlePath);
			return false;
		}

		BattleOutcomeRules outcomeRules = MakeLegacySkirmishOutcomeRules();
		if (schemaVersion == 2)
		{
			const Optional<BattleEndCondition> victoryCondition = ParseBattleEndCondition(toml[U"victory_condition"].getOr<String>(U""));
			const Optional<BattleEndCondition> defeatCondition = ParseBattleEndCondition(toml[U"defeat_condition"].getOr<String>(U""));
			const Optional<double> timeLimitSec = toml[U"time_limit_sec"].getOpt<double>();
			const Optional<BattleOutcome> timeoutOutcome = ParseTimeoutBattleOutcome(toml[U"timeout_outcome"].getOr<String>(U""));
			if (!victoryCondition || !defeatCondition || !timeLimitSec || *timeLimitSec < 0.0 || !timeoutOutcome)
			{
				statusText = U"Invalid skirmish outcome rules: {}"_fmt(battlePath);
				return false;
			}

			outcomeRules.victoryCondition = *victoryCondition;
			outcomeRules.defeatCondition = *defeatCondition;
			outcomeRules.timeLimitSec = *timeLimitSec;
			outcomeRules.timeoutOutcome = *timeoutOutcome;
		}

		request.mod = mod;
		request.scenarioId = U"";
		request.battleId = *battleId;
		request.displayName = toml[U"display_name"].getOr<String>(*battleId);
		request.mapPath = *mapPath;
		request.resourceNodePath = *resourcePath;
		request.aiProfileTag = toml[U"ai_profile"].getOr<String>(U"").lowercased();
		request.outcomeRules = outcomeRules;
		request.valid = true;
		return true;
	}
}
