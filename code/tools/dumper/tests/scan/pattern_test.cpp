#include <doctest/doctest.h>

#include "rml/dumper/scan/pattern.hpp"

using namespace rml::dumper;
using namespace rml::dumper::scan;

static std::vector<std::byte> scan_bytes(const std::initializer_list<std::uint8_t> values)
{
	std::vector<std::byte> result;
	result.reserve(values.size());
	for (const auto value : values)
		result.push_back(static_cast<std::byte>(value));
	return result;
}

TEST_CASE("pattern parses an ida signature")
{
	const auto pattern = Pattern::parse("48 89 5C 24 ? 57");

	REQUIRE(pattern.has_value());
	CHECK(pattern->size() == 6);
	CHECK(pattern->matches_at(scan_bytes({0x48, 0x89, 0x5C, 0x24, 0xFF, 0x57}), 0));
	CHECK(pattern->matches_at(scan_bytes({0x48, 0x89, 0x5C, 0x24, 0x00, 0x57}), 0));
	CHECK_FALSE(pattern->matches_at(scan_bytes({0x48, 0x89, 0x5C, 0x24, 0xFF, 0x58}), 0));
}

TEST_CASE("pattern matches at an offset and respects the buffer end")
{
	const auto pattern = Pattern::parse("AA BB");

	REQUIRE(pattern.has_value());
	CHECK(pattern->matches_at(scan_bytes({0x00, 0xAA, 0xBB}), 1));
	CHECK_FALSE(pattern->matches_at(scan_bytes({0x00, 0xAA, 0xBB}), 2));
	CHECK_FALSE(pattern->matches_at(scan_bytes({0xAA}), 0));
}

TEST_CASE("pattern accepts double question marks")
{
	const auto pattern = Pattern::parse("48 ?? 5C");

	REQUIRE(pattern.has_value());
	CHECK(pattern->size() == 3);
	CHECK(pattern->matches_at(scan_bytes({0x48, 0x12, 0x5C}), 0));
}

TEST_CASE("pattern is case insensitive")
{
	const auto lower = Pattern::parse("de ad be ef");
	const auto upper = Pattern::parse("DE AD BE EF");

	REQUIRE(lower.has_value());
	REQUIRE(upper.has_value());
	CHECK(lower->matches_at(scan_bytes({0xDE, 0xAD, 0xBE, 0xEF}), 0));
	CHECK(upper->matches_at(scan_bytes({0xDE, 0xAD, 0xBE, 0xEF}), 0));
}

TEST_CASE("pattern rejects malformed input")
{
	CHECK_FALSE(Pattern::parse("").has_value());
	CHECK_FALSE(Pattern::parse("   ").has_value());
	CHECK_FALSE(Pattern::parse("4").has_value());
	CHECK_FALSE(Pattern::parse("48 8").has_value());
	CHECK_FALSE(Pattern::parse("ZZ").has_value());
	CHECK_FALSE(Pattern::parse("48 ZZ").has_value());
}

TEST_CASE("pattern rejects a signature with no concrete byte")
{
	const auto pattern = Pattern::parse("? ? ?");

	REQUIRE_FALSE(pattern.has_value());
	CHECK(pattern.error().message().find("concrete") != std::string::npos);
}

TEST_CASE("pattern reports its first concrete byte")
{
	const auto leading = Pattern::parse("? ? 5C 24");
	const auto immediate = Pattern::parse("5C 24");

	REQUIRE(leading.has_value());
	REQUIRE(immediate.has_value());
	CHECK(leading->first_concrete_index() == 2);
	CHECK(leading->first_concrete_byte() == std::byte{0x5C});
	CHECK(immediate->first_concrete_index() == 0);
}
