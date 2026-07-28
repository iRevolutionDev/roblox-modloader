#pragma once

#include <cstdint>
#include <string_view>

namespace rml::dumper::disasm
{
	enum class Register : std::uint8_t
	{
		none,

		rax,
		rcx,
		rdx,
		rbx,
		rsp,
		rbp,
		rsi,
		rdi,
		r8,
		r9,
		r10,
		r11,
		r12,
		r13,
		r14,
		r15,
		rip,

		x0,
		x1,
		x2,
		x3,
		x4,
		x5,
		x6,
		x7,
		x8,
		x9,
		x10,
		x11,
		x12,
		x13,
		x14,
		x15,
		x16,
		x17,
		x18,
		x19,
		x20,
		x21,
		x22,
		x23,
		x24,
		x25,
		x26,
		x27,
		x28,
		x29,
		x30,
		sp,
		zr,
	};

	[[nodiscard]] constexpr std::string_view to_string(const Register value)
	{
		switch (value)
		{
		case Register::none: return "none";
		case Register::rax: return "rax";
		case Register::rcx: return "rcx";
		case Register::rdx: return "rdx";
		case Register::rbx: return "rbx";
		case Register::rsp: return "rsp";
		case Register::rbp: return "rbp";
		case Register::rsi: return "rsi";
		case Register::rdi: return "rdi";
		case Register::r8: return "r8";
		case Register::r9: return "r9";
		case Register::r10: return "r10";
		case Register::r11: return "r11";
		case Register::r12: return "r12";
		case Register::r13: return "r13";
		case Register::r14: return "r14";
		case Register::r15: return "r15";
		case Register::rip: return "rip";
		case Register::x0: return "x0";
		case Register::x1: return "x1";
		case Register::x2: return "x2";
		case Register::x3: return "x3";
		case Register::x4: return "x4";
		case Register::x5: return "x5";
		case Register::x6: return "x6";
		case Register::x7: return "x7";
		case Register::x8: return "x8";
		case Register::x9: return "x9";
		case Register::x10: return "x10";
		case Register::x11: return "x11";
		case Register::x12: return "x12";
		case Register::x13: return "x13";
		case Register::x14: return "x14";
		case Register::x15: return "x15";
		case Register::x16: return "x16";
		case Register::x17: return "x17";
		case Register::x18: return "x18";
		case Register::x19: return "x19";
		case Register::x20: return "x20";
		case Register::x21: return "x21";
		case Register::x22: return "x22";
		case Register::x23: return "x23";
		case Register::x24: return "x24";
		case Register::x25: return "x25";
		case Register::x26: return "x26";
		case Register::x27: return "x27";
		case Register::x28: return "x28";
		case Register::x29: return "x29";
		case Register::x30: return "x30";
		case Register::sp: return "sp";
		case Register::zr: return "zr";
		}
		return "unknown";
	}

	[[nodiscard]] constexpr Register arm64_general_register(const std::uint8_t number)
	{
		if (number > 30)
			return Register::none;

		return static_cast<Register>(static_cast<std::uint8_t>(Register::x0) + number);
	}
}
