#include "RobloxModLoader/hooking/i_hook_engine.hpp"

namespace rml
{
	class DobbyEngine final : public IHookEngine
	{
	public:
		std::expected<void, HookError> create(const std::string& name, void*, void*, void**) override
		{
			return not_implemented(name);
		}

		std::expected<void, HookError> remove(const std::string& name, void*) override
		{
			return not_implemented(name);
		}

		std::expected<void, HookError> queue_enable(const std::string& name, void*) override
		{
			return not_implemented(name);
		}

		std::expected<void, HookError> queue_disable(const std::string& name, void*) override
		{
			return not_implemented(name);
		}

		void apply_queued() override
		{
		}

		void* resolve_thunk(void* target) const override
		{
			return target;
		}

	private:
		static std::expected<void, HookError> not_implemented(const std::string& name)
		{
			return std::unexpected(HookError::from_status(name, 0, "Dobby hook engine not yet implemented"));
		}
	};

	std::unique_ptr<IHookEngine> create_hook_engine()
	{
		return std::make_unique<DobbyEngine>();
	}
}
