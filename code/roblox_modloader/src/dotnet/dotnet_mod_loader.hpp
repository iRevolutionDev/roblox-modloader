#pragma once
#include "dotnet_runtime.hpp"
#include "interop_registry.hpp"
#include "managed_bridge.hpp"
#include "mod/imod_loader.hpp"

#include <cstdint>

namespace rml::dotnet
{
	class DotnetModLoader final : public IModLoader
	{
	public:
		explicit DotnetModLoader(const std::filesystem::path& runtime_path, const std::filesystem::path& mods_root);

		void notify_data_model_changed(uint64_t old_dm, uint64_t new_dm, int32_t dm_type) const;

		[[nodiscard]] std::expected<void, std::string> load(const std::filesystem::path& path) override;
		[[nodiscard]] std::expected<void, std::string> unload(const std::filesystem::path& path) override;
		[[nodiscard]] std::expected<void, std::string> reload(const std::filesystem::path& path) override;
		[[nodiscard]] std::vector<std::filesystem::path> extensions() const override
		{
			return {".dll"};
		}

		void unload_all() override;

	private:
		[[nodiscard]] std::expected<void, std::string> ensure_initialized();

		DotnetRuntime m_runtime;
		InteropRegistry m_registry;
		ManagedBridge m_bridge;

		std::filesystem::path m_native_host_dll;
		std::filesystem::path m_runtime_config;
		std::filesystem::path m_mods_root;
		bool m_initialized{false};
	};

	inline DotnetModLoader* g_dotnet_mod_loader{};
}