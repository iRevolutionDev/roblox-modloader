#pragma once

#include "../isubsystem.hpp"
#include "pointers.hpp"

namespace rml
{
	class PointersSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			try
			{
				m_instance = std::make_unique<Pointers>();
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
			return "Pointers";
		}

	private:
		std::unique_ptr<Pointers> m_instance;
	};
}
