#pragma once

#include "rml/dumper/emit/emitter.hpp"

namespace rml::dumper::emit
{
	class ReportEmitter final : public TextEmitter
	{
	public:
		[[nodiscard]] std::string_view id() const override { return "report"; }
		[[nodiscard]] std::string_view default_filename() const override { return "report.txt"; }
		[[nodiscard]] std::expected<void, Error> emit(const schema::LayoutSet& layouts,
		                                              std::ostream& out) const override;
	};
}
