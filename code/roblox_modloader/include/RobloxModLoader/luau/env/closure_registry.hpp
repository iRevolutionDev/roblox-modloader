#pragma once

#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include <unordered_map>

struct Closure;

namespace rml::luau
{
	enum class FunctionKind : std::uint8_t
	{
		CClosure = 0,
		LuauClosure = 1,
		NewCClosure = 2
	};

	struct HookRecord
	{
		vm::Ref original;
		vm::Ref replacement;
		FunctionKind original_kind{FunctionKind::LuauClosure};
		FunctionKind replacement_kind{FunctionKind::LuauClosure};
	};

	class ClosureRegistry final
	{
	public:
		ClosureRegistry() = default;

		ClosureRegistry(const ClosureRegistry&) = delete;
		ClosureRegistry& operator=(const ClosureRegistry&) = delete;
		ClosureRegistry(ClosureRegistry&&) = delete;
		ClosureRegistry& operator=(ClosureRegistry&&) = delete;

		void register_hook(Closure* target, HookRecord record);
		void unregister_hook(Closure* target);
		[[nodiscard]] bool is_hooked(Closure* target) const noexcept;
		[[nodiscard]] const HookRecord* hook_for(Closure* target) const noexcept;

		void register_wrapper(Closure* wrapper, vm::Ref original);
		void unregister_wrapper(Closure* wrapper);
		[[nodiscard]] bool is_wrapper(Closure* wrapper) const noexcept;
		[[nodiscard]] const vm::Ref* wrapped_original(Closure* wrapper) const noexcept;

		void protect(Closure* closure);
		[[nodiscard]] bool is_protected(Closure* closure) const noexcept;

		void adopt(Closure* closure);
		[[nodiscard]] bool is_ours(Closure* closure) const noexcept;

		void clear() noexcept;

	private:
		std::unordered_map<Closure*, HookRecord> m_hooks;
		std::unordered_map<Closure*, vm::Ref> m_wrappers;
		std::unordered_set<Closure*> m_protected;
		std::unordered_set<Closure*> m_ours;
	};
}
