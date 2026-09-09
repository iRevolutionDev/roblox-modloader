#include "rml/dumper/schema/report.hpp"

#include <format>

namespace rml::dumper::schema
{
	void Report::record_recovered(std::string struct_name, std::string field_name, std::string probe)
	{
		m_recovered.push_back({std::move(struct_name), std::move(field_name), std::move(probe)});
	}

	void Report::record_fixed(std::string struct_name, std::string field_name, std::string reason)
	{
		m_fixed.push_back({std::move(struct_name), std::move(field_name), std::move(reason)});
	}

	void Report::record_failure(std::string struct_name, std::string field_name, std::string reason)
	{
		m_failures.push_back({std::move(struct_name), std::move(field_name), std::move(reason)});
	}

	void Report::record_note(std::string struct_name, std::string reason)
	{
		m_notes.push_back({std::move(struct_name), {}, std::move(reason)});
	}

	std::string Report::summary() const
	{
		std::string text = std::format("{} recovered, {} fixed, {} failed", recovered_count(), fixed_count(),
		                               m_failures.size());

		for (const auto& failure : m_failures)
			text += std::format("\n  {}.{}: {}", failure.struct_name, failure.field_name, failure.reason);

		for (const auto& note : m_notes)
			text += std::format("\n  {}: {}", note.struct_name, note.reason);

		return text;
	}
}
