#pragma once

#include "rml/dumper/recover/recoverer.hpp"

namespace rml::dumper::recover
{
	class CommonHeaderRecoverer final : public Recoverer
	{
	public:
		[[nodiscard]] std::string_view struct_name() const override { return "CommonHeader"; }
		[[nodiscard]] std::span<const std::string_view> depends_on() const override { return {}; }

		[[nodiscard]] std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const override;

		[[nodiscard]] static std::vector<disasm::MemoryAccess> header_writes(const disasm::Trace& trace);

	private:
		static constexpr std::size_t header_field_count = 3;
		static constexpr std::int64_t plausible_header_span = 4;
	};
}
