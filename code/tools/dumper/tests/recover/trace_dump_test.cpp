#include <doctest/doctest.h>

#include "rml/dumper/disasm/decoder.hpp"
#include "rml/dumper/disasm/trace_query.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "support/studio_binary.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <format>

using namespace rml::dumper;

TEST_CASE("dump an anchor trace" * doctest::skip(true))
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
		return;

	const auto* profile = target::TargetRegistry::find("windows-x64");
	const auto image = image::ImageLoader::load(*path, profile->architecture);
	REQUIRE(image.has_value());

	const auto anchors = target::PatternAnchorResolver().resolve(*image, profile->anchors);
	REQUIRE(anchors.has_value());

	const auto decoder = disasm::Decoder::create(profile->architecture);
	REQUIRE(decoder.has_value());

	for (const auto anchor : {target::Anchor::luaE_newthread})
	{
		const auto entry = anchors->at(anchor);
		const auto bounds = image->functions().at(entry);

		MESSAGE("=== ", to_string(anchor), " at 0x", std::format("{:X}", entry), " bounds ",
		        bounds ? std::format("0x{:X}..0x{:X}", bounds->begin, bounds->end) : std::string("none"));

		const auto trace = (*decoder)->trace_function(*image, entry);
		if (!trace)
		{
			MESSAGE("   trace failed: ", trace.error().message());
			continue;
		}

		const disasm::TraceQuery query(*trace);
		MESSAGE("   accesses ", trace->accesses.size(), " calls ", trace->calls.size(), " dominant base ",
		        to_string(query.dominant_base()));

		for (const auto& access : trace->accesses)
		{
			if (!access.is_write || access.base != query.dominant_base())
				continue;
			MESSAGE("   seq ", access.sequence, " ", access.is_write ? "write" : "read ", " base ",
			        to_string(access.base), " disp ", std::format("0x{:X}", access.displacement), " width ",
			        static_cast<int>(access.width), " value ", to_string(access.value_register));
		}
	}
}
