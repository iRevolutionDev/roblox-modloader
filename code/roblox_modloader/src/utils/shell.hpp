#pragma once

#include <filesystem>
#include <string_view>

namespace rml::utils::shell
{
	void open(const std::filesystem::path& path);
	void open_folder(const std::filesystem::path& path);
	void message_box(std::string_view title, std::string_view text);
}
