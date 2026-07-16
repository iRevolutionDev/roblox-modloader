#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/exception/i_crash_handler.hpp"

namespace rml
{
	class CrashDumperSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = exception_filter::create_crash_handler();
			m_instance->enable();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "CrashDumper";
		}

	private:
		std::unique_ptr<exception_filter::ICrashHandler> m_instance;
	};
}
