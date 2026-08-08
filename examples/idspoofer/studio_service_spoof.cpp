#include "studio_service_spoof.hpp"

#include "descriptor_support.hpp"

#include <RobloxModLoader/internal/engine_abi.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/roblox/reflection/generated/reflection_layout.hpp>

#include <atomic>
#include <cstddef>
#include <mutex>
#include <optional>
#include <string_view>

RML_LOG_SCOPE("IdSpoofer")

namespace idspoofer::studio_service
{
	namespace
	{
		constexpr std::string_view kDescriptorRtti =
		    "BoundFuncDesc@VRESTRICTED_StudioService@RBX@@$$A6A_JXZ";

		struct InstalledHook
		{
			void* descriptor{};
			void** target_slot{};
			void* original_target{};
			std::int32_t bound_this_delta{};
		};

		std::mutex s_hook_mutex;
		std::optional<InstalledHook> s_hook;
		std::atomic<std::int64_t> s_user_id{1585524057LL};
		std::atomic<std::uint64_t> s_reflected_calls{};

		std::int64_t RML_ENGINE_CALL spoof_get_user_id(const void*) noexcept
		{
			const std::uint64_t call = s_reflected_calls.fetch_add(1, std::memory_order_relaxed) + 1;
			const std::int64_t user_id = s_user_id.load(std::memory_order_relaxed);
			if (call == 1)
				RML_INFO("[idspoofer] first reflected StudioService:GetUserId() call -> {}", user_id);
			return user_id;
		}
	}

	bool install(const std::int64_t user_id) noexcept
	{
		using namespace rml::roblox::reflection::layout;

		std::lock_guard lock(s_hook_mutex);
		s_user_id.store(user_id, std::memory_order_relaxed);
		if (s_hook)
		{
			RML_INFO("[idspoofer] StudioService.GetUserId spoof updated -> {}", user_id);
			return true;
		}

		void* descriptor = detail::discover_descriptor(
		    "GetUserId", kDescriptorRtti, function_descriptor_size);
		if (!descriptor)
		{
			RML_ERROR("[idspoofer] StudioService.GetUserId descriptor validation failed; StudioService spoof disabled");
			return false;
		}

		auto** target_slot = reinterpret_cast<void**>(
		    static_cast<std::byte*>(descriptor) + function_invoke_target);
		const auto original_target = detail::read_memory<void*>(target_slot);
		const auto bound_this_delta = detail::read_memory<std::int32_t>(
		    static_cast<std::byte*>(descriptor) + function_bound_this_delta);
		if (!original_target || !detail::memory_has_access(*original_target, 1, true)
		    || !bound_this_delta)
		{
			RML_ERROR("[idspoofer] StudioService.GetUserId member-pointer pair failed validation; StudioService spoof disabled");
			return false;
		}

		if (!detail::atomic_replace_pointer(
		        target_slot, *original_target, reinterpret_cast<void*>(&spoof_get_user_id)))
		{
			RML_ERROR("[idspoofer] StudioService.GetUserId target swap failed; StudioService spoof disabled");
			return false;
		}

		s_hook = InstalledHook{
		    .descriptor = descriptor,
		    .target_slot = target_slot,
		    .original_target = *original_target,
		    .bound_this_delta = *bound_this_delta,
		};

		RML_INFO("[idspoofer] StudioService.GetUserId descriptor={:#x} target={:#x} this-delta={} -> {}",
		    reinterpret_cast<std::uintptr_t>(descriptor),
		    reinterpret_cast<std::uintptr_t>(*original_target),
		    *bound_this_delta, user_id);
		return true;
	}

	void uninstall() noexcept
	{
		std::lock_guard lock(s_hook_mutex);
		if (!s_hook)
			return;

		if (!detail::atomic_replace_pointer(s_hook->target_slot,
		        reinterpret_cast<void*>(&spoof_get_user_id), s_hook->original_target))
		{
			RML_WARN("[idspoofer] StudioService.GetUserId target changed externally; not restoring");
		}
		else
		{
			RML_INFO("[idspoofer] restored StudioService.GetUserId after {} reflected call(s)",
			    s_reflected_calls.load(std::memory_order_relaxed));
		}
		s_hook.reset();
	}
}
