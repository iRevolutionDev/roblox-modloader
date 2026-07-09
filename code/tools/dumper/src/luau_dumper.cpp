#include "dumper/luau_dumper.hpp"

#include "dumper/assembly_analyzer.hpp"
#include "dumper/common.hpp"
#include "dumper/pointers.hpp"

namespace dumper
{
	bool LuauDumper::has_field(const StructInfo& structure, std::string_view field_name, std::size_t offset)
	{
		return std::ranges::any_of(structure.fields, [&](const FieldInfo& field) {
			return field.name == field_name && field.offset == offset;
		});
	}

	std::size_t LuauDumper::validate_offset(std::string_view field, uint64_t recovered, std::size_t expected, bool allow_zero)
	{
		const bool implausible = recovered == AssemblyAnalyzer::invalid_displacement || recovered > 0x8000 || (recovered == 0 && !allow_zero);
		if (implausible)
		{
			spdlog::warn("{}: recovery unavailable (got 0x{:X}) -- using verified baseline 0x{:X}", field, recovered, expected);
			return expected;
		}

		if (recovered != expected)
			spdlog::warn("{}: DRIFT recovered 0x{:X} != baseline 0x{:X} -- re-verify this build", field, recovered, expected);

		return recovered;
	}

	void LuauDumper::apply_common_header(StructInfo& structure) const
	{
		const auto it = m_structures.find("CommonHeader");
		if (it == m_structures.end())
			return;

		for (const auto& common_field : it->second.fields)
		{
			if (!has_field(structure, common_field.name, common_field.offset))
			{
				structure.fields.push_back(common_field);
			}
		}
	}

	LuauDumper::LuauDumper(std::unique_ptr<pointers>& pointers, uintptr_t image_base) :
	    m_base_address(image_base),
	    m_pointers(std::move(pointers))
	{
		spdlog::info("Image base: 0x{:X}", m_base_address);
	}

