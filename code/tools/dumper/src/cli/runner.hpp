#pragma once

#include "cli/options.hpp"
#include "rml/dumper/fetch/studio_provider.hpp"
#include "rml/dumper/schema/layout_set.hpp"

namespace rml::dumper::cli
{
	class Runner
	{
	public:
		[[nodiscard]] std::expected<void, Error> run(const Options& options) const;

	private:
		[[nodiscard]] static std::expected<fetch::ResolvedStudio, Error> resolve_studio(const Options& options);
		[[nodiscard]] static std::expected<void, Error> write_outputs(const Options& options,
		                                                              const schema::LayoutSet& layouts);
	};
}
