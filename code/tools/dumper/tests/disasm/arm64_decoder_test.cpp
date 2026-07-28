#include <doctest/doctest.h>

#include "disasm/arm64_decoder.hpp"

using namespace rml::dumper;
using namespace rml::dumper::disasm;

static std::vector<std::byte> arm64(const std::initializer_list<std::uint32_t> words)
{
	std::vector<std::byte> bytes;
	bytes.reserve(words.size() * 4);

	for (const auto word : words)
		for (int i = 0; i < 4; ++i)
			bytes.push_back(static_cast<std::byte>(word >> (i * 8) & 0xFF));

	return bytes;
}

TEST_CASE("arm64 decoder records a scaled store")
{
	// str x8, [x19, #0x28] ; ret
	const auto code = arm64({0xF9001668, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].is_write);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].value_register == Register::x8);
	CHECK(trace->accesses[0].displacement == 0x28);
	CHECK(trace->accesses[0].width == 8);
	CHECK(trace->accesses[0].address == 0x1000);
}

TEST_CASE("arm64 decoder records a scaled load")
{
	// ldr x0, [x1, #0x60] ; ret
	const auto code = arm64({0xF9403020, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK_FALSE(trace->accesses[0].is_write);
	CHECK(trace->accesses[0].base == Register::x1);
	CHECK(trace->accesses[0].value_register == Register::x0);
	CHECK(trace->accesses[0].displacement == 0x60);
	CHECK(trace->accesses[0].width == 8);
}

TEST_CASE("arm64 decoder records an access with no displacement")
{
	// ldr x1, [x9] ; ret
	const auto code = arm64({0xF9400121, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x9);
	CHECK(trace->accesses[0].displacement == 0);
}

TEST_CASE("arm64 decoder records an unscaled load")
{
	// ldur x0, [x1, #8] ; ret
	const auto code = arm64({0xF8408020, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x1);
	CHECK(trace->accesses[0].displacement == 8);
	CHECK(trace->accesses[0].width == 8);
}

TEST_CASE("arm64 decoder records byte and word widths")
{
	// ldrb w0, [x1, #3] ; str w2, [x3, #0x10] ; ret
	const auto code = arm64({0x39400C20, 0xB9001062, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 2);
	CHECK(trace->accesses[0].width == 1);
	CHECK(trace->accesses[0].displacement == 3);
	CHECK_FALSE(trace->accesses[0].is_write);
	CHECK(trace->accesses[1].width == 4);
	CHECK(trace->accesses[1].displacement == 0x10);
	CHECK(trace->accesses[1].is_write);
}

TEST_CASE("arm64 decoder splits a store pair into two accesses")
{
	// stp x20, x21, [x19, #0x10] ; ret
	const auto code = arm64({0xA9015674, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 2);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].displacement == 0x10);
	CHECK(trace->accesses[0].value_register == Register::x20);
	CHECK(trace->accesses[0].width == 8);
	CHECK(trace->accesses[1].base == Register::x19);
	CHECK(trace->accesses[1].displacement == 0x18);
	CHECK(trace->accesses[1].value_register == Register::x21);
	CHECK(trace->accesses[0].sequence != trace->accesses[1].sequence);
}

TEST_CASE("arm64 decoder folds an add immediate into the effective displacement")
{
	// add x9, x19, #1, lsl #12 ; ldr x0, [x9, #8] ; ret
	const auto code = arm64({0x91400669, 0xF9400520, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].displacement == 0x1008);
}

TEST_CASE("arm64 decoder folds a subtract immediate")
{
	// sub x9, x19, #1, lsl #12 ; ldr x0, [x9, #8] ; ret
	const auto code = arm64({0xD1400669, 0xF9400520, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].displacement == 8 - 0x1000);
}

TEST_CASE("arm64 decoder follows a register move when folding")
{
	// mov x9, x19 ; ldr x0, [x9, #8] ; ret
	const auto code = arm64({0xAA1303E9, 0xF9400520, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].displacement == 8);
}

TEST_CASE("arm64 decoder forgets a folded register once it is overwritten")
{
	// add x9, x19, #1, lsl #12 ; mov x9, #0x1234 ; ldr x0, [x9, #8] ; ret
	const auto code = arm64({0x91400669, 0xD2824689, 0xF9400520, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x9);
	CHECK(trace->accesses[0].displacement == 8);
}

TEST_CASE("arm64 decoder records a floating point load with no value register")
{
	// ldr d0, [x19, #0x28] ; ret
	const auto code = arm64({0xFD401660, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x19);
	CHECK(trace->accesses[0].displacement == 0x28);
	CHECK(trace->accesses[0].width == 8);
	CHECK(trace->accesses[0].value_register == Register::none);
}

TEST_CASE("arm64 decoder resolves a branch with link target")
{
	// bl +0x20 ; ret
	const auto code = arm64({0x94000008, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->calls[0].target == std::optional<Rva>{0x1020});
	CHECK(trace->calls[0].address == 0x1000);
}

TEST_CASE("arm64 decoder reports an indirect call without a target")
{
	// blr x8 ; ret
	const auto code = arm64({0xD63F0100, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->calls[0].target == std::nullopt);
}

TEST_CASE("arm64 decoder ignores stack relative accesses")
{
	// str x8, [sp, #8] ; str x8, [x19, #0x28] ; ret
	const auto code = arm64({0xF90007E8, 0xF9001668, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::x19);
}

TEST_CASE("arm64 decoder numbers accesses and calls in a single sequence")
{
	// str x8, [x19, #0x28] ; bl +0x20 ; ldr x0, [x1, #0x60] ; ret
	const auto code = arm64({0xF9001668, 0x94000008, 0xF9403020, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 2);
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->accesses[0].sequence == 0);
	CHECK(trace->calls[0].sequence == 1);
	CHECK(trace->accesses[1].sequence == 2);
}

TEST_CASE("arm64 decoder records the traced range")
{
	const auto code = arm64({0xF9001668, 0xD65F03C0});

	const auto trace = Arm64Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	CHECK(trace->begin == 0x1000);
	CHECK(trace->end == 0x1008);
}

TEST_CASE("the decoder factory hands out an arm64 decoder")
{
	const auto decoder = Decoder::create(Architecture::arm64);

	REQUIRE(decoder.has_value());
	CHECK((*decoder)->architecture() == Architecture::arm64);
}
