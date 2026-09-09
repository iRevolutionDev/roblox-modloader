#include "rml/dumper/index/function_index.hpp"

#include <algorithm>

namespace rml::dumper::index
{
	FunctionIndex::FunctionIndex(std::vector<FunctionBounds> functions) :
	    m_functions(std::move(functions))
	{
		std::ranges::sort(m_functions, {}, &FunctionBounds::begin);

		const auto duplicates = std::ranges::unique(m_functions, {}, &FunctionBounds::begin);
		m_functions.erase(duplicates.begin(), duplicates.end());
	}

	std::optional<FunctionBounds> FunctionIndex::containing(const Rva address) const
	{
		const auto after = std::ranges::upper_bound(m_functions, address, {}, &FunctionBounds::begin);
		if (after == m_functions.begin())
			return std::nullopt;

		const auto candidate = *std::prev(after);
		if (address >= candidate.end)
			return std::nullopt;

		return candidate;
	}

	std::optional<FunctionBounds> FunctionIndex::at(const Rva begin) const
	{
		const auto found = std::ranges::lower_bound(m_functions, begin, {}, &FunctionBounds::begin);
		if (found == m_functions.end() || found->begin != begin)
			return std::nullopt;

		return *found;
	}
}
