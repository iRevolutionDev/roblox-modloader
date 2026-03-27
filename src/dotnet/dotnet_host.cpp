#include "RobloxModLoader/dotnet/dotnet_host.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/dynamic_library.hpp"

#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <nethost.h>

namespace
{
	using rml::dotnet::char_t;

	using get_hostfxr_path_dynamic_fn = int (*)(char_t*, size_t*, const void*);


#if defined(_WIN32)
	using hostfxr_string = std::wstring;

	hostfxr_string to_hostfxr_string(const std::filesystem::path& path)
	{
		return path.native();
	}

	hostfxr_string to_hostfxr_string(std::string_view value)
	{
		if (value.empty())
		{
			return {};
		}

		const int wide_length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);

		if (wide_length <= 0)
		{
			return {};
		}

		hostfxr_string result(static_cast<size_t>(wide_length), L'\0');
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), wide_length);

		return result;
	}

	constexpr std::wstring_view hostfxr_library_name = L"hostfxr.dll";
	constexpr std::wstring_view nethost_library_name = L"nethost.dll";
#elif defined(__APPLE__)
	using hostfxr_string = std::string;

	hostfxr_string to_hostfxr_string(const std::filesystem::path& path)
	{
		return path.string();
	}

	hostfxr_string to_hostfxr_string(std::string_view value)
	{
		return std::string(value);
	}

	constexpr std::string_view hostfxr_library_name = "libhostfxr.dylib";
	constexpr std::string_view nethost_library_name = "libnethost.dylib";
#else
	using hostfxr_string = std::string;

	hostfxr_string to_hostfxr_string(const std::filesystem::path& path)
	{
		return path.string();
	}

	hostfxr_string to_hostfxr_string(std::string_view value)
	{
		return std::string(value);
	}

	constexpr std::string_view hostfxr_library_name = "libhostfxr.so";
	constexpr std::string_view nethost_library_name = "libnethost.so";
#endif

	std::optional<std::filesystem::path> get_current_module_directory()
	{
#if defined(_WIN32)
		HMODULE module = nullptr;
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&get_current_module_directory), &module))
		{
			return std::nullopt;
		}

		wchar_t module_path[MAX_PATH]{};
		if (GetModuleFileNameW(module, module_path, MAX_PATH) == 0)
		{
			return std::nullopt;
		}

		return std::filesystem::path(module_path).parent_path();
#else
		Dl_info info{};
		if (::dladdr(reinterpret_cast<void*>(&get_current_module_directory), &info) == 0 || info.dli_fname == nullptr)
		{
			return std::nullopt;
		}

		return std::filesystem::path(info.dli_fname).parent_path();
