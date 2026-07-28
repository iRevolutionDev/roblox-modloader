#include "target/targets/profiles.hpp"

namespace rml::dumper::target
{
	static constexpr std::array<AnchorSpec, anchor_count> windows_x64_anchors{{
	    {Anchor::luaD_reallocstack, "48 89 5C 24 ? 55 48 83 EC 30 48 63 EA"},
	    {Anchor::luaD_reallocCI, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 48 8B 69 ? 48 8B F9"},
	    {Anchor::luaE_newthread, "48 89 5C 24 ? 57 48 83 EC ? 44 0F B6 41 ? BA"},
	    {Anchor::lua_settop, "48 89 5C 24 ? 57 48 83 EC 20 48 63 FA 48 8B D9 85 D2 0F 88"},
	    {Anchor::lua_resume, "40 53 48 83 EC 20 41 B8 01 00 00 00 48 8B D9 E8 ? ? ? ? 85 C0"},
	    {Anchor::luaM_free, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 ? 49 8D 40"},
	    {Anchor::luaF_findupval,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 41 ? 48 8D 59"},
	    {Anchor::luaH_new, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 8B EA 44 0F B6 41"},
	    {Anchor::luaF_newLclosure,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B EA 49 8B F8"},
	    {Anchor::luaF_newCclosure, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 63 F2 49 8B F8"},
	    {Anchor::luaF_freeproto, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 44 0F B6 4A ? 49 8B F0"},
	    {Anchor::luaU_load,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 49 8B E9 4D 8B F0 4C 8B FA 48 8B F9"},
	    {Anchor::lua_pushnumber, "40 53 48 83 EC 30 80 3D ? ? ? ? ? 48 8B D9 0F 29 74 24 ? 0F 28 F1 74 ? 48 8B 41"},
	    {Anchor::lua_toboolean,
	     "48 83 EC 28 85 D2 7E ? 4C 8B 41 ? 48 8D 05 ? ? ? ? 49 83 C0 F0 48 63 D2 48 C1 E2 04 4C 03 C2 4C 3B 41 ? 49 0F 42 C0 EB ? 81 FA F0 D8 FF FF 7E ? 48 63 C2 48 C1 E0 04 48 03 41 ? EB ? E8 ? ? ? ? 8B 48 ? 85 C9"},
	    {Anchor::luau_precall,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 20 83 7A ? ? 41 8B D8 4C 8B F2 48 8B F9"},
	    {Anchor::lua_getinfo, "48 89 5C 24 ? 55 56 57 41 56 41 57 48 83 EC 20 33 ED 4C 63 DA"},
    {Anchor::lua_pushcclosurek,
     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B D9 49 63 F9"},
	}};

	const TargetProfile& windows_x64_profile()
	{
		static const TargetProfile profile{
		    .name = "windows-x64",
		    .format = ImageFormat::pe,
		    .architecture = Architecture::x86_64,
		    .abi = &disasm::Abi::windows_x64(),
		    .anchors = windows_x64_anchors,
		};

		return profile;
	}
}
