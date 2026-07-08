#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/mod/events.hpp"

namespace rml
{
	class EventManagerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<events::EventManager>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "EventManager";
		}

		[[nodiscard]] events::EventManager& event_manager() const
		{
			return *m_instance;
		}

	private:
		std::unique_ptr<events::EventManager> m_instance;
	};
}
