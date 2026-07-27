#include "dotnet_runtime.hpp"

#include "RobloxModLoader/internal/platform.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "utils/directory.hpp"

#if defined(RML_WINDOWS)
	#include <Windows.h>
	#define RML_LOAD_LIB(p) LoadLibraryW(p)
	#define RML_GET_PROC(h, s) GetProcAddress(static_cast<HMODULE>(h), s)
#else
	#include <dlfcn.h>
	#define RML_LOAD_LIB(p) dlopen(p, RTLD_LAZY | RTLD_LOCAL)
	#define RML_GET_PROC(h, s) dlsym(h, s)
#endif

RML_LOG_SCOPE("DotnetRuntime");

namespace rml::dotnet
{
	std::expected<void, std::string> DotnetRuntime::initialize(const std::filesystem::path& runtime_config)
	{
		if (m_initialized)
			return {};

		if (auto result = load_hostfxr(); !result)
			return result;

		if (auto result = init_runtime(runtime_config); !result)
			return result;

		m_initialized = true;
		RML_INFO("CLR initialized");

		return {};
	}

	void DotnetRuntime::shutdown()
	{
		if (!m_initialized)
			return;
		if (m_host_ctx)
		{
			m_hostfxr_close(m_host_ctx);
			m_host_ctx = nullptr;
		}
		m_initialized = false;

		RML_INFO("CLR shutdown");
	}

	std::expected<void, std::string> DotnetRuntime::load_hostfxr()
	{
		char_t buf[2048]{};
		size_t buf_size = sizeof(buf) / sizeof(char_t);

		auto runtime_path = utils::directory::get_runtime_directory();

#if !defined(RML_WINDOWS)
		const auto bundled_dotnet_root = runtime_path / "dotnet";
		if (std::filesystem::exists(bundled_dotnet_root))
			setenv("DOTNET_ROOT", bundled_dotnet_root.c_str(), 1);
		const auto dotnet_root_str = bundled_dotnet_root.native();
		const get_hostfxr_parameters params{
		    .size = sizeof(get_hostfxr_parameters),
		    .assembly_path = nullptr,
		    .dotnet_root = std::filesystem::exists(bundled_dotnet_root) ? dotnet_root_str.c_str() : nullptr,
		};
#else
		constexpr get_hostfxr_parameters params{
		    .size = sizeof(get_hostfxr_parameters),
		    .assembly_path = nullptr,
		    .dotnet_root = nullptr,
		};
#endif

		using get_hostfxr_path_fn = int (*)(char_t*, size_t*, const get_hostfxr_parameters*);

		void* nethost_lib = nullptr;
#if defined(RML_WINDOWS)
		nethost_lib = RML_LOAD_LIB((runtime_path / "nethost.dll").c_str());
#elif defined(RML_MACOS)
		nethost_lib = RML_LOAD_LIB((runtime_path / "libnethost.dylib").c_str());
#else
		nethost_lib = RML_LOAD_LIB((runtime_path / "libnethost.so").c_str());
#endif

		if (!nethost_lib)
			return std::unexpected("nethost not found - is .NET installed?");

		auto* get_path = reinterpret_cast<get_hostfxr_path_fn>(RML_GET_PROC(nethost_lib, "get_hostfxr_path"));
		if (!get_path)
			return std::unexpected("Failed to locate 'get_hostfxr_path' in nethost");

		if (get_path(buf, &buf_size, &params) != 0)
			return std::unexpected("hostfxr not found - is .NET installed?");

		m_hostfxr_lib = RML_LOAD_LIB(buf);
		if (!m_hostfxr_lib)
			return std::unexpected("Failed to load hostfxr");

		auto load = [&]<typename T>(const char* sym) -> std::expected<T, std::string> {
			auto* ptr = reinterpret_cast<T>(RML_GET_PROC(m_hostfxr_lib, sym));
			if (!ptr)
				return std::unexpected(std::format("Failed to get symbol '{}' from hostfxr", sym));
			return ptr;
		};

		auto init = load.operator()<hostfxr_initialize_for_runtime_config_fn>("hostfxr_initialize_for_runtime_config");
		if (!init)
			return std::unexpected(init.error());

		m_hostfxr_init = *init;

		auto get_del = load.operator()<hostfxr_get_runtime_delegate_fn>("hostfxr_get_runtime_delegate");
		if (!get_del)
			return std::unexpected(get_del.error());

		m_hostfxr_get_delegate = *get_del;

		auto close = load.operator()<hostfxr_close_fn>("hostfxr_close");
		if (!close)
			return std::unexpected(close.error());

		m_hostfxr_close = *close;

		return {};
	}

	std::expected<void, std::string> DotnetRuntime::init_runtime(const std::filesystem::path& config)
	{
		int rc = m_hostfxr_init(config.c_str(), nullptr, &m_host_ctx);
		if (rc != 0 || !m_host_ctx)
			return std::unexpected(std::format("hostfxr_initialize_for_runtime_config failed rc={:#x}", rc));

		rc = m_hostfxr_get_delegate(m_host_ctx, hdt_load_assembly_and_get_function_pointer, reinterpret_cast<void**>(&m_load_assembly_fn));

		if (rc != 0 || !m_load_assembly_fn)
			return std::unexpected(std::format("hdt_load_assembly_and_get_function_pointer failed rc={:#x}", rc));

		return {};
	}
} // namespace rml::dotnet
