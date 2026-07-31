#include "RobloxModLoader/luau/modules/bytecode_cache.hpp"

#include "RobloxModLoader/luau/vm/compile_options.hpp"

#include <Luau/Compiler.h>

RML_LOG_SCOPE("Modules");

namespace rml::luau
{
	static std::expected<std::string, vm::VmError> read_source(const std::filesystem::path& path)
	{
		std::error_code ec;
		const auto size = std::filesystem::file_size(path, ec);
		if (ec)
		{
			return std::unexpected(vm::VmError::internal(std::format("cannot size module '{}': {}", path.generic_string(), ec.message())));
		}

		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			return std::unexpected(vm::VmError::internal(std::format("cannot open module '{}'", path.generic_string())));
		}

		std::string source;
		source.resize(size);

		if (size != 0)
		{
			file.read(source.data(), static_cast<std::streamsize>(size));
			if (file.bad())
			{
				return std::unexpected(vm::VmError::internal(std::format("cannot read module '{}'", path.generic_string())));
			}
			source.resize(static_cast<std::size_t>(file.gcount()));
		}

		return source;
	}

	std::expected<std::vector<std::byte>, vm::VmError> BytecodeCache::compile(const std::string_view source, const std::string_view chunk_name)
	{
		const auto encoded = Luau::compile(std::string{source}, vm::kCompileOptions);
		if (encoded.empty())
		{
			return std::unexpected(vm::VmError::syntax(std::format("'{}' compiled to no bytecode", chunk_name)));
		}

		const auto* first = reinterpret_cast<const std::byte*>(encoded.data());
		return std::vector<std::byte>{first, first + encoded.size()};
	}

	std::expected<std::span<const std::byte>, vm::VmError> BytecodeCache::acquire(const ModuleId& id)
	{
		const std::filesystem::path path{id.string()};

		std::error_code ec;
		const auto mtime = std::filesystem::last_write_time(path, ec);
		if (ec)
		{
			return std::unexpected(vm::VmError::internal(std::format("cannot stat module '{}': {}", id.string(), ec.message())));
		}

		if (const auto existing = m_entries.find(id); existing != m_entries.end())
		{
			if (existing->second.mtime == mtime)
			{
				return std::span<const std::byte>{existing->second.bytecode};
			}

			RML_DEBUG("Module '{}' changed on disk, recompiling", id.display());
		}

		auto source = read_source(path);
		if (!source)
		{
			return std::unexpected(std::move(source.error()));
		}

		auto bytecode = compile(*source, id.display());
		if (!bytecode)
		{
			return std::unexpected(std::move(bytecode.error()));
		}

		auto& entry = m_entries[id];
		entry.mtime = mtime;
		entry.bytecode = std::move(*bytecode);

		return std::span<const std::byte>{entry.bytecode};
	}

	void BytecodeCache::invalidate(const ModuleId& id)
	{
		m_entries.erase(id);
	}

	void BytecodeCache::invalidate_under(const std::string_view canonical_root)
	{
		std::erase_if(m_entries, [canonical_root](const auto& entry) {
			return std::string_view{entry.first.string()}.starts_with(canonical_root);
		});
	}

	void BytecodeCache::clear() noexcept
	{
		m_entries.clear();
	}
}
