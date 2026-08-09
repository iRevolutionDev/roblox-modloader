#include "descriptor_support.hpp"
#include "player_spoof.hpp"
#include "studio_service_spoof.hpp"

#include <RobloxModLoader/config/mod_settings.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/mod/mod_base.hpp>

#include <cstdint>
#include <memory>

namespace idspoofer
{
	constexpr std::int64_t kDefaultUserId = 1585524057LL;

	struct Settings
	{
		bool spoof_player{true};
		bool spoof_studio_service{true};
		std::int64_t player_user_id{kDefaultUserId};
		std::int64_t studio_service_user_id{kDefaultUserId};
	};

	[[nodiscard]] std::int64_t read_user_id(const rml::config::ModSettings& store,
	    const char* key, const std::int64_t fallback,
	    const std::shared_ptr<spdlog::logger>& logger)
	{
		const std::int64_t value = store.get_int(key, fallback);
		if (value > 0)
			return value;

		logger->warn("[idspoofer] '{}' must be a positive 64-bit integer; using {}", key, fallback);
		return fallback;
	}

	[[nodiscard]] Settings read_settings(const rml::config::ModSettings& store,
	    const std::shared_ptr<spdlog::logger>& logger)
	{
		Settings settings;
		settings.spoof_player = store.get_bool("spoof_player", settings.spoof_player);
		settings.spoof_studio_service =
		    store.get_bool("spoof_studio_service", settings.spoof_studio_service);
		settings.player_user_id = read_user_id(
		    store, "player_user_id", settings.player_user_id, logger);
		settings.studio_service_user_id = read_user_id(
		    store, "studio_service_user_id", settings.studio_service_user_id, logger);
		return settings;
	}
}

class IdSpoofer final : public ModBase
{
public:
	IdSpoofer()
	{
		name = "idspoofer";
		version = "1.0.0";
		author = "rml";
		description = "Configurable native Player.UserId and StudioService:GetUserId descriptor spoof";
		m_logger = rml::Logger::get_logger("IdSpoofer");
	}

	void on_load() override
	{
		m_store = std::make_unique<rml::config::ModSettings>(paths().file("mod.toml"));
		if (!m_store->load())
		{
			m_logger->warn("[idspoofer] could not read '{}'; using built-in defaults",
			    m_store->path().string());
		}

		const idspoofer::Settings settings = idspoofer::read_settings(*m_store, m_logger);
		m_logger->info(
		    "[idspoofer] settings: spoof_player={} player_user_id={} spoof_studio_service={} studio_service_user_id={}",
		    settings.spoof_player, settings.player_user_id,
		    settings.spoof_studio_service, settings.studio_service_user_id);

		if (!settings.spoof_player && !settings.spoof_studio_service)
		{
			m_logger->info("[idspoofer] both spoof targets are disabled; no descriptor changes made");
			return;
		}

		if (!idspoofer::detail::initialize_engine_api())
		{
			m_logger->error("[idspoofer] reflection API unavailable; enabled spoofs were not installed");
			return;
		}

		if (settings.spoof_player
		    && !idspoofer::player::install(settings.player_user_id))
		{
			m_logger->error("[idspoofer] Player.UserId spoof was not installed");
		}

		if (settings.spoof_studio_service
		    && !idspoofer::studio_service::install(settings.studio_service_user_id))
		{
			m_logger->error("[idspoofer] StudioService.GetUserId spoof was not installed");
		}
	}

	void on_unload() override
	{
		idspoofer::studio_service::uninstall();
		idspoofer::player::uninstall();
		m_store.reset();
	}

private:
	std::shared_ptr<spdlog::logger> m_logger;
	std::unique_ptr<rml::config::ModSettings> m_store;
};

extern "C"
{
	RML_MOD_ABI_EXPORT ModBase* start_mod() { return new IdSpoofer(); }
	RML_MOD_ABI_EXPORT void uninstall_mod(const ModBase* mod) { delete mod; }
}

RML_EXPORT_MOD_ABI_VERSION()
