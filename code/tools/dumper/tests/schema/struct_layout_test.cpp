#include <doctest/doctest.h>

#include "rml/dumper/schema/layout_set.hpp"

using namespace rml::dumper;
using namespace rml::dumper::schema;

TEST_CASE("provenance defaults to fixed and reports its source")
{
	const Provenance defaulted;
	CHECK_FALSE(defaulted.is_recovered());
	REQUIRE(defaulted.as_fixed() != nullptr);
	CHECK(defaulted.as_recovered() == nullptr);

	const auto recovered = Provenance::recovered("luaD_reallocstack", "write 2 width 8");
	CHECK(recovered.is_recovered());
	REQUIRE(recovered.as_recovered() != nullptr);
	CHECK(recovered.as_recovered()->anchor == "luaD_reallocstack");
	CHECK(recovered.as_recovered()->probe == "write 2 width 8");
	CHECK(recovered.describe().find("luaD_reallocstack") != std::string::npos);

	const auto fixed = Provenance::fixed("not written by any traced anchor");
	CHECK_FALSE(fixed.is_recovered());
	CHECK(fixed.as_fixed()->reason == "not written by any traced anchor");
	CHECK(fixed.describe().find("not written") != std::string::npos);
}

TEST_CASE("validation accepts a well formed layout")
{
	StructLayout layout{.name = "lua_State", .size = 0x20};
	layout.add({.name = "status", .type = "uint8_t", .size = 1, .offset = 0x03,
	            .provenance = Provenance::recovered("lua_resume", "read 0 width 1")});
	layout.add({.name = "global", .type = "global_State*", .size = 8, .offset = 0x08,
	            .provenance = Provenance::fixed("not referenced by any anchor")});

	CHECK(validate(layout).has_value());
	CHECK(layout.recovered_count() == 1);
}

TEST_CASE("fields are kept sorted by offset")
{
	StructLayout layout{.name = "CallInfo", .size = 0x28};
	layout.add({.name = "base", .type = "StkId", .size = 8, .offset = 0x10});
	layout.add({.name = "top", .type = "StkId", .size = 8, .offset = 0x00});
	layout.add({.name = "func", .type = "StkId", .size = 8, .offset = 0x08});

	REQUIRE(layout.fields.size() == 3);
	CHECK(layout.fields[0].name == "top");
	CHECK(layout.fields[1].name == "func");
	CHECK(layout.fields[2].name == "base");
	CHECK(layout.find("func")->offset == 0x08);
	CHECK(layout.find("missing") == nullptr);
}

TEST_CASE("validation rejects overlapping fields")
{
	StructLayout layout{.name = "Proto", .size = 0x20};
	layout.add({.name = "k", .type = "TValue*", .size = 8, .offset = 0x08});
	layout.add({.name = "code", .type = "Instruction*", .size = 8, .offset = 0x0C});

	const auto result = validate(layout);

	REQUIRE_FALSE(result.has_value());
	CHECK(result.error().code() == ErrorCode::validation);
	CHECK(result.error().message().find("k") != std::string::npos);
	CHECK(result.error().message().find("code") != std::string::npos);
}

TEST_CASE("validation rejects a field past the end of the struct")
{
	StructLayout layout{.name = "UpVal", .size = 0x10};
	layout.add({.name = "v", .type = "TValue*", .size = 8, .offset = 0x10});

	const auto result = validate(layout);

	REQUIRE_FALSE(result.has_value());
	CHECK(result.error().message().find("UpVal.v") != std::string::npos);
}

TEST_CASE("validation rejects a zero sized field")
{
	StructLayout layout{.name = "LuaTable", .size = 0x40};
	layout.add({.name = "metatable", .type = "LuaTable*", .size = 0, .offset = 0x08});

	CHECK_FALSE(validate(layout).has_value());
}

TEST_CASE("validation rejects a misaligned field")
{
	StructLayout eight{.name = "LuaTable", .size = 0x40};
	eight.add({.name = "node", .type = "LuaNode*", .size = 8, .offset = 0x14});
	const auto misaligned_eight = validate(eight);
	REQUIRE_FALSE(misaligned_eight.has_value());
	CHECK(misaligned_eight.error().message().find("alignment") != std::string::npos);

	StructLayout four{.name = "Proto", .size = 0x40};
	four.add({.name = "sizecode", .type = "int", .size = 4, .offset = 0x0A});
	CHECK_FALSE(validate(four).has_value());

	StructLayout two{.name = "lua_State", .size = 0x40};
	two.add({.name = "nCcalls", .type = "unsigned short", .size = 2, .offset = 0x0B});
	CHECK_FALSE(validate(two).has_value());
}

TEST_CASE("validation accepts single byte fields at any offset")
{
	StructLayout layout{.name = "GCheader", .size = 0x08};
	layout.add({.name = "tt", .type = "uint8_t", .size = 1, .offset = 0x00});
	layout.add({.name = "marked", .type = "uint8_t", .size = 1, .offset = 0x01});
	layout.add({.name = "memcat", .type = "uint8_t", .size = 1, .offset = 0x02});

	CHECK(validate(layout).has_value());
}

TEST_CASE("an empty layout with a size validates")
{
	const StructLayout layout{.name = "Empty", .size = 0x10};

	CHECK(validate(layout).has_value());
}

TEST_CASE("a layout set looks structures up by name")
{
	LayoutSet layouts;
	layouts.target = "windows-x64";
	layouts.structs.emplace("lua_State", StructLayout{.name = "lua_State", .size = 0x80});

	CHECK(layouts.find("lua_State")->size == 0x80);
	CHECK(layouts.find("Proto") == nullptr);
}
