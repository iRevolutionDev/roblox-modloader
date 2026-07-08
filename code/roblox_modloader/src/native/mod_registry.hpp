#pragma once
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace rml::native
{
	class ModRegistry
	{
	public:
		struct Entry
		{
			std::unique_ptr<memory::module> module;
			ModBase* instance = nullptr;
			using uninstall_t = void (*)(const ModBase*);
			uninstall_t uninstall = nullptr;
		};

		[[nodiscard]] bool contains(const std::filesystem::path& path) const;
		void insert(const std::filesystem::path& path, Entry entry);
		[[nodiscard]] std::optional<Entry> extract(const std::filesystem::path& path);
		[[nodiscard]] std::vector<Entry> extract_all();

	private:
		mutable std::shared_mutex m_mutex;
		std::unordered_map<std::filesystem::path, Entry> m_entries;
	};
}
