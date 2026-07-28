#pragma once

#include "RobloxModLoader/luau/generated/luau_layout.hpp"

#include <cstring>

namespace rml::luau::access
{
	using namespace mirror;

	[[nodiscard]] inline LuaState* state(void* thread) { return static_cast<LuaState*>(thread); }
	[[nodiscard]] inline Closure* closure(void* function) { return static_cast<Closure*>(function); }
	[[nodiscard]] inline Proto* proto(void* prototype) { return static_cast<Proto*>(prototype); }
	[[nodiscard]] inline TValue* value(void* slot) { return static_cast<TValue*>(slot); }
	[[nodiscard]] inline CallInfo* frame(void* info) { return static_cast<CallInfo*>(info); }

	[[nodiscard]] inline const Closure* closure(const void* function)
	{
		return static_cast<const Closure*>(function);
	}

	[[nodiscard]] inline const Proto* proto(const void* prototype)
	{
		return static_cast<const Proto*>(prototype);
	}

	[[nodiscard]] inline const TValue* value(const void* slot) { return static_cast<const TValue*>(slot); }

	[[nodiscard]] inline const TValue* value_at(const TValue* first, const int index)
	{
		return reinterpret_cast<const TValue*>(reinterpret_cast<const std::byte*>(first) + index * tvalue_size);
	}

	[[nodiscard]] inline TValue* value_at(TValue* first, const int index)
	{
		return reinterpret_cast<TValue*>(reinterpret_cast<std::byte*>(first) + index * tvalue_size);
	}

	[[nodiscard]] inline CallInfo* frame_at(CallInfo* first, const int index)
	{
		return reinterpret_cast<CallInfo*>(reinterpret_cast<std::byte*>(first) + index * callinfo_size);
	}

	[[nodiscard]] inline std::ptrdiff_t frames_between(const CallInfo* from, const CallInfo* to)
	{
		return (reinterpret_cast<const std::byte*>(from) - reinterpret_cast<const std::byte*>(to)) /
		       static_cast<std::ptrdiff_t>(callinfo_size);
	}

	[[nodiscard]] inline Closure* closure_in(const TValue* slot)
	{
		return *reinterpret_cast<Closure* const*>(&slot->value);
	}

	[[nodiscard]] inline TValue* upvalues_of(const Closure* function)
	{
		return function->isC != 0 ? function->upvals : function->uprefs;
	}

	inline void copy_value(TValue* destination, const TValue* source)
	{
		std::memcpy(destination, source, tvalue_size);
	}

	inline void advance_top(LuaState* thread)
	{
		thread->top = value_at(thread->top, 1);
	}
}
