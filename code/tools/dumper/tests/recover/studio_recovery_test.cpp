#include <doctest/doctest.h>

#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "recover/call_info_recoverer.hpp"
#include "recover/common_header_recoverer.hpp"
#include "recover/lua_state_recoverer.hpp"
#include "support/studio_binary.hpp"
#include "target/pattern_anchor_resolver.hpp"

#include <format>

using namespace rml::dumper;
using namespace rml::dumper::recover;

static constexpr std::array needed_by_the_loader{"base", "base_ci", "ci", "global", "stack", "top"};

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

TEST_CASE("the recovered lua_State holds together and carries what the loader reads")
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

	for (const auto& field : layout->fields)
		MESSAGE(std::format("{:<14} at 0x{:<3X} {}", field.name, field.offset, field.provenance.describe()));

	for (const auto& failure : report.failures())
		MESSAGE("unrecovered ", failure.field_name, ": ", failure.reason);

	CHECK(validate(*layout).has_value());

	for (const auto& name : needed_by_the_loader)
	{
		INFO("lua_State.", name);
		CHECK(layout->find(name) != nullptr);
	}
}

TEST_CASE("the recovered call info is a frame the loader can walk")
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

	const LuaStateRecoverer state;
	const auto lua_state = state.recover(context);
	REQUIRE(lua_state.has_value());
	context.publish(*lua_state);

	const CallInfoRecoverer recoverer;
	const auto layout = recoverer.recover(context);
	REQUIRE(layout.has_value());

	for (const auto& field : layout->fields)
		MESSAGE(std::format("{:<14} at 0x{:<3X} {}", field.name, field.offset, field.provenance.describe()));

	MESSAGE(std::format("sizeof 0x{:X}", layout->size));

	CHECK(validate(*layout).has_value());

	for (const auto& name : {"base", "func", "top"})
	{
		INFO("CallInfo.", name);
		REQUIRE(layout->find(name) != nullptr);
	}

	const auto* base = layout->find("base");
	const auto* func = layout->find("func");
	const auto* top = layout->find("top");

	CHECK(base->offset != func->offset);
	CHECK(base->offset != top->offset);
	CHECK(func->offset != top->offset);

	CHECK(layout->size % 8 == 0);
	CHECK(layout->size >= base->end());
	CHECK(layout->size >= func->end());
	CHECK(layout->size >= top->end());
}
