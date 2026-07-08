#pragma once

#include "../isubsystem.hpp"
#include "config/config.hpp"
#include "utils/directory.hpp"

namespace rml
{
	class ConfigSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			const auto config_path = utils::directory::get_mod_loader_directory() / "config.toml";
			if (const auto config_result = config::initialize(config_path, true); !config_result)
			{
				return std::unexpected(SubsystemError{std::string(name()), "Failed to initialize configuration system"});
			}

			return {};
		}

		void shutdown() override
		{
			config::shutdown();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Config";
		}
	};
}
