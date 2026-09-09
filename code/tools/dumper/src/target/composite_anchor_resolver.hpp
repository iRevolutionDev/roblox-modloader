#pragma once

#include "rml/dumper/target/anchor_resolver.hpp"

namespace rml::dumper::target
{
	class CompositeAnchorResolver final : public AnchorResolver
	{
	public:
		[[nodiscard]] std::expected<AnchorSet, Error> resolve(const image::Image& image,
		                                                      std::span<const AnchorSpec> anchors) const override;
	};
}
