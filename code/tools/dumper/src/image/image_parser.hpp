#pragma once

#include "rml/dumper/image/image.hpp"

namespace rml::dumper::image
{
	class ImageParser
	{
	public:
		virtual ~ImageParser() = default;

		[[nodiscard]] virtual ImageFormat format() const = 0;
		[[nodiscard]] virtual bool matches(std::span<const std::byte> file) const = 0;
		[[nodiscard]] virtual std::expected<Image, Error> parse(std::vector<std::byte> file,
		                                                        Architecture architecture) const = 0;
	};
}
