#include "target/pattern_anchor_resolver.hpp"

#include "rml/dumper/scan/scanner.hpp"

#include <algorithm>
#include <format>
#include <ranges>

namespace rml::dumper::target
{
	std::expected<AnchorSet, Error> PatternAnchorResolver::resolve(const image::Image& image,
	                                                               const std::span<const AnchorSpec> anchors) const
	{
		if (anchors.empty())
			return std::unexpected(Error::make(ErrorCode::anchor, "the target profile defines no anchors"));

		std::vector<scan::Pattern> patterns;
		patterns.reserve(anchors.size());

		std::string parse_failures;
		for (const auto& spec : anchors)
		{
			auto pattern = scan::Pattern::parse(spec.pattern);
			if (!pattern)
			{
				parse_failures += std::format("\n  {}: {}", to_string(spec.id), pattern.error().message());
				continue;
			}

			patterns.push_back(std::move(*pattern));
		}

		if (!parse_failures.empty())
			return std::unexpected(Error::make(ErrorCode::anchor, "malformed anchor patterns:{}", parse_failures));

		const auto hits = scan::Scanner().scan(image, patterns);

		AnchorSet resolved;
		std::string failures;

		for (std::size_t i = 0; i < anchors.size(); ++i)
		{
			const auto name = to_string(anchors[i].id);

			if (hits[i].empty())
			{
				failures += std::format("\n  {}: no match", name);
				continue;
			}

			if (hits[i].size() > 1)
			{
				failures += std::format("\n  {}: {} matches at", name, hits[i].size());
				for (const auto hit : hits[i] | std::views::take(4))
					failures += std::format(" 0x{:X}", hit);
				continue;
			}

			resolved.set(anchors[i].id, hits[i].front());
		}

		if (!failures.empty())
			return std::unexpected(Error::make(ErrorCode::anchor, "{} of {} anchors did not resolve uniquely:{}",
			                                   std::ranges::count(failures, '\n'), anchors.size(), failures));

		return resolved;
	}
}
