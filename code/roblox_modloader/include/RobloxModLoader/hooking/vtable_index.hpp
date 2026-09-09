#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace rml
{
	namespace detail
	{
		inline constexpr std::size_t max_probed_vtable_slots = 256;

		template<std::size_t Index>
		std::size_t report_vtable_slot(void*)
		{
			return Index;
		}

		template<std::size_t... Indices>
		std::array<void*, sizeof...(Indices)> make_vtable_slot_table(std::index_sequence<Indices...>)
		{
			return {reinterpret_cast<void*>(&report_vtable_slot<Indices>)...};
		}

		inline const void* vtable_slot_table()
		{
			static const auto table =
			    make_vtable_slot_table(std::make_index_sequence<max_probed_vtable_slots>{});
			return table.data();
		}

		struct VtableSlotProbe
		{
			const void* vptr;
		};
	}

	template<typename Class, typename Ret, typename... Args, typename... Actual>
	std::size_t vtable_index_of(Ret (Class::*method)(Args...), Actual&&... args)
	{
		using Reporter = std::size_t (Class::*)(Args...);

		detail::VtableSlotProbe probe{detail::vtable_slot_table()};
		auto* object = reinterpret_cast<Class*>(&probe);

		return (object->*reinterpret_cast<Reporter>(method))(std::forward<Actual>(args)...);
	}

	template<typename Class, typename Ret, typename... Args, typename... Actual>
	std::size_t vtable_index_of(Ret (Class::*method)(Args...) const, Actual&&... args)
	{
		using Reporter = std::size_t (Class::*)(Args...) const;

		detail::VtableSlotProbe probe{detail::vtable_slot_table()};
		const auto* object = reinterpret_cast<const Class*>(&probe);

		return (object->*reinterpret_cast<Reporter>(method))(std::forward<Actual>(args)...);
	}
}
