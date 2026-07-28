#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "rml/dumper/core/error.hpp"
#include "rml/dumper/core/types.hpp"

using namespace rml::dumper;

TEST_CASE("error carries a code and a formatted message")
{
	const auto error = Error::make(ErrorCode::invalid_image, "bad magic 0x{:X}", 0x1234);

	CHECK(error.code() == ErrorCode::invalid_image);
	CHECK(error.message() == "bad magic 0x1234");
}

TEST_CASE("architectures and formats have stable names")
{
	CHECK(to_string(Architecture::x86_64) == "x86_64");
	CHECK(to_string(Architecture::arm64) == "arm64");
	CHECK(to_string(ImageFormat::pe) == "pe");
	CHECK(to_string(ImageFormat::mach_o) == "mach-o");
}
