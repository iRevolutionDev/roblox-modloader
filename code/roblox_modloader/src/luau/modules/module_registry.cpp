#include "RobloxModLoader/luau/modules/module_registry.hpp"

#include "RobloxModLoader/luau/vm/chunk.hpp"
#include "RobloxModLoader/luau/vm/stack_guard.hpp"
#include "RobloxModLoader/luau/vm/thread_identity.hpp"

RML_LOG_SCOPE("Modules");

namespace rml::luau
{
	struct LoadingScope final
	{
		LoadingScope(std::vector<ResolvedModule>& loading, const ResolvedModule& module)
			: m_loading(loading)
		{
			m_loading.push_back(module);
		}

		~LoadingScope() { m_loading.pop_back(); }

		LoadingScope(const LoadingScope&) = delete;
		LoadingScope& operator=(const LoadingScope&) = delete;
		LoadingScope(LoadingScope&&) = delete;
		LoadingScope& operator=(LoadingScope&&) = delete;

	private:
		std::vector<ResolvedModule>& m_loading;
	};

	std::expected<void, vm::VmError> ModuleRegistry::require(
	    lua_State* L, const ResolvedModule& module, const ModEnvironment& env)
	{
		if (const auto cached = m_loaded.find(module.id); cached != m_loaded.end())
		{
			vm::StackGuard guard(L);

			if (!cached->second.push(L))
			{
				return std::unexpected(vm::VmError::internal(
				    std::format("module '{}' lost its cached value", module.logical)));
			}

			guard.release();
			return {};
		}

		const auto repeated = std::ranges::find(m_loading, module.id, &ResolvedModule::id);
		if (repeated != m_loading.end())
		{
			return std::unexpected(vm::VmError::internal(describe_cycle(module)));
		}

		return load(L, module, env);
	}

	std::expected<void, vm::VmError> ModuleRegistry::load(
	    lua_State* L, const ResolvedModule& module, const ModEnvironment& env)
	{
		const LoadingScope scope(m_loading, module);

		const auto bytecode = m_bytecode->acquire(module.id);
		if (!bytecode)
		{
			return std::unexpected(bytecode.error());
		}

		vm::StackGuard guard(L);

		if (auto loaded = vm::load_chunk(L, module.logical, *bytecode, RBX::Security::FULL_CAPABILITIES); !loaded)
		{
			return std::unexpected(std::move(loaded.error()));
		}

		vm::set_identity(L, RBX::Security::Permissions::RobloxEngine, RBX::Security::FULL_CAPABILITIES, false);

		const auto called = vm::protected_call(L, 0, 1);
		if (!called)
		{
			return std::unexpected(called.error());
		}

		if (lua_gettop(L) <= guard.top())
		{
			return std::unexpected(vm::VmError::internal(
			    std::format("module '{}' returned nothing, a module must return exactly one value",
			                module.logical)));
		}

		if (lua_isnil(L, -1))
		{
			return std::unexpected(vm::VmError::internal(
			    std::format("module '{}' returned nil, a module must return exactly one value", module.logical)));
		}

		auto ref = vm::Ref::take(L, -1, m_anchor);
		if (!ref.valid())
		{
			return std::unexpected(
			    vm::VmError::internal(std::format("module '{}' could not be anchored", module.logical)));
		}

		m_loaded.insert_or_assign(module.id, std::move(ref));

		guard.release();

		RML_DEBUG("Loaded module '{}' for '{}'", module.logical, env.mod_name());
		return {};
	}

	std::string ModuleRegistry::describe_cycle(const ResolvedModule& repeated) const
	{
		std::string rendered{"module cycle: "};

		for (const auto& entry : m_loading)
		{
			rendered += entry.logical;
			rendered += " -> ";
		}

		rendered += repeated.logical;
		return rendered;
	}

	void ModuleRegistry::invalidate(const ModuleId& id)
	{
		m_loaded.erase(id);
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

		return std::erase_if(m_loaded, [&prefix](const auto& entry) {
			return entry.first.string().starts_with(prefix);
		});
	}

	void ModuleRegistry::clear() noexcept
	{
		m_loaded.clear();
		m_loading.clear();
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
