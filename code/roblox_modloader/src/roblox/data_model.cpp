#include "RobloxModLoader/roblox/data_model.hpp"

#include "RobloxModLoader/roblox/data_model_job.hpp"

namespace RBX
{
	DataModel* DataModel::from_job(const DataModelJob* job)
	{
		if (job == nullptr)
		{
			return nullptr;
		}

		const auto fake_data_model = job->data_model;
		if (fake_data_model == nullptr)
		{
			return nullptr;
		}

		constexpr std::ptrdiff_t shared_ptr_to_object_start = -0x8;
		const auto data_model = reinterpret_cast<std::uintptr_t>(fake_data_model.get()) + shared_ptr_to_object_start;

		return reinterpret_cast<DataModel*>(data_model);
	}
}
