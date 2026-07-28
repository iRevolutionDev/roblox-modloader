#include "disasm/x86_decoder.hpp"

#include <Zydis/Zydis.h>

#include <array>

namespace rml::dumper::disasm
{
	static Register to_register(const ZydisRegister value)
	{
		switch (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, value))
		{
		case ZYDIS_REGISTER_RAX: return Register::rax;
		case ZYDIS_REGISTER_RCX: return Register::rcx;
		case ZYDIS_REGISTER_RDX: return Register::rdx;
		case ZYDIS_REGISTER_RBX: return Register::rbx;
		case ZYDIS_REGISTER_RSP: return Register::rsp;
		case ZYDIS_REGISTER_RBP: return Register::rbp;
		case ZYDIS_REGISTER_RSI: return Register::rsi;
		case ZYDIS_REGISTER_RDI: return Register::rdi;
		case ZYDIS_REGISTER_R8: return Register::r8;
		case ZYDIS_REGISTER_R9: return Register::r9;
		case ZYDIS_REGISTER_R10: return Register::r10;
		case ZYDIS_REGISTER_R11: return Register::r11;
		case ZYDIS_REGISTER_R12: return Register::r12;
		case ZYDIS_REGISTER_R13: return Register::r13;
		case ZYDIS_REGISTER_R14: return Register::r14;
		case ZYDIS_REGISTER_R15: return Register::r15;
		case ZYDIS_REGISTER_RIP: return Register::rip;
		default: return Register::none;
		}
	}

	class ValueOrigins
	{
	public:
		ValueOrigins()
		{
			for (std::size_t i = 0; i < m_origins.size(); ++i)
				m_origins[i] = static_cast<Register>(i);
		}

		[[nodiscard]] Register of(const Register value) const
		{
			const auto index = static_cast<std::size_t>(value);
			return index < m_origins.size() ? m_origins[index] : value;
		}

		void alias(const Register destination, const Register source)
		{
			if (const auto index = static_cast<std::size_t>(destination); index < m_origins.size())
				m_origins[index] = of(source);
		}

		void forget(const Register destination)
		{
			if (const auto index = static_cast<std::size_t>(destination); index < m_origins.size())
				m_origins[index] = destination;
		}

	private:
		std::array<Register, static_cast<std::size_t>(Register::rip) + 1> m_origins{};
	};

	static bool stops_the_trace(const ZydisMnemonic mnemonic)
	{
		return mnemonic == ZYDIS_MNEMONIC_UD2 || mnemonic == ZYDIS_MNEMONIC_INT3;
	}

	static std::optional<std::uint64_t> first_immediate(const ZydisDecodedInstruction& instruction,
	                                                    const ZydisDecodedOperand* operands)
	{
		for (std::uint8_t i = 0; i < instruction.operand_count_visible; ++i)
			if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
				return operands[i].imm.value.u;

		return std::nullopt;
	}

	static Register companion_register(const ZydisDecodedInstruction& instruction,
	                                   const ZydisDecodedOperand* operands, const std::uint8_t memory_operand)
	{
		for (std::uint8_t i = 0; i < instruction.operand_count_visible; ++i)
		{
			if (i == memory_operand || operands[i].type != ZYDIS_OPERAND_TYPE_REGISTER)
				continue;

			if (const auto mapped = to_register(operands[i].reg.value); mapped != Register::none)
				return mapped;
		}

		return Register::none;
	}

	std::expected<Trace, Error> X86Decoder::trace(const std::span<const std::byte> code, const Rva begin) const
	{
		ZydisDecoder decoder;
		if (ZYAN_FAILED(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)))
			return std::unexpected(Error::make(ErrorCode::recovery, "cannot initialise the x86 decoder"));

		Trace trace;
		trace.begin = begin;
		trace.end = begin;

		ValueOrigins origins;
		std::size_t sequence = 0;
		std::size_t offset = 0;

		while (offset < code.size())
		{
			ZydisDecodedInstruction instruction;
			ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

			if (ZYAN_FAILED(ZydisDecoderDecodeFull(&decoder, code.data() + offset, code.size() - offset,
			                                       &instruction, operands)))
				break;

			if (stops_the_trace(instruction.mnemonic))
				break;

			const auto address = static_cast<Rva>(begin + offset);

			if (instruction.mnemonic == ZYDIS_MNEMONIC_CALL)
			{
				CallSite call;
				call.sequence = sequence++;
				call.address = address;

				for (std::uint8_t i = 0; i < instruction.operand_count_visible; ++i)
				{
					if (operands[i].type != ZYDIS_OPERAND_TYPE_IMMEDIATE || !operands[i].imm.is_relative)
						continue;

					ZyanU64 absolute = 0;
					if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&instruction, &operands[i], address, &absolute)))
						call.target = static_cast<Rva>(absolute);
				}

				trace.calls.push_back(call);
			}

			for (std::uint8_t i = 0; i < instruction.operand_count_visible; ++i)
			{
				const auto& operand = operands[i];
				if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				const auto base = to_register(operand.mem.base);
				if (base == Register::none || base == Register::rsp)
					continue;

				MemoryAccess access;
				access.sequence = sequence++;
				access.address = address;
				access.base = base;
				access.index = to_register(operand.mem.index);
				access.scale = operand.mem.scale != 0 ? operand.mem.scale : 1;
				access.width = static_cast<std::uint8_t>(operand.size / 8);
				access.displacement = operand.mem.disp.has_displacement ? operand.mem.disp.value : 0;
				access.is_write = (operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0;
				access.value_register = origins.of(companion_register(instruction, operands, i));
				access.immediate = first_immediate(instruction, operands);

				trace.accesses.push_back(access);
			}

			if (instruction.mnemonic == ZYDIS_MNEMONIC_MOV && instruction.operand_count_visible == 2 &&
			    operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER && operands[1].type == ZYDIS_OPERAND_TYPE_REGISTER)
			{
				origins.alias(to_register(operands[0].reg.value), to_register(operands[1].reg.value));
			}
			else
			{
				for (std::uint8_t i = 0; i < instruction.operand_count_visible; ++i)
					if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
					    (operands[i].actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0)
						origins.forget(to_register(operands[i].reg.value));
			}

			offset += instruction.length;
			trace.end = static_cast<Rva>(begin + offset);
		}

		return trace;
	}
}
