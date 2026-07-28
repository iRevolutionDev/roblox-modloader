#include "cli/runner.hpp"

#include <spdlog/spdlog.h>

namespace rml::dumper::cli
{
	std::expected<void, Error> Runner::run() const
	{
		spdlog::info("rml dumper");
		return {};
	}
}
