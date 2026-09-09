#include <doctest/doctest.h>

#include "rml/dumper/core/leb128.hpp"
#include "rml/dumper/image/image.hpp"
#include "support/macho_builder.hpp"

using namespace rml::dumper;

static std::vector<std::byte> macho_bytes(const std::initializer_list<std::uint8_t> values)
{
	std::vector<std::byte> result;
	result.reserve(values.size());
	for (const auto value : values)
		result.push_back(static_cast<std::byte>(value));
	return result;
}

TEST_CASE("uleb128 decodes single and multi byte values")
{
	std::size_t cursor = 0;
	const auto encoded = macho_bytes({0x7F, 0xE5, 0x8E, 0x26, 0x00});

	CHECK(Leb128::decode(encoded, cursor).value() == 127);
	CHECK(cursor == 1);
	CHECK(Leb128::decode(encoded, cursor).value() == 624485);
	CHECK(cursor == 4);
	CHECK(Leb128::decode(encoded, cursor).value() == 0);
	CHECK(cursor == 5);
}

TEST_CASE("uleb128 fails when it runs off the end")
{
	std::size_t cursor = 0;
	const auto encoded = macho_bytes({0x80, 0x80});

	CHECK_FALSE(Leb128::decode(encoded, cursor).has_value());
}

TEST_CASE("uleb128 round trips through the encoder")
{
	for (const std::uint64_t value : {0ull, 1ull, 127ull, 128ull, 624485ull, 0xFFFFFFFFull})
	{
		std::vector<std::byte> encoded;
		Leb128::encode(encoded, value);

		std::size_t cursor = 0;
		CHECK(Leb128::decode(encoded, cursor).value() == value);
	}
}

TEST_CASE("mach-o thin image maps its segments")
{
	tests::MachOBuilder builder(Architecture::x86_64);
	builder.add_empty_segment("__PAGEZERO", 0, 0x100000000);
	builder.add_segment("__TEXT", 0x100000000, macho_bytes({0x55, 0x48, 0x89}));
	builder.add_segment("__DATA", 0x100001000, macho_bytes({0x01}), false);

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->format() == ImageFormat::mach_o);
	CHECK(image->architecture() == Architecture::x86_64);
	CHECK(image->preferred_base() == 0x100000000);

	const auto text = image->at(0, 3);
	REQUIRE(text.size() == 3);
	CHECK(text[0] == std::byte{0x55});
	CHECK(text[2] == std::byte{0x89});

	const auto data = image->at(0x1000, 1);
	REQUIRE(data.size() == 1);
	CHECK(data[0] == std::byte{0x01});
}

TEST_CASE("mach-o sections carry the executable flag")
{
	tests::MachOBuilder builder(Architecture::x86_64);
	builder.add_segment("__TEXT", 0x100000000, macho_bytes({0x90}));
	builder.add_segment("__DATA", 0x100001000, macho_bytes({0x00}), false);

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->sections().size() == 2);
	REQUIRE(image->executable_sections().size() == 1);
	CHECK(image->executable_sections()[0].name == "__text");
}

TEST_CASE("mach-o arm64 image is accepted when requested")
{
	tests::MachOBuilder builder(Architecture::arm64);
	builder.add_segment("__TEXT", 0x100000000, macho_bytes({0xFD, 0x7B, 0xBF, 0xA9}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::arm64);

	REQUIRE(image.has_value());
	CHECK(image->architecture() == Architecture::arm64);
}

TEST_CASE("mach-o thin image rejects the wrong architecture")
{
	tests::MachOBuilder builder(Architecture::arm64);
	builder.add_segment("__TEXT", 0x100000000, macho_bytes({0x00}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE_FALSE(image.has_value());
	CHECK(image.error().code() == ErrorCode::invalid_image);
	CHECK(image.error().message().find("x86_64") != std::string::npos);
}

TEST_CASE("fat mach-o selects the requested slice")
{
	tests::MachOBuilder x64(Architecture::x86_64);
	x64.add_segment("__TEXT", 0x100000000, macho_bytes({0xAA}));

	tests::MachOBuilder arm(Architecture::arm64);
	arm.add_segment("__TEXT", 0x100000000, macho_bytes({0xBB}));

	const std::vector<std::vector<std::byte>> slices{x64.build(), arm.build()};
	const auto path = tests::MachOBuilder::write_fat(slices);

	const auto selected = image::ImageLoader::load(path, Architecture::arm64);
	REQUIRE(selected.has_value());
	CHECK(selected->at(0, 1)[0] == std::byte{0xBB});

	const auto other = image::ImageLoader::load(path, Architecture::x86_64);
	REQUIRE(other.has_value());
	CHECK(other->at(0, 1)[0] == std::byte{0xAA});
}

TEST_CASE("fat mach-o reports a missing slice")
{
	tests::MachOBuilder x64(Architecture::x86_64);
	x64.add_segment("__TEXT", 0x100000000, macho_bytes({0xAA}));

	const std::vector<std::vector<std::byte>> slices{x64.build()};
	const auto path = tests::MachOBuilder::write_fat(slices);

	const auto missing = image::ImageLoader::load(path, Architecture::arm64);

	REQUIRE_FALSE(missing.has_value());
	CHECK(missing.error().message().find("arm64") != std::string::npos);
}

TEST_CASE("mach-o function starts decode into a function index")
{
	tests::MachOBuilder builder(Architecture::arm64);
	builder.add_segment("__TEXT", 0x100000000, std::vector<std::byte>(0x400, std::byte{0x1F}));
	builder.set_function_starts(std::vector<Rva>{0x100, 0x140, 0x1C0});

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::arm64);

	REQUIRE(image.has_value());

	const auto& functions = image->functions();
	CHECK(functions.size() == 3);
	CHECK(functions.containing(0x120)->begin == 0x100);
	CHECK(functions.containing(0x120)->end == 0x140);
	CHECK(functions.at(0x140)->end == 0x1C0);
	CHECK(functions.at(0x1C0)->end == 0x400);
	CHECK(functions.containing(0x080) == std::nullopt);
}

TEST_CASE("a mach-o image without function starts has an empty index")
{
	tests::MachOBuilder builder(Architecture::arm64);
	builder.add_segment("__TEXT", 0x100000000, macho_bytes({0x1F, 0x20, 0x03, 0xD5}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::arm64);

	REQUIRE(image.has_value());
	CHECK(image->functions().empty());
}
