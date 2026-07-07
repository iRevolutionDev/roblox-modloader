#include "mod_kind.hpp"

namespace rml
{
	std::optional<ModKind> mod_kind_from_folder(const std::string_view folder_name) noexcept
	{
		for (const auto& entry : kModKindFolders)
		{
			if (entry.folder_name == folder_name)
				return entry.kind;
		}
		return std::nullopt;
	}

	std::string_view mod_kind_folder_name(const ModKind kind) noexcept
	{
		for (const auto& entry : kModKindFolders)
		{
			if (entry.kind == kind)
				return entry.folder_name;
		}
		return {};
	}
}
