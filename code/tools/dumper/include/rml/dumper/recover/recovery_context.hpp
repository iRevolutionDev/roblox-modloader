#pragma once

#include "rml/dumper/disasm/abi.hpp"
#include "rml/dumper/disasm/decoder.hpp"
#include "rml/dumper/disasm/trace_query.hpp"
#include "rml/dumper/image/image.hpp"
#include "rml/dumper/schema/layout_set.hpp"
#include "rml/dumper/target/anchor.hpp"

#include <map>

namespace rml::dumper::recover
{
	class RecoveryContext
	{
	public:
		RecoveryContext(const image::Image& image, const disasm::Decoder& decoder, const disasm::Abi& abi,
		                const target::AnchorSet& anchors, schema::Report& report);

		[[nodiscard]] const image::Image& image() const { return m_image; }
		[[nodiscard]] const disasm::Abi& abi() const { return m_abi; }
		[[nodiscard]] const target::AnchorSet& anchors() const { return m_anchors; }
		[[nodiscard]] schema::Report& report() const { return m_report; }

		[[nodiscard]] std::expected<const disasm::Trace*, Error> trace(target::Anchor anchor) const;
		[[nodiscard]] const schema::StructLayout* layout(std::string_view name) const;

		void publish(schema::StructLayout layout);

	private:
		const image::Image& m_image;
		const disasm::Decoder& m_decoder;
		const disasm::Abi& m_abi;
		const target::AnchorSet& m_anchors;
		schema::Report& m_report;

		mutable std::map<target::Anchor, disasm::Trace> m_traces;
		std::map<std::string, schema::StructLayout, std::less<>> m_layouts;
	};
}
