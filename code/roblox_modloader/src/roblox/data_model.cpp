#include "RobloxModLoader/roblox/data_model.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/data_model_job.hpp"

namespace RBX
{
	DataModelType DataModel::get_type() const
	{
		return m_type;
	}

	bool DataModel::is_initialized() const
	{
		return m_initialized;
	}

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

		constexpr std::uintptr_t fake_job_to_real_data_model_offset = 0x1C0;
		const auto data_model = reinterpret_cast<std::uintptr_t>(fake_data_model.get()) + fake_job_to_real_data_model_offset;

		return reinterpret_cast<DataModel*>(data_model);
	}
}
