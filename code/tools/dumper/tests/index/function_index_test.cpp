#include <doctest/doctest.h>

#include "rml/dumper/image/image.hpp"
#include "rml/dumper/index/function_index.hpp"
#include "support/pe_builder.hpp"

using namespace rml::dumper;
using index::FunctionBounds;
using index::FunctionIndex;

TEST_CASE("function index finds the function containing an address")
{
	const FunctionIndex functions(std::vector<FunctionBounds>{{0x1000, 0x1040}, {0x1040, 0x10C0}, {0x2000, 0x2010}});

	CHECK(functions.size() == 3);
	CHECK(functions.containing(0x1000)->begin == 0x1000);
	CHECK(functions.containing(0x103F)->begin == 0x1000);
	CHECK(functions.containing(0x1040)->begin == 0x1040);
	CHECK(functions.containing(0x10BF)->begin == 0x1040);
	CHECK(functions.containing(0x10C0) == std::nullopt);
	CHECK(functions.containing(0x2005)->end == 0x2010);
	CHECK(functions.containing(0x9999) == std::nullopt);
	CHECK(functions.containing(0x0) == std::nullopt);
}

TEST_CASE("function index looks up an exact start")
{
	const FunctionIndex functions(std::vector<FunctionBounds>{{0x1000, 0x1040}, {0x2000, 0x2010}});

	CHECK(functions.at(0x2000)->size() == 0x10);
	CHECK(functions.at(0x2004) == std::nullopt);
	CHECK(functions.at(0x1000)->end == 0x1040);
}

TEST_CASE("function index sorts entries given out of order")
{
	const FunctionIndex functions(std::vector<FunctionBounds>{{0x2000, 0x2010}, {0x1000, 0x1040}});

	CHECK(functions.containing(0x1010)->begin == 0x1000);
	CHECK(functions.containing(0x2008)->begin == 0x2000);
}

TEST_CASE("an empty function index answers nothing")
{
	const FunctionIndex functions;

	CHECK(functions.empty());
	CHECK(functions.containing(0x1000) == std::nullopt);
	CHECK(functions.at(0x1000) == std::nullopt);
}

TEST_CASE("pe image builds its function index from the exception directory")
{
	const std::vector<std::pair<Rva, Rva>> entries{{0x1000, 0x1040}, {0x1040, 0x1100}};

	tests::PeBuilder builder;
	builder.set_preferred_base(0x140000000);
	builder.add_section(".text", 0x1000, std::vector<std::byte>(0x400, std::byte{0x90}));
	builder.add_data_section(".pdata", 0x2000, tests::PeBuilder::runtime_function_table(entries));
	builder.set_directory(3, 0x2000, static_cast<std::uint32_t>(entries.size() * 12));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);
	REQUIRE(image.has_value());

	const auto& functions = image->functions();
	CHECK(functions.size() == 2);
	CHECK(functions.containing(0x1050)->begin == 0x1040);
	CHECK(functions.containing(0x1050)->end == 0x1100);
	CHECK(functions.at(0x1000)->size() == 0x40);
}

TEST_CASE("the function index drops entries outside executable sections")
{
	const std::vector<std::pair<Rva, Rva>> entries{{0x1000, 0x1040}, {0x5000, 0x5040}, {0x1080, 0x1080}};

	tests::PeBuilder builder;
	builder.add_section(".text", 0x1000, std::vector<std::byte>(0x400, std::byte{0x90}));
	builder.add_data_section(".pdata", 0x2000, tests::PeBuilder::runtime_function_table(entries));
	builder.set_directory(3, 0x2000, static_cast<std::uint32_t>(entries.size() * 12));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);
	REQUIRE(image.has_value());

	CHECK(image->functions().size() == 1);
	CHECK(image->functions().at(0x1000).has_value());
}

TEST_CASE("a pe image without an exception directory has an empty function index")
{
	tests::PeBuilder builder;
	builder.add_section(".text", 0x1000, std::vector<std::byte>(0x40, std::byte{0x90}));

	const auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);

	REQUIRE(image.has_value());
	CHECK(image->functions().empty());
}
