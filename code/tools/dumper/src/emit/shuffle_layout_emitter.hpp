#pragma once

#include "rml/dumper/emit/emitter.hpp"

#include <span>

namespace rml::dumper::emit
{
	class ShuffleLayoutEmitter final : public TextEmitter
	{
	public:
		[[nodiscard]] std::string_view id() const override { return "shuffle"; }
		[[nodiscard]] std::string_view default_filename() const override { return "luau_shuffle_layout.h"; }
		[[nodiscard]] std::expected<void, Error> emit(const schema::LayoutSet& layouts,
		                                              std::ostream& out) const override;

		[[nodiscard]] static std::span<const std::string_view> canonical_fields(std::string_view struct_name);

	private:
		[[nodiscard]] static std::expected<std::vector<std::size_t>, Error> permutation(
		    const schema::StructLayout& layout, std::span<const std::string_view> canonical);
	};
}
