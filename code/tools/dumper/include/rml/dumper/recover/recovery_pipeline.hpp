#pragma once

#include "rml/dumper/recover/recoverer.hpp"

#include <memory>
#include <vector>

namespace rml::dumper::recover
{
	class RecoveryPipeline
	{
	public:
		explicit RecoveryPipeline(std::vector<std::unique_ptr<Recoverer>> recoverers);

		[[nodiscard]] std::expected<schema::LayoutSet, Error> run(RecoveryContext& context) const;

		[[nodiscard]] static RecoveryPipeline make_default();

	private:
		[[nodiscard]] std::expected<std::vector<const Recoverer*>, Error> order() const;

		std::vector<std::unique_ptr<Recoverer>> m_recoverers;
	};
}
