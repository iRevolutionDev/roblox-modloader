#pragma once

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
    };
}
