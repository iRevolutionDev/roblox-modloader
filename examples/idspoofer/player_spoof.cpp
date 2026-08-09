#include "player_spoof.hpp"

#include "descriptor_support.hpp"

#include <RobloxModLoader/internal/engine_abi.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/roblox/reflection/generated/reflection_layout.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

RML_LOG_SCOPE("IdSpoofer")

namespace idspoofer::player
{
	namespace
	{
		using Getter = std::int64_t(RML_ENGINE_CALL*)(const void* accessor, const void* player);

		constexpr std::string_view kDescriptorRtti =
		    "PropDescriptor@VPlayer@RBX@@_J@Reflection@RBX@@";
		constexpr std::string_view kGetterRtti =
		    "GetSetImpl@P8PlayerProp@RBX@@EBA_JXZP812@EAAX_J@Z";
		constexpr std::size_t kMaxAccessorVtableEntries =
		    rml::roblox::reflection::layout::typed_get_set_vtable_entries;

		struct AccessorVtableClone
		{
			void* rtti;
			std::array<void*, kMaxAccessorVtableEntries> slots;
		};

		struct AccessorHook
		{
			void* descriptor{};
			void* accessor{};
			void** original_vtable{};
			Getter original_get{};
			std::size_t vtable_entries{};
			std::string label;
			AccessorVtableClone clone{};

			[[nodiscard]] void** cloned_vtable() noexcept { return clone.slots.data(); }
		};

		std::mutex s_hook_mutex;
		std::array<std::unique_ptr<AccessorHook>, 2> s_hooks;
		std::atomic<std::int64_t> s_user_id{1585524057LL};
		std::atomic<std::uint64_t> s_reflected_reads{};

		std::int64_t RML_ENGINE_CALL spoof_get(const void*, const void*) noexcept
		{
			const std::uint64_t read = s_reflected_reads.fetch_add(1, std::memory_order_relaxed) + 1;
			const std::int64_t user_id = s_user_id.load(std::memory_order_relaxed);
			if (read == 1)
				RML_INFO("[idspoofer] first reflected Player.UserId read -> {}", user_id);
			return user_id;
		}
	}

