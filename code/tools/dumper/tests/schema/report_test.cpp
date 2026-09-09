#include <doctest/doctest.h>

#include "rml/dumper/schema/report.hpp"

using namespace rml::dumper::schema;

TEST_CASE("report separates recovered fixed and failed")
{
	Report report;
	report.record_recovered("lua_State", "top", "write 2 width 8");
	report.record_recovered("lua_State", "base", "write 3 width 8");
	report.record_fixed("lua_State", "gclist", "not referenced by any anchor");
	report.record_failure("lua_State", "namecall", "no width 8 write through the first argument");

	CHECK(report.recovered_count() == 2);
	CHECK(report.fixed_count() == 1);
	REQUIRE(report.failures().size() == 1);
	CHECK(report.has_failures());
	CHECK(report.failures()[0].struct_name == "lua_State");
	CHECK(report.failures()[0].field_name == "namecall");
	CHECK(report.failures()[0].reason.find("no width 8 write") != std::string::npos);
	CHECK(report.recovered()[0].field_name == "top");
	CHECK(report.fixed()[0].reason == "not referenced by any anchor");
}

TEST_CASE("an empty report has no failures")
{
	const Report report;

	CHECK_FALSE(report.has_failures());
	CHECK(report.recovered_count() == 0);
	CHECK(report.fixed_count() == 0);
}

TEST_CASE("the summary names every failing field")
{
	Report report;
	report.record_recovered("Proto", "code", "write 0 width 8");
	report.record_failure("Proto", "k", "no candidate write");
	report.record_failure("LuaTable", "node", "no candidate write");

	const auto summary = report.summary();

	CHECK(summary.find("Proto.k") != std::string::npos);
	CHECK(summary.find("LuaTable.node") != std::string::npos);
	CHECK(summary.find("no candidate write") != std::string::npos);
}
