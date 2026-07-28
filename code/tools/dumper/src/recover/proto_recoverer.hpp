#pragma once

#include "rml/dumper/recover/recoverer.hpp"

namespace rml::dumper::recover
{
	class ProtoRecoverer final : public Recoverer
	{
	public:
		[[nodiscard]] std::string_view struct_name() const override { return "Proto"; }
		[[nodiscard]] std::span<const std::string_view> depends_on() const override { return m_dependencies; }

		[[nodiscard]] std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const override;

		struct FreedArray
		{
			std::int64_t pointer{};
			std::int64_t size{};
		};

		[[nodiscard]] static std::vector<FreedArray> freed_arrays(const disasm::Trace& trace, Rva free_function,
		                                                          disasm::Register proto);

	private:
		static constexpr std::array<std::string_view, 1> m_dependencies{"CommonHeader"};
	};
}
