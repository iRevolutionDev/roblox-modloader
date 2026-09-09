#pragma once

#include "rml/dumper/recover/recoverer.hpp"

namespace rml::dumper::recover
{
	class GlobalStateRecoverer final : public Recoverer
	{
	public:
		[[nodiscard]] std::string_view struct_name() const override { return "global_State"; }
		[[nodiscard]] std::span<const std::string_view> depends_on() const override { return m_dependencies; }

		[[nodiscard]] std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const override;

	private:
		static constexpr std::array<std::string_view, 2> m_dependencies{"lua_State", "CommonHeader"};

		[[nodiscard]] static disasm::Object loaded_by(const disasm::Trace& trace, const disasm::MemoryAccess& read);
	};
}
