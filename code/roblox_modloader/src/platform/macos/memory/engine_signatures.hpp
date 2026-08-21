#pragma once

#include "RobloxModLoader/memory/all.hpp"
#include "pointers.hpp"

namespace rml
{
	constexpr auto Pointers::get_roblox_batch()
	{
		// clang-format off
		constexpr auto batch_and_hash = memory::make_batch<

			{
				"MENU_BUILD_FROM_DOM",
				"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 F4 03 01 AA F3 03 00 AA ? ? ? F9 ? ? ? 91 ? ? ? 94",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.build_menu_bar_from_dom = ptr.as<functions::build_menu_bar_from_dom>();
				},
			},
			{
				"GET_STRING_ATOM",
				"? ? ? A9 ? ? ? A9 ? ? ? 91 F3 03 00 AA ? ? ? F0 ? ? ? F9 ? ? ? B5 ? ? ? 97 E1 03 13 AA ? ? ? A9 ? ? ? A8 ? ? ? 14 ? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.get_string_atom = ptr.as<functions::get_string_atom>();
				},
			},
			{
				"DESCRIPTOR_LOOKUP",
				"? ? ? B9 ? ? ? 34 ? ? ? A9 ? ? ? A9 ? ? ? 91 F3 03 00 AA ? ? ? 52 ? ? ? 94 ? ? ? B7 ? ? ? A9 ? ? ? B8 ? ? ? B9 49 01 09 0A 00 51 29 8B ? ? ? 14 ? ? ? D2 C0 03 5F D6 ? ? ? D2 ? ? ? A9 ? ? ? A8 C0 03 5F D6 ? ? ? F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.descriptor_lookup = ptr.as<functions::descriptor_lookup>();
				},
			},
			{
				"OBJECT_CREATE_BY_NAME",
				"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 F4 03 02 AA F5 03 00 AA F3 03 08 AA E0 03 01 AA ? ? ? 95",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.object_create_by_name = ptr.as<functions::object_create_by_name>();
				},
			},
			{
				"FREEBLOCK",
				"E8 03 01 AA 09 18 40 F9 41 8C 5F F8 2A 14 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.freeblock = ptr.as<functions::freeblock>();
				},
			},
			{
				"LUAA_PSEUDO2ADDR",
				"28 E2 84 12 3F 00 08 6B ? ? ? ? 08 E2 84 12",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaA_pseudo2addr = ptr.as<functions::luaA_pseudo2addr>();
				},
			},
			{
				"LUAC_BARRIERBACK",
				"08 18 40 F9 29 04 40 39 29 79 1D 12 29 04 00 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaC_barrierback = ptr.as<functions::luaC_barrierback>();
				},
			},
			{
				"LUAC_BARRIERF",
				"00 18 40 F9 08 24 41 39 08 05 00 51 1F 09 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaC_barrierf = ptr.as<functions::luaC_barrierf>();
				},
			},
			{
				"LUAC_BARRIERTABLE",
				"00 18 40 F9 08 24 41 39 1F 09 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaC_barriertable = ptr.as<functions::luaC_barriertable>();
				},
			},
			{
				"LUAC_ENUMHEAP",
				"FF 83 01 D1 F8 5F 02 A9 F6 57 03 A9 F4 4F 04 A9 FD 7B 05 A9 FD 43 01 91 F4 03 03 AA F5 03 01 AA F3 03 00 AA 16 18 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaC_enumheap = ptr.as<functions::luaC_enumheap>();
				},
			},
			{
				"LUAD_RAWRUNPROTECTED",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 E8 03 01 AA F5 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaD_rawrunprotected = ptr.as<functions::luaD_rawrunprotected>();
				},
			},
			{
				"LUAD_THROW",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 01 AA F4 03 00 AA 00 03 80 52 ? ? ? ? ? ? ? ? ? ? ? ? 08 50 00 A9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaD_throw = ptr.as<functions::luaD_throw>();
				},
			},
			{
				"LUAE_NEWTHREAD",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 02 10 40 39 01 10 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaE_newthread = ptr.as<functions::luaE_newthread>();
				},
			},
			{
				"LUAF_NEWCCLOSURE",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA F4 03 01 AA F5 03 00 AA 88 7E 7C 93",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaF_newCclosure = ptr.as<functions::luaF_newCclosure>();
				},
			},
			{
				"LUAF_NEWLCLOSURE",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 03 AA F5 03 02 AA F3 03 01 AA F6 03 00 AA 68 7E 7C 93",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaF_newLclosure = ptr.as<functions::luaF_newLclosure>();
				},
			},
			{
				"LUAH_NEW",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA F6 03 01 AA F4 03 00 AA 02 10 40 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaH_new = ptr.as<functions::luaH_new>();
				},
			},
			{
				"LUAH_SETNUM",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F3 03 01 AA 48 04 00 51",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaH_setnum = ptr.as<functions::luaH_setnum>();
				},
			},
			{
				"LUAL_ARGERRORL",
				"FF 43 01 D1 F6 57 02 A9 F4 4F 03 A9 FD 7B 04 A9 FD 03 01 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA ? ? ? ? ? ? ? ? F5 53 00 A9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_argerrorL = ptr.as<functions::luaL_argerrorL>();
				},
			},
			{
				"LUAL_CHECKANY",
				"FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 01 AA F3 03 00 AA ? ? ? ? 1F 04 00 31",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_checkany = ptr.as<functions::luaL_checkany>();
				},
			},
			{
				"LUAL_CHECKLSTRING",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 01 AA F4 03 00 AA ? ? ? ? ? ? ? ? FD 7B 41 A9 F4 4F C2 A8 C0 03 5F D6 E0 03 14 AA E1 03 13 AA C2 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_checklstring = ptr.as<functions::luaL_checklstring>();
				},
			},
			{
				"LUAL_CHECKTYPE",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA F4 03 01 AA F5 03 00 AA ? ? ? ? 1F 00 13 6B",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_checktype = ptr.as<functions::luaL_checktype>();
				},
			},
			{
				"LUAL_ERRORL",
				"FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 01 AA F4 03 00 AA A8 43 00 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_errorL = ptr.as<functions::luaL_errorL>();
				},
			},
			{
				"LUAL_REGISTER",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F3 03 00 AA ? ? ? ? F5 03 01 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_register = ptr.as<functions::luaL_register>();
				},
			},
			{
				"LUAL_SANDBOXTHREAD",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 01 00 80 52 02 00 80 52 ? ? ? ? E0 03 13 AA 01 00 80 52 02 00 80 52 ? ? ? ? E0 03 13 AA 21 E2 84 12",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_sandboxthread = ptr.as<functions::luaL_sandboxthread>();
				},
			},
			{
				"LUAL_TYPEERRORL",
				"FF 43 01 D1 F6 57 02 A9 F4 4F 03 A9 FD 7B 04 A9 FD 03 01 91 F5 03 02 AA F4 03 01 AA F3 03 00 AA ? ? ? ? F6 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_typeerrorL = ptr.as<functions::luaL_typeerrorL>();
				},
			},
			{
				"LUAL_WHERE",
				"FF 03 06 D1 F4 4F 16 A9 FD 7B 17 A9 FD C3 05 91 F3 03 00 AA ? ? ? ? ? ? ? ? ? ? ? ? A8 83 1E F8 ? ? ? ? ? ? ? ? E3 63 00 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_where = ptr.as<functions::luaL_where>();
				},
			},
			{
				"LUAM_VISITGCO",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 08 18 40 F9 00 91 41 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaM_visitgco = ptr.as<functions::luaM_visitgco>();
				},
			},
			{
				"LUA_BREAK",
				"08 10 40 79 09 14 40 79 1F 01 09 6B ? ? ? ? C8 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_break = ptr.as<functions::lua_break>();
				},
			},
			{
				"LUA_CALL",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 28 04 00 11",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_call = ptr.as<functions::lua_call>();
				},
			},
			{
				"LUA_CREATETABLE",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 18 40 F9 08 25 45 A9 3F 01 08 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? 68 06 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 68 5A 45 A9 C9 42 00 91 08 01 40 F9 3F 01 08 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 76 2E 40 F9 E0 03 13 AA E1 03 15 AA E2 03 14 AA ? ? ? ? C0 02 00 F9 E8 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_createtable = ptr.as<functions::lua_createtable>();
				},
			},
			{
				"LUA_GETFIELD",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? BF 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 35 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 15 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_getfield = ptr.as<functions::lua_getfield>();
				},
			},
			{
				"LUA_GETINFO",
				"E9 23 B9 6D FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 7B 06 A9 FD 83 01 91 F4 03 03 AA F5 03 02 AA F3 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_getinfo = ptr.as<functions::lua_getinfo>();
				},
			},
			{
				"LUA_GETMETATABLE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 9F 02 08 6B ? ? ? ? 68 2E 40 F9 00 D1 34 8B ? ? ? ? E0 03 13 AA E1 03 14 AA ? ? ? ? 08 0C 40 B9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_getmetatable = ptr.as<functions::lua_getmetatable>();
				},
			},
			{
				"LUA_GETREADONLY",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 00 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_getreadonly = ptr.as<functions::lua_getreadonly>();
				},
			},
			{
				"LUA_GETTABLE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 21 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_gettable = ptr.as<functions::lua_gettable>();
				},
			},
			{
				"LUA_GETTOP",
				"08 2C 40 F9 09 1C 40 F9 08 01 09 CB 00 FD 44 D3",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_gettop = ptr.as<functions::lua_gettop>();
				},
			},
			{
				"LUA_GETUPVALUE",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? BF 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 35 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_getupvalue = ptr.as<functions::lua_getupvalue>();
				},
			},
			{
				"LUA_INSERT",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 20 31 8A 9A ? ? ? ? C8 E1 84 12 9F 02 08 6B ? ? ? ? 68 2E 40 F9 00 D1 34 8B ? ? ? ? E0 03 13 AA E1 03 14 AA ? ? ? ? 68 2E 40 F9 1F 01 00 EB",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_insert = ptr.as<functions::lua_insert>();
				},
			},
			{
				"LUA_ISNUMBER",
				"FF 83 00 D1 FD 7B 01 A9 FD 43 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_isnumber = ptr.as<functions::lua_isnumber>();
				},
			},
			{
				"LUA_ISSTRING",
				"FD 7B BF A9 FD 03 00 91 ? ? ? ? 1F 18 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_isstring = ptr.as<functions::lua_isstring>();
				},
			},
			{
				"LUA_ISUSERDATA",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 1F 25 00 71 04 19 42 7A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_isuserdata = ptr.as<functions::lua_isuserdata>();
				},
			},
			{
				"LUA_NEWTHREAD",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 08 18 40 F9 08 25 45 A9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_newthread = ptr.as<functions::lua_newthread>();
				},
			},
			{
				"LUA_NEXT",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 20 31 8A 9A ? ? ? ? C8 E1 84 12 9F 02 08 6B ? ? ? ? 68 2E 40 F9 00 D1 34 8B ? ? ? ? E0 03 13 AA E1 03 14 AA ? ? ? ? 68 2E 40 F9 01 00 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_next = ptr.as<functions::lua_next>();
				},
			},
			{
				"LUA_OBJLEN",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_objlen = ptr.as<functions::lua_objlen>();
				},
			},
			{
				"LUA_PCALL",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F6 03 03 AA F4 03 02 AA F5 03 01 AA F3 03 00 AA 28 04 00 11",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pcall = ptr.as<functions::lua_pcall>();
				},
			},
			{
				"LUA_PUSHBOOLEAN",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 09 20 45 A9 0A 41 00 91 29 01 40 F9 5F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 68 2E 40 F9 9F 02 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushboolean = ptr.as<functions::lua_pushboolean>();
				},
			},
			{
				"LUA_PUSHCCLOSUREK",
				"F8 5F BC A9 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F6 03 04 AA F4 03 03 AA F5 03 02 AA F7 03 01 AA F3 03 00 AA 08 18 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushcclosurek = ptr.as<functions::lua_pushcclosurek>();
				},
			},
			{
				"LUA_PUSHINTEGER",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 09 20 45 A9 0A 41 00 91 29 01 40 F9 5F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 68 2E 40 F9 80 02 62 1E",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushinteger = ptr.as<functions::lua_pushinteger>();
				},
			},
			{
				"LUA_PUSHLSTRING",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 18 40 F9 08 25 45 A9 3F 01 08 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? 68 06 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 68 5A 45 A9 C9 42 00 91 08 01 40 F9 3F 01 08 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 76 2E 40 F9 E0 03 13 AA E1 03 15 AA E2 03 14 AA ? ? ? ? C0 02 00 F9 C8 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushlstring = ptr.as<functions::lua_pushlstring>();
				},
			},
			{
				"LUA_PUSHNIL",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 09 20 45 A9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushnil = ptr.as<functions::lua_pushnil>();
				},
			},
			{
				"LUA_PUSHNUMBER",
				"E9 23 BD 6D F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 08 40 60 1E F3 03 00 AA 09 20 45 A9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushnumber = ptr.as<functions::lua_pushnumber>();
				},
			},
			{
				"LUA_PUSHSTRING",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 00 AA ? ? ? ? F3 03 01 AA E0 03 01 AA ? ? ? ? E2 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushstring = ptr.as<functions::lua_pushstring>();
				},
			},
			{
				"LUA_PUSHVALUE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 20 31 8A 9A ? ? ? ? C8 E1 84 12 9F 02 08 6B ? ? ? ? 68 2E 40 F9 00 D1 34 8B ? ? ? ? E0 03 13 AA E1 03 14 AA ? ? ? ? 68 2E 40 F9 00 00 C0 3D",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_pushvalue = ptr.as<functions::lua_pushvalue>();
				},
			},
			{
				"LUA_RAWGET",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 9F 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 34 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 20 31 8A 9A ? ? ? ? C8 E1 84 12 9F 02 08 6B ? ? ? ? 68 2E 40 F9 00 D1 34 8B ? ? ? ? E0 03 13 AA E1 03 14 AA ? ? ? ? 68 2E 40 F9 00 00 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawget = ptr.as<functions::lua_rawget>();
				},
			},
			{
				"LUA_RAWGETFIELD",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? BF 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 35 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 15 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawgetfield = ptr.as<functions::lua_rawgetfield>();
				},
			},
			{
				"LUA_RAWGETI",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 41 00 91 29 01 40 F9 1F 01 09 EB ? ? ? ? E0 03 13 AA 21 00 80 52 ? ? ? ? ? ? ? ? BF 06 00 71 ? ? ? ? 68 1E 40 F9 08 51 35 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawgeti = ptr.as<functions::lua_rawgeti>();
				},
			},
			{
				"LUA_RAWITER",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F5 03 01 AA F3 03 00 AA 08 04 40 39 ? ? ? ? 62 62 00 91 E0 03 13 AA E1 03 13 AA ? ? ? ? 69 22 45 A9 08 81 00 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawiter = ptr.as<functions::lua_rawiter>();
				},
			},
			{
				"LUA_RAWSET",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 00 AA 3F 04 00 71 ? ? ? ? 68 1E 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawset = ptr.as<functions::lua_rawset>();
				},
			},
			{
				"LUA_RAWSETFIELD",
				"F8 5F BC A9 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F3 03 00 AA 3F 04 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawsetfield = ptr.as<functions::lua_rawsetfield>();
				},
			},
			{
				"LUA_RAWSETI",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F3 03 00 AA 3F 04 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_rawseti = ptr.as<functions::lua_rawseti>();
				},
			},
			{
				"LUA_REF",
				"FF 43 01 D1 F8 5F 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 7B 04 A9 FD 03 01 91 F3 03 00 AA ? ? ? ? ? ? ? ? 08 01 40 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_ref = ptr.as<functions::lua_ref>();
				},
			},
			{
				"LUA_RESUME",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 02 AA F3 03 00 AA ? ? ? ? F5 03 00 AA ? ? ? ? 68 1A 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_resume = ptr.as<functions::lua_resume>();
				},
			},
			{
				"LUA_SETFIELD",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F3 03 00 AA 3F 04 00 71 ? ? ? ? 68 1E 40 F9 08 51 21 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 15 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 68 2E 40 F9 15 D1 21 8B ? ? ? ? E0 03 13 AA ? ? ? ? F5 03 00 AA E0 03 14 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_setfield = ptr.as<functions::lua_setfield>();
				},
			},
			{
				"LUA_SETMETATABLE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 3F 04 00 71 ? ? ? ? 68 1E 40 F9 08 51 21 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 20 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 68 2E 40 F9 00 D1 21 8B ? ? ? ? E0 03 13 AA ? ? ? ? 68 2E 40 F9 09 C1 5F B8",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_setmetatable = ptr.as<functions::lua_setmetatable>();
				},
			},
			{
				"LUA_SETREADONLY",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 02 AA 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 00 40 F9 7F 02 00 71 E9 07 9F 1A 09 19 00 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_setreadonly = ptr.as<functions::lua_setreadonly>();
				},
			},
			{
				"LUA_SETSAFEENV",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 02 AA 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 00 40 F9 7F 02 00 71 E9 07 9F 1A 09 0D 00 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_setsafeenv = ptr.as<functions::lua_setsafeenv>();
				},
			},
			{
				"LUA_SETTABLE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 3F 04 00 71 ? ? ? ? 68 1E 40 F9 08 51 21 8B 09 41 00 D1 68 2E 40 F9 ? ? ? ? ? ? ? ? 3F 01 08 EB 21 31 8A 9A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_settable = ptr.as<functions::lua_settable>();
				},
			},
			{
				"LUA_SETTOP",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F4 03 01 AA F3 03 00 AA 0A 2C 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_settop = ptr.as<functions::lua_settop>();
				},
			},
			{
				"LUA_SETUPVALUE",
				"FF 03 01 D1 F6 57 01 A9 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F4 03 02 AA F3 03 00 AA 3F 04 00 71 ? ? ? ? 68 1E 40 F9 08 51 21 8B 08 41 00 D1 69 2E 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 15 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 68 2E 40 F9 15 D1 21 8B ? ? ? ? E0 03 13 AA ? ? ? ? F5 03 00 AA E2 23 00 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_setupvalue = ptr.as<functions::lua_setupvalue>();
				},
			},
			{
				"LUA_TOBOOLEAN",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 ? ? ? ? 1F 05 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_toboolean = ptr.as<functions::lua_toboolean>();
				},
			},
			{
				"LUA_TOINTEGERX",
				"FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 1F 0D 00 71 ? ? ? ? E1 03 00 91 ? ? ? ? ? ? ? ? 00 00 40 FD 00 00 78 1E",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_tointegerx = ptr.as<functions::lua_tointegerx>();
				},
			},
			{
				"LUA_TOLSTRING",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA F4 03 01 AA F5 03 00 AA 3F 04 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_tolstring = ptr.as<functions::lua_tolstring>();
				},
			},
			{
				"LUA_TONUMBERX",
				"FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 02 AA 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 1F 0D 00 71 ? ? ? ? E1 03 00 91 ? ? ? ? ? ? ? ? ? ? ? ? 28 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_tonumberx = ptr.as<functions::lua_tonumberx>();
				},
			},
			{
				"LUA_TOPOINTER",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 1F 09 00 71 ? ? ? ? 1F 25 00 71 ? ? ? ? 08 00 40 F9 00 41 00 91 ? ? ? ? 1F 19 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_topointer = ptr.as<functions::lua_topointer>();
				},
			},
			{
				"LUA_TYPE",
				"FD 7B BF A9 FD 03 00 91 E8 03 00 AA 3F 04 00 71",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_type = ptr.as<functions::lua_type>();
				},
			},
			{
				"LUA_UNREF",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F3 03 01 AA ? ? ? ? ? ? ? ? 08 01 40 39",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_unref = ptr.as<functions::lua_unref>();
				},
			},
			{
				"LUA_YIELD",
				"08 10 40 79 09 14 40 79 1F 01 09 6B ? ? ? ? 08 2C 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_yield = ptr.as<functions::lua_yield>();
				},
			},
			{
				"LUAU_LOAD",
				"FF 83 02 D1 FA 67 05 A9 F8 5F 06 A9 F6 57 07 A9 F4 4F 08 A9 FD 7B 09 A9 FD 43 02 91 F4 03 04 AA F5 03 03 AA F6 03 02 AA F7 03 01 AA F3 03 00 AA 18 18 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luau_load = ptr.as<functions::luau_load>();
				},
			},
			{
				"RBX_THREAD_IDENTITY_CONTEXT",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 08 AA ? ? ? ? ? ? ? ? 08 3C 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.rbx_thread_identity_context = ptr.as<functions::rbx_thread_identity_context>();
				},
			},
			{
				"SIGNAL_DISCONNECT",
				"FF 03 01 D1 F4 4F 02 A9 FD 7B 03 A9 FD C3 00 91 F3 03 00 AA FF 0B 00 F9 ? ? ? ? F4 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.signal_disconnect = ptr.as<functions::signal_disconnect>();
				},
			},
			{
				"SIGNAL_MUTEX_GET",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 ? ? ? ? ? ? ? ? 08 C1 BF 38 ? ? ? ? ? ? ? ? ? ? ? ? FD 7B 41 A9 F4 4F C2 A8 C0 03 5F D6 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 00 08 80 52 ? ? ? ? E8 74 95 52 48 55 A6 72 08 00 00 F9 00 E4 00 6F 00 80 80 3C 00 80 81 3C 00 80 82 3C 1F 1C 00 F9 60 2A 01 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.signal_mutex_get = ptr.as<functions::signal_mutex_get>();
				},
			},
			{
				"LUAL_CHECKINTEGER",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 41 E2 84 12 ? ? ? ? E1 03 00 AA E0 03 13 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_checkinteger = ptr.add(20).bl().as<functions::luaL_checkinteger>();
				},
			},
			{
				"LUAL_CHECKNUMBER",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 21 00 80 52 ? ? ? ? 00 C0 60 1E E0 03 13 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_checknumber = ptr.add(20).bl().as<functions::luaL_checknumber>();
				},
			},
			{
				"LUAL_OPTBOOLEAN",
				"FF 83 01 D1 F4 4F 04 A9 FD 7B 05 A9 FD 43 01 91 F3 03 00 AA 21 00 80 52",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_optboolean = ptr.add(28).bl().as<functions::luaL_optboolean>();
				},
			},
			{
				"LUAL_OPTINTEGER",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA 41 00 80 52 22 00 80 52 ? ? ? ? F4 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaL_optinteger = ptr.add(24).bl().as<functions::luaL_optinteger>();
				},
			},
			{
				"LUAO_NILOBJECT",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 ? ? ? ? ? ? ? ? 3F 04 00 71 ? ? ? ? 08 1C 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luaO_nilobject = ptr.add(12).adrp().as<void*>();
				},
			},
			{
				"LUA_ERROR",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 00 AA ? ? ? ? ? ? ? ? 42 06 80 52 ? ? ? ? E0 03 13 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_error = ptr.add(36).bl().as<functions::lua_error>();
				},
			},
			{
				"LUAU_EXECUTE",
				"F6 57 BD A9 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 03 03 AA F3 03 00 AA ? ? ? ? ? ? ? ? 68 2A 40 F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.luau_execute = ptr.add(148).bl().as<functions::luau_execute>();
				},
			},
			{
				"RBX_GET_GLOBAL_STATE",
				"F4 4F BE A9 FD 7B 01 A9 FD 43 00 91 F3 03 01 AA ? ? ? ? F4 03 00 AA E0 03 13 AA ? ? ? ? E1 03 00 AA",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.get_global_state = ptr.as<functions::get_global_state>();
				},
			},
			{
				"LUA_ISCFUNCTION",
				"FD 7B BF A9 FD 03 00 91 3F 04 00 71 ? ? ? ? 08 1C 40 F9 08 51 21 8B 08 41 00 D1 09 2C 40 F9 ? ? ? ? ? ? ? ? 1F 01 09 EB 00 31 8A 9A ? ? ? ? C8 E1 84 12 3F 00 08 6B ? ? ? ? 08 2C 40 F9 00 D1 21 8B ? ? ? ? ? ? ? ? 08 0C 40 B9 1F 21 00 71 ? ? ? ? 08 00 40 F9 08 0D 40 39 1F 01 00 71 E0 07 9F 1A",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.lua_iscfunction = ptr.as<functions::lua_iscfunction>();
				},
			}
		>();
		// clang-format on

		return batch_and_hash;
	}
}
