#include "rml/dumper/schema/field.hpp"

#include <format>

namespace rml::dumper::schema
{
	Provenance Provenance::fixed(std::string reason)
	{
		Provenance provenance;
		provenance.m_source = Fixed{std::move(reason)};
		return provenance;
	}

	Provenance Provenance::recovered(std::string anchor, std::string probe)
	{
		Provenance provenance;
		provenance.m_source = Recovered{std::move(anchor), std::move(probe)};
		return provenance;
	}

	bool Provenance::is_recovered() const
	{
		return std::holds_alternative<Recovered>(m_source);
	}

	const Provenance::Fixed* Provenance::as_fixed() const
	{
		return std::get_if<Fixed>(&m_source);
	}

	const Provenance::Recovered* Provenance::as_recovered() const
	{
		return std::get_if<Recovered>(&m_source);
	}

	std::string Provenance::describe() const
	{
		if (const auto* source = as_recovered())
			return std::format("recovered from {} ({})", source->anchor, source->probe);

		return std::format("fixed ({})", as_fixed()->reason);
	}
}
