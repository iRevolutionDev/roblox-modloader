#include "rml/dumper/recover/recovery_context.hpp"

namespace rml::dumper::recover
{
	RecoveryContext::RecoveryContext(const image::Image& image, const disasm::Decoder& decoder,
	                                 const disasm::Abi& abi, const target::AnchorSet& anchors,
	                                 schema::Report& report) :
	    m_image(image),
	    m_decoder(decoder),
	    m_abi(abi),
	    m_anchors(anchors),
	    m_report(report)
	{
	}

	std::expected<const disasm::Trace*, Error> RecoveryContext::trace(const target::Anchor anchor) const
	{
		if (const auto cached = m_traces.find(anchor); cached != m_traces.end())
			return &cached->second;

		if (!m_anchors.has(anchor))
			return std::unexpected(
			    Error::make(ErrorCode::recovery, "anchor {} was never resolved", to_string(anchor)));

		auto traced = m_decoder.trace_function(m_image, m_anchors.at(anchor));
		if (!traced)
			return std::unexpected(traced.error());

		const auto [entry, inserted] = m_traces.emplace(anchor, std::move(*traced));
		return &entry->second;
	}

	const schema::StructLayout* RecoveryContext::layout(const std::string_view name) const
	{
		const auto found = m_layouts.find(name);
		return found != m_layouts.end() ? &found->second : nullptr;
	}

	void RecoveryContext::publish(schema::StructLayout layout)
	{
		auto name = layout.name;
		m_layouts.insert_or_assign(std::move(name), std::move(layout));
	}
}
