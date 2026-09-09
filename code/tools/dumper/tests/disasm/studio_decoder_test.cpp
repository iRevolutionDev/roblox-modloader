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
	std::size_t empty_and_tiny = 0;
	std::size_t empty_and_large = 0;
	std::uint32_t largest_empty = 0;

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
			else if (bounds->size() <= 32)
				++empty_and_tiny;
			else
			{
				++empty_and_large;
				largest_empty = std::max(largest_empty, bounds->size());
			}
			accesses += trace->accesses.size();
		}

		++traced;
		cursor = bounds->end;
	}

	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
	    std::chrono::steady_clock::now() - started);

	CHECK(traced == limit);
	CHECK(failed == 0);
	CHECK(empty_and_large * 100 < traced);

	MESSAGE(to_string(architecture), ": traced ", traced, " functions in ", elapsed.count(), " ms, ", with_content,
	        " carried memory accesses or calls, ", accesses, " accesses total, ", empty_and_tiny,
	        " empty and <= 32 bytes, ", empty_and_large, " empty and larger (largest ", largest_empty, " bytes)");
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

TEST_CASE("the arm64 decoder survives thousands of real studio functions")
{
	const auto path = tests::StudioBinary::macos(Architecture::arm64);
	if (!path)
	{
		MESSAGE("no macos arm64 binary configured, skipping");
		return;
	}

	trace_many_functions(*path, Architecture::arm64, 5000);
}
