#pragma once

#include <cstddef>
#include <new>
#include <utility>

namespace rml::qt::detail
{
	[[nodiscard]] void* core_export(const char* mangled);

	[[nodiscard]] void* widgets_export(const char* mangled);

	[[nodiscard]] void* gui_export(const char* mangled);

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

	template<typename Fn>
	[[nodiscard]] Fn gui(const char* mangled)
	{
		return reinterpret_cast<Fn>(gui_export(mangled));
	}

	inline constexpr std::size_t WIDGET_INSTANCE_SIZE = 128;

	template<typename T, typename Ctor, typename... Args>
	[[nodiscard]] T* heap_construct(const std::size_t size, Ctor ctor, Args&&... args)
	{
		if (!ctor)
			return nullptr;

		void* memory = ::operator new(size);
		ctor(memory, std::forward<Args>(args)...);
		return static_cast<T*>(memory);
	}

	template<typename T, typename Dtor>
	void heap_destroy(Dtor dtor, T* instance)
	{
		if (!instance)
			return;

		if (dtor)
			dtor(instance);
		::operator delete(instance);
	}
}
