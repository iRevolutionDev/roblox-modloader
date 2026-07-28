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
