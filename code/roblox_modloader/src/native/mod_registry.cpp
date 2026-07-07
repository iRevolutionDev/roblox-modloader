#include "mod_registry.hpp"

#include <ranges>
#include <utility>

namespace rml::native
{
	bool ModRegistry::contains(const std::filesystem::path& path) const
	{
		std::shared_lock lock(m_mutex);
		return m_entries.contains(path);
	}

	void ModRegistry::insert(const std::filesystem::path& path, Entry entry)
	{
		std::unique_lock lock(m_mutex);
		m_entries[path] = std::move(entry);
	}

	std::optional<ModRegistry::Entry> ModRegistry::extract(const std::filesystem::path& path)
	{
		std::unique_lock lock(m_mutex);
		const auto it = m_entries.find(path);
		if (it == m_entries.end())
			return std::nullopt;

		Entry entry = std::move(it->second);
		m_entries.erase(it);
		return entry;
	}

	std::vector<ModRegistry::Entry> ModRegistry::extract_all()
	{
		std::unique_lock lock(m_mutex);
		std::vector<Entry> entries;
		entries.reserve(m_entries.size());
		for (auto& entry : m_entries | std::views::values)
			entries.push_back(std::move(entry));

		m_entries.clear();
		return entries;
	}
}
