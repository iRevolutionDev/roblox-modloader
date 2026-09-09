#pragma once

#include "rml/dumper/disasm/decoder.hpp"

namespace rml::dumper::disasm
{
	class Arm64Decoder final : public Decoder
	{
	public:
		Arm64Decoder();
		~Arm64Decoder() override;

		Arm64Decoder(Arm64Decoder&&) noexcept;
		Arm64Decoder& operator=(Arm64Decoder&&) noexcept;

		[[nodiscard]] Architecture architecture() const override { return Architecture::arm64; }
		[[nodiscard]] std::expected<Trace, Error> trace(std::span<const std::byte> code, Rva begin) const override;

	private:
		class Session;

		std::unique_ptr<Session> m_session;
	};
}
