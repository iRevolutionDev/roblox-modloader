
#pragma once
#include "mod/imod_loader.hpp"

#include <RobloxModLoader/common.hpp>
#include <RobloxModLoader/memory/module.hpp>
#include <RobloxModLoader/mod/mod_base.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace rml::native
{
	class NativeModLoader final : public IModLoader
	{
	public:
		explicit NativeModLoader() = default;
		~NativeModLoader() override;

		std::expected<void, std::string> load(const std::filesystem::path& path) override;
		std::expected<void, std::string> unload(const std::filesystem::path& path) override;
		std::expected<void, std::string> reload(const std::filesystem::path& path) override;
		[[nodiscard]] std::vector<std::filesystem::path> extensions() const override
		{
			return {".dll"};
		}
		void unload_all() override;

	private:
		struct LoadedMod
		{
			std::unique_ptr<memory::module> module;
			ModBase* instance = nullptr;
			using uninstall_t = void (*)(const ModBase*);
			uninstall_t uninstall = nullptr;
		};

		std::unordered_map<std::filesystem::path, LoadedMod> m_loaded_mods;
	};

} // namespace rml::native
