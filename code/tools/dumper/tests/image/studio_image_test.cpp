#include <doctest/doctest.h>

#include "rml/dumper/image/image.hpp"
#include "support/studio_binary.hpp"

using namespace rml::dumper;

TEST_CASE("the real windows studio image maps and exposes executable sections")
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
	{
		MESSAGE("RML_TEST_STUDIO is not set, skipping");
		return;
	}

	const auto image = image::ImageLoader::load(*path, Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->format() == ImageFormat::pe);
	CHECK(image->architecture() == Architecture::x86_64);
	CHECK(image->preferred_base() == 0x140000000);
	CHECK(image->sections().size() > 4);
	CHECK_FALSE(image->executable_sections().empty());

	const auto& text = image->executable_sections()[0];
	CHECK(text.name == ".text");
	CHECK(text.size > 0x1000000);

	MESSAGE("mapped ", image->memory().size(), " bytes, ", image->sections().size(), " sections, .text is ",
	        text.size, " bytes");
}

TEST_CASE("the real windows studio image indexes its functions from pdata")
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
	{
		MESSAGE("RML_TEST_STUDIO is not set, skipping");
		return;
	}

	const auto image = image::ImageLoader::load(*path, Architecture::x86_64);
	REQUIRE(image.has_value());

	const auto& functions = image->functions();
	CHECK(functions.size() > 100000);

	const auto& text = image->executable_sections()[0];
	const auto middle = functions.containing(text.address + text.size / 2);
	REQUIRE(middle.has_value());
	CHECK(middle->size() > 0);
	CHECK(middle->size() < 0x10000);

	MESSAGE("indexed ", functions.size(), " functions");
}

static void check_real_macos_studio(const Architecture architecture)
{
	const auto path = tests::StudioBinary::macos(architecture);
	if (!path)
	{
		MESSAGE("no macos ", to_string(architecture), " binary configured, skipping");
		return;
	}

	const auto image = image::ImageLoader::load(*path, architecture);

	REQUIRE(image.has_value());
	CHECK(image->format() == ImageFormat::mach_o);
	CHECK(image->architecture() == architecture);
	CHECK(image->preferred_base() == 0x100000000);
	CHECK_FALSE(image->executable_sections().empty());
	CHECK(image->functions().size() > 100000);

	const auto& text = image->executable_sections()[0];
	CHECK(text.name == "__text");

	const auto middle = image->functions().containing(text.address + text.size / 2);
	REQUIRE(middle.has_value());
	CHECK(middle->size() > 0);

	MESSAGE("macos ", to_string(architecture), ": mapped ", image->memory().size(), " bytes, ",
	        image->sections().size(), " sections, ", image->functions().size(), " functions");
}

TEST_CASE("the real macos arm64 studio image parses")
{
	check_real_macos_studio(Architecture::arm64);
}

TEST_CASE("the real macos x64 studio image parses")
{
	check_real_macos_studio(Architecture::x86_64);
}