	bool LuauDumper::analyze()
	{
		spdlog::info("Initializing Luau Dumper...");

		bool success = true;
		success &= analyze_common();
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

		const auto& fns = m_pointers->m_luau_functions;
		const auto disp = [](const MemAccess* a) { return a ? a->disp : AssemblyAnalyzer::invalid_displacement; };

		const FunctionTrace rs = AssemblyAnalyzer::trace_function(fns.luaD_reallocstack);
		const ZydisRegister l_rs = rs.dominant_write_base();
		const std::size_t after_alloc = rs.calls.empty() ? 0 : rs.calls.front().seq;

		const MemAccess* w_stack = rs.nth_write(l_rs, 0, 8, after_alloc);
		const MemAccess* w_slast = rs.nth_write(l_rs, 1, 8, after_alloc);
		const MemAccess* w_top   = rs.nth_write(l_rs, 2, 8, after_alloc);
		const MemAccess* w_base  = rs.nth_write(l_rs, 3, 8, after_alloc);
		const MemAccess* w_ssize = rs.nth_write(l_rs, 0, 4, after_alloc);

		const std::size_t ci_from = w_top ? w_top->seq : 0;
		const std::size_t ci_to   = w_base ? w_base->seq : SIZE_MAX;
		const MemAccess* ci_top    = nullptr;
		for (const auto& a : rs.accesses)
			if (a.write && a.width == 8 && a.index == ZYDIS_REGISTER_NONE && a.disp == 0 && a.base != l_rs && a.base != ZYDIS_REGISTER_NONE
			    && a.seq > ci_from && a.seq < ci_to)
			{
				ci_top = &a;
				break;
			}
		const ZydisRegister ci_reg = ci_top ? ci_top->base : ZYDIS_REGISTER_NONE;
		const MemAccess* ci_base   = ci_reg != ZYDIS_REGISTER_NONE ? rs.nth_write(ci_reg, 1, 8, ci_top->seq) : nullptr;
		const MemAccess* ci_func   = ci_reg != ZYDIS_REGISTER_NONE ? rs.nth_write(ci_reg, 2, 8, ci_top->seq) : nullptr;

		const FunctionTrace rc = AssemblyAnalyzer::trace_function(fns.luaD_reallocCI);
		const ZydisRegister l_rc = rc.dominant_write_base();
		const MemAccess* w_baseci = rc.nth_write(l_rc, 0, 8);
		const MemAccess* w_sizeci = rc.nth_write(l_rc, 0, 4);
		const MemAccess* w_ci     = rc.nth_write(l_rc, 1, 8);
		const MemAccess* w_endci  = rc.nth_write(l_rc, 2, 8);

		const FunctionTrace mf = AssemblyAnalyzer::trace_function(fns.luaM_free);
		const MemAccess* r_global = mf.nth_read(ZYDIS_REGISTER_RCX, 0, 8);

		const FunctionTrace lr = AssemblyAnalyzer::trace_function(fns.lua_resume);
		const ZydisRegister l_lr = lr.dominant_write_base();
		const MemAccess* r_status = lr.nth_read(ZYDIS_REGISTER_RCX, 0, 1);
		const MemAccess* w_active = lr.first_imm_write(l_lr, 1);
		const MemAccess* w_ncc    = lr.nth_write(l_lr, 0, 2);
		const MemAccess* w_bcc    = lr.nth_write(l_lr, 1, 2);

		const FunctionTrace nl = AssemblyAnalyzer::trace_function(fns.luaF_newLclosure);
		const MemAccess* r_amc = nl.nth_read(ZYDIS_REGISTER_RCX, 0, 1);

		StructInfo lua_state;
		lua_state.name = "lua_State";
		apply_common_header(lua_state);
		lua_state.fields.push_back({"status", validate_offset("lua_State.status", disp(r_status), 0x03), 1, "uint8_t"});
		lua_state.fields.push_back({"activememcat", validate_offset("lua_State.activememcat", disp(r_amc), 0x04), 1, "uint8_t"});
		lua_state.fields.push_back({"isactive", validate_offset("lua_State.isactive", disp(w_active), 0x06), 1, "bool"});
		lua_state.fields.push_back({"singlestep", 0x05, 1, "bool"});
		lua_state.fields.push_back({"namecall", 0x08, 8, "TString*"});
		lua_state.fields.push_back({"openupval", 0x10, 8, "UpVal*"});
		lua_state.fields.push_back({"ci", validate_offset("lua_State.ci", disp(w_ci), 0x18), 8, "CallInfo*"});
		lua_state.fields.push_back({"global", validate_offset("lua_State.global", disp(r_global), 0x20), 8, "global_State*"});
		lua_state.fields.push_back({"base", validate_offset("lua_State.base", disp(w_base), 0x28), 8, "StkId"});
		lua_state.fields.push_back({"stack_last", validate_offset("lua_State.stack_last", disp(w_slast), 0x30), 8, "StkId"});
		lua_state.fields.push_back({"stack", validate_offset("lua_State.stack", disp(w_stack), 0x38), 8, "StkId"});
		lua_state.fields.push_back({"top", validate_offset("lua_State.top", disp(w_top), 0x40), 8, "StkId"});
		lua_state.fields.push_back({"gclist", 0x48, 8, "GCObject*"});
		lua_state.fields.push_back({"userdata", 0x50, 8, "void*"});
		lua_state.fields.push_back({"gt", 0x58, 8, "LuaTable*"});
		lua_state.fields.push_back({"stacksize", validate_offset("lua_State.stacksize", disp(w_ssize), 0x60), 4, "int"});
		lua_state.fields.push_back({"size_ci", validate_offset("lua_State.size_ci", disp(w_sizeci), 0x64), 4, "int"});
		lua_state.fields.push_back({"nCcalls", validate_offset("lua_State.nCcalls", disp(w_ncc), 0x68), 2, "unsigned short"});
		lua_state.fields.push_back({"baseCcalls", validate_offset("lua_State.baseCcalls", disp(w_bcc), 0x6A), 2, "unsigned short"});
		lua_state.fields.push_back({"cachedslot", 0x6C, 4, "int"});
		lua_state.fields.push_back({"end_ci", validate_offset("lua_State.end_ci", disp(w_endci), 0x70), 8, "CallInfo*"});
		lua_state.fields.push_back({"base_ci", validate_offset("lua_State.base_ci", disp(w_baseci), 0x78), 8, "CallInfo*"});
		lua_state.size = 0x80;
		m_structures["lua_State"] = lua_state;

		StructInfo ci;
		ci.name = "CallInfo";
		ci.fields.push_back({"top", validate_offset("CallInfo.top", disp(ci_top), 0x00, true), 8, "StkId"});
		ci.fields.push_back({"func", validate_offset("CallInfo.func", disp(ci_func), 0x08), 8, "StkId"});
		ci.fields.push_back({"base", validate_offset("CallInfo.base", disp(ci_base), 0x10), 8, "StkId"});
		ci.fields.push_back({"savedpc", 0x18, 8, "const Instruction*"});
		ci.fields.push_back({"nresults", 0x20, 4, "int"});
		ci.fields.push_back({"flags", 0x24, 4, "unsigned int"});
		ci.size = 0x28;
		m_structures["CallInfo"] = ci;
		return true;
	}

