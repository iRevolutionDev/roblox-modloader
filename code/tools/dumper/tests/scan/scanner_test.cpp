#include <doctest/doctest.h>

#include "rml/dumper/image/image.hpp"
#include "rml/dumper/scan/scanner.hpp"
#include "support/pe_builder.hpp"

#include <algorithm>
#include <functional>

using namespace rml::dumper;

static image::Image build_image(const std::function<void(tests::PeBuilder&)>& configure)
{
	tests::PeBuilder builder;
	configure(builder);

	auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);
	REQUIRE(image.has_value());

	return std::move(*image);
}

TEST_CASE("scanner finds every occurrence of every pattern in one pass")
{
	std::vector<std::byte> code(0x800, std::byte{0x90});
	const std::vector<std::byte> needle_a{std::byte{0x48}, std::byte{0x89}, std::byte{0x5C}};
	const std::vector<std::byte> needle_b{std::byte{0xE8}, std::byte{0x11}, std::byte{0x22}};
	std::ranges::copy(needle_a, code.begin() + 0x100);
	std::ranges::copy(needle_a, code.begin() + 0x600);
	std::ranges::copy(needle_b, code.begin() + 0x300);

	const auto image = build_image([&code](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, code);
	});

	const std::vector patterns{*scan::Pattern::parse("48 89 5C"), *scan::Pattern::parse("E8 ? 22"),
	                           *scan::Pattern::parse("CC CC CC CC")};

	const auto hits = scan::Scanner().scan(image, patterns);

	REQUIRE(hits.size() == 3);
	CHECK(hits[0] == std::vector<Rva>{0x1100, 0x1600});
	CHECK(hits[1] == std::vector<Rva>{0x1300});
	CHECK(hits[2].empty());
}

TEST_CASE("scanner finds a match that straddles its internal block boundary")
{
	std::vector<std::byte> code(1 << 20, std::byte{0x90});
	const std::vector<std::byte> needle{std::byte{0x11}, std::byte{0x22}, std::byte{0x33}, std::byte{0x44},
	                                    std::byte{0x55}, std::byte{0x66}, std::byte{0x77}, std::byte{0x88}};

	const std::size_t position = (1 << 19) - 3;
	std::ranges::copy(needle, code.begin() + position);

	const auto image = build_image([&code](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, code);
	});

	const std::vector patterns{*scan::Pattern::parse("11 22 33 44 55 66 77 88")};
	const auto hits = scan::Scanner().scan(image, patterns);

	REQUIRE(hits[0].size() == 1);
	CHECK(hits[0][0] == 0x1000 + position);
}

TEST_CASE("scanner only searches executable sections")
{
	const std::vector<std::byte> needle{std::byte{0x48}, std::byte{0x89}, std::byte{0x5C}};

	const auto image = build_image([&needle](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, std::vector<std::byte>(0x100, std::byte{0x90}));
		builder.add_data_section(".rdata", 0x2000, needle);
	});

	const std::vector patterns{*scan::Pattern::parse("48 89 5C")};

	CHECK(scan::Scanner().scan(image, patterns)[0].empty());
}

TEST_CASE("scanner searches every executable section")
{
	std::vector<std::byte> first(0x100, std::byte{0x90});
	std::vector<std::byte> second(0x100, std::byte{0x90});
	first[0x10] = std::byte{0xAB};
	second[0x20] = std::byte{0xAB};

	const auto image = build_image([&](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, first);
		builder.add_section(".text2", 0x2000, second);
	});

	const std::vector patterns{*scan::Pattern::parse("AB")};
	const auto hits = scan::Scanner().scan(image, patterns);

	REQUIRE(hits[0].size() == 2);
	CHECK(hits[0][0] == 0x1010);
	CHECK(hits[0][1] == 0x2020);
}

TEST_CASE("scanner reports hits in ascending order")
{
	std::vector<std::byte> code(1 << 21, std::byte{0x90});
	for (const std::size_t position : {0x100u, 0x40000u, 0x80000u, 0x100000u, 0x1F0000u})
		code[position] = std::byte{0xCC};

	const auto image = build_image([&code](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, code);
	});

	const std::vector patterns{*scan::Pattern::parse("CC")};
	const auto hits = scan::Scanner().scan(image, patterns);

	REQUIRE(hits[0].size() == 5);
	CHECK(std::ranges::is_sorted(hits[0]));
	CHECK(hits[0][0] == 0x1100);
	CHECK(hits[0][4] == 0x1000 + 0x1F0000);
}

TEST_CASE("scanning without patterns returns nothing")
{
	const auto image = build_image([](tests::PeBuilder& builder) {
		builder.add_section(".text", 0x1000, std::vector<std::byte>(0x100, std::byte{0x90}));
	});

	CHECK(scan::Scanner().scan(image, {}).empty());
}
