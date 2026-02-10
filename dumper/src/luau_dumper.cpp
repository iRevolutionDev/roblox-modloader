#include "dumper/luau_dumper.hpp"

#include "dumper/assembly_analyzer.hpp"
#include "dumper/common.hpp"
#include "dumper/pointers.hpp"

namespace dumper
{

	LuauDumper::LuauDumper(std::unique_ptr<pointers>& pointers) :
	    m_base_address(0),
	    m_pointers(std::move(pointers))
	{
		m_base_address = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
		spdlog::info("Base address: 0x{:X}", m_base_address);
	}

	bool LuauDumper::analyze()
	{
		spdlog::info("Initializing Luau Dumper...");

		bool success = true;
		success &= analyze_proto();
		success &= analyze_lua_state();
		success &= analyze_table();
		success &= analyze_closure();
		success &= analyze_upval();
		success &= analyze_tvalue();
		success &= analyze_global_state();

		success &= generate_offsets();

		if (success)
		{
			spdlog::info("Analysis successful! {} structures found", m_structures.size());
		}
		else
		{
			spdlog::error("Analysis failed!");
		}

		return success;
	}

	const std::map<std::string, StructInfo>& LuauDumper::get_structures() const
	{
		return m_structures;
	}

	const StructInfo* LuauDumper::find_structure(const std::string& name) const
	{
		const auto it = m_structures.find(name);
		return it != m_structures.end() ? &it->second : nullptr;
	}

