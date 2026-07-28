#pragma once

#include "rml/dumper/recover/recovery_context.hpp"

#include <span>

namespace rml::dumper::recover
{
	class Recoverer
	{
	public:
		virtual ~Recoverer() = default;

		[[nodiscard]] virtual std::string_view struct_name() const = 0;
		[[nodiscard]] virtual std::span<const std::string_view> depends_on() const = 0;
		[[nodiscard]] virtual std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const = 0;
	};
}