#endif
	}

	std::optional<std::filesystem::path> get_local_runtime_root()
	{
		if (const auto module_dir = get_current_module_directory())
		{
			if (const auto runtime_root = *module_dir / "RobloxModLoader" / "runtime" / "dotnet"; std::filesystem::exists(runtime_root))
			{
				return runtime_root;
			}
		}

		return std::nullopt;
	}

	std::optional<std::filesystem::path> resolve_hostfxr_from_dynamic_nethost()
	{
		auto nethost_invoker = [](const std::filesystem::path& nethost_path) -> std::optional<std::filesystem::path> {
			DynamicLibrary nethost_lib;
			if (!nethost_lib.load(nethost_path))
			{
				return std::nullopt;
			}

			const auto get_hostfxr_path_fn = reinterpret_cast<get_hostfxr_path_dynamic_fn>(nethost_lib.symbol("get_hostfxr_path"));
			if (!get_hostfxr_path_fn)
			{
				return std::nullopt;
			}

			std::array<char_t, 4096> hostfxr_buffer{};
			size_t hostfxr_buffer_size = hostfxr_buffer.size();

			const int nethost_result = get_hostfxr_path_fn(hostfxr_buffer.data(), &hostfxr_buffer_size, nullptr);
			if (nethost_result != 0)
			{
				return std::nullopt;
			}

			return std::filesystem::path(hostfxr_buffer.data());
		};

		if (const auto module_dir = get_current_module_directory())
		{
			const auto local_nethost = *module_dir / "RobloxModLoader" / "runtime" / nethost_library_name;
			if (std::filesystem::exists(local_nethost))
			{
				if (const auto hostfxr_path = nethost_invoker(local_nethost))
				{
					return hostfxr_path;
				}
			}
		}

		return std::nullopt;
	}

	std::optional<std::filesystem::path> read_env_path(const char* name)
	{
		const char* raw_value = std::getenv(name);
		if (!raw_value || *raw_value == '\0')
		{
			return std::nullopt;
		}
		return std::filesystem::path(raw_value);
	}

	int parse_int_part(const std::string_view value)
	{
		int parsed = 0;
		if (const auto [ptr, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
		    error != std::errc{} || ptr != value.data() + value.size())
		{
			return 0;
		}
		return parsed;
	}

	std::tuple<int, int, int, std::string> parse_version(std::string_view directory_name)
	{
		const size_t first_dot = directory_name.find('.');
		const size_t second_dot = first_dot == std::string_view::npos ? std::string_view::npos : directory_name.find('.', first_dot + 1);

		const std::string_view major_sv = first_dot == std::string_view::npos ? directory_name : directory_name.substr(0, first_dot);
		const std::string_view minor_sv = second_dot == std::string_view::npos ? std::string_view{} : directory_name.substr(first_dot + 1, second_dot - first_dot - 1);
		const std::string_view patch_sv = second_dot == std::string_view::npos ? std::string_view{} : directory_name.substr(second_dot + 1);

		return {parse_int_part(major_sv), parse_int_part(minor_sv), parse_int_part(patch_sv), std::string(directory_name)};
	}

	std::optional<std::filesystem::path> find_highest_hostfxr_from_root(const std::filesystem::path& dotnet_root)
	{
		const std::filesystem::path fxr_root = dotnet_root / "host" / "fxr";
		if (!std::filesystem::exists(fxr_root) || !std::filesystem::is_directory(fxr_root))
		{
			return std::nullopt;
		}

		std::vector<std::filesystem::path> candidates;
		for (const auto& entry : std::filesystem::directory_iterator(fxr_root))
		{
			if (!entry.is_directory())
			{
				continue;
			}

			const auto candidate = entry.path() / hostfxr_library_name;
			if (std::filesystem::exists(candidate))
			{
				candidates.push_back(candidate);
			}
		}

		if (candidates.empty())
		{
			return std::nullopt;
		}

		std::ranges::sort(candidates, [](const auto& lhs, const auto& rhs) {
			return parse_version(lhs.parent_path().filename().string()) < parse_version(rhs.parent_path().filename().string());
		});

		return candidates.back();
	}

	std::optional<std::filesystem::path> resolve_hostfxr_path()
	{
		if (const auto explicit_hostfxr = read_env_path("RML_HOSTFXR_PATH"); explicit_hostfxr && std::filesystem::exists(*explicit_hostfxr))
		{
			return *explicit_hostfxr;
		}

		if (const auto dotnet_root = read_env_path("DOTNET_ROOT"); dotnet_root)
		{
			if (const auto hostfxr_path = find_highest_hostfxr_from_root(*dotnet_root))
			{
				return hostfxr_path;
			}
		}

#if defined(_WIN32)
	#if defined(_M_X64)
		if (const auto dotnet_root_x64 = read_env_path("DOTNET_ROOT(x64)"); dotnet_root_x64)
		{
			if (const auto hostfxr_path = find_highest_hostfxr_from_root(*dotnet_root_x64))
			{
				return hostfxr_path;
			}
		}
	#endif

		constexpr std::array<std::wstring_view, 2> common_roots = {L"C:/Program Files/dotnet", L"C:/Program Files (x86)/dotnet"};

		for (const auto root : common_roots)
		{
			if (const auto hostfxr_path = find_highest_hostfxr_from_root(std::filesystem::path(root)); hostfxr_path)
			{
				return hostfxr_path;
			}
		}
#elif defined(__APPLE__)
		constexpr std::array<std::string_view, 2> common_roots = {"/usr/local/share/dotnet", "/usr/share/dotnet"};

		for (const auto root : common_roots)
		{
			if (const auto hostfxr_path = find_highest_hostfxr_from_root(std::filesystem::path(root)); hostfxr_path)
			{
				return hostfxr_path;
			}
		}
#else
		constexpr std::array<std::string_view, 2> common_roots = {"/usr/share/dotnet", "/usr/local/share/dotnet"};

		for (const auto root : common_roots)
		{
			if (const auto hostfxr_path = find_highest_hostfxr_from_root(std::filesystem::path(root)); hostfxr_path)
			{
				return hostfxr_path;
			}
		}
#endif

		if (const auto local_runtime_root = get_local_runtime_root())
		{
			if (const auto local_hostfxr = find_highest_hostfxr_from_root(*local_runtime_root))
			{
				return local_hostfxr;
			}
		}

#if defined(RML_DOTNET_RUNTIME_ROOT)
		if (const auto bundled_hostfxr = find_highest_hostfxr_from_root(std::filesystem::path(RML_DOTNET_RUNTIME_ROOT)); bundled_hostfxr)
		{
			return bundled_hostfxr;
		}
#endif

		if (const auto hostfxr_from_nethost = resolve_hostfxr_from_dynamic_nethost())
		{
			return hostfxr_from_nethost;
		}

		return std::nullopt;
	}
}

