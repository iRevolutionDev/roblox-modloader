#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau
{
	class ModuleId final
	{
	public:
		ModuleId() = default;

		explicit ModuleId(std::string canonical_path);

		[[nodiscard]] std::string_view native() const noexcept { return m_path; }
		[[nodiscard]] const std::string& string() const noexcept { return m_path; }
		[[nodiscard]] std::size_t hash() const noexcept { return m_hash; }
		[[nodiscard]] bool empty() const noexcept { return m_path.empty(); }

		[[nodiscard]] std::string_view display() const noexcept;

		friend bool operator==(const ModuleId& lhs, const ModuleId& rhs) noexcept
		{
			return lhs.m_hash == rhs.m_hash && lhs.m_key == rhs.m_key;
		}

	private:
		std::string m_path;
		std::string m_key;
		std::size_t m_hash{0};
	};

	struct ModuleIdHash
	{
		[[nodiscard]] std::size_t operator()(const ModuleId& id) const noexcept { return id.hash(); }
	};
}