	bool install(const std::int64_t user_id) noexcept
	{
		using namespace rml::roblox::reflection::layout;

		std::lock_guard lock(s_hook_mutex);
		s_user_id.store(user_id, std::memory_order_relaxed);
		if (s_hooks[0])
		{
			RML_INFO("[idspoofer] Player.UserId spoof updated -> {}", user_id);
			return true;
		}

		void* descriptor = detail::discover_descriptor(
		    "UserId", kDescriptorRtti, typed_property_variant_accessor + sizeof(void*));
		if (!descriptor)
		{
			RML_ERROR("[idspoofer] Player.UserId descriptor validation failed; player spoof disabled");
			return false;
		}

		const auto get_set = detail::read_memory<void*>(
		    static_cast<std::byte*>(descriptor) + typed_property_get_set);
		if (!get_set || !detail::has_rtti(*get_set, kGetterRtti))
		{
			RML_ERROR("[idspoofer] Player.UserId GetSet accessor failed RTTI validation; player spoof disabled");
			return false;
		}

		const auto variant_accessor = detail::read_memory<void*>(
		    static_cast<std::byte*>(descriptor) + typed_property_variant_accessor);
		if (!variant_accessor || (*variant_accessor && !detail::is_engine_object(*variant_accessor)))
		{
			RML_ERROR("[idspoofer] Player.UserId VariantAccessor failed validation; player spoof disabled");
			return false;
		}

		auto make_replacement = [descriptor](void* accessor, const std::size_t entries,
		                            const std::array<std::size_t, 2>& getter_slots,
		                            const std::size_t getter_slot_count, std::string label)
		{
			const auto original_vtable = detail::read_memory<void**>(accessor);
			if (!original_vtable || entries > kMaxAccessorVtableEntries
			    || !detail::memory_has_access(*original_vtable - 1,
			        sizeof(void*) * (entries + 1)))
				return std::unique_ptr<AccessorHook>{};

			for (std::size_t slot = 0; slot != entries; ++slot)
			{
				if (!detail::memory_has_access((*original_vtable)[slot], 1, true))
					return std::unique_ptr<AccessorHook>{};
			}

			auto replacement = std::make_unique<AccessorHook>();
			replacement->descriptor = descriptor;
			replacement->accessor = accessor;
			replacement->original_vtable = *original_vtable;
			replacement->original_get = reinterpret_cast<Getter>((*original_vtable)[getter_slots[0]]);
			replacement->vtable_entries = entries;
			replacement->label = std::move(label);
			replacement->clone.rtti = (*original_vtable)[-1];
			std::copy_n(*original_vtable, entries, replacement->clone.slots.begin());
			for (std::size_t index = 0; index != getter_slot_count; ++index)
				replacement->clone.slots[getter_slots[index]] = reinterpret_cast<void*>(&spoof_get);
			return replacement;
		};

		auto get_set_replacement = make_replacement(*get_set, typed_get_set_vtable_entries,
		    {typed_get_set_get_vtable_slot, 0}, 1, "GetSet");
		std::unique_ptr<AccessorHook> variant_replacement;
		if (*variant_accessor)
		{
			variant_replacement = make_replacement(*variant_accessor,
			    typed_variant_accessor_vtable_entries,
			    {typed_variant_accessor_get_vtable_slot,
			        typed_variant_accessor_const_get_vtable_slot},
			    2, "VariantAccessor");
		}

		if (!get_set_replacement || (*variant_accessor && !variant_replacement))
		{
			RML_ERROR("[idspoofer] a Player.UserId accessor vtable failed validation; player spoof disabled");
			return false;
		}

		s_hooks[0] = std::move(get_set_replacement);
		if (!detail::atomic_replace_pointer(
		        s_hooks[0]->accessor, s_hooks[0]->original_vtable, s_hooks[0]->cloned_vtable()))
		{
			s_hooks[0].reset();
			RML_ERROR("[idspoofer] Player.UserId GetSet vtable swap failed; player spoof disabled");
			return false;
		}

		if (variant_replacement)
		{
			s_hooks[1] = std::move(variant_replacement);
			if (!detail::atomic_replace_pointer(
			        s_hooks[1]->accessor, s_hooks[1]->original_vtable, s_hooks[1]->cloned_vtable()))
			{
				(void)detail::atomic_replace_pointer(
				    s_hooks[0]->accessor, s_hooks[0]->cloned_vtable(), s_hooks[0]->original_vtable);
				s_hooks[0].reset();
				s_hooks[1].reset();
				RML_ERROR("[idspoofer] Player.UserId VariantAccessor swap failed; restored GetSet");
				return false;
			}
		}

		RML_INFO("[idspoofer] Player.UserId descriptor={:#x} GetSet={:#x} VariantAccessor={:#x} -> {}",
		    reinterpret_cast<std::uintptr_t>(descriptor),
		    reinterpret_cast<std::uintptr_t>(s_hooks[0]->accessor),
		    reinterpret_cast<std::uintptr_t>(s_hooks[1] ? s_hooks[1]->accessor : nullptr),
		    user_id);
		return true;
	}

	void uninstall() noexcept
	{
		std::lock_guard lock(s_hook_mutex);
		bool restored_any = false;
		for (auto& installed : s_hooks)
		{
			if (!installed)
				continue;

			if (!detail::atomic_replace_pointer(
			        installed->accessor, installed->cloned_vtable(), installed->original_vtable))
			{
				RML_WARN("[idspoofer] Player.UserId {} vtable changed externally; not restoring",
				    installed->label);
			}
			else
			{
				restored_any = true;
			}
			installed.reset();
		}

		if (restored_any)
		{
			RML_INFO("[idspoofer] restored Player.UserId accessors after {} reflected read(s)",
			    s_reflected_reads.load(std::memory_order_relaxed));
		}
	}
}
