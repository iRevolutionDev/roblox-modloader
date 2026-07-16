#include "signal_crash_handler.hpp"

#include "RobloxModLoader/internal/common.hpp"

#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>

RML_LOG_SCOPE("SignalCrashHandler");

namespace rml::exception_filter
{
	static SignalCrashHandler* s_instance = nullptr;

	static const char* signal_name(const int signal)
	{
		switch (signal)
		{
		case SIGSEGV:
			return "SIGSEGV (segmentation fault)";
		case SIGBUS:
			return "SIGBUS (bus error)";
		case SIGILL:
			return "SIGILL (illegal instruction)";
		case SIGFPE:
			return "SIGFPE (floating-point exception)";
		case SIGABRT:
			return "SIGABRT (abort)";
		case SIGTRAP:
			return "SIGTRAP (trap)";
		default:
			return "unknown signal";
		}
	}

	static void write_raw(const int fd, const char* text)
	{
		(void)::write(fd, text, std::strlen(text));
	}

	static void write_hex(const int fd, const std::uintptr_t value)
	{
		char buffer[2 + sizeof(value) * 2];
		buffer[0] = '0';
		buffer[1] = 'x';
		for (std::size_t i = 0; i < sizeof(value) * 2; ++i)
		{
			const auto nibble = static_cast<unsigned>((value >> ((sizeof(value) * 2 - 1 - i) * 4)) & 0xF);
			buffer[2 + i] = static_cast<char>(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
		}
		(void)::write(fd, buffer, sizeof(buffer));
	}

	static void write_report(const int fd, const int signal, siginfo_t* info, void* const* frames, const int frame_count)
	{
		write_raw(fd, "=== RobloxModLoader crash ===\nSignal: ");
		write_raw(fd, signal_name(signal));
		write_raw(fd, "\nFault address: ");
		write_hex(fd, reinterpret_cast<std::uintptr_t>(info ? info->si_addr : nullptr));
		write_raw(fd, "\nBacktrace:\n");
		backtrace_symbols_fd(frames, frame_count, fd);
		write_raw(fd, "=== end crash ===\n");
	}

	SignalCrashHandler::SignalCrashHandler()
	{
		s_instance = this;
		g_crash_dumper = this;
	}

	SignalCrashHandler::~SignalCrashHandler()
	{
		disable();
		g_crash_dumper = nullptr;
		s_instance = nullptr;
	}

	void SignalCrashHandler::enable()
	{
		if (m_enabled)
			return;

		struct sigaction action{};
		action.sa_sigaction = &SignalCrashHandler::handle_signal;
		action.sa_flags = SA_SIGINFO | SA_ONSTACK;
		sigemptyset(&action.sa_mask);

		for (std::size_t i = 0; i < k_signals.size(); ++i)
			sigaction(k_signals[i], &action, &m_previous[i]);

		m_enabled = true;
		RML_INFO("Signal crash handler installed");
	}

	void SignalCrashHandler::disable()
	{
		if (!m_enabled)
			return;

		for (std::size_t i = 0; i < k_signals.size(); ++i)
			sigaction(k_signals[i], &m_previous[i], nullptr);

		m_enabled = false;
	}

	void SignalCrashHandler::set_full_memory_dump(const bool enabled)
	{
		m_full_memory_dump = enabled;
	}

	void SignalCrashHandler::handle_signal(const int signal, siginfo_t* info, void*)
	{
		void* frames[128];
		const int frame_count = backtrace(frames, 128);

		if (const int fd = ::open("roblox_modloader_crash.log", O_WRONLY | O_CREAT | O_TRUNC, 0644); fd >= 0)
		{
			write_report(fd, signal, info, frames, frame_count);
			::close(fd);
		}

		write_report(STDERR_FILENO, signal, info, frames, frame_count);
		
		if (s_instance)
		{
			for (std::size_t i = 0; i < k_signals.size(); ++i)
			{
				if (k_signals[i] == signal)
				{
					sigaction(signal, &s_instance->m_previous[i], nullptr);
					break;
				}
			}
		}

		raise(signal);
	}

	std::unique_ptr<ICrashHandler> create_crash_handler()
	{
		return std::make_unique<SignalCrashHandler>();
	}
}
