#pragma once

#include "../isubsystem.hpp"
#include "mod/mod_manager.hpp"
#include "event_manager_subsystem.hpp"

namespace rml
{
	class ModManagerSubsystem final : public ISubsystem
	{
	public:
		explicit ModManagerSubsystem(EventManagerSubsystem& event_manager_subsystem) :
		    m_event_manager_subsystem(event_manager_subsystem)
		{
		}

		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<ModManager>();

			if (const auto result = m_instance->initialize(m_event_manager_subsystem.event_manager()); !result)
			{
				return std::unexpected(SubsystemError{std::string(name()), result.error().message});
			}

			return {};
		}

		void shutdown() override
		{
			if (m_instance)
			{
				m_instance->shutdown();
			}

			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "ModManager";
		}

	private:
		EventManagerSubsystem& m_event_manager_subsystem;
		std::unique_ptr<ModManager> m_instance;
	};
}
