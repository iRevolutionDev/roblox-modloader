#pragma once

#include <memory>
#include <optional>
#include <string_view>

namespace rml::memory
{
	class IRttiProvider
	{
	public:
		virtual ~IRttiProvider() = default;
		[[nodiscard]] virtual std::optional<void**> find_class_vtable(std::string_view class_name) = 0;
	};

	[[nodiscard]] std::unique_ptr<IRttiProvider> create_rtti_provider();
}

inline rml::memory::IRttiProvider* g_rtti_provider{};
