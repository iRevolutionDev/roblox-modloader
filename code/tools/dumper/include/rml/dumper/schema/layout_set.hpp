#pragma once

#include "rml/dumper/schema/report.hpp"
#include "rml/dumper/schema/struct_layout.hpp"

#include <map>
#include <string>

namespace rml::dumper::schema
{
	struct LayoutSet
	{
		std::string target;
		std::string studio_version;
		std::string studio_guid;
		std::map<std::string, StructLayout> structs;
		Report report;

		[[nodiscard]] const StructLayout* find(std::string_view name) const;
	};
}
