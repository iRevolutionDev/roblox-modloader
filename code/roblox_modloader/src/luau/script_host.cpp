#include "RobloxModLoader/luau/script_host.hpp"

#include "RobloxModLoader/luau/script_runtime.hpp"
#include "RobloxModLoader/luau/vm/stack_guard.hpp"
#include "RobloxModLoader/luau/vm/thread_identity.hpp"
#include "RobloxModLoader/luau/vm/vm_api.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "pointers.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"

RML_LOG_SCOPE("ScriptHost");

namespace rml::luau
{
	ScriptHost::ScriptHost(ScriptRuntime& runtime, const RBX::DataModelType type, lua_State* global_state)
		: m_runtime(&runtime), m_type(type), m_global(global_state)
	{
	}

	ScriptHost::~ScriptHost()
	{
		shutdown();
	}

	bool ScriptHost::on_owner_thread() const noexcept
	{
		const auto owner = m_owner.load(std::memory_order_acquire);
		return owner == std::thread::id{} || owner == std::this_thread::get_id();
	}

	std::expected<ScriptEnv*, vm::VmError> ScriptHost::env_for(const ModManifestPtr& mod)
	{
		const auto key = mod ? mod->name : std::string{};

		if (const auto it = m_mod_envs.find(key); it != m_mod_envs.end())
		{
			return it->second.get();
		}

		auto thread = vm::Thread::spawn(m_global);
		if (!thread)
		{
			return std::unexpected(thread.error());
		}

		auto* raw = thread->get();
		luaL_sandboxthread(raw);

		vm::set_identity(raw, RBX::Security::Permissions::RobloxEngine, RBX::Security::FULL_CAPABILITIES);

		auto env = std::make_unique<ScriptEnv>(*this, std::move(*thread), m_runtime->environment_for(mod));

		vm::StackGuard guard(raw);
		if (!bind_globals(*env, raw))
		{
			RML_WARN("Some globals failed to bind for mod '{}'", key.empty() ? "<loader>" : key);
		}

		auto [it, _] = m_mod_envs.emplace(key, std::move(env));
		return it->second.get();
	}

	std::expected<void, vm::VmError> ScriptHost::post_script(const ModManifestPtr& mod, const ScriptAsset& asset)
	{
		if (!vm::api_ready())
		{
			vm::report_api_unavailable_once();
			return std::unexpected(vm::VmError::unavailable("the Luau C API is not resolved on this Studio build"));
		}

		auto source = read_source(asset.path);
		if (!source)
		{
			return std::unexpected(vm::VmError::internal(std::move(source.error())));
		}

		const auto chunk_name = asset.chunk_name();

		auto bytecode = BytecodeCache::compile(*source, chunk_name);
		if (!bytecode)
		{
			return std::unexpected(std::move(bytecode.error()));
		}

		m_dispatcher.post(RunChunk{
		    .chunk_name = chunk_name,
		    .bytecode = std::move(*bytecode),
		    .owner = mod,
		    .want_result = false,
		});

		return {};
	}

	void ScriptHost::pump(const Budget& budget) noexcept
	{
		m_owner.store(std::this_thread::get_id(), std::memory_order_release);

		if (const auto pending = m_dispatcher.pending_count(); pending > 0)
		{
			RML_INFO("Pumping {} queued item(s) for DataModel type {}", pending, static_cast<int>(m_type));
		}

		m_dispatcher.pump(*this, budget);
	}

	static std::expected<void, vm::VmError> push_value(lua_State* L, const Value& value, ScriptHost& host)
	{
		return std::visit(
		    [&](const auto& held) -> std::expected<void, vm::VmError> {
			    using Held = std::decay_t<decltype(held)>;

			    if constexpr (std::is_same_v<Held, std::monostate>)
			    {
				    lua_pushnil(L);
			    }
			    else if constexpr (std::is_same_v<Held, bool>)
			    {
				    lua_pushboolean(L, held ? 1 : 0);
			    }
			    else if constexpr (std::is_same_v<Held, double>)
			    {
				    lua_pushnumber(L, held);
			    }
			    else if constexpr (std::is_same_v<Held, std::string>)
			    {
				    lua_pushlstring(L, held.data(), held.size());
			    }
			    else if constexpr (std::is_same_v<Held, LuauRefHandle>)
			    {
				    auto* ref = host.lookup(held.id);
				    if (ref == nullptr)
				    {
					    return std::unexpected(vm::VmError::internal("the Luau handle passed in is no longer live"));
				    }
				    ref->push(L);
			    }
			    else
			    {
				    return std::unexpected(
				        vm::VmError::unavailable("passing an engine Instance into Luau is not wired up yet"));
			    }

			    return {};
		    },
		    value);
	}

