#include <doctest/doctest.h>

#include "rml/dumper/recover/recovery_pipeline.hpp"
#include "support/pe_builder.hpp"

using namespace rml::dumper;
using namespace rml::dumper::recover;

class StubRecoverer final : public Recoverer
{
public:
	StubRecoverer(std::string name, std::vector<std::string_view> dependencies) :
	    m_name(std::move(name)),
	    m_dependencies(std::move(dependencies))
	{
	}

	[[nodiscard]] std::string_view struct_name() const override { return m_name; }
	[[nodiscard]] std::span<const std::string_view> depends_on() const override { return m_dependencies; }

	[[nodiscard]] std::expected<schema::StructLayout, Error> recover(const RecoveryContext& context) const override
	{
		for (const auto& dependency : m_dependencies)
			if (context.layout(dependency) == nullptr)
				return std::unexpected(
				    Error::make(ErrorCode::recovery, "{} ran before {}", m_name, dependency));

		schema::StructLayout layout{.name = m_name, .size = 0x10};
		layout.add({.name = "field", .type = "void*", .size = 8, .offset = 0});
		return layout;
	}

private:
	std::string m_name;
	std::vector<std::string_view> m_dependencies;
};

class FailingRecoverer final : public Recoverer
{
public:
	[[nodiscard]] std::string_view struct_name() const override { return "Broken"; }
	[[nodiscard]] std::span<const std::string_view> depends_on() const override { return {}; }

	[[nodiscard]] std::expected<schema::StructLayout, Error> recover(const RecoveryContext& context) const override
	{
		context.report().record_failure("Broken", "field", "the probe found nothing");

		schema::StructLayout layout{.name = "Broken", .size = 0x10};
		layout.add({.name = "field", .type = "void*", .size = 8, .offset = 0});
		return layout;
	}
};

class OverlappingRecoverer final : public Recoverer
{
public:
	[[nodiscard]] std::string_view struct_name() const override { return "Overlapping"; }
	[[nodiscard]] std::span<const std::string_view> depends_on() const override { return {}; }

	[[nodiscard]] std::expected<schema::StructLayout, Error> recover(const RecoveryContext&) const override
	{
		schema::StructLayout layout{.name = "Overlapping", .size = 0x20};
		layout.add({.name = "first", .type = "void*", .size = 8, .offset = 0});
		layout.add({.name = "second", .type = "void*", .size = 8, .offset = 4});
		return layout;
	}
};

class PipelineFixture
{
public:
	PipelineFixture()
	{
		tests::PeBuilder builder;
		builder.add_section(".text", 0x1000, std::vector<std::byte>(0x100, std::byte{0x90}));

		auto loaded = image::ImageLoader::load(builder.write_to_temporary_file(), Architecture::x86_64);
		REQUIRE(loaded.has_value());
		m_image = std::make_unique<image::Image>(std::move(*loaded));

		auto decoder = disasm::Decoder::create(Architecture::x86_64);
		REQUIRE(decoder.has_value());
		m_decoder = std::move(*decoder);

		m_context = std::make_unique<RecoveryContext>(*m_image, *m_decoder, disasm::Abi::windows_x64(), m_anchors,
		                                              m_report);
	}

	[[nodiscard]] RecoveryContext& context() const { return *m_context; }

private:
	std::unique_ptr<image::Image> m_image;
	std::unique_ptr<disasm::Decoder> m_decoder;
	target::AnchorSet m_anchors;
	schema::Report m_report;
	std::unique_ptr<RecoveryContext> m_context;
};

TEST_CASE("the pipeline runs recoverers in dependency order")
{
	std::vector<std::unique_ptr<Recoverer>> recoverers;
	recoverers.push_back(std::make_unique<StubRecoverer>("Proto", std::vector<std::string_view>{"CommonHeader"}));
	recoverers.push_back(std::make_unique<StubRecoverer>("CommonHeader", std::vector<std::string_view>{}));

	const PipelineFixture fixture;
	const RecoveryPipeline pipeline(std::move(recoverers));
	const auto layouts = pipeline.run(fixture.context());

	REQUIRE(layouts.has_value());
	CHECK(layouts->structs.size() == 2);
	CHECK(layouts->structs.contains("Proto"));
	CHECK(layouts->structs.contains("CommonHeader"));
	CHECK_FALSE(layouts->report.has_failures());
}

TEST_CASE("the pipeline reports a dependency cycle")
{
	std::vector<std::unique_ptr<Recoverer>> recoverers;
	recoverers.push_back(std::make_unique<StubRecoverer>("A", std::vector<std::string_view>{"B"}));
	recoverers.push_back(std::make_unique<StubRecoverer>("B", std::vector<std::string_view>{"A"}));

	const PipelineFixture fixture;
	const auto layouts = RecoveryPipeline(std::move(recoverers)).run(fixture.context());

	REQUIRE_FALSE(layouts.has_value());
	CHECK(layouts.error().message().find("cycle") != std::string::npos);
}

TEST_CASE("the pipeline reports an unknown dependency")
{
	std::vector<std::unique_ptr<Recoverer>> recoverers;
	recoverers.push_back(std::make_unique<StubRecoverer>("A", std::vector<std::string_view>{"Missing"}));

	const PipelineFixture fixture;
	const auto layouts = RecoveryPipeline(std::move(recoverers)).run(fixture.context());

	REQUIRE_FALSE(layouts.has_value());
	CHECK(layouts.error().message().find("Missing") != std::string::npos);
}

TEST_CASE("the pipeline carries failures out so a report can still be written")
{
	std::vector<std::unique_ptr<Recoverer>> recoverers;
	recoverers.push_back(std::make_unique<FailingRecoverer>());

	const PipelineFixture fixture;
	const auto layouts = RecoveryPipeline(std::move(recoverers)).run(fixture.context());

	REQUIRE(layouts.has_value());
	CHECK(layouts->report.has_failures());
	REQUIRE(layouts->report.failures().size() == 1);
	CHECK(layouts->report.failures()[0].struct_name == "Broken");
	CHECK(layouts->report.failures()[0].field_name == "field");
	CHECK(layouts->report.summary().find("the probe found nothing") != std::string::npos);
}

TEST_CASE("the pipeline refuses a layout that does not validate")
{
	std::vector<std::unique_ptr<Recoverer>> recoverers;
	recoverers.push_back(std::make_unique<OverlappingRecoverer>());

	const PipelineFixture fixture;
	const auto layouts = RecoveryPipeline(std::move(recoverers)).run(fixture.context());

	REQUIRE_FALSE(layouts.has_value());
	CHECK(layouts.error().code() == ErrorCode::validation);
}

TEST_CASE("the context refuses to trace an unresolved anchor")
{
	const PipelineFixture fixture;

	const auto trace = fixture.context().trace(target::Anchor::luaH_new);

	REQUIRE_FALSE(trace.has_value());
	CHECK(trace.error().message().find("luaH_new") != std::string::npos);
}
