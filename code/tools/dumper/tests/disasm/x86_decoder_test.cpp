#include <doctest/doctest.h>

#include "disasm/x86_decoder.hpp"

using namespace rml::dumper;
using namespace rml::dumper::disasm;

static std::vector<std::byte> x86(const std::initializer_list<std::uint8_t> values)
{
	std::vector<std::byte> result;
	result.reserve(values.size());
	for (const auto value : values)
		result.push_back(static_cast<std::byte>(value));
	return result;
}

TEST_CASE("x86 decoder records a qword write through a base register")
{
	// mov [rcx+0x28], rax ; ret
	const auto code = x86({0x48, 0x89, 0x41, 0x28, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);

	const auto& access = trace->accesses[0];
	CHECK(access.is_write);
	CHECK(access.base == Register::rcx);
	CHECK(access.index == Register::none);
	CHECK(access.displacement == 0x28);
	CHECK(access.width == 8);
	CHECK(access.value_register == Register::rax);
	CHECK(access.address == 0x1000);
}

TEST_CASE("x86 decoder records a dword read")
{
	// mov eax, [rdx+0x60] ; ret
	const auto code = x86({0x8B, 0x42, 0x60, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x2000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK_FALSE(trace->accesses[0].is_write);
	CHECK(trace->accesses[0].base == Register::rdx);
	CHECK(trace->accesses[0].displacement == 0x60);
	CHECK(trace->accesses[0].width == 4);
	CHECK(trace->accesses[0].value_register == Register::rax);
}

TEST_CASE("x86 decoder normalises sub registers to their widest form")
{
	// mov byte ptr [rcx+3], 1 ; ret
	const auto code = x86({0xC6, 0x41, 0x03, 0x01, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x3000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::rcx);
	CHECK(trace->accesses[0].width == 1);
	CHECK(trace->accesses[0].immediate == std::optional<std::uint64_t>{1});
}

TEST_CASE("x86 decoder keeps the index register and scale")
{
	// mov rax, [rcx+rdx*8+0x10] ; ret
	const auto code = x86({0x48, 0x8B, 0x44, 0xD1, 0x10, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::rcx);
	CHECK(trace->accesses[0].index == Register::rdx);
	CHECK(trace->accesses[0].scale == 8);
	CHECK(trace->accesses[0].displacement == 0x10);
}

TEST_CASE("x86 decoder resolves a relative call target")
{
	// call +0x10 ; ret
	const auto code = x86({0xE8, 0x10, 0x00, 0x00, 0x00, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->calls[0].target == std::optional<Rva>{0x1015});
	CHECK(trace->calls[0].address == 0x1000);
}

TEST_CASE("x86 decoder reports an indirect call without a target")
{
	// call qword ptr [rax] ; ret
	const auto code = x86({0xFF, 0x10, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->calls[0].target == std::nullopt);
}

TEST_CASE("x86 decoder numbers accesses and calls in a single sequence")
{
	// mov [rcx+8], rax ; call +0 ; mov [rcx+0x10], rdx ; ret
	const auto code = x86({0x48, 0x89, 0x41, 0x08,
	                       0xE8, 0x00, 0x00, 0x00, 0x00,
	                       0x48, 0x89, 0x51, 0x10, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 2);
	REQUIRE(trace->calls.size() == 1);
	CHECK(trace->accesses[0].sequence == 0);
	CHECK(trace->calls[0].sequence == 1);
	CHECK(trace->accesses[1].sequence == 2);
}

TEST_CASE("x86 decoder stops at an invalid instruction")
{
	const auto code = x86({0x48, 0x89, 0x41, 0x28, 0x0F, 0x0B, 0x48, 0x89, 0x41, 0x30});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	CHECK(trace->accesses.size() == 1);
}

TEST_CASE("x86 decoder ignores stack relative accesses")
{
	// mov [rsp+8], rax ; mov [rcx+8], rax ; ret
	const auto code = x86({0x48, 0x89, 0x44, 0x24, 0x08, 0x48, 0x89, 0x41, 0x08, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	REQUIRE(trace->accesses.size() == 1);
	CHECK(trace->accesses[0].base == Register::rcx);
}

TEST_CASE("x86 decoder records the traced range")
{
	const auto code = x86({0x48, 0x89, 0x41, 0x28, 0xC3});

	const auto trace = X86Decoder().trace(code, 0x1000);

	REQUIRE(trace.has_value());
	CHECK(trace->begin == 0x1000);
	CHECK(trace->end == 0x1005);
}
