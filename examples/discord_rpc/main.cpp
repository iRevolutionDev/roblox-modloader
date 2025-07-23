#include <RobloxModLoader/common.hpp>
#include <RobloxModLoader/mod/mod_base.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/luau/environment/bridge_provider.hpp>
#include "discord_sdk.hpp"

#include <spdlog/spdlog.h>
#include <chrono>
#include <memory>

using namespace rml::luau::environment;

class discord_rpc final : public mod_base {
    std::shared_ptr<spdlog::logger> logger;
    std::unique_ptr<discord_integration::DiscordGameSDK> discord_sdk;

    std::chrono::steady_clock::time_point start_time;

public:
    discord_rpc() {
        name = "Discord Game SDK Mod";
        version = "1.0.0";
        author = "Revolution";
        description = "Discord integration using Game SDK for Roblox Studio with rich presence.";

        logger = logger::get_logger("DiscordGameSDKMod");
        discord_sdk = std::make_unique<discord_integration::DiscordGameSDK>(this->logger);
        start_time = std::chrono::steady_clock::now();
    }

    void on_load() override {
        logger->info("Initializing Discord Game SDK...");
        logger->info("Using RobloxModLoader header-only interface");

        if (!discord_sdk->initialize(DISCORD_CLIENT_ID)) {
            logger->error("Failed to initialize Discord Game SDK");
            return;
        }

        setup_default_activity();

        logger->info("Discord Game SDK mod loaded successfully");
    }

    void on_script_manager_load() override {
        setup_luau_communication();
    }

    void on_unload() override {
        logger->info("Shutting down Discord Game SDK...");

        if (discord_sdk) {
            discord_sdk->clear_activity();
            discord_sdk->shutdown();
        }

        logger->info("Discord Game SDK mod unloaded");
    }

private:
    void setup_luau_communication() const {
        const auto &bridge = BridgeProvider::instance();

        bridge->register_native_function(
                    "discord_rpc", "update_activity",
                    [this](const LuaParameters &params) -> LuaParameters {
                        if (const auto table = std::get_if<LuaTable>(&params[0]);
                            table && table->has_key("state") && table->has_key("details")) {
                            const auto state = table->get<std::string>("state", "Developing in Studio");
                            const auto details = table->get<std::string>("details", "Building experiences");
                            const auto large_image = table->get<std::string>("large_image");
                            const auto large_image_text = table->get<std::string>("large_image_text", "Roblox Studio");
                            const auto small_image = table->get<std::string>("small_image");
                            const auto small_image_text = table->get<std::string>("small_image_text", "With ModLoader");
                            const auto start_time_str = table->get<std::string>("start_timestamp", "now");
                            const auto type_str = table->get<std::string>("type", "Playing");

                            std::chrono::steady_clock::time_point start_time;
                            if (start_time_str == "now") {
                                start_time = std::chrono::steady_clock::now();
                            } else {
                                try {
                                    start_time = std::chrono::steady_clock::time_point(
                                        std::chrono::seconds(std::stoll(start_time_str))
                                    );
                                } catch (...) {
                                    logger->warn("Invalid start_time value, using current time");
                                    start_time = std::chrono::steady_clock::now();
                                }
                            }

                            discord_integration::ActivityDetails activity;
                            activity.state = state;
                            activity.details = details;

                            activity.large_image_key = large_image.empty()
                                                           ? "studio"
                                                           : large_image;
                            activity.large_image_text = large_image_text.empty() ? "Roblox Studio" : large_image_text;

                            activity.small_image_key = small_image.empty() ? "modloader_icon" : small_image;
                            activity.small_image_text = small_image_text.empty() ? "With ModLoader" : small_image_text;

                            activity.start_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                start_time.time_since_epoch()
                            ).count();

                            activity.type = discord_integration::activity_from_string(type_str);

                            discord_sdk->update_activity(activity);
                        }

                        return {false};
                    })
                .transform_error([this](const std::string &error) {
                    logger->error("Failed to update Discord activity: {}", error);
                    return LuaParameters{false};
                });

        bridge->register_native_function(
                    "discord_rpc", "clear_activity",
                    [this](const LuaParameters &params) -> LuaParameters {
                        if (params.size() != 0) {
                            return {false};
                        }

                        if (discord_sdk && discord_sdk->is_initialized()) {
                            discord_sdk->clear_activity();
                            logger->info("Cleared Discord activity");
                            return {true};
                        }

                        logger->warn("Discord SDK not initialized, cannot clear activity");
                        return {false};
                    })
                .transform_error([this](const std::string &error) {
                    logger->error("Failed to clear Discord activity: {}", error);
                    return LuaParameters{false};
                });
    }

    void setup_default_activity() const {
        if (!discord_sdk || !discord_sdk->is_initialized()) {
            return;
        }

        discord_integration::ActivityDetails activity;
        activity.state = "In menu";
        activity.details = "Building amazing games";
        activity.large_image_key = "studio";
        activity.large_image_text = "Roblox Studio";

        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            start_time.time_since_epoch()
        ).count();
        activity.start_timestamp = now;

        activity.type = discord_integration::ActivityType::Playing;

        discord_sdk->update_activity(activity);
        logger->info("Updated Discord activity for Roblox Studio");
    }
};


#define DISCORD_RPC_MOD_API __declspec(dllexport)

extern "C" {
DISCORD_RPC_MOD_API mod_base *start_mod() {
    return new discord_rpc();
}

DISCORD_RPC_MOD_API void uninstall_mod(const mod_base *mod) {
    delete mod;
}
}
