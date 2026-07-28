#include <doctest/doctest.h>

#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "rml/dumper/target/target_profile.hpp"
#include "support/studio_binary.hpp"
#include "target/pattern_anchor_resolver.hpp"

using namespace rml::dumper;
using namespace rml::dumper::recover;

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

	const auto layouts = RecoveryPipeline::make_default().run(context);

	if (!layouts)
	{
		MESSAGE("recovery failed: ", layouts.error().message());
		FAIL("the collectable header could not be recovered");
		return;
	}

	const auto* header = layouts->find("CommonHeader");
	REQUIRE(header != nullptr);

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
