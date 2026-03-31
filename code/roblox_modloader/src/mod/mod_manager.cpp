#include "mod_manager.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/config/config.hpp"
#include "RobloxModLoader/config/config_helpers.hpp"
#include "dotnet/dotnet_mod_loader.hpp"
#include "utils/directory.hpp"

#include <algorithm>
#include <cctype>

namespace rml
{
	ModManager::ModManager()
	{
		const auto mods_path = get_mods_dir();

		if (!mods_path.has_value())
		{
			LOG_ERROR("[ModManager] Failed to get mods directory: {}", mods_path.error());
			return;
		}

		const auto runtime_path = utils::directory::get_runtime_directory();

		register_loader(std::make_unique<dotnet::DotnetModLoader>(runtime_path, mods_path.value() / "dotnet"), {"dotnet"});

		for (auto mod_dir : std::filesystem::directory_iterator(mods_path.value()))
		{
			if (!mod_dir.is_directory())
			{
				continue;
			}

			auto native_result = load_directory(mod_dir.path() / "native");
			if (!native_result.has_value())
			{
				LOG_ERROR("[ModManager] Failed to load native mods: {}", native_result.error());
			}

			auto dotnet_result = load_directory(mod_dir.path() / "dotnet");
			if (!dotnet_result.has_value())
			{
				LOG_ERROR("[ModManager] Failed to load .NET mods: {}", dotnet_result.error());
			}

			auto scripts_result = load_directory(mod_dir.path() / "scripts");
			if (!scripts_result.has_value())
			{
				LOG_ERROR("[ModManager] Failed to load script mods: {}", scripts_result.error());
			}
		}
	}

	ModManager::~ModManager()
	{
		for (const auto& loader : m_loaders | std::views::values)
			loader->unload_all();
	}

	void ModManager::register_loader(std::unique_ptr<IModLoader> loader, const std::vector<std::string>& folders)
	{
		for (const auto& folder : folders)
		{
			m_loaders[folder] = std::move(loader);
		}
	}

	std::expected<void, std::string> ModManager::load_directory(const std::filesystem::path& directory) const
	{
		if (!std::filesystem::exists(directory))
		{
			return std::unexpected("Directory does not exist: " + directory.string());
		}

		if (!std::filesystem::is_directory(directory))
		{
			return std::unexpected("Path is not a directory: " + directory.string());
		}

		const auto loader = find_loader_for_path(directory);

		if (!loader.has_value())
		{
			return std::unexpected("No loader found for directory: " + directory.string());
		}

		std::vector<std::string> errors;
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			const auto& extensions = (*loader)->extensions();
			if (std::ranges::find_if(extensions,
			        [&entry](const auto& ext) {
				        return entry.path().extension() == ext;
			        })
			    == extensions.end())
			{
				continue;
			}

			if (const auto result = (*loader)->load(entry.path()); !result.has_value())
			{
				errors.push_back("Failed to load " + entry.path().string() + ": " + result.error());
			}
		}

		if (!errors.empty())
		{
			std::string error_message = "Errors occurred while loading mods from directory:\n";
			for (const auto& error : errors)
			{
				error_message += " - " + error + "\n";
			}
			return std::unexpected(std::move(error_message));
		}

		return {};
	}

	std::expected<void, std::string> ModManager::load(const std::filesystem::path& path) const
	{
		const auto loader = find_loader_for_path(path);
		if (!loader.has_value())
		{
			return std::unexpected("No loader found for file: " + path.string());
		}

		return (*loader)->load(path);
	}

	std::expected<void, std::string> ModManager::unload(const std::filesystem::path& path) const
	{
		const auto loader = find_loader_for_path(path);
		if (!loader.has_value())
		{
			return std::unexpected("No loader found for file: " + path.string());
		}

		return (*loader)->unload(path);
	}

	std::expected<void, std::string> ModManager::reload(const std::filesystem::path& path)
	{
		const auto loader = find_loader_for_path(path);
		if (!loader.has_value())
		{
			return std::unexpected("No loader found for file: " + path.string());
		}

		return (*loader)->reload(path);
	}

	std::expected<std::filesystem::path, std::string> ModManager::get_mods_dir() noexcept
	{
		auto mod_loader_dir = utils::directory::get_mod_loader_directory() / "mods";
		if (!std::filesystem::exists(mod_loader_dir))
		{
			if (std::error_code ec; !std::filesystem::create_directories(mod_loader_dir, ec))
			{
				return std::unexpected("Failed to create mods directory: " + ec.message());
			}
		}

		return std::filesystem::path(mod_loader_dir);
	}

	std::optional<IModLoader*> ModManager::find_loader_for_path(const std::filesystem::path& path) const noexcept
	{
		const auto& filename = path.filename().string();
		for (const auto& [folder, loader] : m_loaders)
		{
			if (filename.starts_with(folder))
			{
				return &*loader;
			}
		}
		return std::nullopt;
	}
}