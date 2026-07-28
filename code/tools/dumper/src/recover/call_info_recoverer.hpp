#pragma once

#include "rml/dumper/recover/recoverer.hpp"

namespace rml::dumper::recover
{
	class CallInfoRecoverer final : public Recoverer
	{
	public:
		[[nodiscard]] std::string_view struct_name() const override { return "CallInfo"; }
		[[nodiscard]] std::span<const std::string_view> depends_on() const override { return m_dependencies; }

		[[nodiscard]] std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const override;

		[[nodiscard]] static std::vector<std::int64_t> relocated_slots(const disasm::Trace& trace,
		                                                               disasm::Register state);

	private:
		static constexpr std::array<std::string_view, 1> m_dependencies{"lua_State"};
	};
}
