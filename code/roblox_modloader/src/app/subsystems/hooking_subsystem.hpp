#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

namespace rml
{
	class HookingSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<Hooking>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Hooking";
		}

	private:
		std::unique_ptr<Hooking> m_instance;
	};
}
