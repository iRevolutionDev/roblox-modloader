#include "emit/report_emitter.hpp"

#include <format>

namespace rml::dumper::emit
{
	std::expected<void, Error> ReportEmitter::emit(const schema::LayoutSet& layouts, std::ostream& out) const
	{
		write_banner(out, layouts, "#");

		for (const auto& [name, layout] : layouts.structs)
		{
			out << std::format("{} (0x{:X} bytes, {} of {} fields recovered)\n", name, layout.size,
			                   layout.recovered_count(), layout.fields.size());

			for (const auto& field : layout.fields)
				out << std::format("  0x{:<4X} {:<16} {:<20} {}\n", field.offset, field.size, field.name,
				                   field.provenance.describe());

			out << '\n';
		}

		out << std::format("{} recovered, {} fixed, {} failed\n", layouts.report.recovered_count(),
		                   layouts.report.fixed_count(), layouts.report.failures().size());

		for (const auto& failure : layouts.report.failures())
			out << std::format("  {}.{}: {}\n", failure.struct_name, failure.field_name, failure.reason);

		return {};
	}
}
