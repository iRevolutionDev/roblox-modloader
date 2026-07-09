#pragma once

#include <Zydis/Zydis.h>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace dumper
{
	struct ArrayFree
	{
		uint64_t ptr_disp;
		uint64_t size_disp;
		uint32_t elem_size;
	};

	struct MemAccess
	{
		std::size_t seq;
		uintptr_t ip;
		ZydisMnemonic mnemonic;
		bool write;
		ZydisRegister base;
		ZydisRegister index;
		uint8_t scale;
		ZydisRegister reg;
		uint8_t width;
		uint64_t disp;
		bool has_imm;
		uint64_t imm;
	};

	struct CallLandmark
	{
		std::size_t seq;
		uintptr_t ip;
		uintptr_t target;
	};

	struct FunctionTrace
	{
		std::vector<MemAccess> accesses;
		std::vector<CallLandmark> calls;

		std::size_t call_seq(uintptr_t target, int skip = 0) const;
		const MemAccess* nth_write(ZydisRegister base, int n, uint8_t width = 0, std::size_t after = 0, ZydisRegister src = ZYDIS_REGISTER_NONE) const;
		const MemAccess* nth_read(ZydisRegister base, int n, uint8_t width = 0, std::size_t after = 0, ZydisRegister dst = ZYDIS_REGISTER_NONE) const;
		const MemAccess* first_imm_write(ZydisRegister base, uint64_t value, std::size_t after = 0) const;
		const MemAccess* first_indexed(ZydisRegister base, bool write, std::size_t after = 0) const;
		ZydisRegister dominant_write_base() const;
	};

	class AssemblyAnalyzer
	{
	public:
		static constexpr uint64_t invalid_displacement = UINT64_MAX;
		static uint64_t get_displacement(uintptr_t instruction_address);
		static uint64_t get_immediate(uintptr_t instruction_address);
		static uint64_t find_next_instruction(uintptr_t start_address, ZydisMnemonic_ instruction, int skip_count = 0);
		static bool is_valid_code_pointer(uintptr_t ptr);
		static uintptr_t get_call_target(uintptr_t instruction_address);
		static std::vector<ArrayFree> recover_freeproto_arrays(uintptr_t freeproto, uintptr_t luaM_free, std::size_t max_bytes = 0x400);
		static ZydisRegister normalize_register(ZydisRegister reg);
		static FunctionTrace trace_function(uintptr_t fn, std::size_t max_bytes = 0x600);
	};

}
