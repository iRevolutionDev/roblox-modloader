#pragma once

#include "RobloxModLoader/common.hpp"

namespace RBX::Security {
    enum class Permissions : std::uint32_t;
}

namespace rml::luau {
    class ScriptScheduler final {
    public:
        enum class Priority : std::uint8_t {
            Critical = 0,
            High = 1,
            Normal = 2,
            Low = 3,
            Background = 4
        };

        struct ExecutionContext {
            lua_State *L{nullptr};
            lua_State *rL{nullptr};
            std::string chunk_name;
            RBX::Security::Permissions security_level{0};
            Priority priority{Priority::Normal};
            std::chrono::steady_clock::time_point scheduled_time;
            std::chrono::milliseconds timeout{5000};
            std::promise<void> completion_promise;
            std::atomic<bool> is_cancelled{false};
        };

        struct YieldedCoroutine {
            lua_State *thread{nullptr};
            std::chrono::steady_clock::time_point yield_time;
            std::chrono::milliseconds yield_duration{0};
            std::function<bool()> resume_condition;
            std::any resume_data;
        };

        struct SchedulerConfig {
            std::size_t max_worker_threads{std::thread::hardware_concurrency()};
            std::chrono::milliseconds time_slice{10};
            std::chrono::milliseconds idle_sleep{1};
            std::size_t max_queue_size{10000};
            bool enable_load_balancing{true};
            bool enable_priority_boost{true};
        };

    private:
        SchedulerConfig m_config;

        std::array<std::queue<std::unique_ptr<ExecutionContext> >, 5> m_execution_queues;
        std::mutex m_queue_mutex;
        std::condition_variable m_queue_cv;

        std::vector<YieldedCoroutine> m_yielded_coroutines;
        std::mutex m_yield_mutex;

        std::atomic<bool> m_is_running{false};
        std::atomic<bool> m_is_shutting_down{false};

        std::atomic<std::size_t> m_total_executed{0};
        std::atomic<std::size_t> m_currently_executing{0};
        std::atomic<std::chrono::nanoseconds> m_total_execution_time;

    public:
        explicit ScriptScheduler(const SchedulerConfig &config = {});

        ~ScriptScheduler() noexcept;

        ScriptScheduler(const ScriptScheduler &) = delete;

        ScriptScheduler &operator=(const ScriptScheduler &) = delete;

        ScriptScheduler(ScriptScheduler &&) = delete;

        ScriptScheduler &operator=(ScriptScheduler &&) = delete;

        void shutdown() noexcept;

        [[nodiscard]] std::future<void> schedule_script(std::unique_ptr<ExecutionContext> context) noexcept;

        [[nodiscard]] std::future<void> schedule_script_delayed(
            std::unique_ptr<ExecutionContext> context,
            std::chrono::milliseconds delay
        ) noexcept;

        int yield_script(

            lua_State *L,
            std::chrono::milliseconds duration = std::chrono::milliseconds{0},
            std::function<bool()> resume_condition = nullptr
        ) noexcept;

        bool resume_script(lua_State *L, std::any resume_data = {}) noexcept;

        bool cancel_script(lua_State *L) noexcept;

        void cancel_all_scripts() noexcept;

        [[nodiscard]] bool is_running() const noexcept;

        [[nodiscard]] std::size_t get_total_queue_size() const noexcept;

        [[nodiscard]] std::size_t get_executing_script_count() const noexcept;

        [[nodiscard]] std::size_t get_yielded_script_count() noexcept;

        struct Statistics {
            std::size_t total_executed{0};
            std::size_t currently_executing{0};
            std::size_t queued_scripts{0};
            std::size_t yielded_scripts{0};
            std::chrono::nanoseconds total_execution_time{0};
            std::chrono::nanoseconds average_execution_time{0};
            std::array<std::size_t, 5> queue_sizes{};
        };

        [[nodiscard]] Statistics get_statistics();

        void set_config(const SchedulerConfig &config) noexcept;

        [[nodiscard]] const SchedulerConfig &get_config() const noexcept;

        bool step() noexcept;

    private:
        void process_yielded_coroutines() noexcept;

        [[nodiscard]] std::unique_ptr<ExecutionContext> get_next_script() noexcept;

        void execute_script(const std::unique_ptr<ExecutionContext> &context) noexcept;
    };

    int luau_yield(lua_State *L);

    int luau_wait(lua_State *L);

    int luau_spawn(lua_State *L);

    int luau_delay(lua_State *L);
}
