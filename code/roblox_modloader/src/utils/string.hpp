#pragma once

#include <string>
#include <string_view>

namespace rml::utils::string
{
	[[nodiscard]] std::wstring to_wide(std::string_view utf8);
}
