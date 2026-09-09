#pragma once

#include "RobloxModLoader/hooking/vtable_index.hpp"
#include "RobloxModLoader/roblox/data_model_job.hpp"

#include <cstddef>

namespace rml
{
	inline std::size_t job_step_slot()
	{
		static const std::size_t slot =
		    vtable_index_of(&RBX::DataModelJob::step_data_model_job, RBX::Stats{});
		return slot;
	}
}
