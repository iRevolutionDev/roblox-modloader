#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace rml::dotnet
{
#if defined(_WIN32)
	using char_t = wchar_t;
#else
	using char_t = char;
#endif

	inline constexpr std::string_view unmanaged_callers_only_method = "UNMANAGEDCALLERSONLY_METHOD";

	using load_assembly_and_get_function_pointer_fn = int (*)(const char_t* assembly_path, const char_t* type_name, const char_t* method_name, const char_t* delegate_type_name, void* reserved, void** delegate);

	class RML_EXPORT DotnetHost
	{
	public:
		DotnetHost();
		~DotnetHost();

		DotnetHost(const DotnetHost&)            = delete;
		DotnetHost& operator=(const DotnetHost&) = delete;

		DotnetHost(DotnetHost&& other) noexcept;
		DotnetHost& operator=(DotnetHost&& other) noexcept;

		bool initialize(const std::filesystem::path& runtime_config_path, std::string* error_message = nullptr) const;

		bool load_assembly_entrypoint(const std::filesystem::path& assembly_path, std::string_view type_name, std::string_view method_name, const std::optional<std::string_view>& delegate_type_name, void** entrypoint, std::string* error_message = nullptr) const;

		[[nodiscard]] bool is_initialized() const noexcept;

		void shutdown() const;

	private:
		struct impl;
		impl* m_impl;
	};
}
