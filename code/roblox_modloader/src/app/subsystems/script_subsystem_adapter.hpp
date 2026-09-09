#pragma once

#include "../isubsystem.hpp"
#include "script_subsystem.hpp"

namespace rml
{
	class ScriptSubsystemAdapter final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<ScriptSubsystem>();

			if (auto started = m_instance->initialize(); !started)
			{
				return std::unexpected(SubsystemError{std::string{name()}, std::move(started.error())});
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
			return "ScriptSubsystem";
		}

	private:
		std::unique_ptr<ScriptSubsystem> m_instance;
	};
}
