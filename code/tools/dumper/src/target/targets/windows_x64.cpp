#include "target/targets/profiles.hpp"

namespace rml::dumper::target
{
	static constexpr std::array<AnchorSpec, anchor_count> windows_x64_anchors{{
	    {Anchor::luaF_newLclosure,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B EA 49 8B F8"},
	    {Anchor::luaF_newCclosure, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 63 F2 49 8B F8"},
	    {Anchor::luaF_newproto, "40 53 48 83 EC 20 44 0F B6 41 ? BA C0 00 00 00"},
	    {Anchor::luaF_freeproto, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 44 0F B6 4A ? 49 8B F0"},
	    {Anchor::luaF_findupval,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 41 ? 48 8D 59"},
	    {Anchor::luaU_load,
	     "48 89 5C 24 ? 4C 89 44 24 ? 48 89 54 24 ? 48 89 4C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC E0 01 00 00"},
	    {Anchor::luaM_free, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 ? 49 8D 40"},
	    {Anchor::luaD_reallocstack, "48 89 5C 24 ? 55 48 83 EC 30 48 63 EA"},
	    {Anchor::luaD_reallocCI, "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 48 8B 69 ? 48 8B F9"},
	    {Anchor::luaH_new,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 8B F0 8B EA 44 0F B6 41"},
	    {Anchor::setnodevector,
	     "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 20 41 8B E8"},
	    {Anchor::propagatemark, "48 89 5C 24 ? 57 48 83 EC 20 48 8B 59 ? 48 8B F9 0F B6 03"},
	    {Anchor::traversetable,
	     "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 20 45 33 E4 48 8B F2"},
	    {Anchor::lua_resume, "48 89 5C 24 ? 57 48 83 EC 20 0F B6 41 ? 48 8B F9 3C 01"},
	    {Anchor::index2addr, "48 89 5C 24 ? 57 48 83 EC 20 F6 41 ? ? 48 8B D9 48 63 FA 74 ? 4C 8D 41"},
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
