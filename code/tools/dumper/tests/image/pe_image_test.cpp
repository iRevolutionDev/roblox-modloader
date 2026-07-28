#include <doctest/doctest.h>

#include "rml/dumper/image/image.hpp"
#include "support/pe_builder.hpp"
#include "support/temporary_file.hpp"

using namespace rml::dumper;

static std::vector<std::byte> bytes_of(const std::initializer_list<std::uint8_t> values)
{
	std::vector<std::byte> result;
	result.reserve(values.size());
	for (const auto value : values)
		result.push_back(static_cast<std::byte>(value));
	return result;
}

TEST_CASE("pe image maps sections to their virtual addresses")
{
	tests::PeBuilder builder;
	builder.set_preferred_base(0x140000000);
	builder.add_section(".text", 0x1000, bytes_of({0x48, 0x89, 0x5C}));
	builder.add_data_section(".rdata", 0x2000, bytes_of({0xAA, 0xBB}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->format() == ImageFormat::pe);
	CHECK(image->architecture() == Architecture::x86_64);
	CHECK(image->preferred_base() == 0x140000000);
	CHECK(image->sections().size() == 2);

	const auto text = image->at(0x1000, 3);
	REQUIRE(text.size() == 3);
	CHECK(text[0] == std::byte{0x48});
	CHECK(text[2] == std::byte{0x5C});

	const auto rdata = image->at(0x2000, 2);
	REQUIRE(rdata.size() == 2);
	CHECK(rdata[1] == std::byte{0xBB});
}

TEST_CASE("pe image separates executable sections")
{
	tests::PeBuilder builder;
	builder.add_section(".text", 0x1000, bytes_of({0x90}));
	builder.add_data_section(".rdata", 0x2000, bytes_of({0x00}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	REQUIRE(image->executable_sections().size() == 1);
	CHECK(image->executable_sections()[0].name == ".text");
	CHECK(image->section_containing(0x2000)->name == ".rdata");
	CHECK(image->section_containing(0x9999) == nullptr);
}

TEST_CASE("pe image rejects a mismatched architecture")
{
	tests::PeBuilder builder;
	builder.set_machine(0x014C);
	builder.add_section(".text", 0x1000, bytes_of({0x90}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE_FALSE(image.has_value());
	CHECK(image.error().code() == ErrorCode::invalid_image);
}

TEST_CASE("pe image rejects a 32 bit optional header")
{
	tests::PeBuilder builder;
	builder.set_optional_magic(0x10B);
	builder.add_section(".text", 0x1000, bytes_of({0x90}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE_FALSE(image.has_value());
	CHECK(image.error().code() == ErrorCode::invalid_image);
}

TEST_CASE("image loader rejects an unknown container")
{
	const auto path = tests::TemporaryFile::write(bytes_of({0x00, 0x01, 0x02, 0x03}));

	const auto image = image::ImageLoader::load(path, Architecture::x86_64);

	REQUIRE_FALSE(image.has_value());
	CHECK(image.error().code() == ErrorCode::invalid_image);
}

TEST_CASE("image loader reports a missing file")
{
	const auto image = image::ImageLoader::load("does/not/exist.exe", Architecture::x86_64);

	REQUIRE_FALSE(image.has_value());
	CHECK(image.error().message().find("does/not/exist.exe") != std::string::npos);
}

TEST_CASE("virtual address converts to rva")
{
	tests::PeBuilder builder;
	builder.set_preferred_base(0x140000000);
	builder.add_section(".text", 0x1000, bytes_of({0x90}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->to_rva(0x140001000) == std::optional<Rva>{0x1000});
	CHECK(image->to_rva(0x100) == std::nullopt);
}

TEST_CASE("reads past the end of the image yield nothing")
{
	tests::PeBuilder builder;
	builder.add_section(".text", 0x1000, bytes_of({0x90, 0x90}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->at(0x1000, 2).size() == 2);
	CHECK(image->at(0xFFFFFF0, 2).empty());
	CHECK(image->from(0xFFFFFF0).empty());
}
