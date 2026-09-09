#include "RobloxModLoader/memory/signature_cache.hpp"

#include "RobloxModLoader/internal/platform.hpp"
#include "RobloxModLoader/memory/batch.hpp"
#include "RobloxModLoader/memory/handle.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "filesystem/directory.hpp"

#if defined(RML_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
#endif

#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

RML_LOG_SCOPE("SigCache");

namespace rml::memory
{
	class SignatureCache
	{
	public:
		static bool run(std::span<const signature> entries, range region, std::uint32_t sigset_hash)
		{
			const std::uintptr_t base = region.begin().as<std::uintptr_t>();
			const ExeIdentity id = read_exe_identity(base);

			if (const auto cached = load();
			    cached && id.size_of_image != 0 && cached->sigset_hash == sigset_hash && cached->id == id)
			{
				if (apply(entries, base, id, *cached))
				{
					LOG_INFO("Applied {} signatures from cache (Studio PE ts=0x{:X})", entries.size(), id.pe_timestamp);
					return true;
				}
				LOG_WARN("Signature cache present but incomplete/out-of-range - rescanning");
			}
			else
			{
				LOG_INFO("No valid signature cache for this Studio build - scanning");
			}

			std::unordered_map<std::uint32_t, std::uint32_t> rvas;
			rvas.reserve(entries.size());
			const bool found_all = scan(entries, region, base, rvas);

			if (found_all && id.size_of_image != 0)
			{
				save(CacheData{id, sigset_hash, std::move(rvas)});
				LOG_INFO("Wrote signature cache for Studio PE ts=0x{:X} ({} signatures)", id.pe_timestamp, entries.size());
			}
			else if (!found_all)
			{
				LOG_WARN("Some signatures missing - cache not written (signatures need updating for this build)");
			}

			return found_all;
		}

	private:
		static constexpr std::uint32_t MAGIC = 0x434C4D52; // "RMLC"
		static constexpr std::uint32_t FORMAT = 1;
		static constexpr std::uint32_t MAX_ENTRIES = 100000;

		struct ExeIdentity
		{
			std::uint32_t pe_timestamp{};
			std::uint32_t size_of_image{};

			bool operator==(const ExeIdentity&) const = default;
		};

		struct CacheData
		{
			ExeIdentity id{};
			std::uint32_t sigset_hash{};
			std::unordered_map<std::uint32_t, std::uint32_t> rvas;
		};

		static ExeIdentity read_exe_identity(const std::uintptr_t base) noexcept
		{
			ExeIdentity id{};
			if (!base) return id;

#if defined(RML_WINDOWS)
			const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
			if (dos->e_magic != IMAGE_DOS_SIGNATURE) return id;

			const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE) return id;

			id.pe_timestamp = nt->FileHeader.TimeDateStamp;
			id.size_of_image = nt->OptionalHeader.SizeOfImage;
#endif
			return id;
		}

		static std::filesystem::path cache_path()
		{
			return filesystem::directory::get_mod_loader_directory() / "cache" / "signatures.bin";
		}

		static std::uint32_t name_hash(const signature& entry) noexcept
		{
			return signature_hasher::fnv1a_32(entry.m_name.c_str());
		}

		static std::optional<CacheData> load() noexcept
		{
			try
			{
				std::ifstream file(cache_path(), std::ios::binary);
				if (!file) return std::nullopt;

				const auto read = [&](auto& value) {
					file.read(reinterpret_cast<char*>(&value), sizeof(value));
				};

				std::uint32_t magic{}, format{}, count{};
				read(magic);
				read(format);
				if (!file || magic != MAGIC || format != FORMAT) return std::nullopt;

				CacheData data{};
				read(data.sigset_hash);
				read(data.id.pe_timestamp);
				read(data.id.size_of_image);
				read(count);
				if (!file || count > MAX_ENTRIES) return std::nullopt;

				data.rvas.reserve(count);
				for (std::uint32_t i = 0; i < count; ++i)
				{
					std::uint32_t nh{}, rva{};
					read(nh);
					read(rva);
					if (!file) return std::nullopt;
					data.rvas[nh] = rva;
				}
				return data;
			}
			catch (const std::exception& e)
			{
				LOG_WARN("Failed to read signature cache: {}", e.what());
				return std::nullopt;
			}
		}

		static void save(const CacheData& data) noexcept
		{
			try
			{
				const auto path = cache_path();
				std::error_code ec;
				std::filesystem::create_directories(path.parent_path(), ec);

				std::ofstream file(path, std::ios::binary | std::ios::trunc);
				if (!file)
				{
					LOG_WARN("Could not open signature cache for writing: {}", path.string());
					return;
				}

				const auto write = [&](auto value) {
					file.write(reinterpret_cast<const char*>(&value), sizeof(value));
				};

				write(MAGIC);
				write(FORMAT);
				write(data.sigset_hash);
				write(data.id.pe_timestamp);
				write(data.id.size_of_image);
				write(static_cast<std::uint32_t>(data.rvas.size()));
				for (const auto& [nh, rva] : data.rvas)
				{
					write(nh);
					write(rva);
				}
			}
			catch (const std::exception& e)
			{
				LOG_WARN("Failed to write signature cache: {}", e.what());
			}
		}

		static bool apply(std::span<const signature> entries, const std::uintptr_t base, const ExeIdentity& id,
		                  const CacheData& cached)
		{
			for (const auto& entry : entries)
			{
				const auto it = cached.rvas.find(name_hash(entry));
				if (it == cached.rvas.end() || it->second >= id.size_of_image)
					return false;

				if (entry.m_on_signature_found)
					entry.m_on_signature_found(handle(base + it->second));
			}
			return true;
		}

		static bool scan(std::span<const signature> entries, range region, const std::uintptr_t base,
		                 std::unordered_map<std::uint32_t, std::uint32_t>& out_rvas)
		{
			std::mutex mutex;
			std::vector<std::future<bool>> futures;
			futures.reserve(entries.size());

			for (const auto& entry : entries)
			{
				futures.emplace_back(std::async(std::launch::async, [&, entry]() -> bool {
					const auto result = region.scan(entry.m_ida.c_str());
					if (!result.has_value())
					{
						LOG_INFO("Failed to find '{}'.", entry.m_name.c_str());
						return false;
					}

					const auto rva = static_cast<std::uint32_t>(result.value().as<std::uintptr_t>() - base);

					std::lock_guard lock(mutex);
					if (entry.m_on_signature_found)
						entry.m_on_signature_found(result.value());
					out_rvas[name_hash(entry)] = rva;
					LOG_INFO("Found '{}' RobloxStudioBeta.exe+0x{:X}", entry.m_name.c_str(), rva);
					return true;
				}));
			}

			bool found_all = true;
			for (auto& future : futures)
			{
				future.wait();
				if (!future.get()) found_all = false;
			}
			return found_all;
		}
	};

	bool run_batch_cached(const std::span<const signature> entries, range region, const std::uint32_t sigset_hash)
	{
		return SignatureCache::run(entries, region, sigset_hash);
	}
}