	bool LuauDumper::analyze_lua_state()
	{
		spdlog::info("Analyzing lua_State...");

		if (m_pointers->m_luau_functions.thread == 0)
		{
			spdlog::error("lua_State function pointer is null, cannot analyze lua_State structure");
			return false;
		}

		const auto stack_size = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.thread, ZYDIS_MNEMONIC_LEA);
		const auto size_ci = stack_size - 0x3;
		const auto gt = AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(size_ci, ZYDIS_MNEMONIC_MOVZX), ZYDIS_MNEMONIC_MOV, 1);
		const auto base_ci = AssemblyAnalyzer::find_next_instruction(gt, ZYDIS_MNEMONIC_MOV, 2);
		const auto ci      = base_ci + 0x4;
		const auto top     = AssemblyAnalyzer::find_next_instruction(m_proto.linedefined, ZYDIS_MNEMONIC_CMP, 0);
		const auto stack   = AssemblyAnalyzer::find_next_instruction(m_proto.linedefined, ZYDIS_MNEMONIC_MOV, 2);

		StructInfo lua_state;
		lua_state.name = "lua_State";
		lua_state.fields.push_back({"stacksize", stack_size, sizeof(int32_t), "int32_t"});
		lua_state.fields.push_back({"size_ci", size_ci, sizeof(int32_t), "int32_t"});
		lua_state.fields.push_back({"gt", gt, sizeof(void*), "void*"});
		lua_state.fields.push_back({"base_ci", base_ci, sizeof(void*), "void*"});
		lua_state.fields.push_back({"ci", ci, sizeof(void*), "void*"});
		lua_state.fields.push_back({"top", top, sizeof(void*), "void*"});
		lua_state.fields.push_back({"stack", stack, sizeof(void*), "void*"});
		lua_state.size = 0x40;

		m_structures["lua_State"] = lua_state;

		return true;
	}

	bool LuauDumper::analyze_closure()
	{
		spdlog::info("Analyzing Closure...");
		StructInfo closure;
		closure.name = "Closure";

		closure.fields.push_back({"tt", 0x0, 1, "uint8_t"});
		closure.fields.push_back({"marked", 0x1, 1, "uint8_t"});
		closure.fields.push_back({"nupvalues", 0x2, 1, "uint8_t"});
		closure.fields.push_back({"stacksize", 0x3, 1, "uint8_t"});
		closure.fields.push_back({"preload", 0x4, 1, "uint8_t"});
		closure.fields.push_back({"isC", 0x5, 1, "bool"});
		closure.fields.push_back({"env", 0x8, 8, "Table*"});
		closure.fields.push_back({"gclist", 0x10, 8, "Closure*"});

		closure.fields.push_back({"f", 0x18, 8, "lua_CFunction"});
		closure.fields.push_back({"cont", 0x20, 8, "lua_Continuation"});
		closure.fields.push_back({"debugname", 0x28, 8, "char*"});

		closure.fields.push_back({"l.p", 0x18, 8, "Proto*"});

		closure.size = 0x30;

		m_structures["Closure"] = closure;
		spdlog::info("Closure analyzed: {} fields, size {}", closure.fields.size(), closure.size);

		return true;
	}

	bool LuauDumper::analyze_proto()
	{
		spdlog::info("Analyzing Proto...");

		if (m_pointers->m_luau_functions.luaF_freeproto == 0)
		{
			spdlog::error("luaF_freeproto function pointer is null, cannot analyze Proto structure");
			return false;
		}

		const auto sizecode = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 0);
		const auto code = AssemblyAnalyzer::find_next_instruction(sizecode, ZYDIS_MNEMONIC_MOV, 1);
		const auto sizep = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 1);
		const auto p = AssemblyAnalyzer::find_next_instruction(sizep, ZYDIS_MNEMONIC_MOV, 1);
		const auto sizek = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 2);
		const auto k        = AssemblyAnalyzer::find_next_instruction(sizek, ZYDIS_MNEMONIC_MOV, 1);
		const auto lineinfo = AssemblyAnalyzer::find_next_instruction(k, ZYDIS_MNEMONIC_MOV, 1);
		const auto sizelineinfo = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 3);
		const auto sizelocvars = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 4);
		const auto locvars  = AssemblyAnalyzer::find_next_instruction(sizelocvars, ZYDIS_MNEMONIC_LEA, 0);
		const auto upvalues = AssemblyAnalyzer::find_next_instruction(locvars, ZYDIS_MNEMONIC_LEA, 2);
		const auto sizeupvalues = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_freeproto, ZYDIS_MNEMONIC_MOVSXD, 5);
		const auto debuginsn = AssemblyAnalyzer::find_next_instruction(sizeupvalues, ZYDIS_MNEMONIC_MOV, 1);
		const auto execdata = AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(debuginsn, ZYDIS_MNEMONIC_CALL, 0), ZYDIS_MNEMONIC_CMP, 0);
		const auto typeinfo    = AssemblyAnalyzer::find_next_instruction(execdata, ZYDIS_MNEMONIC_LEA, 0);
		const auto sizeofProto = AssemblyAnalyzer::find_next_instruction(typeinfo, ZYDIS_MNEMONIC_MOV, 1);
		const auto source =
		    AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.thread, ZYDIS_MNEMONIC_JNZ, 0), ZYDIS_MNEMONIC_MOV, 3);
		const auto linedefined = AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(source, ZYDIS_MNEMONIC_CALL, 0), ZYDIS_MNEMONIC_MOV, 0);

		StructInfo proto;
		proto.name = "Proto";
		proto.fields.push_back({"sizecode", sizecode, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"code", code, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizep", sizep, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"p", p, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizek", sizek, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"k", k, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"lineinfo", lineinfo, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizelineinfo", sizelineinfo, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizelocvars", sizelocvars, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"locvars", locvars, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"upvalues", upvalues, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizeupvalues", sizeupvalues, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"debuginsn", debuginsn, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"execdata", execdata, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"typeinfo", typeinfo, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"sizeofProto", sizeofProto, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"source", source, sizeof(uint64_t), "uint64_t"});
		proto.fields.push_back({"linedefined", linedefined, sizeof(uint64_t), "uint64_t"});

		proto.size = 0x100;

		m_structures["Proto"] = proto;
		spdlog::info("Proto analyzed: {} fields, size {}", proto.fields.size(), proto.size);

		m_proto = {
		    .linedefined = linedefined,
		};

		return true;
	}

	bool LuauDumper::analyze_upval()
	{
		spdlog::info("Analyzing UpVal...");

		StructInfo upval;
		upval.name = "UpVal";

		upval.fields.push_back({"tt", 0x0, 1, "uint8_t"});
		upval.fields.push_back({"marked", 0x1, 1, "uint8_t"});
		upval.fields.push_back({"v", 0x8, 8, "TValue*"});
		upval.fields.push_back({"next", 0x10, 8, "UpVal*"});
		upval.fields.push_back({"value", 0x18, 16, "TValue"});

		upval.size = 0x28;

		m_structures["UpVal"] = upval;
		spdlog::info("UpVal analyzed: {} fields, size {}", upval.fields.size(), upval.size);

		return true;
	}

	bool LuauDumper::analyze_tvalue()
	{
		spdlog::info("Analyzing TValue...");

		StructInfo tvalue;
		tvalue.name = "TValue";

		tvalue.fields.push_back({"value", 0x0, 8, "Value"});
		tvalue.fields.push_back({"extra", 0x8, 4, "int32_t"});
		tvalue.fields.push_back({"tt", 0xC, 4, "int32_t"});

		tvalue.size = 0x10;

		m_structures["TValue"] = tvalue;
		spdlog::info("TValue analyzed: {} fields, size {}", tvalue.fields.size(), tvalue.size);

		return true;
	}

	bool LuauDumper::analyze_global_state()
	{
		spdlog::info("Analyzing GlobalState...");

		const auto gray = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.propagatemark, ZYDIS_MNEMONIC_MOV, 0);
		const auto gcstate = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.gc, ZYDIS_MNEMONIC_MOVZX, 0);
		const auto gcstats =
		    AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.gc, ZYDIS_MNEMONIC_CMP, 9), ZYDIS_MNEMONIC_MOV, 1);
		const auto frealloc =
		    AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.page, ZYDIS_MNEMONIC_XOR, 0), ZYDIS_MNEMONIC_MOV, 0);
		const auto ud =
		    AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.page, ZYDIS_MNEMONIC_XOR, 0), ZYDIS_MNEMONIC_MOV, 1);

		StructInfo global_state;
		global_state.name = "GlobalState";
		global_state.fields.push_back({"gray", gray, sizeof(uint64_t), "uint64_t"});
		global_state.fields.push_back({"gcstate", gcstate, sizeof(uint64_t), "uint64_t"});
		global_state.fields.push_back({"gcstats", gcstats, sizeof(uint64_t), "uint64_t"});
		global_state.fields.push_back({"frealloc", frealloc, sizeof(uint64_t), "uint64_t"});
		global_state.fields.push_back({"ud", ud, sizeof(uint64_t), "uint64_t"});

		global_state.size = 0x50;

		m_structures["GlobalState"] = global_state;
		spdlog::info("GlobalState analyzed: {} fields, size {}", global_state.fields.size(), global_state.size);

		return true;
	}

	bool LuauDumper::analyze_table()
	{
		spdlog::info("Analyzing Table...");

		const auto node = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.table, ZYDIS_MNEMONIC_CMP, 0);
		const auto lsizenode = AssemblyAnalyzer::find_next_instruction(node, ZYDIS_MNEMONIC_MOVZX, 0);
		const auto sizearray = AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(lsizenode, ZYDIS_MNEMONIC_ADD, 1), ZYDIS_MNEMONIC_CMP, 1);
		const auto array = AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(sizearray, ZYDIS_MNEMONIC_MOVSXD, 0), ZYDIS_MNEMONIC_MOV, 1);
		const auto metatable = AssemblyAnalyzer::find_next_instruction(array, ZYDIS_MNEMONIC_CMP, 0);
		const auto gclist =
		    AssemblyAnalyzer::find_next_instruction(AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.propagatemark, ZYDIS_MNEMONIC_RET, 0), ZYDIS_MNEMONIC_MOV, 0);

		StructInfo lua_table;
		lua_table.name = "Table";
		lua_table.fields.push_back({"node", node, sizeof(uint64_t), "uint64_t"});
		lua_table.fields.push_back({"lsizenode", lsizenode, sizeof(uint64_t), "uint64_t"});
		lua_table.fields.push_back({"sizearray", sizearray, sizeof(uint64_t), "uint64_t"});
		lua_table.fields.push_back({"array", array, sizeof(uint64_t), "uint64_t"});
		lua_table.fields.push_back({"metatable", metatable, sizeof(uint64_t), "uint64_t"});
		lua_table.fields.push_back({"gclist", gclist, sizeof(uint64_t), "uint64_t"});

		lua_table.size = 0x40;

		m_structures["Table"] = lua_table;
		spdlog::info("LuaTable analyzed: {} fields, size {}", lua_table.fields.size(), lua_table.size);

		return true;
	}

	bool LuauDumper::generate_offsets()
	{
		spdlog::info("Generating offsets...");
		spdlog::info("Image base: 0x{:X}", m_base_address);

		for (auto& [name, structure] : m_structures)
		{
			spdlog::info("Processing structure: {}", name);

			for (auto& field : structure.fields)
			{
				const auto original_offset = field.offset;
				if (original_offset >= m_base_address)
				{
					field.offset = original_offset - m_base_address;
				}

				spdlog::info("Field: {} | Offset: 0x{:X}", field.name, field.offset);
			}
		}

		spdlog::info("Offsets generated successfully.");
		return true;
	}
}
