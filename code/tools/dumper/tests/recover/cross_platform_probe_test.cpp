#include <doctest/doctest.h>

#include "rml/dumper/disasm/decoder.hpp"
#include "rml/dumper/disasm/trace_query.hpp"
#include "support/studio_binary.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <map>
#include <ranges>

using namespace rml::dumper;

TEST_CASE("the macos build lays lua_State out exactly like the windows build")
{
	const auto path = tests::StudioBinary::macos(Architecture::arm64);
	if (!path)
	{
		MESSAGE("no macos arm64 binary configured, skipping");
		return;
	}

	const auto image = image::ImageLoader::load(*path, Architecture::arm64);
	REQUIRE(image.has_value());

	const auto decoder = disasm::Decoder::create(Architecture::arm64);
	REQUIRE(decoder.has_value());

	const auto& text = image->executable_sections()[0];

	struct Candidate
	{
		Rva entry{};
		std::vector<std::int64_t> qwords;
		std::int64_t dword{};
	};

	std::vector<Candidate> candidates;
	std::size_t scanned = 0;

	const auto started = std::chrono::steady_clock::now();

	for (Rva cursor = text.address; cursor < text.end();)
	{
		const auto bounds = image->functions().containing(cursor);
		if (!bounds)
		{
			++cursor;
			continue;
		}

		cursor = bounds->end;
		++scanned;

		if (bounds->size() < 0x40 || bounds->size() > 0x600)
			continue;

		const auto trace = (*decoder)->trace_function(*image, bounds->begin);
		if (!trace || trace->calls.empty())
			continue;

		const auto after = trace->calls.front().sequence;

		std::map<disasm::Register, std::vector<std::int64_t>> qwords;
		std::map<disasm::Register, std::int64_t> dwords;

		for (const auto& access : trace->accesses)
		{
			if (!access.is_write || access.sequence <= after || access.displacement < 0 ||
			    access.displacement >= 0x100)
				continue;

			if (access.width == 8)
			{
				auto& offsets = qwords[access.base];
				if (std::ranges::find(offsets, access.displacement) == offsets.end())
					offsets.push_back(access.displacement);
			}
			else if (access.width == 4 && !dwords.contains(access.base))
			{
				dwords[access.base] = access.displacement;
			}
		}

		for (const auto& [base, offsets] : qwords)
		{
			if (offsets.size() != 4 || !dwords.contains(base))
				continue;

			candidates.push_back({bounds->begin, offsets, dwords.at(base)});
		}
	}

	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
	    std::chrono::steady_clock::now() - started);

	MESSAGE("scanned ", scanned, " functions in ", elapsed.count(), " ms, ", candidates.size(),
	        " match the reallocation shape");

	std::map<std::string, std::size_t> shapes;
	for (const auto& candidate : candidates)
	{
		std::string key;
		for (const auto offset : candidate.qwords)
			key += std::format("{:X} ", offset);
		key += std::format("/ {:X}", candidate.dword);
		++shapes[key];
	}

	std::vector<std::pair<std::string, std::size_t>> ranked(shapes.begin(), shapes.end());
	std::ranges::sort(ranked, [](const auto& left, const auto& right) { return left.second > right.second; });

	for (std::size_t i = 0; i < ranked.size() && i < 12; ++i)
		MESSAGE("  qwords ", ranked[i].first, " seen ", ranked[i].second, " times");

	const std::vector<std::int64_t> windows_qwords{0x40, 0x50, 0x58, 0x68};
	std::size_t exact = 0;

	for (const auto& candidate : candidates)
	{
		auto sorted = candidate.qwords;
		std::ranges::sort(sorted);

		if (sorted == windows_qwords && candidate.dword == 0x20)
		{
			++exact;
			MESSAGE("EXACT windows shape at 0x", std::format("{:X}", candidate.entry));
		}
	}

	MESSAGE("candidates carrying the windows offsets 40 50 58 68 with a dword at 20: ", exact);

	std::size_t same_set = 0;
	for (const auto& candidate : candidates)
	{
		auto sorted = candidate.qwords;
		std::ranges::sort(sorted);
		if (sorted == windows_qwords)
			++same_set;
	}

	MESSAGE("candidates carrying those four offsets in any order: ", same_set);

	CHECK(exact == 1);
	CHECK(same_set == 1);
}
