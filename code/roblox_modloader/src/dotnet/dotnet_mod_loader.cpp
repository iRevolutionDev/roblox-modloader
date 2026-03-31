#include "dotnet_mod_loader.hpp"

#include "RobloxModLoader/common.hpp"
#include "roblox_interop_provider.hpp"

namespace rml::dotnet
{

	DotnetModLoader::DotnetModLoader(const std::filesystem::path& runtime_path, const std::filesystem::path& mods_root) :
	    m_bridge(m_runtime, m_registry),
	    m_native_host_dll(runtime_path / "RML.NativeHost.dll"),
	    m_runtime_config(runtime_path / "RML.runtimeconfig.json"),
	    m_mods_root(mods_root)
	{
	}

	std::expected<void, std::string> DotnetModLoader::ensure_initialized()
	{
		if (m_initialized)
			return {};

		RobloxInteropProvider::populate(*m_registry.table());

		if (auto r = m_runtime.initialize(m_runtime_config); !r)
			return r;

		if (auto r = m_bridge.initialize(m_native_host_dll, m_mods_root); !r)
			return r;

		m_initialized = true;
		return {};
	}

	std::expected<void, std::string> DotnetModLoader::load(const std::filesystem::path& path)
	{
		if (auto r = ensure_initialized(); !r)
			return r;
		return m_bridge.load_mod(path);
	}

	std::expected<void, std::string> DotnetModLoader::unload(const std::filesystem::path& path)
	{
		if (!m_initialized)
			return {};
		return m_bridge.unload_mod(path);
	}

	std::expected<void, std::string> DotnetModLoader::reload(const std::filesystem::path& path)
	{
		if (auto r = unload(path); !r)
			return r;
		return load(path);
	}

	void DotnetModLoader::unload_all()
	{
		if (!m_initialized)
			return;

		m_bridge.shutdown();
	}
} // namespace rml::dotnet