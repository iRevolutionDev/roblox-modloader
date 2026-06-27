#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace rml::config
{
	class RML_EXPORT ModSettings
	{
	public:
		explicit ModSettings(std::filesystem::path config_path);

		~ModSettings();

		ModSettings(const ModSettings&) = delete;
		ModSettings& operator=(const ModSettings&) = delete;

		ModSettings(ModSettings&&) noexcept;
		ModSettings& operator=(ModSettings&&) noexcept;

		bool load() const;
		[[nodiscard]] bool save() const;

		bool write_default_if_missing(std::string_view contents) const;

		[[nodiscard]] const std::filesystem::path& path() const noexcept;

		[[nodiscard]] bool exists_on_disk() const;

		[[nodiscard]] bool get_bool(std::string_view key, bool fallback = false) const;
		[[nodiscard]] std::int64_t get_int(std::string_view key, std::int64_t fallback = 0) const;
		[[nodiscard]] double get_double(std::string_view key, double fallback = 0.0) const;
		[[nodiscard]] std::string get_string(std::string_view key, std::string_view fallback = {}) const;

		[[nodiscard]] bool contains(std::string_view key) const;

		void set_bool(std::string_view key, bool value) const;
		void set_int(std::string_view key, std::int64_t value) const;
		void set_double(std::string_view key, double value) const;
		void set_string(std::string_view key, std::string_view value) const;

		void watch(std::function<void()> on_change);

		void stop_watching() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}
