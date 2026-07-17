#pragma once

#include "RobloxModLoader/exception/i_crash_handler.hpp"

#include <array>
#include <csignal>

namespace rml::exception_filter
{
	class SignalCrashHandler final : public ICrashHandler
	{
	public:
		SignalCrashHandler();
		~SignalCrashHandler() override;

		SignalCrashHandler(const SignalCrashHandler&) = delete;
		SignalCrashHandler& operator=(const SignalCrashHandler&) = delete;
		SignalCrashHandler(SignalCrashHandler&&) = delete;
		SignalCrashHandler& operator=(SignalCrashHandler&&) = delete;

		void enable() override;
		void disable() override;
		void set_full_memory_dump(bool enabled) override;

		[[nodiscard]] bool is_enabled() const override
		{
			return m_enabled;
		}

	private:
		static constexpr std::array<int, 6> k_signals{SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGTRAP};

		static void handle_signal(int signal, siginfo_t* info, void* context);

		bool m_enabled = false;
		bool m_full_memory_dump = false;
		std::array<struct sigaction, k_signals.size()> m_previous{};
	};
}
