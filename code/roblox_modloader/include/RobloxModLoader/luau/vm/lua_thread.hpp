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

		bool push(lua_State* L) const { return m_anchor.push(L); }

		void release() noexcept
		{
			m_thread = nullptr;
			m_anchor.release();
		}

		explicit operator bool() const noexcept { return valid(); }

	private:
		lua_State* m_thread{nullptr};
		Ref m_anchor;
	};

	struct ResumeOutcome
	{
		bool suspended{false};
		int results{0};
	};

	[[nodiscard]] std::expected<ResumeOutcome, VmError> resume(lua_State* L, int nargs) noexcept;

	[[nodiscard]] bool is_suspended(const Thread& thread) noexcept;

	bool close_thread(lua_State* host, const Thread& thread) noexcept;
}
