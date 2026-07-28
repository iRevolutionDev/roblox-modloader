#pragma once

#include "rml/dumper/core/error.hpp"

#include <expected>

namespace rml::dumper::cli
{
	class Runner
	{
	public:
		[[nodiscard]] std::expected<void, Error> run() const;
	};
}
