#include <doctest/doctest.h>

#include "rml/dumper/image/image.hpp"
#include "support/pe_builder.hpp"
#include "target/composite_anchor_resolver.hpp"
#include "target/text_anchor_resolver.hpp"

#include <array>
#include <cstring>
#include <string>
#include <vector>

using namespace rml::dumper;
using namespace rml::dumper::target;

static constexpr Rva text_base = 0x1000;
static constexpr Rva data_base = 0x2000;
static constexpr Rva pdata_base = 0x3000;
static constexpr std::size_t exception_directory = 3;

static void write_rip_lea(std::vector<std::byte>& code, const std::size_t offset, const Rva target)
{
	const auto next = static_cast<std::int64_t>(text_base + offset + 7);
	const auto displacement = static_cast<std::int32_t>(static_cast<std::int64_t>(target) - next);

	code[offset + 0] = std::byte{0x48};
	code[offset + 1] = std::byte{0x8D};
	code[offset + 2] = std::byte{0x15};
	std::memcpy(code.data() + offset + 3, &displacement, sizeof(displacement));
}

static std::vector<std::byte> literal_pool()
{
	static constexpr char contents[] = "\0hello\0helloworld\0";
	static constexpr std::size_t length = sizeof(contents) - 1;

	std::vector<std::byte> pool(length);
	std::memcpy(pool.data(), contents, length);

	return pool;
}

static image::Image build(std::vector<std::byte> code, std::span<const std::pair<Rva, Rva>> functions)
{
	tests::PeBuilder builder;
	builder.add_section(".text", text_base, code);
	builder.add_data_section(".rdata", data_base, literal_pool());

	const auto table = tests::PeBuilder::runtime_function_table(functions);
	builder.add_data_section(".pdata", pdata_base, table);
	builder.set_directory(exception_directory, pdata_base, static_cast<std::uint32_t>(table.size()));

	auto image = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);
	REQUIRE(image.has_value());

	return std::move(*image);
}

TEST_CASE("a literal is found only as a whole nul terminated string")
{
	const auto image = build(std::vector<std::byte>(0x200, std::byte{0x90}), std::array<std::pair<Rva, Rva>, 0>{});

	const auto hello = TextAnchorResolver::find_literal(image, "hello");
	REQUIRE(hello.size() == 1);
	CHECK(hello.front() == data_base + 1);

	CHECK(TextAnchorResolver::find_literal(image, "helloworld").size() == 1);
	CHECK(TextAnchorResolver::find_literal(image, "ellowor").empty());
	CHECK(TextAnchorResolver::find_literal(image, "absent").empty());
}

TEST_CASE("text anchors resolve to the function that references the literal")
{
	std::vector<std::byte> code(0x200, std::byte{0x90});
	write_rip_lea(code, 0x40, data_base + 1);

	const std::array<std::pair<Rva, Rva>, 2> functions{
	    {{text_base, text_base + 0x100}, {text_base + 0x100, text_base + 0x200}}};

	const auto image = build(std::move(code), functions);

	const std::array<AnchorSpec, 1> anchors{{{Anchor::lua_getinfo, "", "hello"}}};

	const auto resolved = TextAnchorResolver().resolve(image, anchors);
	REQUIRE(resolved.has_value());
	CHECK(resolved->at(Anchor::lua_getinfo) == text_base);
}

TEST_CASE("a literal referenced from two functions is refused rather than guessed")
{
	std::vector<std::byte> code(0x200, std::byte{0x90});
	write_rip_lea(code, 0x40, data_base + 1);
	write_rip_lea(code, 0x140, data_base + 1);

	const std::array<std::pair<Rva, Rva>, 2> functions{
	    {{text_base, text_base + 0x100}, {text_base + 0x100, text_base + 0x200}}};

	const auto image = build(std::move(code), functions);

	const std::array<AnchorSpec, 1> anchors{{{Anchor::lua_getinfo, "", "hello"}}};

	const auto resolved = TextAnchorResolver().resolve(image, anchors);
	REQUIRE_FALSE(resolved.has_value());
	CHECK(resolved.error().message().find("2 functions") != std::string::npos);
}

TEST_CASE("an absent literal names itself in the failure")
{
	const auto image = build(std::vector<std::byte>(0x200, std::byte{0x90}), std::array<std::pair<Rva, Rva>, 0>{});

	const std::array<AnchorSpec, 1> anchors{{{Anchor::lua_getinfo, "", "absent"}}};

	const auto resolved = TextAnchorResolver().resolve(image, anchors);
	REQUIRE_FALSE(resolved.has_value());
	CHECK(resolved.error().message().find("absent") != std::string::npos);
}

TEST_CASE("the composite resolver dispatches each anchor by what it declares")
{
	std::vector<std::byte> code(0x200, std::byte{0x90});
	write_rip_lea(code, 0x40, data_base + 1);
	code[0x180] = std::byte{0xE8};
	code[0x181] = std::byte{0x11};
	code[0x182] = std::byte{0x22};

	const std::array<std::pair<Rva, Rva>, 2> functions{
	    {{text_base, text_base + 0x100}, {text_base + 0x100, text_base + 0x200}}};

	const auto image = build(std::move(code), functions);

	const std::array<AnchorSpec, 2> anchors{{
	    {Anchor::lua_getinfo, "", "hello"},
	    {Anchor::lua_resume, "E8 11 22", ""},
	}};

	const auto resolved = CompositeAnchorResolver().resolve(image, anchors);
	const std::string reason = resolved.has_value() ? std::string{} : resolved.error().message();
	INFO(reason);
	REQUIRE(resolved.has_value());
	CHECK(resolved->at(Anchor::lua_getinfo) == text_base);
	CHECK(resolved->at(Anchor::lua_resume) == text_base + 0x180);
}

TEST_CASE("an anchor with neither a pattern nor a literal is refused")
{
	const auto image = build(std::vector<std::byte>(0x200, std::byte{0x90}), std::array<std::pair<Rva, Rva>, 0>{});

	const std::array<AnchorSpec, 1> anchors{{{Anchor::lua_getinfo, "", ""}}};

	const auto resolved = CompositeAnchorResolver().resolve(image, anchors);
	REQUIRE_FALSE(resolved.has_value());
	CHECK(resolved.error().message().find("nothing to locate them by") != std::string::npos);
}
