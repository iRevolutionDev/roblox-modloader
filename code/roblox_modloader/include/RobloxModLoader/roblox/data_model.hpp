#pragma once

#include <memory>

#include "instance.hpp"
#include "job_types.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

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

    private:
        RML_LAYOUT_GUARD_BEGIN()
            RML_ASSERT_LAYOUT_SIZE(DataModel, 0x650);
            RML_ASSERT_LAYOUT_OFFSET(DataModel, pad, 0xB8);
            RML_ASSERT_LAYOUT_OFFSET(DataModel, isInitialized, 0x649);
        RML_LAYOUT_GUARD_END()
    };
}
