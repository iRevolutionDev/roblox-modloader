#pragma once

#include "rml/dumper/emit/emitter.hpp"

namespace rml::dumper::emit
{
	class JsonEmitter final : public Emitter
	{
	public:
		[[nodiscard]] std::string_view id() const override { return "json"; }
		[[nodiscard]] std::string_view default_filename() const override { return "luau_layout.json"; }
		[[nodiscard]] std::expected<void, Error> emit(const schema::LayoutSet& layouts,
		                                              std::ostream& out) const override;
	};
}
