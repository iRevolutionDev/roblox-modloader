#include "RobloxModLoader/config/mod_settings.hpp"

#include "RobloxModLoader/internal/common.hpp"

#include "utils/file_watcher.hpp"

#include <chrono>
#include <fstream>
#include <shared_mutex>
#include <toml++/toml.hpp>
#include <utility>

namespace rml::config
{
	using namespace std::chrono_literals;

	struct ModSettings::Impl
	{
		explicit Impl(std::filesystem::path path) :
		    config_path(std::move(path))
		{
		}

		std::filesystem::path config_path;

		mutable std::shared_mutex mutex;
		toml::table table;

		std::function<void()> on_change;
		utils::FileWatcher watcher;
	};

	ModSettings::ModSettings(std::filesystem::path config_path) :
	    m_impl(std::make_unique<Impl>(std::move(config_path)))
	{
	}

	ModSettings::~ModSettings()
	{
		stop_watching();
	}

	ModSettings::ModSettings(ModSettings&&) noexcept = default;

	ModSettings& ModSettings::operator=(ModSettings&& other) noexcept
	{
		if (this != &other)
		{
			stop_watching();
			m_impl = std::move(other.m_impl);
		}
		return *this;
	}

	bool ModSettings::load() const
	{
		auto result = toml::parse_file(m_impl->config_path.string());
		if (!result)
			return false;

		std::unique_lock lock(m_impl->mutex);
		m_impl->table = std::move(result).table();
		m_impl->watcher.acknowledge(utils::read_last_write_time(m_impl->config_path));
		return true;
	}

	bool ModSettings::save() const
	{
		try
		{
			if (const auto parent = m_impl->config_path.parent_path(); !parent.empty())
			{
				std::error_code ec;
				std::filesystem::create_directories(parent, ec);
			}

			{
				std::shared_lock lock(m_impl->mutex);

				std::ofstream file(m_impl->config_path, std::ios::trunc);
				if (!file.is_open())
					return false;

				file << m_impl->table;
				if (!file.good())
					return false;
			}

			m_impl->watcher.acknowledge(utils::read_last_write_time(m_impl->config_path));
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	bool ModSettings::write_default_if_missing(const std::string_view contents) const
	{
		if (exists_on_disk())
			return false;

		try
		{
			if (const auto parent = m_impl->config_path.parent_path(); !parent.empty())
			{
				std::error_code ec;
				std::filesystem::create_directories(parent, ec);
			}

			std::ofstream file(m_impl->config_path, std::ios::trunc);
			if (!file.is_open())
				return false;

			file << contents;
			return file.good();
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	const std::filesystem::path& ModSettings::path() const noexcept
	{
		return m_impl->config_path;
	}

	bool ModSettings::exists_on_disk() const
	{
		std::error_code ec;
		return std::filesystem::exists(m_impl->config_path, ec);
	}

	bool ModSettings::get_bool(const std::string_view key, const bool fallback) const
	{
		std::shared_lock lock(m_impl->mutex);
		return m_impl->table[key].value<bool>().value_or(fallback);
	}

	std::int64_t ModSettings::get_int(const std::string_view key, const std::int64_t fallback) const
	{
		std::shared_lock lock(m_impl->mutex);
		return m_impl->table[key].value<std::int64_t>().value_or(fallback);
	}

	double ModSettings::get_double(const std::string_view key, const double fallback) const
	{
		std::shared_lock lock(m_impl->mutex);
		if (const auto as_double = m_impl->table[key].value<double>())
			return *as_double;
		if (const auto as_int = m_impl->table[key].value<std::int64_t>())
			return static_cast<double>(*as_int);
		return fallback;
	}

	std::string ModSettings::get_string(const std::string_view key, const std::string_view fallback) const
	{
		std::shared_lock lock(m_impl->mutex);
		return m_impl->table[key].value<std::string>().value_or(std::string(fallback));
	}

	bool ModSettings::contains(const std::string_view key) const
	{
		std::shared_lock lock(m_impl->mutex);
		return m_impl->table.contains(key);
	}

	void ModSettings::set_bool(const std::string_view key, const bool value) const
	{
		std::unique_lock lock(m_impl->mutex);
		m_impl->table.insert_or_assign(key, value);
	}

	void ModSettings::set_int(const std::string_view key, const std::int64_t value) const
	{
		std::unique_lock lock(m_impl->mutex);
		m_impl->table.insert_or_assign(key, value);
	}

	void ModSettings::set_double(const std::string_view key, const double value) const
	{
		std::unique_lock lock(m_impl->mutex);
		m_impl->table.insert_or_assign(key, value);
	}

	void ModSettings::set_string(const std::string_view key, const std::string_view value) const
	{
		std::unique_lock lock(m_impl->mutex);
		m_impl->table.insert_or_assign(key, std::string(value));
	}

	void ModSettings::watch(std::function<void()> on_change)
	{
		stop_watching();

		{
			std::unique_lock lock(m_impl->mutex);
			m_impl->on_change = std::move(on_change);
		}

		m_impl->watcher.start(m_impl->config_path, 1s, [this] {
			if (!load())
				return false;

			std::function<void()> callback;
			{
				std::shared_lock lock(m_impl->mutex);
				callback = m_impl->on_change;
			}
			if (callback)
				callback();

			return true;
		});
	}

	void ModSettings::stop_watching() const
	{
		if (!m_impl)
			return;

		m_impl->watcher.stop();

		std::unique_lock lock(m_impl->mutex);
		m_impl->on_change = nullptr;
	}
}
