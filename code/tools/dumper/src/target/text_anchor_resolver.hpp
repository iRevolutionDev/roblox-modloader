#pragma once

#include "rml/dumper/target/anchor_resolver.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace rml::dumper::target
{
	class TextAnchorResolver final : public AnchorResolver
	{
	public:
		[[nodiscard]] std::expected<AnchorSet, Error> resolve(const image::Image& image,
		                                                      std::span<const AnchorSpec> anchors) const override;

		[[nodiscard]] static std::vector<Rva> find_literal(const image::Image& image, std::string_view text);
		[[nodiscard]] static std::vector<Rva> find_rip_references(const image::Image& image, Rva target);
		[[nodiscard]] static std::optional<Rva> owning_function(const image::Image& image, Rva address);
	};
}
