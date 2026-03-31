#pragma once

namespace rml::utils::directory
{
	std::filesystem::path get_module_directory();

	std::filesystem::path get_executable_directory();

	std::filesystem::path get_mod_loader_directory();

	std::filesystem::path get_runtime_directory();
}
