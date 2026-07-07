#include "qt_connect.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

#include <cstdint>
#include <utility>

namespace rml::qt::detail
{
	namespace
	{
		using impl_fn = void (*)(int which, void* self, void* receiver, void** args, bool* ret);

		struct FunctionSlot
		{
			int ref;
			impl_fn impl;
			std::function<void(void**)> callable;
		};

		void slot_impl(const int which, void* self, void*, void** args, bool* ret)
		{
			const auto* const slot = static_cast<FunctionSlot*>(self);
			switch (which)
			{
			case 0: delete slot; break;
			case 1:
				if (slot->callable)
					slot->callable(args);
				break;
			case 2:
				if (ret)
					*ret = false;
				break;
			default: break;
			}
		}

		using connect_impl_fn = void* (*)(void* ret_connection, const void* sender, void** signal, const void* receiver, void** slot, void* slot_obj, int type, const int* types, const void* sender_meta);

		struct MemberFnPtr
		{
			void* code;
			std::int64_t adjust;
		};
	}

	bool connect_function(const void* sender, void* signal_addr, const void* sender_meta, std::function<void(void**)> slot)
	{
		if (!sender || !signal_addr || !sender_meta)
			return false;

		static const auto connect = reinterpret_cast<connect_impl_fn>(core_export("?connectImpl@QObject@@CA?AVConnection@QMetaObject@@PEBV1@PEAPEAX01PEAVQSlotObjectBase@QtPrivate@@W4ConnectionType@Qt@@PEBHPEBU3@@Z"));
		if (!connect)
			return false;

		auto* const function = new FunctionSlot{1, &slot_impl, std::move(slot)};

		MemberFnPtr signal_pmf{signal_addr, 0};
		const auto signal = reinterpret_cast<void**>(&signal_pmf);

		alignas(void*) unsigned char connection[16]{};

		connect(connection, sender, signal, sender, nullptr, function, 0, nullptr, sender_meta);
		return true;
	}
}
