#pragma once

#include "RobloxModLoader/internal/platform.hpp"

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#if defined(RML_WINDOWS)
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif

#include <filesystem>
#include <utility>

namespace rml::memory
{
	struct DynamicLibrary
	{
#if defined(RML_WINDOWS)
		HMODULE handle{nullptr};
#else
		void* handle{nullptr};
#endif

		DynamicLibrary() = default;
		DynamicLibrary(const DynamicLibrary&) = delete;
		DynamicLibrary& operator=(const DynamicLibrary&) = delete;

		DynamicLibrary(DynamicLibrary&& other) noexcept
		{
			handle = std::exchange(other.handle, nullptr);
		}

		DynamicLibrary& operator=(DynamicLibrary&& other) noexcept
		{
			if (this != &other)
			{
				unload();
				handle = std::exchange(other.handle, nullptr);
			}
			return *this;
		}

		~DynamicLibrary()
		{
			unload();
		}

		[[nodiscard]] bool load(const std::filesystem::path& path)
		{
#if defined(RML_WINDOWS)
			handle = LoadLibraryW(path.c_str());
#else
			handle = dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif
			return handle != nullptr;
		}

		[[nodiscard]] void* symbol(const char* name) const
		{
			if (!handle)
			{
				return nullptr;
			}

#if defined(RML_WINDOWS)
			return reinterpret_cast<void*>(GetProcAddress(handle, name));
#else
			return dlsym(handle, name);
#endif
		}

		void unload()
		{
			if (!handle)
			{
				return;
			}

#if defined(RML_WINDOWS)
			FreeLibrary(handle);
#else
			dlclose(handle);
#endif
			handle = nullptr;
		}
	};
}