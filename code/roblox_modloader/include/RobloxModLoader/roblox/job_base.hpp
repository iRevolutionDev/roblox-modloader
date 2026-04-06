#pragma once
#include "data_model_job.hpp"
#include "RobloxModLoader/roblox/job.hpp"
#include "RobloxModLoader/common.hpp"

namespace rml::jobs {
    class JobBase : public IJob {
    public:
        explicit JobBase(std::string_view name, JobPriority priority = JobPriority::Normal,
                         JobKind target_kind = JobKind::Custom, bool thread_safe = true) noexcept
            : m_name(name)
              , m_priority(priority)
              , m_target_kind(target_kind)
              , m_thread_safe(thread_safe)
              , m_state(JobState::Running) {
        }

        ~JobBase() override = default;

        JobBase(const JobBase &) = delete;

        JobBase &operator=(const JobBase &) = delete;

        JobBase(JobBase &&) noexcept = delete;

        JobBase &operator=(JobBase &&) noexcept = delete;

        bool should_execute(const JobExecutionContext &context) noexcept override {
            if (m_target_kind != JobKind::Custom && !has_job_kind(m_target_kind, context.kind)) {
                return false;
            }

            return should_execute_impl(context);
        }

        void execute(const JobExecutionContext &context) final {
            try {
                execute_impl(context);
            } catch (const std::exception &e) {
                LOG_ERROR("[JobBase::execute] Exception in job '{}': {}", m_name, e.what());
            } catch (...) {
                LOG_ERROR("[JobBase::execute] Unknown exception in job '{}'", m_name);
            }

            m_state.store(JobState::Finished, std::memory_order_release);
        }

        void destroy() noexcept override {
            m_state.store(JobState::Destroyed, std::memory_order_release);
            destroy_impl();
        }

        std::string_view get_name() const noexcept override {
            return m_name;
        }

        JobPriority get_priority() const noexcept override {
            return m_priority;
        }

        JobKind get_target_kind() const noexcept override {
            return m_target_kind;
        }

        JobState get_state() const noexcept override {
            return m_state.load(std::memory_order_acquire);
        }

        bool is_thread_safe() const noexcept override {
            return m_thread_safe;
        }

    protected:
        virtual bool should_execute_impl(const JobExecutionContext &context) noexcept = 0;

        virtual void execute_impl(const JobExecutionContext &context) = 0;

        virtual void destroy_impl() noexcept = 0;

        void set_priority(JobPriority priority) noexcept {
            m_priority = priority;
        }

        void set_target_kind(JobKind kind) noexcept {
            m_target_kind = kind;
        }

    private:
        const std::string m_name;
        std::atomic<JobPriority> m_priority;
        std::atomic<JobKind> m_target_kind;
        const bool m_thread_safe;
        std::atomic<JobState> m_state;
    };
}
