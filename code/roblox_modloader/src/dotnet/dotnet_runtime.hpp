#pragma once

#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <nethost.h>

namespace rml::dotnet
{
	class DotnetRuntime
	{
	public:
		DotnetRuntime() = default;
		~DotnetRuntime()
		{
			shutdown();
		}

		DotnetRuntime(const DotnetRuntime&)            = delete;
		DotnetRuntime& operator=(const DotnetRuntime&) = delete;

		[[nodiscard]] std::expected<void, std::string> initialize(const std::filesystem::path& runtime_config);
		void shutdown();

		[[nodiscard]] bool is_initialized() const noexcept
		{
			return m_initialized;
		}

		template<typename FnT>
		[[nodiscard]] std::expected<FnT, std::string> get_function(const std::filesystem::path& assembly, const std::wstring_view type_name, const std::wstring_view method_name) const
		{
			if (!m_initialized)
				return std::unexpected("DotnetRuntime not initialized");

			void* fn_ptr{};

			if (int rc = m_load_assembly_fn(assembly.c_str(), type_name.data(), method_name.data(), UNMANAGEDCALLERSONLY_METHOD, nullptr, &fn_ptr); rc != 0 || !fn_ptr)
				return std::unexpected(std::format("get_function failed for '{}' rc={:#x}", assembly.filename().string(), rc));

			return reinterpret_cast<FnT>(fn_ptr);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> load_hostfxr();
		[[nodiscard]] std::expected<void, std::string> init_runtime(const std::filesystem::path& config);

		bool m_initialized{false};
		void* m_hostfxr_lib{nullptr};

		hostfxr_initialize_for_runtime_config_fn m_hostfxr_init{};
		hostfxr_get_runtime_delegate_fn m_hostfxr_get_delegate{};
		hostfxr_close_fn m_hostfxr_close{};
		hostfxr_handle m_host_ctx{};

		load_assembly_and_get_function_pointer_fn m_load_assembly_fn{};
	};
} // namespace rml::dotnet