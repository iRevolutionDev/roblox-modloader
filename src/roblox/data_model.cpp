#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/data_model_job.hpp"

namespace RBX {
    DataModelType DataModel::get_type() const {
        return static_cast<DataModelType>(*reinterpret_cast<int32_t *>(reinterpret_cast<std::uintptr_t>(this) + 0x330));
    }

    bool DataModel::is_initialized() const {
        return *reinterpret_cast<bool *>(
            reinterpret_cast<std::uintptr_t>(this) + 0x5C1);
    }

    DataModel *DataModel::from_job(const DataModelJob *job) {
        if (job == nullptr) {
            return nullptr;
        }

        const auto fake_data_model = job->data_model;
        if (fake_data_model == nullptr) {
            return nullptr;
        }

        // Job has fake data model then I need to offset it to the real data model.
        const auto data_model = reinterpret_cast<uintptr_t>(fake_data_model.get()) + 0x1A0;

        return reinterpret_cast<DataModel *>(data_model);
    }
}