	bool LuauDumper::analyze_common()
	{
		spdlog::info("Analyzing CommonHeader...");

		if (m_pointers->m_luau_functions.luaF_newLclosure == 0)
		{
			spdlog::error("luaF_newLclosure function pointer is null, cannot analyze CommonHeader");
			return false;
		}

		const auto newgco_call = AssemblyAnalyzer::find_next_instruction(m_pointers->m_luau_functions.luaF_newLclosure, ZYDIS_MNEMONIC_CALL, 0);
		const auto tt_insn     = AssemblyAnalyzer::find_next_instruction(newgco_call, ZYDIS_MNEMONIC_MOV, 2);
		const auto marked_insn = AssemblyAnalyzer::find_next_instruction(newgco_call, ZYDIS_MNEMONIC_MOV, 3);
		const auto memcat_insn = AssemblyAnalyzer::find_next_instruction(newgco_call, ZYDIS_MNEMONIC_MOV, 4);

		StructInfo common;
		common.name = "CommonHeader";
		common.fields.push_back({"tt", tt_insn, 1, "uint8_t"});
		common.fields.push_back({"marked", marked_insn, 1, "uint8_t"});
		common.fields.push_back({"memcat", memcat_insn, 1, "uint8_t"});
		common.size = 0x3;

		m_structures["CommonHeader"] = common;
		spdlog::info("CommonHeader analyzed: {} fields, size {}", common.fields.size(), common.size);

		return true;
	}

	bool LuauDumper::analyze_closure()
	{
		spdlog::info("Analyzing Closure...");

		const auto& fns = m_pointers->m_luau_functions;
		const auto disp = [](const MemAccess* a) { return a ? a->disp : AssemblyAnalyzer::invalid_displacement; };
		if (fns.luaF_newLclosure == 0 || fns.luaF_newCclosure == 0)
		{
			spdlog::error("luaF_new{L,C}closure null, cannot analyze Closure");
			return false;
		}

		const FunctionTrace nl = AssemblyAnalyzer::trace_function(fns.luaF_newLclosure);
		const ZydisRegister cl = nl.dominant_write_base();
		const MemAccess* w_isC = nl.nth_write(cl, 3, 1);
		const MemAccess* w_nup = nl.nth_write(cl, 4, 1);
		const MemAccess* w_ssz = nl.nth_write(cl, 5, 1);
		const MemAccess* w_prl = nl.nth_write(cl, 6, 1);
		const MemAccess* w_env = nl.nth_write(cl, 0, 8);
		const MemAccess* w_res = nl.nth_write(cl, 1, 8);
		const MemAccess* w_lp  = nl.nth_write(cl, 2, 8);

		const FunctionTrace nc = AssemblyAnalyzer::trace_function(fns.luaF_newCclosure);
		const ZydisRegister cc = nc.dominant_write_base();
		const MemAccess* w_cont = nc.nth_write(cc, 2, 8);
		const MemAccess* w_dbg  = nc.nth_write(cc, 3, 8);

		StructInfo closure;
		closure.name = "Closure";
		apply_common_header(closure);
		closure.fields.push_back({"isC", validate_offset("Closure.isC", disp(w_isC), 0x3), 1, "uint8_t"});
		closure.fields.push_back({"nupvalues", validate_offset("Closure.nupvalues", disp(w_nup), 0x4), 1, "uint8_t"});
		closure.fields.push_back({"stacksize", validate_offset("Closure.stacksize", disp(w_ssz), 0x5), 1, "uint8_t"});
		closure.fields.push_back({"preload", validate_offset("Closure.preload", disp(w_prl), 0x6), 1, "uint8_t"});
		closure.fields.push_back({"rbx_reserved8", validate_offset("Closure.reserved8", disp(w_res), 0x8), 8, "void*"});
		closure.fields.push_back({"env", validate_offset("Closure.env", disp(w_env), 0x10), 8, "LuaTable*"});
		closure.fields.push_back({"gclist", 0x18, 8, "GCObject*"});
		closure.fields.push_back({"l.p", validate_offset("Closure.l.p", disp(w_lp), 0x20), 8, "Proto*"});
		closure.fields.push_back({"c.cont", validate_offset("Closure.c.cont", disp(w_cont), 0x28), 8, "lua_Continuation"});
		closure.fields.push_back({"c.debugname", validate_offset("Closure.c.debugname", disp(w_dbg), 0x30), 8, "char*"});
		closure.size = 0x38;
		m_structures["Closure"] = closure;
		spdlog::info("Closure analyzed via solver: {} fields", closure.fields.size());
		return true;
	}

