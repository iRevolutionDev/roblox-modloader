#pragma once

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "imod_loader.hpp"

namespace RBX
{
	class DataModel;
	enum class DataModelType : std::int32_t;
}

namespace rml
{
	class ModManager
	{
	public:
		explicit ModManager();
		~ModManager();

		ModManager(const ModManager&)            = delete;
		ModManager& operator=(const ModManager&) = delete;

		void register_loader(std::unique_ptr<IModLoader> loader, const std::vector<std::string>& folders);

		std::expected<void, std::string> load_directory(const std::filesystem::path& directory) const;
		std::expected<void, std::string> load(const std::filesystem::path& path) const;
		std::expected<void, std::string> unload(const std::filesystem::path& path) const;
		std::expected<void, std::string> reload(const std::filesystem::path& path);

		[[nodiscard]] static std::expected<std::filesystem::path, std::string> get_mods_dir() noexcept;

	private:
		[[nodiscard]] std::optional<IModLoader*> find_loader_for_path(const std::filesystem::path& path) const noexcept;

		std::unordered_map<std::string, std::unique_ptr<IModLoader>> m_loaders;
	};
}