struct rml::dotnet::DotnetHost::impl
{
	DynamicLibrary hostfxr_library;
	void* hostfxr_context{nullptr};

	hostfxr_initialize_for_runtime_config_fn initialize_for_runtime_config{nullptr};
	hostfxr_get_runtime_delegate_fn get_runtime_delegate{nullptr};
	hostfxr_close_fn close{nullptr};

	load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer{nullptr};
};

rml::dotnet::DotnetHost::DotnetHost() :
    m_impl(new impl())
{
}

rml::dotnet::DotnetHost::~DotnetHost()
{
	shutdown();
	delete m_impl;
}

rml::dotnet::DotnetHost::DotnetHost(DotnetHost&& other) noexcept :
    m_impl(std::exchange(other.m_impl, nullptr))
{
}

rml::dotnet::DotnetHost& rml::dotnet::DotnetHost::operator=(DotnetHost&& other) noexcept
{
	if (this != &other)
	{
		shutdown();
		delete m_impl;
		m_impl = std::exchange(other.m_impl, nullptr);
	}
	return *this;
}

bool rml::dotnet::DotnetHost::initialize(const std::filesystem::path& runtime_config_path, std::string* error_message) const
{
	if (!m_impl)
	{
		if (error_message)
		{
			*error_message = "hostfxr runtime host is not in a valid state.";
		}
		return false;
	}

	shutdown();

	const auto hostfxr_path = resolve_hostfxr_path();
	if (!hostfxr_path)
	{
		if (error_message)
		{
			*error_message = "Unable to locate hostfxr. Set DOTNET_ROOT or RML_HOSTFXR_PATH.";
		}
		return false;
	}

	if (!m_impl->hostfxr_library.load(*hostfxr_path))

	{
		if (error_message)
		{
			*error_message = "Failed to load hostfxr dynamic library from: " + hostfxr_path->string();
		}
		return false;
	}

	m_impl->initialize_for_runtime_config =
	    reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(m_impl->hostfxr_library.symbol("hostfxr_initialize_for_runtime_config"));
	m_impl->get_runtime_delegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(m_impl->hostfxr_library.symbol("hostfxr_get_runtime_delegate"));
	m_impl->close = reinterpret_cast<hostfxr_close_fn>(m_impl->hostfxr_library.symbol("hostfxr_close"));

	if (!m_impl->initialize_for_runtime_config || !m_impl->get_runtime_delegate || !m_impl->close)
	{
		if (error_message)
		{
			*error_message = "Failed to resolve required hostfxr exports.";
		}
		shutdown();
		return false;
	}

	const hostfxr_string runtime_config_native = to_hostfxr_string(runtime_config_path);
	const int init_result = m_impl->initialize_for_runtime_config(runtime_config_native.c_str(), nullptr, &m_impl->hostfxr_context);

	if (init_result != 0 || m_impl->hostfxr_context == nullptr)
	{
		if (error_message)
		{
			*error_message = "hostfxr_initialize_for_runtime_config failed with code: " + std::to_string(init_result);
		}
		shutdown();
		return false;
	}

	void* delegate = nullptr;

	const int get_delegate_result = m_impl->get_runtime_delegate(m_impl->hostfxr_context, hdt_load_assembly_and_get_function_pointer, &delegate);

	if (get_delegate_result != 0 || delegate == nullptr)
	{
		if (error_message)
		{
			*error_message = "hostfxr_get_runtime_delegate failed with code: " + std::to_string(get_delegate_result);
		}
		shutdown();
		return false;
	}

	m_impl->load_assembly_and_get_function_pointer = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(delegate);
	return true;
}

