#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/internal/memory/rtti_scanner.hpp"

namespace rml
{
	class RttiManagerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			try
			{
				m_instance = std::make_unique<memory::rtti::RTTIManager>();
			}
			catch (const std::exception& e)
			{
				return std::unexpected(SubsystemError{std::string(name()), e.what()});
			}

			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "RttiManager";
		}

	private:
		std::unique_ptr<memory::rtti::RTTIManager> m_instance;
	};
}
