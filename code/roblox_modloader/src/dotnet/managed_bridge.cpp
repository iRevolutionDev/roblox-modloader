#include "managed_bridge.hpp"

#include "RobloxModLoader/common.hpp"

namespace rml::dotnet
{
	namespace
	{
		constexpr std::wstring_view k_type     = L"RML.NativeHost.NativeHost, RML.NativeHost";
		constexpr std::wstring_view k_init     = L"Initialize";
		constexpr std::wstring_view k_load     = L"LoadMod";
		constexpr std::wstring_view k_unload   = L"UnloadMod";
		constexpr std::wstring_view k_shutdown = L"Shutdown";
	}

	std::expected<void, std::string> ManagedBridge::initialize(const std::filesystem::path& native_host_dll, const std::filesystem::path& mods_root)
	{
		auto init = m_runtime.get_function<InitFn>(native_host_dll, k_type, k_init);
		if (!init)
			return std::unexpected(init.error());

		auto load = m_runtime.get_function<LoadModFn>(native_host_dll, k_type, k_load);
		if (!load)
			return std::unexpected(load.error());

		auto unload = m_runtime.get_function<UnloadFn>(native_host_dll, k_type, k_unload);
		if (!unload)
			return std::unexpected(unload.error());

		auto shutdown = m_runtime.get_function<ShutdownFn>(native_host_dll, k_type, k_shutdown);
		if (!shutdown)
			return std::unexpected(shutdown.error());

		m_initialize = *init;
		m_load_mod   = *load;
		m_unload_mod = *unload;
		m_shutdown = *shutdown;

		auto* table     = m_registry.table();
		const auto root = mods_root.string();

		if (int32_t rc = m_initialize(root.c_str(), table); rc != 0)
			return std::unexpected(std::format("rml_initialize returned {}", rc));

		LOG_INFO("[ManagedBridge] C# side initialized");
		return {};
	}

	std::expected<void, std::string> ManagedBridge::load_mod(const std::filesystem::path& path) const
	{
		if (!m_load_mod)
			return std::unexpected("Bridge not initialized");

		const auto p = path.string();
		if (int32_t rc = m_load_mod(p.c_str()); rc != 0)
			return std::unexpected(std::format("rml_load_mod failed for '{}' rc={}", path.filename().string(), rc));
		return {};
	}

	std::expected<void, std::string> ManagedBridge::unload_mod(const std::filesystem::path& path) const
	{
		const auto p = path.string();

		m_unload_mod(p.c_str());

		return {};
	}

	std::expected<void, std::string> ManagedBridge::shutdown() const
	{
		if (m_shutdown)
			m_shutdown();
		return {};
	}
} // namespace rml::dotnet