	bool LuauDumper::analyze_proto()
	{
		spdlog::info("Analyzing Proto...");

		const auto freeproto = m_pointers->m_luau_functions.luaF_freeproto;
		const auto luaM_free = m_pointers->m_luau_functions.luaM_free;
		if (freeproto == 0 || luaM_free == 0)
		{
			spdlog::error("luaF_freeproto/luaM_free null, cannot analyze Proto");
			return false;
		}

		const auto arrays = AssemblyAnalyzer::recover_freeproto_arrays(freeproto, luaM_free);

		static constexpr uint32_t expected[] = {4, 8, 16, 1, 24, 8, 1, 1, 16};
		if (arrays.size() < std::size(expected))
		{
			spdlog::error("Proto: recovered {} array frees, expected {}", arrays.size(), std::size(expected));
			return false;
		}
		for (std::size_t i = 0; i < std::size(expected); ++i)
		{
			if (arrays[i].elem_size != expected[i])
			{
				spdlog::error("Proto: array #{} elem {} != expected {} (layout drift?)", i, arrays[i].elem_size, expected[i]);
				return false;
			}
		}

		StructInfo proto;
		proto.name = "Proto";
		apply_common_header(proto);
		proto.fields.push_back({"nups", 0x3, 1, "uint8_t"});
		proto.fields.push_back({"numparams", 0x4, 1, "uint8_t"});
		proto.fields.push_back({"is_vararg", 0x5, 1, "uint8_t"});
		proto.fields.push_back({"maxstacksize", 0x6, 1, "uint8_t"});
		proto.fields.push_back({"flags", 0x7, 1, "uint8_t"});

		const auto add = [&](const char* ptr_name, const char* size_name, std::size_t idx, const char* elem_type) {
			proto.fields.push_back({ptr_name, arrays[idx].ptr_disp, 8, elem_type});
			if (size_name)
				proto.fields.push_back({size_name, arrays[idx].size_disp, 4, "int"});
		};
		add("code", "sizecode", 0, "Instruction*");
		add("p", "sizep", 1, "Proto**");
		add("k", "sizek", 2, "TValue*");
		add("lineinfo", "sizelineinfo", 3, "uint8_t*");
		add("locvars", "sizelocvars", 4, "LocVar*");
		add("upvalues", "sizeupvalues", 5, "TString**");
		add("debuginsn", nullptr, 6, "uint8_t*");
		add("typeinfo", "sizetypeinfo", 7, "uint8_t*");
		add("feedbackvec", "feedbackvecsize", 8, "void*");

		recover_proto_metadata(proto);

		proto.size = 0xC0;
		m_structures["Proto"] = proto;
		spdlog::info("Proto analyzed via solver: {} array frees, {} fields", arrays.size(), proto.fields.size());
		return true;
	}

