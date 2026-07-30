#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau::vm
{
	struct ApiAvailability
	{
		bool ready{false};
		std::vector<std::string_view> missing;

		explicit operator bool() const noexcept { return ready; }
	};

	[[nodiscard]] const ApiAvailability& api_availability() noexcept;

	[[nodiscard]] inline bool api_ready() noexcept { return api_availability().ready; }

	void report_api_unavailable_once() noexcept;
}
