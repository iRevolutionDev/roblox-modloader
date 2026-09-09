#pragma once

#include "rml/dumper/core/types.hpp"
#include "rml/dumper/disasm/abi.hpp"
#include "rml/dumper/target/anchor.hpp"

#include <span>
#include <string>
#include <string_view>

namespace rml::dumper::target
{
	struct TargetProfile
	{
		std::string_view name;
		ImageFormat format{};
		Architecture architecture{};
		const disasm::Abi* abi{};
		std::span<const AnchorSpec> anchors;
	};

	class TargetRegistry
	{
	public:
		[[nodiscard]] static const TargetProfile* find(std::string_view name);
		[[nodiscard]] static std::span<const TargetProfile> all();
		[[nodiscard]] static std::string names();
	};
}
