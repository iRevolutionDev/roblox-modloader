#include "target/targets/profiles.hpp"

namespace rml::dumper::target
{
	const TargetProfile& macos_arm64_profile()
	{
		static const TargetProfile profile{
		    .name = "macos-arm64",
		    .format = ImageFormat::mach_o,
		    .architecture = Architecture::arm64,
		    .abi = &disasm::Abi::aapcs64(),
		    .anchors = {},
		};

		return profile;
	}
}
