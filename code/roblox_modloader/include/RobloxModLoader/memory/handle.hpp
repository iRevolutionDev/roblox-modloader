#pragma once
#include "RobloxModLoader/rml_export.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace rml::memory
{
	class RML_EXPORT handle
	{
	public:
		handle(void* ptr = nullptr);

		explicit handle(std::uintptr_t ptr);

		template<typename T>
		std::enable_if_t<std::is_pointer_v<T>, T> as() const;

		template<typename T>
		std::enable_if_t<std::is_lvalue_reference_v<T>, T> as() const;

		template<typename T>
		std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> as() const;

		template<typename T>
		handle add(T offset) const;

		template<typename T>
		handle sub(T offset) const;

		handle rip() const;

		handle adrp() const;

		explicit operator bool();

		friend bool operator==(handle a, handle b);

		friend bool operator!=(handle a, handle b);

	private:
		void* ptr;
	};

	inline handle::handle(void* ptr) :
	    ptr(ptr)
	{
	}

	inline handle::handle(std::uintptr_t ptr) :
	    ptr(reinterpret_cast<void*>(ptr))
	{
	}

	template<typename T>
	inline std::enable_if_t<std::is_pointer_v<T>, T> handle::as() const
	{
		if constexpr (std::is_function_v<std::remove_pointer_t<T>>)
			return reinterpret_cast<T>(ptr);
		else
			return static_cast<T>(ptr);
	}

	template<typename T>
	inline std::enable_if_t<std::is_lvalue_reference_v<T>, T> handle::as() const
	{
		return *static_cast<std::add_pointer_t<std::remove_reference_t<T>>>(ptr);
	}

	template<typename T>
	inline std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> handle::as() const
	{
		return reinterpret_cast<std::uintptr_t>(ptr);
	}

	template<typename T>
	inline handle handle::add(T offset) const
	{
		return handle(as<std::uintptr_t>() + offset);
	}

	template<typename T>
	inline handle handle::sub(T offset) const
	{
		return handle(as<std::uintptr_t>() - offset);
	}

	inline handle handle::rip() const
	{
		return add(as<std::int32_t&>()).add(4);
	}

	inline handle handle::adrp() const
	{
		constexpr std::uintptr_t page_mask = 0xFFF;
		constexpr std::uint32_t load_store_mask = 0x3B000000;
		constexpr std::uint32_t load_store_value = 0x39000000;
		constexpr std::uint32_t add_immediate_mask = 0xFF800000;
		constexpr std::uint32_t add_immediate_value = 0x91000000;
		constexpr std::int64_t page_offset_sign = std::int64_t{1} << 32;
		constexpr std::uint32_t shifted_immediate = 1;

		const auto* const instructions = as<const std::uint32_t*>();
		const std::uint32_t adrp_instruction = instructions[0];
		const std::uint32_t offset_instruction = instructions[1];

		const std::int64_t immediate_low = (adrp_instruction >> 29) & 0x3;
		const std::int64_t immediate_high = (adrp_instruction >> 5) & 0x7FFFF;

		std::int64_t page_offset = ((immediate_high << 2) | immediate_low) << 12;
		if (page_offset & page_offset_sign)
			page_offset -= page_offset_sign << 1;

		const auto page = (as<std::uintptr_t>() & ~page_mask) + page_offset;

		if ((offset_instruction & load_store_mask) == load_store_value)
		{
			const std::uint32_t scale = offset_instruction >> 30;
			const std::uint32_t immediate = (offset_instruction >> 10) & 0xFFF;
			return handle(page + (static_cast<std::uintptr_t>(immediate) << scale));
		}

		if ((offset_instruction & add_immediate_mask) == add_immediate_value)
		{
			const std::uint32_t shift = (offset_instruction >> 22) & 0x3;
			const std::uint32_t immediate = (offset_instruction >> 10) & 0xFFF;
			return handle(page + (static_cast<std::uintptr_t>(immediate) << (shift == shifted_immediate ? 12 : 0)));
		}

		return handle(page);
	}

	inline bool operator==(handle a, handle b)
	{
		return a.ptr == b.ptr;
	}

	inline bool operator!=(handle a, handle b)
	{
		return a.ptr != b.ptr;
	}

	inline handle::operator bool()
	{
		return ptr != nullptr;
	}
}
