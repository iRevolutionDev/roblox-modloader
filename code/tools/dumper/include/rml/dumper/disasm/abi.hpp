#pragma once

#include "rml/dumper/core/types.hpp"
#include "rml/dumper/disasm/register.hpp"

#include <span>

namespace rml::dumper::disasm
{
	class Abi
	{
	public:
		virtual ~Abi() = default;

		[[nodiscard]] virtual std::string_view name() const = 0;
		[[nodiscard]] virtual Register argument(std::size_t index) const = 0;
		[[nodiscard]] virtual Register return_value() const = 0;

		[[nodiscard]] static const Abi& windows_x64();
		[[nodiscard]] static const Abi& system_v_x64();
		[[nodiscard]] static const Abi& aapcs64();
	};
}
