#pragma once
# include <Siv3D.hpp>
# include <filesystem>

namespace LT3
{
	inline String EscapeTomlBasicString(StringView text)
	{
		String result;
		for (const char32 ch : text)
		{
			if (ch == U'\\')
			{
				result += U"\\\\";
			}
			else if (ch == U'\"')
			{
				result += U"\\\"";
			}
			else if (ch == U'\n')
			{
				result += U"\\n";
			}
			else if (ch == U'\r')
			{
				result += U"\\r";
			}
			else if (ch == U'\t')
			{
				result += U"\\t";
			}
			else
			{
				result += ch;
			}
		}

		return result;
	}

	inline Array<String> ReadTomlStringArrayValue(const TOMLValue& value)
	{
		Array<String> result;
		if (!value.isArray())
		{
			return result;
		}

		for (const auto item : value.arrayView())
		{
			if (const Optional<String> text = item.getOpt<String>())
			{
				result << *text;
			}
		}

		return result;
	}

	inline String BuildTomlStringArrayValue(const Array<String>& values)
	{
		String result = U"[";
		for (size_t i = 0; i < values.size(); ++i)
		{
			if (i > 0)
			{
				result += U", ";
			}

			result += U"\"" + EscapeTomlBasicString(values[i]) + U"\"";
		}

		result += U"]";
		return result;
	}

	inline FilePath ResolveFirstExistingPath(const Array<FilePath>& candidates)
	{
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}

		return candidates.isEmpty() ? FilePath{} : candidates.back();
	}

	inline bool WriteUtf8TextFile(FilePathView path, StringView text, String& statusText)
	{
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path, OpenMode::Trunc, TextEncoding::UTF8_NO_BOM };
		if (!writer)
		{
			statusText = U"Text file write failed: {}"_fmt(path);
			return false;
		}

		writer.write(text);
		return true;
	}

	struct TomlTransactionFile
	{
		FilePath path;
		FilePath temporaryPath;
		FilePath backupPath;
	};

	inline bool ReplaceTomlTransactionFile(const TomlTransactionFile& file, bool hadOriginalFile, std::error_code& error)
	{
		const std::filesystem::path path{ file.path.toWstr() };
		const std::filesystem::path backupPath{ file.backupPath.toWstr() };
		const std::filesystem::path temporaryPath{ file.temporaryPath.toWstr() };
		std::filesystem::remove(backupPath, error);
		error.clear();
		if (hadOriginalFile)
		{
			std::filesystem::rename(path, backupPath, error);
			if (error)
			{
				return false;
			}
		}

		std::filesystem::rename(temporaryPath, path, error);
		if (!error)
		{
			return true;
		}

		if (hadOriginalFile)
		{
			std::error_code rollbackError;
			std::filesystem::rename(backupPath, path, rollbackError);
		}
		return false;
	}

	inline bool SaveTomlFilesTransaction(Array<TomlTransactionFile> files, String& statusText)
	{
		if (files.isEmpty())
		{
			statusText = U"TOML transaction requires files";
			return false;
		}

		Array<bool> hadOriginalFiles;
		hadOriginalFiles.reserve(files.size());
		for (auto& file : files)
		{
			if (!FileSystem::Exists(file.temporaryPath))
			{
				statusText = U"TOML transaction temporary file missing: {}"_fmt(file.temporaryPath);
				return false;
			}
			hadOriginalFiles << FileSystem::Exists(file.path);
		}

		Array<int32> replacedIndices;
		for (int32 index = 0; index < static_cast<int32>(files.size()); ++index)
		{
			std::error_code error;
			if (ReplaceTomlTransactionFile(files[index], hadOriginalFiles[index], error))
			{
				replacedIndices << index;
				continue;
			}

			bool rollbackSucceeded = true;
			for (const int32 replacedIndex : replacedIndices)
			{
				const TomlTransactionFile& replaced = files[replacedIndex];
				std::error_code rollbackError;
				std::filesystem::remove(std::filesystem::path{ replaced.path.toWstr() }, rollbackError);
				if (hadOriginalFiles[replacedIndex])
				{
					rollbackError.clear();
					std::filesystem::rename(std::filesystem::path{ replaced.backupPath.toWstr() }, std::filesystem::path{ replaced.path.toWstr() }, rollbackError);
					rollbackSucceeded = rollbackSucceeded && !rollbackError;
				}
			}

			statusText = rollbackSucceeded
				? U"TOML transaction failed and was rolled back: {}"_fmt(files[index].path)
				: U"TOML transaction rollback failed; recover from {} and {}"_fmt(files[index].temporaryPath, files[index].backupPath);
			return false;
		}

		for (const auto& file : files)
		{
			FileSystem::Remove(file.backupPath);
		}
		return true;
	}

	// 一時ファイルへのコピー成功後に既存資産を置換し、失敗時は旧資産を復元する。
	inline bool ReplaceAssetFileSafely(FilePathView sourcePath, FilePathView targetPath, String& statusText)
	{
		const FilePath temporaryPath = FilePath{ targetPath } + U".tmp";
		const FilePath backupPath = FilePath{ targetPath } + U".bak";
		FileSystem::Remove(temporaryPath);
		if (!FileSystem::Copy(sourcePath, temporaryPath))
		{
			statusText = U"Asset copy failed: {}"_fmt(targetPath);
			return false;
		}

		const std::filesystem::path target{ String{ targetPath }.toWstr() };
		const std::filesystem::path temporary{ temporaryPath.toWstr() };
		const std::filesystem::path backup{ backupPath.toWstr() };
		const bool hadTarget = FileSystem::Exists(targetPath);
		std::error_code error;
		std::filesystem::remove(backup, error);
		error.clear();
		if (hadTarget)
		{
			std::filesystem::rename(target, backup, error);
			if (error)
			{
				statusText = U"Asset backup failed; original kept: {}"_fmt(targetPath);
				return false;
			}
		}

		std::filesystem::rename(temporary, target, error);
		if (!error)
		{
			if (hadTarget)
			{
				FileSystem::Remove(backupPath);
			}
			return true;
		}

		bool rollbackSucceeded = true;
		if (hadTarget)
		{
			std::error_code rollbackError;
			std::filesystem::rename(backup, target, rollbackError);
			rollbackSucceeded = !rollbackError;
		}

		statusText = rollbackSucceeded
			? U"Asset replace failed; original restored: {}"_fmt(targetPath)
			: U"Asset replace rollback failed; recover from {} and {}"_fmt(temporaryPath, backupPath);
		return false;
	}
}
