#include <doctest/doctest.h>

#include "rml/dumper/emit/emitter.hpp"

#include <nlohmann/json.hpp>
#include <sstream>

using namespace rml::dumper;

static schema::LayoutSet sample_layouts()
{
	schema::LayoutSet layouts;
	layouts.target = "windows-x64";
	layouts.studio_version = "0.731.0.7310942";
	layouts.studio_guid = "version-14d8b191232f4ddd";

	schema::StructLayout state{.name = "lua_State", .size = 0x80};
	state.add({.name = "top", .type = "StkId", .size = 8, .offset = 0x40,
	           .provenance = schema::Provenance::recovered("luaD_reallocstack", "write 2 width 8")});
	state.add({.name = "base", .type = "StkId", .size = 8, .offset = 0x28,
	           .provenance = schema::Provenance::fixed("not written by any traced anchor")});
	layouts.structs.emplace("lua_State", std::move(state));

	layouts.report.record_recovered("lua_State", "top", "write 2 width 8");
	layouts.report.record_fixed("lua_State", "base", "not written by any traced anchor");

	return layouts;
}

static schema::LayoutSet call_info_layouts()
{
	schema::LayoutSet layouts;
	layouts.target = "windows-x64";

	schema::StructLayout info{.name = "CallInfo", .size = 0x28};
	info.add({.name = "top", .type = "StkId", .size = 8, .offset = 0x00});
	info.add({.name = "func", .type = "StkId", .size = 8, .offset = 0x08});
	info.add({.name = "base", .type = "StkId", .size = 8, .offset = 0x10});
	info.add({.name = "savedpc", .type = "const Instruction*", .size = 8, .offset = 0x18});
	info.add({.name = "nresults", .type = "int", .size = 4, .offset = 0x20});
	info.add({.name = "flags", .type = "unsigned", .size = 4, .offset = 0x24});
	layouts.structs.emplace("CallInfo", std::move(info));

	return layouts;
}

TEST_CASE("the mirror keeps clear of the names luau claims for macros")
{
	schema::LayoutSet layouts;
	layouts.target = "windows-x64";

	schema::StructLayout header{.name = "CommonHeader", .size = 3};
	header.add({.name = "tt", .type = "uint8_t", .size = 1, .offset = 0});
	header.add({.name = "marked", .type = "uint8_t", .size = 1, .offset = 1});
	header.add({.name = "memcat", .type = "uint8_t", .size = 1, .offset = 2});
	layouts.structs.emplace("CommonHeader", std::move(header));

	std::ostringstream out;
	REQUIRE(emit::EmitterRegistry().create("mirror")->emit(layouts, out).has_value());

	const auto text = out.str();

	CHECK(text.find("struct CommonHeader") == std::string::npos);
	CHECK(text.find("struct GcHeader") != std::string::npos);
	CHECK(text.find("gcheader_size") != std::string::npos);
	CHECK(text.find("namespace rml::luau::mirror") != std::string::npos);
}

TEST_CASE("the registry exposes every emitter by id")
{
	const emit::EmitterRegistry registry;

	CHECK(registry.create("json") != nullptr);
	CHECK(registry.create("shuffle") != nullptr);
	CHECK(registry.create("report") != nullptr);
	CHECK(registry.create("genny") == nullptr);
	CHECK(registry.create("mirror") != nullptr);
	CHECK(emit::EmitterRegistry::ids().size() == 4);
	CHECK(registry.create_all().size() == 4);
}

TEST_CASE("the json emitter writes structs fields and provenance")
{
	const auto layouts = sample_layouts();
	std::ostringstream stream;

	REQUIRE(emit::EmitterRegistry().create("json")->emit(layouts, stream).has_value());

	const auto parsed = nlohmann::json::parse(stream.str());
	CHECK(parsed.at("target") == "windows-x64");
	CHECK(parsed.at("studio_version") == "0.731.0.7310942");

	const auto& fields = parsed.at("structs").at("lua_State").at("fields");
	REQUIRE(fields.size() == 2);
	CHECK(fields[0].at("name") == "base");
	CHECK(fields[0].at("offset") == 0x28);
	CHECK(fields[0].at("provenance").at("kind") == "fixed");
	CHECK(fields[1].at("name") == "top");
	CHECK(fields[1].at("provenance").at("kind") == "recovered");
	CHECK(fields[1].at("provenance").at("anchor") == "luaD_reallocstack");
	CHECK(parsed.at("report").at("recovered") == 1);
	CHECK(parsed.at("report").at("fixed") == 1);
}

TEST_CASE("the report emitter separates recovered from fixed")
{
	const auto layouts = sample_layouts();
	std::ostringstream stream;

	REQUIRE(emit::EmitterRegistry().create("report")->emit(layouts, stream).has_value());

	const auto text = stream.str();
	CHECK(text.find("lua_State") != std::string::npos);
	CHECK(text.find("recovered from luaD_reallocstack") != std::string::npos);
	CHECK(text.find("not written by any traced anchor") != std::string::npos);
	CHECK(text.find("1 recovered, 1 fixed, 0 failed") != std::string::npos);
}

TEST_CASE("the shuffle emitter writes the permutation of a complete struct")
{
	const auto layouts = call_info_layouts();
	std::ostringstream stream;

	REQUIRE(emit::EmitterRegistry().create("shuffle")->emit(layouts, stream).has_value());

	const auto text = stream.str();
	CHECK(text.find("#pragma once") != std::string::npos);
	CHECK(text.find("RBX_SHUFFLE_CallInfo(a0, a1, a2, a3, a4, a5)") != std::string::npos);
	CHECK(text.find("a2; a1; a0; a3; a4; a5") != std::string::npos);
}

TEST_CASE("the shuffle emitter refuses a struct with a missing field")
{
	const auto layouts = sample_layouts();
	std::ostringstream stream;

	const auto emitted = emit::EmitterRegistry().create("shuffle")->emit(layouts, stream);

	REQUIRE_FALSE(emitted.has_value());
	CHECK(emitted.error().message().find("never recovered") != std::string::npos);
}

TEST_CASE("the emitters are deterministic")
{
	const auto layouts = call_info_layouts();

	for (const auto& id : emit::EmitterRegistry::ids())
	{
		std::ostringstream first;
		std::ostringstream second;

		const auto emitter = emit::EmitterRegistry().create(id);
		REQUIRE(emitter->emit(layouts, first).has_value());
		REQUIRE(emitter->emit(layouts, second).has_value());

		CHECK_MESSAGE(first.str() == second.str(), id);
	}
}