	void LuauDumper::recover_proto_metadata(StructInfo& proto) const
	{
		const auto& fns = m_pointers->m_luau_functions;
		const auto disp = [](const MemAccess* a) { return a ? a->disp : AssemblyAnalyzer::invalid_displacement; };

		const FunctionTrace ul = AssemblyAnalyzer::trace_function(fns.luaU_load, 0x2000);
		const ZydisRegister p_reg = ul.dominant_write_base();
		const MemAccess* w_source = p_reg != ZYDIS_REGISTER_NONE ? ul.nth_write(p_reg, 0, 8) : nullptr;
		const MemAccess* w_bcid   = p_reg != ZYDIS_REGISTER_NONE ? ul.nth_write(p_reg, 0, 4) : nullptr;

		proto.fields.push_back({"source", validate_offset("Proto.source", disp(w_source), 0x10), 8, "TString*"});
		proto.fields.push_back({"userdata", 0x18, 8, "void*"});
		proto.fields.push_back({"debugname", 0x20, 8, "TString*"});
		proto.fields.push_back({"gclist", 0x48, 8, "GCObject*"});
		proto.fields.push_back({"abslineinfo", 0x60, 8, "int*"});
		proto.fields.push_back({"codeentry", 0x68, 8, "const Instruction*"});
		proto.fields.push_back({"execdata", 0x70, 8, "void*"});
		proto.fields.push_back({"exectarget", 0x78, 8, "uintptr_t"});
		proto.fields.push_back({"linedefined", 0x8C, 4, "int"});
		proto.fields.push_back({"linegaplog2", 0xA4, 4, "int"});
		proto.fields.push_back({"bytecodeid", validate_offset("Proto.bytecodeid", disp(w_bcid), 0xAC), 4, "int"});
		proto.fields.push_back({"funid", 0xBC, 4, "int"});
	}

	bool LuauDumper::analyze_upval()
	{
		spdlog::info("Analyzing UpVal...");

		StructInfo upval;
		upval.name = "UpVal";

		apply_common_header(upval);
		upval.fields.push_back({"markedopen", 0x4, 1, "uint8_t"});
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
		spdlog::info("Analyzing global_State...");

		const auto& fns = m_pointers->m_luau_functions;
		const auto disp = [](const MemAccess* a) { return a ? a->disp : AssemblyAnalyzer::invalid_displacement; };

		const FunctionTrace mf = AssemblyAnalyzer::trace_function(fns.luaM_free);
		const MemAccess* r_gload = mf.nth_read(ZYDIS_REGISTER_RCX, 0, 8);
		const ZydisRegister greg = r_gload ? r_gload->reg : ZYDIS_REGISTER_NONE;

		uint64_t frealloc_disp   = AssemblyAnalyzer::invalid_displacement;
		uint64_t totalbytes_disp = AssemblyAnalyzer::invalid_displacement;
		uint64_t memcat_disp     = AssemblyAnalyzer::invalid_displacement;
		if (greg != ZYDIS_REGISTER_NONE)
		{
			for (const auto& a : mf.accesses)
			{
				if (a.base != greg)
					continue;
				if (a.mnemonic == ZYDIS_MNEMONIC_CALL && frealloc_disp == AssemblyAnalyzer::invalid_displacement)
					frealloc_disp = a.disp;
				if (a.mnemonic == ZYDIS_MNEMONIC_SUB && a.write)
				{
					if (a.index != ZYDIS_REGISTER_NONE)
						memcat_disp = a.disp;
					else if (totalbytes_disp == AssemblyAnalyzer::invalid_displacement)
						totalbytes_disp = a.disp;
				}
			}
		}

		const MemAccess* r_ud = greg != ZYDIS_REGISTER_NONE ? mf.nth_read(greg, 0, 8) : nullptr;

		StructInfo g;
		g.name = "global_State";
		g.fields.push_back({"strt", 0x00, 16, "stringtable"});
		g.fields.push_back({"weak", 0x10, 8, "GCObject*"});
		g.fields.push_back({"grayagain", 0x18, 8, "GCObject*"});
		g.fields.push_back({"gray", 0x20, 8, "GCObject*"});
		g.fields.push_back({"GCthreshold", 0x28, 8, "size_t"});
		g.fields.push_back({"totalbytes", validate_offset("global_State.totalbytes", totalbytes_disp, 0x30), 8, "size_t"});
		g.fields.push_back({"gcgoal", 0x38, 4, "int"});
		g.fields.push_back({"gcstepmul", 0x3C, 4, "int"});
		g.fields.push_back({"gcstepsize", 0x40, 4, "int"});
		g.fields.push_back({"frealloc", validate_offset("global_State.frealloc", frealloc_disp, 0x48), 8, "lua_Alloc"});
		g.fields.push_back({"ud", validate_offset("global_State.ud", disp(r_ud), 0x50), 8, "void*"});
		g.fields.push_back({"currentwhite", 0x58, 1, "uint8_t"});
		g.fields.push_back({"gcstate", 0x59, 1, "uint8_t"});
		g.fields.push_back({"sweepgcopage", 0x60, 8, "lua_Page*"});
		g.fields.push_back({"uvhead", 0x70, 0x28, "UpVal"});
		g.fields.push_back({"freepages", 0x98, 8 * 40, "lua_Page*[40]"});
		g.fields.push_back({"mainthread", 0x1D8, 8, "lua_State*"});
		g.fields.push_back({"allgcopages", 0x1E0, 8, "lua_Page*"});
		g.fields.push_back({"freegcopages", 0x1E8, 8 * 40, "lua_Page*[40]"});
		g.fields.push_back({"mt", 0x440, 8 * 14, "LuaTable*[14]"});
		g.fields.push_back({"pseudotemp", 0x4B0, 16, "TValue"});
		g.fields.push_back({"registry", 0x4C0, 16, "TValue"});
		g.fields.push_back({"memcatbytes", validate_offset("global_State.memcatbytes", memcat_disp, 0x2C30), 8, "size_t[256]"});
		g.size = 0x46D0;
		m_structures["global_State"] = g;
		spdlog::info("global_State analyzed: {} fields (frealloc/ud/totalbytes/memcatbytes recovered)", g.fields.size());
		return true;
	}

