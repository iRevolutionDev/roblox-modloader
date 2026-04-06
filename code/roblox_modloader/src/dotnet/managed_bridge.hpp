#pragma once
#include "dotnet_runtime.hpp"
#include "interop_registry.hpp"

#include <filesystem>
#include <cstdint>

namespace rml::dotnet
{
	class ManagedBridge
	{
	public:
		explicit ManagedBridge(DotnetRuntime& runtime, InteropRegistry& registry) :
		    m_runtime(runtime),
		    m_registry(registry)
		{
		}

		[[nodiscard]] std::expected<void, std::string> initialize(const std::filesystem::path& native_host_dll, const std::filesystem::path& mods_root);

		[[nodiscard]] std::expected<void, std::string> load_mod(const std::filesystem::path& path) const;
		[[nodiscard]] std::expected<void, std::string> unload_mod(const std::filesystem::path& path) const;
		[[nodiscard]] std::expected<void, std::string> shutdown() const;

	private:
		using InitFn     = int32_t(__cdecl*)(const char* mods_root, InteropTable* table);
		using LoadModFn  = int32_t(__cdecl*)(const char* assembly_path);
		using UnloadFn   = int32_t(__cdecl*)(const char* assembly_path);
		using ShutdownFn = void(__cdecl*)();
		using NotifyDataModelFn = int32_t(__cdecl*)(uint64_t old_data_model, uint64_t new_data_model, int32_t data_model_type);

		DotnetRuntime& m_runtime;
		InteropRegistry& m_registry;

		InitFn m_initialize{};
		LoadModFn m_load_mod{};
		UnloadFn m_unload_mod{};
		ShutdownFn m_shutdown{};
		NotifyDataModelFn m_notify_data_model{};

public:
		[[nodiscard]] std::expected<void, std::string> notify_data_model_changed(uint64_t old_dm, uint64_t new_dm, int32_t dm_type) const;
	};

} // namespace rml::dotnet