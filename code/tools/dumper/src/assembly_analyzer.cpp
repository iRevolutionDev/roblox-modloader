#include "dumper/assembly_analyzer.hpp"

#include <Windows.h>
#include <Zydis/Zydis.h>

namespace dumper
{
	uint64_t AssemblyAnalyzer::get_displacement(uintptr_t instruction_address)
	{
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		uint8_t buffer[32];
		SIZE_T bytes_read;

		if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(instruction_address), buffer, sizeof(buffer), &bytes_read) || bytes_read == 0)
		{
			return invalid_displacement;
		}

		ZydisDecodedInstruction instruction;
		ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

		if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, buffer, bytes_read, &instruction, operands)))
		{
			return invalid_displacement;
		}

		for (uint32_t i = 0; i < instruction.operand_count; i++)
		{
			if (const ZydisDecodedOperand& op = operands[i]; op.type == ZYDIS_OPERAND_TYPE_MEMORY)
			{
				return static_cast<uint64_t>(op.mem.disp.value);
			}
		}

		return invalid_displacement;
	}
	uint64_t AssemblyAnalyzer::find_next_instruction(uintptr_t start_address, ZydisMnemonic_ instruction, int skip_count)
	{
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		uint8_t buffer[32];
		SIZE_T bytes_read;
		ZydisDecodedInstruction decoded_instruction;

		uintptr_t current = start_address;
		while (true)
		{
			if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(current), buffer, sizeof(buffer), &bytes_read) || bytes_read == 0)
			{
				break;
			}

			if (ZyanStatus status = ZydisDecoderDecodeInstruction(&decoder, nullptr, buffer, bytes_read, &decoded_instruction); ZYAN_SUCCESS(status))
			{
				if (decoded_instruction.mnemonic == instruction)
				{
					if (skip_count-- == 0)
					{
						return current;
					}
				}
			}

			current += decoded_instruction.length ? decoded_instruction.length : 1;
		}

		return 0;
	}

	uint64_t AssemblyAnalyzer::get_immediate(uintptr_t instruction_address)
	{
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		uint8_t buffer[32];
		SIZE_T bytes_read;

		if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(instruction_address), buffer, sizeof(buffer), &bytes_read) || bytes_read == 0)
		{
			return invalid_displacement;
		}

		ZydisDecodedInstruction instruction;
		ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

		if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, buffer, bytes_read, &instruction, operands)))
		{
			return invalid_displacement;
		}

		for (uint32_t i = 0; i < instruction.operand_count; i++)
		{
			if (const ZydisDecodedOperand& op = operands[i]; op.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
			{
				return op.imm.is_signed ? static_cast<uint64_t>(op.imm.value.s) : op.imm.value.u;
			}
		}

		return invalid_displacement;
	}

	bool AssemblyAnalyzer::is_valid_code_pointer(const uintptr_t ptr)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(reinterpret_cast<void*>(ptr), &mbi, sizeof(mbi)) == 0)
		{
			return false;
		}

		return mbi.State == MEM_COMMIT && mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
	}

	uintptr_t AssemblyAnalyzer::get_call_target(uintptr_t instruction_address)
	{
		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		uint8_t buffer[16];
		SIZE_T bytes_read;
		if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(instruction_address), buffer, sizeof(buffer), &bytes_read) || bytes_read == 0)
			return 0;

		ZydisDecodedInstruction insn;
		ZydisDecodedOperand ops[ZYDIS_MAX_OPERAND_COUNT];
		if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, buffer, bytes_read, &insn, ops)))
			return 0;

		if (insn.mnemonic != ZYDIS_MNEMONIC_CALL)
			return 0;

		for (uint32_t i = 0; i < insn.operand_count; ++i)
		{
			if (ops[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE && ops[i].imm.is_relative)
			{
				ZyanU64 target = 0;
				if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&insn, &ops[i], instruction_address, &target)))
					return static_cast<uintptr_t>(target);
			}
		}
		return 0;
	}

	std::vector<ArrayFree> AssemblyAnalyzer::recover_freeproto_arrays(uintptr_t freeproto, uintptr_t luaM_free, std::size_t max_bytes)
	{
		std::vector<ArrayFree> out;
		if (!freeproto || !luaM_free)
			return out;

		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		std::vector<uint8_t> code(max_bytes);
		SIZE_T bytes_read = 0;
		if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(freeproto), code.data(), code.size(), &bytes_read) || bytes_read == 0)
			return out;

		uint64_t ptr_disp = invalid_displacement;
		uint64_t size_disp = invalid_displacement;
		int shift = 0;
		bool mul3 = false;

		std::size_t offset = 0;
		while (offset < bytes_read)
		{
			const uintptr_t ip = freeproto + offset;

			ZydisDecodedInstruction insn;
			ZydisDecodedOperand ops[ZYDIS_MAX_OPERAND_COUNT];
			if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, code.data() + offset, bytes_read - offset, &insn, ops)))
			{
				++offset;
				continue;
			}

			const auto reg0 = insn.operand_count > 0 && ops[0].type == ZYDIS_OPERAND_TYPE_REGISTER ? ops[0].reg.value : ZYDIS_REGISTER_NONE;

			switch (insn.mnemonic)
			{
			case ZYDIS_MNEMONIC_MOV:
				if (insn.operand_count >= 2 && ops[1].type == ZYDIS_OPERAND_TYPE_MEMORY)
				{
					if (reg0 == ZYDIS_REGISTER_RDX)
						ptr_disp = static_cast<uint64_t>(ops[1].mem.disp.value);
					else if (reg0 == ZYDIS_REGISTER_R8D)
						size_disp = static_cast<uint64_t>(ops[1].mem.disp.value);
				}
				break;

			case ZYDIS_MNEMONIC_MOVSXD:
				if (insn.operand_count >= 2 && ops[1].type == ZYDIS_OPERAND_TYPE_MEMORY)
					size_disp = static_cast<uint64_t>(ops[1].mem.disp.value);
				break;

			case ZYDIS_MNEMONIC_SHL:
				if (reg0 == ZYDIS_REGISTER_R8 && insn.operand_count >= 2 && ops[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
					shift = static_cast<int>(ops[1].imm.value.u);
				break;

			case ZYDIS_MNEMONIC_LEA:
				if (reg0 == ZYDIS_REGISTER_R8 && insn.operand_count >= 2 && ops[1].type == ZYDIS_OPERAND_TYPE_MEMORY
				    && ops[1].mem.index != ZYDIS_REGISTER_NONE && ops[1].mem.scale == 2)
					mul3 = true;
				break;

			case ZYDIS_MNEMONIC_CALL:
				if (get_call_target(ip) == luaM_free && (ptr_disp != invalid_displacement || size_disp != invalid_displacement))
				{
					uint32_t elem = mul3 ? 24u : (shift ? (1u << shift) : 1u);
					out.push_back({ptr_disp, size_disp, elem});
					ptr_disp = invalid_displacement;
					size_disp = invalid_displacement;
					shift = 0;
					mul3 = false;
				}
				break;

			default: break;
			}

			offset += insn.length ? insn.length : 1;
		}

		return out;
	}

	ZydisRegister AssemblyAnalyzer::normalize_register(ZydisRegister reg)
	{
		if (reg == ZYDIS_REGISTER_NONE)
			return ZYDIS_REGISTER_NONE;

		const ZydisRegister enclosing = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, reg);
		return enclosing == ZYDIS_REGISTER_NONE ? reg : enclosing;
	}

	std::size_t FunctionTrace::call_seq(uintptr_t target, int skip) const
	{
		for (const auto& c : calls)
		{
			if (c.target == target && skip-- == 0)
				return c.seq;
		}
		return SIZE_MAX;
	}

	const MemAccess* FunctionTrace::nth_write(ZydisRegister base, int n, uint8_t width, std::size_t after, ZydisRegister src) const
	{
		base = AssemblyAnalyzer::normalize_register(base);
		src = AssemblyAnalyzer::normalize_register(src);
		for (const auto& a : accesses)
		{
			if (!a.write || a.base != base || a.index != ZYDIS_REGISTER_NONE || a.seq < after)
				continue;
			if (width && a.width != width)
				continue;
			if (src != ZYDIS_REGISTER_NONE && a.reg != src)
				continue;
			if (n-- == 0)
				return &a;
		}
		return nullptr;
	}

	const MemAccess* FunctionTrace::nth_read(ZydisRegister base, int n, uint8_t width, std::size_t after, ZydisRegister dst) const
	{
		base = AssemblyAnalyzer::normalize_register(base);
		dst = AssemblyAnalyzer::normalize_register(dst);
		for (const auto& a : accesses)
		{
			if (a.write || a.base != base || a.index != ZYDIS_REGISTER_NONE || a.seq < after)
				continue;
			if (width && a.width != width)
				continue;
			if (dst != ZYDIS_REGISTER_NONE && a.reg != dst)
				continue;
			if (n-- == 0)
				return &a;
		}
		return nullptr;
	}

	const MemAccess* FunctionTrace::first_imm_write(ZydisRegister base, uint64_t value, std::size_t after) const
	{
		base = AssemblyAnalyzer::normalize_register(base);
		for (const auto& a : accesses)
		{
			if (a.write && a.has_imm && a.imm == value && a.base == base && a.seq >= after)
				return &a;
		}
		return nullptr;
	}

	const MemAccess* FunctionTrace::first_indexed(ZydisRegister base, bool write, std::size_t after) const
	{
		base = AssemblyAnalyzer::normalize_register(base);
		for (const auto& a : accesses)
		{
			if (a.write == write && a.base == base && a.index != ZYDIS_REGISTER_NONE && a.seq >= after)
				return &a;
		}
		return nullptr;
	}

	ZydisRegister FunctionTrace::dominant_write_base() const
	{
		std::map<ZydisRegister, int> counts;
		for (const auto& a : accesses)
		{
			if (a.write && a.base != ZYDIS_REGISTER_NONE && a.base != ZYDIS_REGISTER_RSP && a.base != ZYDIS_REGISTER_RBP && a.index == ZYDIS_REGISTER_NONE)
				++counts[a.base];
		}

		ZydisRegister best = ZYDIS_REGISTER_NONE;
		int best_count = 0;
		for (const auto& [reg, count] : counts)
		{
			if (count > best_count)
			{
				best = reg;
				best_count = count;
			}
		}
		return best;
	}

	FunctionTrace AssemblyAnalyzer::trace_function(uintptr_t fn, std::size_t max_bytes)
	{
		FunctionTrace trace;
		if (!fn)
			return trace;

		ZydisDecoder decoder;
		ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

		std::vector<uint8_t> code(max_bytes);
		SIZE_T bytes_read = 0;
		if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(fn), code.data(), code.size(), &bytes_read) || bytes_read == 0)
			return trace;

		std::size_t seq = 0;
		std::size_t offset = 0;
		while (offset < bytes_read)
		{
			const uintptr_t ip = fn + offset;

			ZydisDecodedInstruction insn;
			ZydisDecodedOperand ops[ZYDIS_MAX_OPERAND_COUNT];
			if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, code.data() + offset, bytes_read - offset, &insn, ops)))
			{
				++offset;
				++seq;
				continue;
			}

			if (insn.mnemonic == ZYDIS_MNEMONIC_INT3)
				break;

			const ZydisDecodedOperand* mem = nullptr;
			ZydisRegister role_reg = ZYDIS_REGISTER_NONE;
			bool has_imm = false;
			uint64_t imm_value = 0;

			for (uint32_t i = 0; i < insn.operand_count; ++i)
			{
				const ZydisDecodedOperand& op = ops[i];
				if (op.visibility != ZYDIS_OPERAND_VISIBILITY_EXPLICIT)
					continue;

				if (op.type == ZYDIS_OPERAND_TYPE_MEMORY && op.mem.base != ZYDIS_REGISTER_NONE && op.mem.base != ZYDIS_REGISTER_RIP)
				{
					if (!mem)
						mem = &op;
				}
				else if (op.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
				{
					has_imm = true;
					imm_value = op.imm.is_signed ? static_cast<uint64_t>(op.imm.value.s) : op.imm.value.u;
				}
			}

			const bool real_access = insn.mnemonic != ZYDIS_MNEMONIC_LEA && insn.mnemonic != ZYDIS_MNEMONIC_NOP;

			if (mem && real_access)
			{
				for (uint32_t i = 0; i < insn.operand_count; ++i)
				{
					const ZydisDecodedOperand& op = ops[i];
					if (op.visibility == ZYDIS_OPERAND_VISIBILITY_EXPLICIT && op.type == ZYDIS_OPERAND_TYPE_REGISTER)
					{
						role_reg = op.reg.value;
						break;
					}
				}

				MemAccess access{};
				access.seq = seq;
				access.ip = ip;
				access.mnemonic = insn.mnemonic;
				access.write = (mem->actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0;
				access.base = normalize_register(mem->mem.base);
				access.index = normalize_register(mem->mem.index);
				access.scale = mem->mem.scale;
				access.reg = normalize_register(role_reg);
				access.width = static_cast<uint8_t>(mem->size / 8);
				access.disp = static_cast<uint64_t>(mem->mem.disp.value);
				access.has_imm = has_imm;
				access.imm = imm_value;
				trace.accesses.push_back(access);
			}

			if (insn.mnemonic == ZYDIS_MNEMONIC_CALL)
				trace.calls.push_back({seq, ip, get_call_target(ip)});

			offset += insn.length ? insn.length : 1;
			++seq;
		}

		return trace;
	}
}
