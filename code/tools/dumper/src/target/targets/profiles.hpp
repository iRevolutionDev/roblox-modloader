#pragma once

#include "rml/dumper/target/target_profile.hpp"

#include <array>

namespace rml::dumper::target
{
	[[nodiscard]] const TargetProfile& windows_x64_profile();
	[[nodiscard]] const TargetProfile& macos_x64_profile();
	[[nodiscard]] const TargetProfile& macos_arm64_profile();
}
