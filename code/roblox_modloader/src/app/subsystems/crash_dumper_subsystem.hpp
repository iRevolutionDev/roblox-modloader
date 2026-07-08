#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/exception/crash_dumper.hpp"

namespace rml
{
	class CrashDumperSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<exception_filter::CrashDumper>();
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
		std::unique_ptr<exception_filter::CrashDumper> m_instance;
	};
}
