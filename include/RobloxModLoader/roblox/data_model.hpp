#pragma once

#include <memory>

#include "instance.hpp"
#include "job_types.hpp"

namespace RBX {
    class DataModelJob;
}

namespace RBX {
    enum class DataModelType : std::int32_t {
        Edit = 0,
        Client = 1,
        Server = 2,
        Standalone = 3,
        Null = 4,
    };

    class DataModel : public Instance {
        char pad[0x591];

    public:
        bool isInitialized;

        DataModelType get_type() const;

        bool is_initialized() const;

        static DataModel *from_job(const DataModelJob *job);
    };
}
