#pragma once

#include "rml/dumper/disasm/decoder.hpp"

namespace rml::dumper::disasm
{
	class X86Decoder final : public Decoder
	{
	public:
		[[nodiscard]] Architecture architecture() const override { return Architecture::x86_64; }
		[[nodiscard]] std::expected<Trace, Error> trace(std::span<const std::byte> code, Rva begin) const override;
	};
}
