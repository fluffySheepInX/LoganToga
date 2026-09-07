#pragma once
# include <Siv3D.hpp>
# include <filesystem>

namespace LT3
{
	struct AudioAssetCacheEntry
	{
		Audio audio;
		std::filesystem::file_time_type lastWriteTime{};
		double lastUsedTimeSec = 0.0;
	};

	struct AudioAssetCache
	{
		static constexpr size_t DefaultCapacity = 32;

		size_t capacity = DefaultCapacity;
		HashTable<FilePath, AudioAssetCacheEntry> entries;

		void clear()
		{
			for (auto& [path, entry] : entries)
			{
				(void)path;
				entry.audio.stop();
			}
			entries.clear();
		}

		Audio* findOrLoad(const FilePath& requestedPath)
		{
			const Optional<FilePath> canonicalPath = CanonicalizeExistingAudioAssetPath(requestedPath);
			if (!canonicalPath)
			{
				return nullptr;
			}

			const std::filesystem::path nativePath{ canonicalPath->toWstr() };
			std::error_code error;
			const std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(nativePath, error);
			if (error)
			{
				return nullptr;
			}

			const double nowSec = Scene::Time();
			if (auto it = entries.find(*canonicalPath); it != entries.end())
			{
				AudioAssetCacheEntry& entry = it->second;
				if (entry.lastWriteTime != lastWriteTime)
				{
					entry.audio.stop();
					entry.audio = Audio{ *canonicalPath };
					entry.lastWriteTime = lastWriteTime;
				}
				entry.lastUsedTimeSec = nowSec;
				return entry.audio ? &entry.audio : nullptr;
			}

			if (!evictOneUnusedEntry())
			{
				return nullptr;
			}

			AudioAssetCacheEntry entry;
			entry.audio = Audio{ *canonicalPath };
			entry.lastWriteTime = lastWriteTime;
			entry.lastUsedTimeSec = nowSec;
			if (!entry.audio)
			{
				return nullptr;
			}

			return &entries.emplace(*canonicalPath, std::move(entry)).first->second.audio;
		}

	private:
		static Optional<FilePath> CanonicalizeExistingAudioAssetPath(const FilePath& requestedPath)
		{
			if (requestedPath.isEmpty())
			{
				return none;
			}

			std::error_code error;
			const std::filesystem::path canonicalPath = std::filesystem::canonical(std::filesystem::path{ requestedPath.toWstr() }, error);
			if (error || !std::filesystem::is_regular_file(canonicalPath, error) || error)
			{
				return none;
			}

			return Unicode::FromWstring(canonicalPath.wstring());
		}

		bool evictOneUnusedEntry()
		{
			if (entries.size() < capacity)
			{
				return true;
			}

			auto oldest = entries.end();
			for (auto it = entries.begin(); it != entries.end(); ++it)
			{
				if (it->second.audio.isPlaying())
				{
					continue;
				}
				if (oldest == entries.end() || it->second.lastUsedTimeSec < oldest->second.lastUsedTimeSec)
				{
					oldest = it;
				}
			}

			if (oldest == entries.end())
			{
				return false;
			}

			oldest->second.audio.stop();
			entries.erase(oldest);
			return true;
		}
	};
}
