#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/memory/i_rtti_provider.hpp"

namespace rml
{
	class RttiManagerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			try
			{
				m_instance = memory::create_rtti_provider();
			}
			catch (const std::exception& e)
			{
				return std::unexpected(SubsystemError{std::string(name()), e.what()});
			}

			g_rtti_provider = m_instance.get();

			return {};
		}

		void shutdown() override
		{
			g_rtti_provider = nullptr;
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "RttiManager";
		}

	private:
		std::unique_ptr<memory::IRttiProvider> m_instance;
	};
}
