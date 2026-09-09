#include "rml/dumper/disasm/decoder.hpp"

#include "disasm/arm64_decoder.hpp"
#include "disasm/x86_decoder.hpp"

namespace rml::dumper::disasm
{
	std::expected<Trace, Error> Decoder::trace_function(const image::Image& image, const Rva entry) const
	{
		const auto bounds = image.functions().at(entry);
		if (!bounds)
			return std::unexpected(Error::make(ErrorCode::recovery,
			                                   "0x{:X} is not the start of an indexed function", entry));

		const auto code = image.at(bounds->begin, bounds->size());
		if (code.empty())
			return std::unexpected(
			    Error::make(ErrorCode::recovery, "function at 0x{:X} lies outside the image", entry));

		return trace(code, bounds->begin);
	}

	std::expected<std::unique_ptr<Decoder>, Error> Decoder::create(const Architecture architecture)
	{
		switch (architecture)
		{
		case Architecture::x86_64:
			return std::make_unique<X86Decoder>();
		case Architecture::arm64:
			return std::make_unique<Arm64Decoder>();
		}

		return std::unexpected(Error::make(ErrorCode::usage, "unknown architecture"));
	}
}
