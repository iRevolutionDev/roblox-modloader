#pragma once

namespace rml::qt::detail
{
	[[nodiscard]] void* core_export(const char* mangled);

	[[nodiscard]] void* widgets_export(const char* mangled);

	template<typename Fn>
	[[nodiscard]] Fn core(const char* mangled)
	{
		return reinterpret_cast<Fn>(core_export(mangled));
	}

	template<typename Fn>
	[[nodiscard]] Fn widgets(const char* mangled)
	{
		return reinterpret_cast<Fn>(widgets_export(mangled));
	}
}
