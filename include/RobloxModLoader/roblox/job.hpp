#pragma once
#include "job_types.hpp"

namespace rml {
    class IJob {
    public:
        virtual ~IJob() = default;

        virtual bool should_execute(const JobExecutionContext &context) noexcept = 0;

        virtual void execute(const JobExecutionContext &context) = 0;

        virtual void destroy() noexcept = 0;

        virtual std::string_view get_name() const noexcept = 0;

        virtual JobPriority get_priority() const noexcept = 0;

        virtual JobKind get_target_kind() const noexcept = 0;

        virtual JobState get_state() const noexcept = 0;

        virtual bool is_thread_safe() const noexcept = 0;
    };
}
