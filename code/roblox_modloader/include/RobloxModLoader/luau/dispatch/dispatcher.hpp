#pragma once

#include "RobloxModLoader/luau/dispatch/work.hpp"

namespace rml::dotnet
{
	struct InteropVariant;
}

namespace rml::luau
{
	class ScriptHost;

	using ManagedCompletionFn = void(
#if defined(RML_WINDOWS)
	    __cdecl
#endif
	    *)(void* state, const dotnet::InteropVariant* result, const char* error_message);

	struct ManagedCompletion
	{
		ManagedCompletionFn callback{nullptr};
		void* state{nullptr};
	};

	struct Budget
	{
		std::size_t max_items{32};
		std::chrono::microseconds max_time{2000};
	};

	class RML_EXPORT Dispatcher final
	{
	public:
		Dispatcher() = default;
		~Dispatcher();

		Dispatcher(const Dispatcher&) = delete;
		Dispatcher& operator=(const Dispatcher&) = delete;
		Dispatcher(Dispatcher&&) = delete;
		Dispatcher& operator=(Dispatcher&&) = delete;

		void post(Work work);
		[[nodiscard]] std::future<WorkResult> post_for_result(Work work);
		void post_managed(Work work, ManagedCompletion completion);

		void pump(ScriptHost& host, const Budget& budget) noexcept;

		[[nodiscard]] bool has_pending() const noexcept;
		[[nodiscard]] std::size_t pending_count() const noexcept;

		void drain_cancelled(const vm::VmError& reason) noexcept;

	private:
		using Completion = std::variant<std::monostate, std::promise<WorkResult>, ManagedCompletion>;

		struct Entry
		{
			Work work;
			Completion completion;
		};

		void enqueue(Entry entry);
		static void settle(Completion& completion, WorkResult result) noexcept;

		mutable std::mutex m_mutex;
		std::vector<Entry> m_incoming;
		std::vector<Entry> m_draining;
		bool m_closed{false};
	};
}
