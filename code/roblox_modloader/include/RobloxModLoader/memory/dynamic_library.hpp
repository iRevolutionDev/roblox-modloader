#pragma once

#include "RobloxModLoader/common.hpp"

struct DynamicLibrary
{
#if defined(_WIN32)
	HMODULE handle{nullptr};
#else
	void* handle{nullptr};
#endif

	DynamicLibrary()                                 = default;
	DynamicLibrary(const DynamicLibrary&)            = delete;
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
#if defined(_WIN32)
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

#if defined(_WIN32)
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

#if defined(_WIN32)
		FreeLibrary(handle);
#else
		dlclose(handle);
#endif
		handle = nullptr;
	}
};