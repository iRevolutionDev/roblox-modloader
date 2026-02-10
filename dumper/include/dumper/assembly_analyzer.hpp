#pragma once

#include <Zydis/Zydis.h>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace dumper
{
	class AssemblyAnalyzer
	{
	public:
		static uint64_t get_displacement(uintptr_t instruction_address);

		static uint64_t find_next_instruction(uintptr_t start_address, ZydisMnemonic_ instruction, int skip_count = 0);

		static bool is_valid_code_pointer(uintptr_t ptr);
	};

}
