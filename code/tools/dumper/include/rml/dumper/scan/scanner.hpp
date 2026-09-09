#pragma once

#include "rml/dumper/image/image.hpp"
#include "rml/dumper/scan/pattern.hpp"

#include <array>
#include <span>
#include <vector>

namespace rml::dumper::scan
{
	class Scanner
	{
	public:
		Scanner() = default;
		explicit Scanner(std::size_t thread_count) :
		    m_thread_count(thread_count)
		{
		}

		[[nodiscard]] std::vector<std::vector<Rva>> scan(const image::Image& image,
		                                                 std::span<const Pattern> patterns) const;

	private:
		struct Block
		{
			Rva begin{};
			Rva end{};
			const image::Section* section{};
		};

		using Buckets = std::array<std::vector<std::size_t>, 256>;

		[[nodiscard]] static Buckets bucket_by_first_byte(std::span<const Pattern> patterns);
		[[nodiscard]] std::vector<Block> plan(const image::Image& image) const;
		[[nodiscard]] static std::vector<std::vector<Rva>> scan_block(const image::Image& image,
		                                                              std::span<const Pattern> patterns,
		                                                              const Buckets& buckets, const Block& block);
		[[nodiscard]] std::size_t worker_count() const;

		static constexpr std::size_t minimum_block_size = 64 * 1024;

		std::size_t m_thread_count{0};
	};
}
