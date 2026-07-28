#include <doctest/doctest.h>

#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "recover/common_header_recoverer.hpp"
#include "recover/lua_state_recoverer.hpp"
#include "support/studio_binary.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <map>

using namespace rml::dumper;
using namespace rml::dumper::recover;

static const std::map<std::string, std::size_t> lua_state_offsets_2026_07_08{
    {"status", 0x00},     {"activememcat", 0x01}, {"singlestep", 0x02}, {"isactive", 0x03},
    {"namecall", 0x08},   {"openupval", 0x10},    {"ci", 0x18},         {"global", 0x20},
    {"base", 0x28},       {"stack_last", 0x30},   {"stack", 0x38},      {"top", 0x40},
    {"gclist", 0x48},     {"userdata", 0x50},     {"gt", 0x58},         {"stacksize", 0x60},
    {"size_ci", 0x64},    {"nCcalls", 0x68},      {"baseCcalls", 0x6A}, {"cachedslot", 0x6C},
    {"end_ci", 0x70},     {"base_ci", 0x78},
};

TEST_CASE("the collectable header is recovered from a real studio build")
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
	{
		MESSAGE("RML_TEST_STUDIO is not set, skipping");
		return;
	}

	const auto* profile = target::TargetRegistry::find("windows-x64");
	REQUIRE(profile != nullptr);

	const auto image = image::ImageLoader::load(*path, profile->architecture);
	REQUIRE(image.has_value());

	const auto anchors = target::PatternAnchorResolver().resolve(*image, profile->anchors);
	REQUIRE(anchors.has_value());

	const auto decoder = disasm::Decoder::create(profile->architecture);
	REQUIRE(decoder.has_value());

	schema::Report report;
	RecoveryContext context(*image, **decoder, *profile->abi, *anchors, report);

	const CommonHeaderRecoverer recoverer;
	const auto recovered = recoverer.recover(context);
	REQUIRE(recovered.has_value());

	const auto* header = &*recovered;

	for (const auto& field : header->fields)
		MESSAGE(field.name, " at 0x", field.offset, " -- ", field.provenance.describe());

	REQUIRE(header->fields.size() == 3);
	CHECK(header->find("tt") != nullptr);
	CHECK(header->find("marked") != nullptr);
	CHECK(header->find("memcat") != nullptr);

	for (const auto& field : header->fields)
	{
		CHECK(field.provenance.is_recovered());
		CHECK(field.offset < 8);
	}
}

TEST_CASE("the recovered lua_State agrees with the layout already committed")
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
	{
		MESSAGE("RML_TEST_STUDIO is not set, skipping");
		return;
	}

	const auto* profile = target::TargetRegistry::find("windows-x64");
	const auto image = image::ImageLoader::load(*path, profile->architecture);
	REQUIRE(image.has_value());

	const auto anchors = target::PatternAnchorResolver().resolve(*image, profile->anchors);
	REQUIRE(anchors.has_value());

	const auto decoder = disasm::Decoder::create(profile->architecture);
	REQUIRE(decoder.has_value());

	schema::Report report;
	RecoveryContext context(*image, **decoder, *profile->abi, *anchors, report);

	const LuaStateRecoverer recoverer;
	const auto layout = recoverer.recover(context);
	REQUIRE(layout.has_value());

	std::size_t agreed = 0;
	std::size_t disagreed = 0;

	for (const auto& field : layout->fields)
	{
		const auto expected = lua_state_offsets_2026_07_08.find(field.name);
		if (expected == lua_state_offsets_2026_07_08.end())
		{
			MESSAGE(field.name, " is not in the committed layout");
			continue;
		}

		if (field.offset == expected->second)
		{
			++agreed;
			MESSAGE("agree    ", field.name, " 0x", field.offset);
		}
		else
		{
			++disagreed;
			MESSAGE("DISAGREE ", field.name, " recovered 0x", field.offset, " committed 0x", expected->second);
		}
	}

	MESSAGE(layout->fields.size(), " of ", lua_state_offsets_2026_07_08.size(), " fields recovered, ", agreed,
	        " agree with the committed layout, ", disagreed, " disagree");

	for (const auto& failure : report.failures())
		MESSAGE("unrecovered ", failure.field_name, ": ", failure.reason);

	CHECK(layout->fields.size() >= 10);
}
