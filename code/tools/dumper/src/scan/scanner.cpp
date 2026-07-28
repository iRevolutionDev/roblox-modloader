#include "rml/dumper/scan/scanner.hpp"

#include <algorithm>
#include <atomic>
#include <thread>

namespace rml::dumper::scan
{
	std::size_t Scanner::worker_count() const
	{
		if (m_thread_count != 0)
			return m_thread_count;

		const auto available = std::thread::hardware_concurrency();
		return available != 0 ? available : 1;
	}

	Scanner::Buckets Scanner::bucket_by_first_byte(const std::span<const Pattern> patterns)
	{
		Buckets buckets;

		for (std::size_t i = 0; i < patterns.size(); ++i)
			buckets[static_cast<std::uint8_t>(patterns[i].first_concrete_byte())].push_back(i);

		return buckets;
	}

	std::vector<Scanner::Block> Scanner::plan(const image::Image& image) const
	{
		const auto target_blocks = worker_count() * 4;

		std::uint64_t total = 0;
		for (const auto& section : image.executable_sections())
			total += section.size;

		const auto block_size = std::max<std::uint64_t>(minimum_block_size, total / std::max<std::size_t>(target_blocks, 1) + 1);

		std::vector<Block> blocks;
		for (const auto& section : image.executable_sections())
		{
			for (Rva cursor = section.address; cursor < section.end();)
			{
				const auto end = static_cast<Rva>(std::min<std::uint64_t>(cursor + block_size, section.end()));
				blocks.push_back({cursor, end, &section});
				cursor = end;
			}
		}

		return blocks;
	}

	std::vector<std::vector<Rva>> Scanner::scan_block(const image::Image& image,
	                                                  const std::span<const Pattern> patterns, const Buckets& buckets,
	                                                  const Block& block)
	{
		std::vector<std::vector<Rva>> hits(patterns.size());

		const auto memory = image.memory();
		const auto limit = std::min<std::size_t>(block.end, memory.size());

		for (std::size_t position = block.begin; position < limit; ++position)
		{
			const auto& candidates = buckets[static_cast<std::uint8_t>(memory[position])];
			if (candidates.empty())
				continue;

			for (const auto index : candidates)
			{
				const auto& pattern = patterns[index];
				if (position < pattern.first_concrete_index())
					continue;

				const auto start = position - pattern.first_concrete_index();
				if (start < block.section->address || start + pattern.size() > block.section->end())
					continue;

				if (pattern.matches_at(memory, start))
					hits[index].push_back(static_cast<Rva>(start));
			}
		}

		return hits;
	}

	std::vector<std::vector<Rva>> Scanner::scan(const image::Image& image,
	                                            const std::span<const Pattern> patterns) const
	{
		if (patterns.empty())
			return {};

		const auto buckets = bucket_by_first_byte(patterns);
		const auto blocks = plan(image);

		std::vector<std::vector<std::vector<Rva>>> results(blocks.size());
		std::atomic<std::size_t> next{0};

		const auto workers = std::min(worker_count(), blocks.size());
		{
			std::vector<std::jthread> threads;
			threads.reserve(workers);

			for (std::size_t i = 0; i < workers; ++i)
				threads.emplace_back([&] {
					for (auto index = next.fetch_add(1); index < blocks.size(); index = next.fetch_add(1))
						results[index] = scan_block(image, patterns, buckets, blocks[index]);
				});
		}

		std::vector<std::vector<Rva>> merged(patterns.size());
		for (const auto& block_hits : results)
			for (std::size_t i = 0; i < block_hits.size(); ++i)
				merged[i].insert(merged[i].end(), block_hits[i].begin(), block_hits[i].end());

		return merged;
	}
}
