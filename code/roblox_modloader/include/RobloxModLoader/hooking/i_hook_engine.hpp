#pragma once

#include "RobloxModLoader/hooking/i_hook.hpp"
#include "RobloxModLoader/rml_export.hpp"

#include <expected>
#include <memory>
#include <string>

namespace rml
{
	class RML_EXPORT IHookEngine
	{
	public:
		virtual ~IHookEngine() = default;

		[[nodiscard]] virtual std::expected<void, HookError> create(const std::string& name, void* target, void* detour, void** original) = 0;
		[[nodiscard]] virtual std::expected<void, HookError> remove(const std::string& name, void* target) = 0;

		[[nodiscard]] virtual std::expected<void, HookError> queue_enable(const std::string& name, void* target) = 0;
		[[nodiscard]] virtual std::expected<void, HookError> queue_disable(const std::string& name, void* target) = 0;
		virtual void apply_queued() = 0;

		[[nodiscard]] virtual void* resolve_thunk(void* target) const = 0;
	};

	[[nodiscard]] std::unique_ptr<IHookEngine> create_hook_engine();
}

inline rml::IHookEngine* g_hook_engine{};
