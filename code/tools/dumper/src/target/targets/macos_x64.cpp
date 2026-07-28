#include "target/targets/profiles.hpp"

namespace rml::dumper::target
{
	const TargetProfile& macos_x64_profile()
	{
		static const TargetProfile profile{
		    .name = "macos-x64",
		    .format = ImageFormat::mach_o,
		    .architecture = Architecture::x86_64,
		    .abi = &disasm::Abi::system_v_x64(),
		    .anchors = {},
		};

		return profile;
	}
}
