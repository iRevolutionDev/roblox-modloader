#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml
{
	class EntryPoint
	{
	public:
		using Driver = void (*)();

		static void attach(void* module_handle, Driver driver) noexcept;

		template<typename App>
		static void run() noexcept
		{
			try
			{
				App application;

				if (const auto result = application.initialize(); !result)
				{
					std::cerr << "[RML] Fatal error during bootstrap: " << result.error().subsystem_name << ": " << result.error().message << std::endl;
					application.shutdown();
				}
				else
				{
					application.run();
					application.shutdown();
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "[RML] Fatal error during bootstrap: " << e.what() << std::endl;
			}
			catch (...)
			{
				std::cerr << "[RML] Fatal error during bootstrap (unknown exception)" << std::endl;
			}
		}
	};
}

#if defined(RML_WINDOWS)
	#define RML_RUN_LOADER(App)                                             \
		BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)         \
		{                                                                   \
			if (reason == DLL_PROCESS_ATTACH)                               \
				::rml::EntryPoint::attach(module, &::rml::EntryPoint::run<App>); \
			return TRUE;                                                    \
		}
#else
	#define RML_RUN_LOADER(App)                                             \
		__attribute__((constructor)) static void rml_loader_entry()         \
		{                                                                   \
			::rml::EntryPoint::attach(reinterpret_cast<void*>(&rml_loader_entry), &::rml::EntryPoint::run<App>); \
		}
#endif
