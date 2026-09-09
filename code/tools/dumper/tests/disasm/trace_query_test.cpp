#include <doctest/doctest.h>

#include "rml/dumper/disasm/abi.hpp"
#include "rml/dumper/disasm/trace_query.hpp"

using namespace rml::dumper;
using namespace rml::dumper::disasm;

static constexpr Object state = entry_object(Register::rcx);
static constexpr Object other = entry_object(Register::rbx);

static Trace sample_trace()
{
	Trace trace;
	trace.accesses = {
	    {.sequence = 0, .object = state, .base = Register::rcx, .value_register = Register::rax, .width = 8,
	     .displacement = 0x38, .is_write = true},
	    {.sequence = 1, .object = state, .base = Register::rcx, .value_register = Register::rdx, .width = 8,
	     .displacement = 0x30, .is_write = true},
	    {.sequence = 3, .object = state, .base = Register::rcx, .width = 4, .displacement = 0x60, .is_write = true},
	    {.sequence = 4, .object = other, .base = Register::rbx, .value_register = Register::r8, .width = 8,
	     .displacement = 0x10, .is_write = false},
	    {.sequence = 5, .object = state, .base = Register::rcx, .width = 1, .displacement = 0x06, .immediate = 1,
	     .is_write = true},
	    {.sequence = 6, .object = state, .base = Register::rcx, .index = Register::rdx, .scale = 8, .width = 8,
	     .displacement = 0x20, .is_write = true},
	};
	trace.calls = {{.sequence = 2, .address = 0x1020, .target = Rva{0x5000}},
	               {.sequence = 7, .address = 0x1040, .target = Rva{0x5000}}};
	return trace;
}

TEST_CASE("nth write filters by object and width")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.nth_write(state, 0, 8)->displacement == 0x38);
	CHECK(query.nth_write(state, 1, 8)->displacement == 0x30);
	CHECK(query.nth_write(state, 2, 8)->displacement == 0x20);
	CHECK(query.nth_write(state, 3, 8) == nullptr);
	CHECK(query.nth_write(state, 0, 4)->displacement == 0x60);
}

TEST_CASE("nth write can start after a sequence number")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.nth_write(state, 0, 8, 0)->displacement == 0x30);
	CHECK(query.nth_write(state, 0, 8, 3)->displacement == 0x20);
}

TEST_CASE("nth write can filter by the stored register")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.nth_write(state, 0, 8, std::nullopt, Register::rdx)->displacement == 0x30);
	CHECK(query.nth_write(state, 0, 8, std::nullopt, Register::r15) == nullptr);
}

TEST_CASE("nth read filters by object")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.nth_read(other, 0, 8)->displacement == 0x10);
	CHECK(query.nth_read(state, 0, 8) == nullptr);
}

TEST_CASE("nth distinct write skips a field written more than once")
{
	Trace trace;
	trace.accesses = {
	    {.sequence = 0, .object = state, .base = Register::rcx, .width = 2, .displacement = 0x78, .is_write = true},
	    {.sequence = 1, .object = state, .base = Register::rcx, .width = 2, .displacement = 0x78, .is_write = true},
	    {.sequence = 2, .object = state, .base = Register::rcx, .width = 2, .displacement = 0x7A, .is_write = true},
	};

	const TraceQuery query(trace);

	CHECK(query.nth_distinct_write(state, 0, 2)->displacement == 0x78);
	CHECK(query.nth_distinct_write(state, 1, 2)->displacement == 0x7A);
	CHECK(query.nth_write(state, 1, 2)->displacement == 0x78);
}

TEST_CASE("two values living in one register stay apart")
{
	Trace trace;
	trace.accesses = {
	    {.sequence = 0, .object = state, .base = Register::rcx, .width = 8, .displacement = 0x50, .is_write = true},
	    {.sequence = 1, .object = first_derived_object, .base = Register::rcx, .width = 8, .displacement = 0x8,
	     .is_write = true},
	};

	const TraceQuery query(trace);

	CHECK(query.nth_write(state, 0, 8)->displacement == 0x50);
	CHECK(query.nth_write(state, 1, 8) == nullptr);
	CHECK(query.nth_write(first_derived_object, 0, 8)->displacement == 0x8);
}

TEST_CASE("first immediate write matches on the stored value")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.first_immediate_write(state, 1)->displacement == 0x06);
	CHECK(query.first_immediate_write(state, 9) == nullptr);
}

TEST_CASE("first indexed finds a scaled access")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	const auto* indexed = query.first_indexed(state, true);
	REQUIRE(indexed != nullptr);
	CHECK(indexed->index == Register::rdx);
	CHECK(indexed->scale == 8);
}

TEST_CASE("dominant object is the one written through most often")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.dominant_object() == state);
}

TEST_CASE("dominant object of an empty trace is none")
{
	const Trace empty;
	const TraceQuery query(empty);

	CHECK(query.dominant_object() == no_object);
}

TEST_CASE("call sequence locates the nth call to a target")
{
	const Trace trace = sample_trace();
	const TraceQuery query(trace);

	CHECK(query.call_sequence(0x5000, 0) == std::optional<std::size_t>{2});
	CHECK(query.call_sequence(0x5000, 1) == std::optional<std::size_t>{7});
	CHECK(query.call_sequence(0x5000, 2) == std::nullopt);
	CHECK(query.call_sequence(0x9999, 0) == std::nullopt);
	CHECK(query.first_call_sequence() == std::optional<std::size_t>{2});
}

TEST_CASE("windows x64 passes the first four integer arguments in rcx rdx r8 r9")
{
	const auto& abi = Abi::windows_x64();

	CHECK(abi.argument(0) == Register::rcx);
	CHECK(abi.argument(1) == Register::rdx);
	CHECK(abi.argument(2) == Register::r8);
	CHECK(abi.argument(3) == Register::r9);
	CHECK(abi.argument(4) == Register::none);
	CHECK(abi.return_value() == Register::rax);
}

TEST_CASE("system v passes the first six integer arguments in rdi rsi rdx rcx r8 r9")
{
	const auto& abi = Abi::system_v_x64();

	CHECK(abi.argument(0) == Register::rdi);
	CHECK(abi.argument(1) == Register::rsi);
	CHECK(abi.argument(2) == Register::rdx);
	CHECK(abi.argument(3) == Register::rcx);
	CHECK(abi.argument(5) == Register::r9);
	CHECK(abi.argument(6) == Register::none);
	CHECK(abi.return_value() == Register::rax);
}

TEST_CASE("aapcs64 passes the first eight arguments in x0 through x7")
{
	const auto& abi = Abi::aapcs64();

	CHECK(abi.argument(0) == Register::x0);
	CHECK(abi.argument(7) == Register::x7);
	CHECK(abi.argument(8) == Register::none);
	CHECK(abi.return_value() == Register::x0);
}
