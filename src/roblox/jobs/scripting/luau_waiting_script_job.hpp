#pragma once
#include "RobloxModLoader/roblox/job_base.hpp"
#include "RobloxModLoader/common.hpp"

namespace rml::jobs {
    class LuauWaitingScriptJob final : public JobBase {
    public:
        LuauWaitingScriptJob() noexcept;

        ~LuauWaitingScriptJob() override = default;

        LuauWaitingScriptJob(const LuauWaitingScriptJob &) = delete;

        LuauWaitingScriptJob &operator=(const LuauWaitingScriptJob &) = delete;

        LuauWaitingScriptJob(LuauWaitingScriptJob &&) noexcept = delete;

        LuauWaitingScriptJob &operator=(LuauWaitingScriptJob &&) noexcept = delete;

    private:
        bool should_execute_impl(const JobExecutionContext &context) noexcept override;

        void execute_impl(const JobExecutionContext &context) override;

        void destroy_impl() noexcept override;

        static constexpr std::string_view JOB_NAME = "LuauWaitingScriptJob";
    };
}
