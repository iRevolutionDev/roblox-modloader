#pragma once

#include "rml/dumper/core/types.hpp"

#include <optional>
#include <vector>

namespace rml::dumper::index
{
	struct FunctionBounds
	{
		Rva begin{};
		Rva end{};

		[[nodiscard]] std::uint32_t size() const { return end - begin; }
	};

	class FunctionIndex
	{
	public:
		FunctionIndex() = default;
		explicit FunctionIndex(std::vector<FunctionBounds> functions);

		[[nodiscard]] std::optional<FunctionBounds> containing(Rva address) const;
		[[nodiscard]] std::optional<FunctionBounds> at(Rva begin) const;

		[[nodiscard]] std::size_t size() const { return m_functions.size(); }
		[[nodiscard]] bool empty() const { return m_functions.empty(); }

	private:
		std::vector<FunctionBounds> m_functions;
	};
}