bool rml::dotnet::DotnetHost::load_assembly_entrypoint(const std::filesystem::path& assembly_path, const std::string_view type_name, const std::string_view method_name, const std::optional<std::string_view>& delegate_type_name, void** entrypoint, std::string* error_message) const
{
	if (!m_impl || !m_impl->load_assembly_and_get_function_pointer)
	{
		if (error_message)
		{
			*error_message = "hostfxr runtime is not initialized.";
		}
		return false;
	}

	if (!entrypoint)
	{
		if (error_message)
		{
			*error_message = "entrypoint pointer cannot be null.";
		}
		return false;
	}

	const hostfxr_string assembly_native = to_hostfxr_string(assembly_path);
	const hostfxr_string type_native     = to_hostfxr_string(type_name);
	const hostfxr_string method_native   = to_hostfxr_string(method_name);
	const hostfxr_string delegate_native = delegate_type_name ? to_hostfxr_string(*delegate_type_name) : hostfxr_string{};

	const char_t* delegate_name_ptr = nullptr;
	if (delegate_type_name)
	{
		if (*delegate_type_name == unmanaged_callers_only_method)
		{
			delegate_name_ptr = reinterpret_cast<const char_t*>(static_cast<intptr_t>(-1));
		}
		else
		{
			delegate_name_ptr = delegate_native.c_str();
		}
	}
	void* loaded_delegate = nullptr;

	const int load_result =
	    m_impl->load_assembly_and_get_function_pointer(assembly_native.c_str(), type_native.c_str(), method_native.c_str(), delegate_name_ptr, nullptr, &loaded_delegate);

	if (load_result != 0 || loaded_delegate == nullptr)
	{
		if (error_message)
		{
			*error_message = "load_assembly_and_get_function_pointer failed with code: " + std::to_string(load_result);
		}
		return false;
	}

	*entrypoint = loaded_delegate;
	return true;
}

bool rml::dotnet::DotnetHost::is_initialized() const noexcept
{
	return m_impl && m_impl->load_assembly_and_get_function_pointer != nullptr;
}

void rml::dotnet::DotnetHost::shutdown() const
{
	if (!m_impl)
	{
		return;
	}

	m_impl->load_assembly_and_get_function_pointer = nullptr;

	if (m_impl->hostfxr_context && m_impl->close)
	{
		m_impl->close(m_impl->hostfxr_context);
		m_impl->hostfxr_context = nullptr;
	}

	m_impl->initialize_for_runtime_config = nullptr;
	m_impl->get_runtime_delegate          = nullptr;
	m_impl->close                         = nullptr;
	m_impl->hostfxr_library.unload();
}
