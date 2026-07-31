#include "RobloxModLoader/luau/modules/module_registry.hpp"

#include "RobloxModLoader/luau/vm/stack_guard.hpp"

#include "pointers.hpp"

RML_LOG_SCOPE("Modules");

namespace rml::luau
{
	struct LoadingScope final
	{
		LoadingScope(std::vector<ModuleId>& loading, const ModuleId& id)
			: m_loading(loading)
		{
			m_loading.push_back(id);
		}

		~LoadingScope() { m_loading.pop_back(); }

		LoadingScope(const LoadingScope&) = delete;
		LoadingScope& operator=(const LoadingScope&) = delete;
		LoadingScope(LoadingScope&&) = delete;
		LoadingScope& operator=(LoadingScope&&) = delete;

	private:
		std::vector<ModuleId>& m_loading;
	};

	std::expected<void, vm::VmError> ModuleRegistry::require(
	    lua_State* L, const ModuleId& id, const ModEnvironment& env)
	{
		if (const auto cached = m_loaded.find(id); cached != m_loaded.end())
		{
			vm::StackGuard guard(L);

			if (!cached->second.push(L))
			{
				return std::unexpected(vm::VmError::internal(
				    std::format("module '{}' lost its cached value", id.display())));
			}

			guard.release();
			return {};
		}

		if (std::ranges::find(m_loading, id) != m_loading.end())
		{
			return std::unexpected(vm::VmError::internal(describe_cycle(id)));
		}

		return load(L, id, env);
	}

	std::expected<void, vm::VmError> ModuleRegistry::load(
	    lua_State* L, const ModuleId& id, const ModEnvironment& env)
	{
		const LoadingScope scope(m_loading, id);

		const auto bytecode = m_bytecode.acquire(id);
		if (!bytecode)
		{
			return std::unexpected(bytecode.error());
		}

		const auto chunk_name = std::format("={}", id.display());

		vm::StackGuard guard(L);

		const auto status = g_pointers->m_roblox_pointers.luau_load(
		    L, chunk_name.c_str(), reinterpret_cast<const char*>(bytecode->data()), bytecode->size(), 0);

		if (status != 0)
		{
			return std::unexpected(vm::error_from_stack(L, vm::VmError::Kind::Syntax));
		}

		const auto called = vm::protected_call(L, 0, 1);
		if (!called)
		{
			return std::unexpected(called.error());
		}

		if (lua_gettop(L) <= guard.top())
		{
			return std::unexpected(vm::VmError::internal(
			    std::format("module '{}' returned nothing, a module must return exactly one value",
			                id.display())));
		}

		if (lua_isnil(L, -1))
		{
			return std::unexpected(vm::VmError::internal(
			    std::format("module '{}' returned nil, a module must return exactly one value", id.display())));
		}

		auto ref = vm::Ref::take(L, -1);
		if (!ref.valid())
		{
			return std::unexpected(
			    vm::VmError::internal(std::format("module '{}' could not be anchored", id.display())));
		}

		m_loaded.insert_or_assign(id, std::move(ref));

		guard.release();

		RML_DEBUG("Loaded module '{}' for '{}'", id.display(), env.mod_name());
		return {};
	}

	std::string ModuleRegistry::describe_cycle(const ModuleId& repeated) const
	{
		std::string rendered{"module cycle: "};

		for (const auto& entry : m_loading)
		{
			rendered += entry.display();
			rendered += " -> ";
		}

		rendered += repeated.display();
		return rendered;
	}

	void ModuleRegistry::invalidate(const ModuleId& id)
	{
		m_loaded.erase(id);
		m_bytecode.invalidate(id);
	}

	std::size_t ModuleRegistry::invalidate_under(const std::filesystem::path& root)
	{
		std::error_code ec;
		auto canonical = std::filesystem::weakly_canonical(root, ec);
		if (ec)
		{
			canonical = root;
		}

		auto prefix = canonical.generic_string();
		if (!prefix.empty() && prefix.back() != '/')
		{
			prefix += '/';
		}

		const auto dropped = std::erase_if(m_loaded, [&prefix](const auto& entry) {
			return entry.first.string().starts_with(prefix);
		});

		m_bytecode.invalidate_under(prefix);

		return dropped;
	}

	void ModuleRegistry::clear() noexcept
	{
		m_loaded.clear();
		m_loading.clear();
		m_bytecode.clear();
	}

	void ModuleRegistry::release() noexcept
	{
		for (auto& ref : m_loaded | std::views::values)
		{
			ref.release();
		}

		clear();
	}
}