	static Value read_value(lua_State* L, const int index, ScriptHost& host)
	{
		switch (lua_type(L, index))
		{
		case LUA_TNIL:
			return Value{};
		case LUA_TBOOLEAN:
			return lua_toboolean(L, index) != 0;
		case LUA_TNUMBER:
			return lua_tonumberx(L, index, nullptr);
		case LUA_TSTRING:
		{
			std::size_t length = 0;
			const auto* text = lua_tolstring(L, index, &length);
			return std::string{text ? text : "", text ? length : 0};
		}
		default:
			break;
		}

		lua_pushvalue(L, index);
		auto ref = vm::Ref::take(L, -1);
		lua_pop(L, 1);

		return LuauRefHandle{host.retain(std::move(ref))};
	}

	static WorkResult run_chunk(ScriptHost& host, RunChunk& chunk)
	{
		auto env = host.env_for(chunk.owner);
		if (!env)
		{
			return std::unexpected(env.error());
		}

		auto thread = vm::Thread::spawn((*env)->thread());
		if (!thread)
		{
			return std::unexpected(thread.error());
		}

		auto* L = thread->get();
		vm::StackGuard guard(L);

		const auto name = std::format("={}", chunk.chunk_name);
		if (g_pointers->m_roblox_pointers.luau_load(L, name.c_str(),
		                                            reinterpret_cast<const char*>(chunk.bytecode.data()),
		                                            chunk.bytecode.size(), 0) != 0)
		{
			return std::unexpected(vm::error_from_stack(L, vm::VmError::Kind::Syntax));
		}

		const auto called = vm::protected_call(L, 0, chunk.want_result ? 1 : 0);
		if (!called)
		{
			return std::unexpected(called.error());
		}

		if (!chunk.want_result || *called < 1)
		{
			return Value{};
		}

		return read_value(L, -1, host);
	}

	static WorkResult call_ref(ScriptHost& host, CallRef& call)
	{
		auto* target = host.lookup(call.target);
		if (target == nullptr)
		{
			return std::unexpected(vm::VmError::internal("the Luau handle being called is no longer live"));
		}

		auto env = host.loader_env();
		if (!env)
		{
			return std::unexpected(env.error());
		}

		auto* L = (*env)->thread();
		vm::StackGuard guard(L);

		target->push(L);

		for (const auto& argument : call.args)
		{
			if (auto pushed = push_value(L, argument, host); !pushed)
			{
				return std::unexpected(std::move(pushed.error()));
			}
		}

		const auto called = vm::protected_call(L, static_cast<int>(call.args.size()), 1);
		if (!called)
		{
			return std::unexpected(called.error());
		}

		return read_value(L, -1, host);
	}

	static WorkResult index_ref(ScriptHost& host, IndexRef& index)
	{
		auto* target = host.lookup(index.target);
		if (target == nullptr)
		{
			return std::unexpected(vm::VmError::internal("the Luau handle being indexed is no longer live"));
		}

		auto env = host.loader_env();
		if (!env)
		{
			return std::unexpected(env.error());
		}

		auto* L = (*env)->thread();
		vm::StackGuard guard(L);

		target->push(L);
		lua_getfield(L, -1, index.key.c_str());

		return read_value(L, -1, host);
	}

	void ScriptHost::execute(Work& work, std::move_only_function<void(WorkResult)> settle) noexcept
	{
		auto result = std::visit(
		    [this](auto& held) -> WorkResult {
			    using Held = std::decay_t<decltype(held)>;

			    if constexpr (std::is_same_v<Held, RunChunk>)
			    {
				    return run_chunk(*this, held);
			    }
			    else if constexpr (std::is_same_v<Held, CallRef>)
			    {
				    return call_ref(*this, held);
			    }
			    else if constexpr (std::is_same_v<Held, IndexRef>)
			    {
				    return index_ref(*this, held);
			    }
			    else
			    {
				    release(held.target);
				    return Value{};
			    }
		    },
		    work);

		settle(std::move(result));
	}

	RefId ScriptHost::retain(vm::Ref ref)
	{
		const auto id = m_runtime->next_ref_id();
		m_refs.emplace(id, std::move(ref));
		m_runtime->map_ref(id, this);
		return id;
	}

	vm::Ref* ScriptHost::lookup(const RefId id) noexcept
	{
		const auto it = m_refs.find(id);
		return it == m_refs.end() ? nullptr : &it->second;
	}

	void ScriptHost::release(const RefId id) noexcept
	{
		m_refs.erase(id);
		m_runtime->unmap_ref(id);
	}

	void ScriptHost::shutdown() noexcept
	{
		if (m_shutdown)
		{
			return;
		}
		m_shutdown = true;

		m_dispatcher.drain_cancelled(vm::VmError::unavailable("the DataModel this script host served went away"));

		m_runtime->unmap_refs_of(this);
		m_refs.clear();
		m_modules.clear();
		m_closures.clear();
		m_mod_envs.clear();
	}
}
