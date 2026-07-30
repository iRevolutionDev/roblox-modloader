#pragma once

#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include "RobloxModLoader/luau/vm/vm_error.hpp"

namespace rml::luau::vm
{
	class Thread final
	{
	public:
		Thread() = default;

		Thread(lua_State* thread, Ref anchor) noexcept
			: m_thread(thread), m_anchor(std::move(anchor))
		{
		}

		Thread(const Thread&) = delete;
		Thread& operator=(const Thread&) = delete;
		Thread(Thread&&) noexcept = default;
		Thread& operator=(Thread&&) noexcept = default;

		[[nodiscard]] static std::expected<Thread, VmError> spawn(lua_State* parent);

		[[nodiscard]] lua_State* get() const noexcept { return m_thread; }
		[[nodiscard]] bool valid() const noexcept { return m_thread != nullptr && m_anchor.valid(); }

		explicit operator bool() const noexcept { return valid(); }

	private:
		lua_State* m_thread{nullptr};
		Ref m_anchor;
	};
}
