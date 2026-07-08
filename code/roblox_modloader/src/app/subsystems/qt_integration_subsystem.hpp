#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/qt/qt_integration.hpp"

namespace rml
{
	class QtIntegrationSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<qt::QtIntegration>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "QtIntegration";
		}

	private:
		std::unique_ptr<qt::QtIntegration> m_instance;
	};
}
