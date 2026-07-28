#pragma once

#include "rml/dumper/core/error.hpp"
#include "rml/dumper/disasm/trace.hpp"
#include "rml/dumper/image/image.hpp"

#include <expected>
#include <memory>
#include <span>

namespace rml::dumper::disasm
{
	class Decoder
	{
	public:
		virtual ~Decoder() = default;

		[[nodiscard]] virtual Architecture architecture() const = 0;
		[[nodiscard]] virtual std::expected<Trace, Error> trace(std::span<const std::byte> code, Rva begin) const = 0;

		[[nodiscard]] std::expected<Trace, Error> trace_function(const image::Image& image, Rva entry) const;

		[[nodiscard]] static std::expected<std::unique_ptr<Decoder>, Error> create(Architecture architecture);
	};
}
