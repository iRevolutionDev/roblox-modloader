#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"

namespace RBX {
    class DataModel;

    struct Stats {
        double now;
        double last_step;
        double delta_time;
    };

    struct Error {
        double error;
    };

    class TaskSchedulerJob {
    protected:
        virtual ~TaskSchedulerJob() = default;

        virtual void unknown1() = 0;

        virtual void unknown2() = 0;

        virtual void unknown3() = 0;

        virtual void unknown4() = 0;

    public:
        std::shared_ptr<TaskSchedulerJob> self;
        std::string &name;
        uint64_t thread_id;
        uint64_t start_time;
        uint64_t end_time;
        std::shared_ptr<DataModel> data_model;

        virtual void destroy(bool delete_after) = 0;

    private:
        RML_LAYOUT_GUARD_BEGIN()
            RML_ASSERT_LAYOUT_SIZE(TaskSchedulerJob, 0x48);
            RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, self, 0x8);
            RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, thread_id, 0x20);
            RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, start_time, 0x28);
            RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, end_time, 0x30);
            RML_ASSERT_LAYOUT_OFFSET(TaskSchedulerJob, data_model, 0x38);
        RML_LAYOUT_GUARD_END()
    };
}
