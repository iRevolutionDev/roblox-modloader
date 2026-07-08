#pragma once

#if defined(_WIN32)
	#define RML_WINDOWS
#elif defined(__linux__)
	#define RML_LINUX
#elif defined(__APPLE__)
	#define RML_MACOS
#endif

#include "RobloxModLoader/rml_export.hpp"
#include "range.hpp"

#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace rml::memory
{
	class RML_EXPORT module : public range
	{
	public:
		explicit module(std::string_view name);

		explicit module(std::filesystem::path path);

		module(const module&) = delete;
		module& operator=(const module&) = delete;
		module(module&&) = delete;
		module& operator=(module&&) = delete;
		~module() = default;
		[[nodiscard]] bool loaded() const noexcept;
		[[nodiscard]] size_t size() const noexcept;

		[[nodiscard]] const std::string& name() const noexcept
		{
			return m_name;
		}

		/// Full filesystem path (only set in by-path mode).
		[[nodiscard]] const std::optional<std::filesystem::path>& path() const noexcept
		{
			return m_path;
		}

		[[nodiscard]] handle get_export(std::string_view symbol_name) const;

		/// Spin-wait until the module appears in the process.
		bool wait_for_module(std::optional<std::chrono::steady_clock::duration> timeout = std::nullopt);

		/// Load the module into the process.
		///   by-name mode → LoadLibrary(name)   uses OS search order
		///   by-path mode → LoadLibrary(path)   loads the exact file
		[[nodiscard]] std::expected<void, std::string> attach();

		/// Release the reference acquired by attach().
		[[nodiscard]] std::expected<void, std::string> detach();

	private:
		bool try_get_module_locked();
		void reset_state_locked() noexcept;

#if defined(RML_LINUX)
		struct PhdrSearchCtx
		{
			const char* search_name = nullptr;
			std::uintptr_t found_base = 0;
			std::size_t found_size = 0;
		};
		static int phdr_callback(struct ::dl_phdr_info*, size_t, void*) noexcept;
#endif

		mutable std::mutex m_mtx;

		std::string m_name;                          // base name for lookups
		std::optional<std::filesystem::path> m_path; // full path (by-path mode)

		bool m_loaded = false;

		void* m_attached_handle = nullptr;
		bool m_attached_owner = false;
	};
}
