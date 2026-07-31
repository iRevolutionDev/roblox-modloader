#pragma once

#include "rml/dumper/core/types.hpp"
#include "rml/dumper/disasm/register.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace rml::dumper::disasm
{
	using Object = std::uint32_t;

	inline constexpr Object no_object = 0;
	inline constexpr Object zero_object = 255;
	inline constexpr Object first_derived_object = 256;

	[[nodiscard]] constexpr Object entry_object(const Register value)
	{
		return value == Register::none ? no_object : static_cast<Object>(value);
	}

	struct MemoryAccess
	{
		std::size_t sequence{};
		Rva address{};
		Object object{no_object};
		Register base{Register::none};
		Register index{Register::none};
		Register value_register{Register::none};
		std::uint8_t scale{1};
		std::uint8_t width{};
		std::int64_t displacement{};
		std::optional<std::uint64_t> immediate;
		bool is_write{};
	};

	struct CallSite
	{
		std::size_t sequence{};
		Rva address{};
		std::optional<Rva> target;
	};

	enum class ConstantKind : std::uint8_t
	{
		scale,
		step,
		literal,
	};

	struct ConstantUse
	{
		std::size_t sequence{};
		Rva address{};
		Register destination{Register::none};
		std::int64_t value{};
		ConstantKind kind{};
	};

	struct Trace
	{
		Rva begin{};
		Rva end{};
		std::vector<MemoryAccess> accesses;
		std::vector<CallSite> calls;
		std::vector<ConstantUse> constants;
	};
}
