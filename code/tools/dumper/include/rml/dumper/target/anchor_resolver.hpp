#pragma once

#include "rml/dumper/core/error.hpp"
#include "rml/dumper/image/image.hpp"
#include "rml/dumper/target/anchor.hpp"

#include <expected>
#include <span>

namespace rml::dumper::target
{
	class AnchorResolver
	{
	public:
		virtual ~AnchorResolver() = default;

		[[nodiscard]] virtual std::expected<AnchorSet, Error> resolve(const image::Image& image,
		                                                              std::span<const AnchorSpec> anchors) const = 0;
	};
}
