#pragma once

#include "rml/dumper/recover/recoverer.hpp"

namespace rml::dumper::recover
{
	class LuaStateRecoverer final : public Recoverer
	{
	public:
		[[nodiscard]] std::string_view struct_name() const override { return "lua_State"; }
		[[nodiscard]] std::span<const std::string_view> depends_on() const override { return {}; }

		[[nodiscard]] std::expected<schema::StructLayout, Error> recover(
		    const RecoveryContext& context) const override;

	private:
		class Probe
		{
		public:
			Probe(const RecoveryContext& context, schema::StructLayout& layout) :
			    m_context(context),
			    m_layout(layout)
			{
			}

			void take(std::string name, std::string type, std::size_t size, const disasm::MemoryAccess* access,
			          target::Anchor anchor, std::string description);
			void confirm(std::string_view name, const disasm::MemoryAccess* access, std::string description) const;

		private:
			const RecoveryContext& m_context;
			schema::StructLayout& m_layout;
		};
	};
}
