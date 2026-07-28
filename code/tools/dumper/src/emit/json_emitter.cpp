#include "emit/json_emitter.hpp"

#include <nlohmann/json.hpp>

namespace rml::dumper::emit
{
	std::expected<void, Error> JsonEmitter::emit(const schema::LayoutSet& layouts, std::ostream& out) const
	{
		nlohmann::ordered_json document;
		document["target"] = layouts.target;
		document["studio_version"] = layouts.studio_version;
		document["studio_guid"] = layouts.studio_guid;

		auto& structs = document["structs"];
		for (const auto& [name, layout] : layouts.structs)
		{
			nlohmann::ordered_json entry;
			entry["size"] = layout.size;

			auto& fields = entry["fields"];
			for (const auto& field : layout.fields)
			{
				nlohmann::ordered_json record;
				record["name"] = field.name;
				record["type"] = field.type;
				record["offset"] = field.offset;
				record["size"] = field.size;

				nlohmann::ordered_json provenance;
				if (const auto* recovered = field.provenance.as_recovered())
				{
					provenance["kind"] = "recovered";
					provenance["anchor"] = recovered->anchor;
					provenance["probe"] = recovered->probe;
				}
				else
				{
					provenance["kind"] = "fixed";
					provenance["reason"] = field.provenance.as_fixed()->reason;
				}

				record["provenance"] = provenance;
				fields.push_back(record);
			}

			structs[name] = entry;
		}

		auto& report = document["report"];
		report["recovered"] = layouts.report.recovered_count();
		report["fixed"] = layouts.report.fixed_count();

		auto& failures = report["failures"];
		for (const auto& failure : layouts.report.failures())
		{
			nlohmann::ordered_json record;
			record["struct"] = failure.struct_name;
			record["field"] = failure.field_name;
			record["reason"] = failure.reason;
			failures.push_back(record);
		}

		out << document.dump(2) << '\n';

		return {};
	}
}
