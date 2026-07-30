#pragma once

#include "RobloxModLoader/luau/modules/module_id.hpp"
#include "RobloxModLoader/luau/vm/vm_error.hpp"
#include <unordered_map>

namespace rml::luau
{
	class RML_EXPORT BytecodeCache final
	{
	public:
		[[nodiscard]] std::expected<std::span<const std::byte>, vm::VmError> acquire(const ModuleId& id);

		void invalidate(const ModuleId& id);
		void clear() noexcept;

		[[nodiscard]] std::size_t size() const noexcept { return m_entries.size(); }

		[[nodiscard]] static std::expected<std::vector<std::byte>, vm::VmError> compile(
		    std::string_view source, std::string_view chunk_name);

	private:
		struct Entry
		{
			std::filesystem::file_time_type mtime{};
			std::vector<std::byte> bytecode;
		};

		std::unordered_map<ModuleId, Entry, ModuleIdHash> m_entries;
	};
}
