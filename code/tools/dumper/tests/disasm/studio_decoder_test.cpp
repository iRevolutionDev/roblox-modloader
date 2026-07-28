#include <doctest/doctest.h>

#include "rml/dumper/disasm/decoder.hpp"
#include "support/studio_binary.hpp"

#include <chrono>

using namespace rml::dumper;

static void trace_many_functions(const std::filesystem::path& path, const Architecture architecture,
                                 const std::size_t limit)
{
	const auto image = image::ImageLoader::load(path, architecture);
	REQUIRE(image.has_value());

	const auto decoder = disasm::Decoder::create(architecture);
	REQUIRE(decoder.has_value());

	const auto& text = image->executable_sections()[0];

	std::size_t traced = 0;
	std::size_t failed = 0;
	std::size_t with_content = 0;
	std::size_t accesses = 0;

	const auto started = std::chrono::steady_clock::now();

	for (Rva cursor = text.address; cursor < text.end() && traced < limit;)
	{
		const auto bounds = image->functions().containing(cursor);
		if (!bounds)
		{
			++cursor;
			continue;
		}

		const auto trace = (*decoder)->trace_function(*image, bounds->begin);
		if (!trace)
			++failed;
		else
		{
			if (!trace->accesses.empty() || !trace->calls.empty())
				++with_content;
			accesses += trace->accesses.size();
		}

		++traced;
		cursor = bounds->end;
	}

	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
	    std::chrono::steady_clock::now() - started);

	CHECK(traced == limit);
	CHECK(failed == 0);
	CHECK(with_content * 10 > traced * 8);

	MESSAGE(to_string(architecture), ": traced ", traced, " functions in ", elapsed.count(), " ms, ", with_content,
	        " carried memory accesses or calls, ", accesses, " accesses total");
}

TEST_CASE("the x86 decoder survives thousands of real studio functions")
{
	const auto path = tests::StudioBinary::windows();
	if (!path)
	{
		MESSAGE("RML_TEST_STUDIO is not set, skipping");
		return;
	}

	trace_many_functions(*path, Architecture::x86_64, 5000);
}
