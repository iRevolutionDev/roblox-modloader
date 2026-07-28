#include <doctest/doctest.h>

#include "rml/dumper/disasm/decoder.hpp"
#include "rml/dumper/disasm/trace_query.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "support/studio_binary.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <cstdlib>
#include <format>
#include <string>
#include <vector>

using namespace rml::dumper;

static std::vector<target::Anchor> requested_anchors()
{
	const char* selection = std::getenv("RML_DUMP_ANCHORS");
	if (selection == nullptr)
		return {};

	const std::string names(selection);
	std::vector<target::Anchor> anchors;

	for (std::size_t index = 0; index < target::anchor_count; ++index)
	{
		const auto anchor = static_cast<target::Anchor>(index);
		if (names.find(to_string(anchor)) != std::string::npos)
			anchors.push_back(anchor);
	}

	return anchors;
}

static std::string describe(const disasm::MemoryAccess& access)
{
	std::string text = std::format("{:>4} {} #{:<3} [{}", access.sequence, access.is_write ? "write" : "read ",
	                               access.object, to_string(access.base));

	if (access.index != disasm::Register::none)
		text += std::format(" + {}*{}", to_string(access.index), access.scale);

	text += std::format(" + 0x{:X}] width {}", access.displacement, access.width);

	if (access.value_register != disasm::Register::none)
		text += std::format(" from {}", to_string(access.value_register));
	if (access.immediate)
		text += std::format(" = 0x{:X}", *access.immediate);

	return text + std::format("   @0x{:X}", access.address);
}

static std::string_view describe(const disasm::ConstantKind kind)
{
	switch (kind)
	{
	case disasm::ConstantKind::scale: return "scale";
	case disasm::ConstantKind::step: return "step";
	case disasm::ConstantKind::literal: return "literal";
	}
	return "unknown";
}

TEST_CASE("dump an anchor trace")
{
	const auto anchors_wanted = requested_anchors();
	if (anchors_wanted.empty())
		return;

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

	std::vector<std::pair<std::string, Rva>> functions;

	const char* wanted_path = std::getenv("RML_DUMP_CALLEE");

	for (const auto anchor : anchors_wanted)
	{
		auto entry = anchors->at(anchor);
		functions.emplace_back(std::string(to_string(anchor)), entry);

		std::string path = wanted_path != nullptr ? wanted_path : std::string();
		std::string walked;

		while (!path.empty())
		{
			const auto dot = path.find('.');
			const auto step = static_cast<std::size_t>(std::stoul(path.substr(0, dot)));
			path = dot == std::string::npos ? std::string() : path.substr(dot + 1);

			const auto caller = (*decoder)->trace_function(*image, entry);
			if (!caller || step >= caller->calls.size() || !caller->calls[step].target)
				break;

			entry = *caller->calls[step].target;
			walked += std::format(" {}", step);
			functions.emplace_back(std::format("{} callee{}", to_string(anchor), walked), entry);
		}
	}

	for (const auto& [name, entry] : functions)
	{
		const auto bounds = image->functions().at(entry);

		MESSAGE("=== ", name, " at 0x", std::format("{:X}", entry), " bounds ",
		        bounds ? std::format("0x{:X}..0x{:X}", bounds->begin, bounds->end) : std::string("none"));

		std::string opening;
		for (const auto byte : image->at(entry, 40))
			opening += std::format("{:02X} ", static_cast<std::uint8_t>(byte));

		MESSAGE("   bytes ", opening);

		const auto trace = (*decoder)->trace_function(*image, entry);
		if (!trace)
		{
			MESSAGE("   trace failed: ", trace.error().message());
			continue;
		}

		const disasm::TraceQuery query(*trace);
		MESSAGE("   accesses ", trace->accesses.size(), " calls ", trace->calls.size(), " dominant object ",
		        query.dominant_object());

		std::size_t next_call = 0;

		for (const auto& access : trace->accesses)
		{
			while (next_call < trace->calls.size() && trace->calls[next_call].sequence <= access.sequence)
			{
				const auto& call = trace->calls[next_call++];
				MESSAGE("   ", std::format("{:>4}", call.sequence), " call  ",
				        call.target ? std::format("0x{:X}", *call.target) : std::string("indirect"));
			}

			MESSAGE("   ", describe(access));
		}

		for (; next_call < trace->calls.size(); ++next_call)
			MESSAGE("   ", std::format("{:>4}", trace->calls[next_call].sequence), " call  ",
			        trace->calls[next_call].target
			            ? std::format("0x{:X}", *trace->calls[next_call].target)
			            : std::string("indirect"));

		for (const auto& constant : trace->constants)
			MESSAGE("   ", std::format("{:>4}", constant.sequence), " const 0x",
			        std::format("{:X}", constant.value), " ", describe(constant.kind), " into ",
			        to_string(constant.destination));
	}
}
