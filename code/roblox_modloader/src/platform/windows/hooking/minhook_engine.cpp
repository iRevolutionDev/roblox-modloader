#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/i_hook_engine.hpp"
#include "RobloxModLoader/memory/handle.hpp"

#include <MinHook.h>

namespace rml
{
	class MinHookEngine final : public IHookEngine
	{
	public:
		MinHookEngine()
		{
			MH_Initialize();
		}

		~MinHookEngine() override
		{
			MH_Uninitialize();
		}

		std::expected<void, HookError> create(const std::string& name, void* target, void* detour, void** original) override
		{
			if (const auto status = MH_CreateHook(target, detour, original); status != MH_OK)
				return std::unexpected(HookError::from_status(name, reinterpret_cast<std::uintptr_t>(target), MH_StatusToString(status)));
			return {};
		}

		std::expected<void, HookError> remove(const std::string& name, void* target) override
		{
			if (const auto status = MH_RemoveHook(target); status != MH_OK)
				return std::unexpected(HookError::from_status(name, reinterpret_cast<std::uintptr_t>(target), MH_StatusToString(status)));
			return {};
		}

		std::expected<void, HookError> queue_enable(const std::string& name, void* target) override
		{
			if (!target)
				return std::unexpected(HookError::from_status(name, 0, MH_StatusToString(MH_ERROR_NOT_CREATED)));

			if (const auto status = MH_QueueEnableHook(target); status != MH_OK)
				return std::unexpected(HookError::from_status(name, reinterpret_cast<std::uintptr_t>(target), MH_StatusToString(status)));
			return {};
		}

		std::expected<void, HookError> queue_disable(const std::string& name, void* target) override
		{
			if (!target)
				return std::unexpected(HookError::from_status(name, 0, MH_StatusToString(MH_ERROR_NOT_CREATED)));

			if (const auto status = MH_QueueDisableHook(target); status != MH_OK)
				return std::unexpected(HookError::from_status(name, reinterpret_cast<std::uintptr_t>(target), MH_StatusToString(status)));
			return {};
		}

		void apply_queued() override
		{
			MH_ApplyQueued();
		}

		void* resolve_thunk(void* target) const override
		{
			auto ptr = memory::handle(target);
			while (ptr.as<uint8_t&>() == 0xE9)
				ptr = ptr.add(1).rip();
			return ptr.as<void*>();
		}
	};

	std::unique_ptr<IHookEngine> create_hook_engine()
	{
		return std::make_unique<MinHookEngine>();
	}
}
