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
			return 0;
		}

		ZydisDecodedInstruction instruction;
		ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

		if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, buffer, bytes_read, &instruction, operands)))
		{
			return 0;
		}

		for (uint32_t i = 0; i < instruction.operand_count; i++)
		{
			if (const ZydisDecodedOperand& op = operands[i]; op.type == ZYDIS_OPERAND_TYPE_MEMORY)
			{
				if (op.mem.disp.value != 0)
				{
					return static_cast<uint32_t>(op.mem.disp.value);
				}
			}
		}

		return 0;
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

	bool AssemblyAnalyzer::is_valid_code_pointer(const uintptr_t ptr)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(reinterpret_cast<void*>(ptr), &mbi, sizeof(mbi)) == 0)
		{
			return false;
		}

		return mbi.State == MEM_COMMIT && mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
	}
}
