#pragma once

#include <memory>

namespace rml::exception_filter
{
	class ICrashHandler
	{
	public:
		virtual ~ICrashHandler() = default;

		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual void set_full_memory_dump(bool enabled) = 0;

		[[nodiscard]] virtual bool is_enabled() const = 0;
	};
	
	[[nodiscard]] std::unique_ptr<ICrashHandler> create_crash_handler();
}

inline rml::exception_filter::ICrashHandler* g_crash_dumper{};