	bool LuauDumper::analyze_table()
	{
		spdlog::info("Analyzing LuaTable...");

		const auto& fns = m_pointers->m_luau_functions;
		const auto disp = [](const MemAccess* a) { return a ? a->disp : AssemblyAnalyzer::invalid_displacement; };

		const FunctionTrace sv = AssemblyAnalyzer::trace_function(fns.setnodevector);
		const ZydisRegister t_reg = sv.dominant_write_base();
		const MemAccess* w_node   = sv.nth_write(t_reg, 0, 8);
		const MemAccess* w_lsize  = sv.nth_write(t_reg, 0, 1);
		const MemAccess* w_mask   = sv.nth_write(t_reg, 1, 1);
		const MemAccess* w_lfree  = sv.nth_write(t_reg, 0, 4);

		const FunctionTrace tt = AssemblyAnalyzer::trace_function(fns.traversetable);
		const MemAccess* r_mt   = tt.nth_read(ZYDIS_REGISTER_RSI, 0, 8);
		const MemAccess* r_sarr = tt.nth_read(ZYDIS_REGISTER_RSI, 0, 4);

		StructInfo t;
		t.name = "Table";
		apply_common_header(t);
		t.fields.push_back({"nodemask8", validate_offset("Table.nodemask8", disp(w_mask), 0x03), 1, "uint8_t"});
		t.fields.push_back({"readonly", 0x04, 1, "uint8_t"});
		t.fields.push_back({"tmcache", 0x05, 1, "uint8_t"});
		t.fields.push_back({"safeenv", 0x06, 1, "uint8_t"});
		t.fields.push_back({"lsizenode", validate_offset("Table.lsizenode", disp(w_lsize), 0x07), 1, "uint8_t"});
		t.fields.push_back({"sizearray", validate_offset("Table.sizearray", disp(r_sarr), 0x08), 4, "int"});
		t.fields.push_back({"lastfree", validate_offset("Table.lastfree", disp(w_lfree), 0x0C), 4, "int"});
		t.fields.push_back({"metatable", validate_offset("Table.metatable", disp(r_mt), 0x10), 8, "LuaTable*"});
		t.fields.push_back({"gclist", 0x18, 8, "GCObject*"});
		t.fields.push_back({"array", 0x20, 8, "TValue*"});
		t.fields.push_back({"node", validate_offset("Table.node", disp(w_node), 0x28), 8, "LuaNode*"});
		t.size = 0x30;
		m_structures["Table"] = t;
		spdlog::info("LuaTable analyzed: {} fields, size 0x{:X}", t.fields.size(), t.size);
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
				if (const auto original_offset = field.offset; original_offset >= m_base_address && AssemblyAnalyzer::is_valid_code_pointer(original_offset))
				{
					if (const auto displacement = AssemblyAnalyzer::get_displacement(original_offset); displacement != AssemblyAnalyzer::invalid_displacement)
					{
						if (const auto adjusted = static_cast<int64_t>(displacement) + field.adjustment; adjusted >= 0 && adjusted <= 0x2000)
						{
							field.offset = static_cast<std::size_t>(adjusted);
						}
						else
						{
							field.offset = original_offset - m_base_address;
						}
					}
					else
					{
						field.offset = original_offset - m_base_address;
					}
				}
				else if (original_offset >= m_base_address)
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
