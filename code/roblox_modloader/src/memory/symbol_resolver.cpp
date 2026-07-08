#include "RobloxModLoader/memory/symbol_resolver.hpp"

#if defined(RML_WINDOWS)
	#include <dbghelp.h>
	#pragma comment(lib, "dbghelp.lib")
#else
	#include <cstdlib>
	#include <cxxabi.h>
#endif

namespace rml::memory
{
	std::string demangle(const char* mangled)
	{
		if (!mangled || !*mangled)
		{
			return {};
		}

#if defined(RML_WINDOWS)
		if (mangled[0] != '?')
		{
			return mangled;
		}

		char buffer[2048];
		constexpr DWORD flags = UNDNAME_NAME_ONLY | UNDNAME_NO_ARGUMENTS | UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_LEADING_UNDERSCORES;
		const DWORD written = UnDecorateSymbolName(mangled, buffer, sizeof(buffer), flags);
		return written != 0 ? std::string{buffer, written} : std::string{};
#else
		if (mangled[0] != '_' || mangled[1] != 'Z')
		{
			return mangled;
		}

		int status = 0;
		char* result = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
		if (status != 0 || !result)
		{
			std::free(result);
			return {};
		}

		std::string demangled{result};
		std::free(result);

		if (const auto paren = demangled.find('('); paren != std::string::npos)
		{
			demangled.resize(paren);
		}
		return demangled;
#endif
	}

	void* resolve_export_exact(void* module, const char* mangled_name)
	{
		if (!module || !mangled_name)
		{
			return nullptr;
		}
#if defined(RML_WINDOWS)
		return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(module), mangled_name));
#else
		return dlsym(module, mangled_name);
#endif
	}

	void* loaded_module(std::initializer_list<std::string_view> names)
	{
		for (const auto name : names)
		{
			const std::string n{name};
#if defined(RML_WINDOWS)
			if (HMODULE module = GetModuleHandleA(n.c_str()))
			{
				return module;
			}
#else
			if (void* handle = dlopen(n.c_str(), RTLD_NOLOAD | RTLD_NOW))
			{
				return handle;
			}
#endif
		}
		return nullptr;
	}
}
