#include "RobloxModLoader/luau/script_scheduler.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/luau/script_context.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

namespace rml::luau {
    ScriptScheduler::ScriptScheduler(const SchedulerConfig &config)
        : m_config(config) {
        m_is_running.store(true, std::memory_order_release);
    }

    ScriptScheduler::~ScriptScheduler() noexcept {
        shutdown();
    }

    void ScriptScheduler::shutdown() noexcept {
        try {
            LOG_INFO("Shutting down script scheduler...");

            m_is_shutting_down.store(true, std::memory_order_release);
            m_is_running.store(false, std::memory_order_release);

            m_queue_cv.notify_all();

            cancel_all_scripts();

            LOG_INFO("Script scheduler shutdown complete");
        } catch (const std::exception &e) {
            LOG_ERROR("Exception during script scheduler shutdown: {}", e.what());
        }
    }

    std::future<void> ScriptScheduler::schedule_script(std::unique_ptr<ExecutionContext> context) noexcept {
        auto future = context->completion_promise.get_future();

        try {
            if (!m_is_running.load(std::memory_order_acquire)) {
                context->completion_promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Scheduler is not running"))
                );
                return future;
            } {
                const auto priority_index = static_cast<std::size_t>(context->priority);
                std::lock_guard lock(m_queue_mutex);

                if (get_total_queue_size() >= m_config.max_queue_size) {
                    context->completion_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error("Queue is full"))
                    );
                    return future;
                }

                m_execution_queues[priority_index].push(std::move(context));
            }

            m_queue_cv.notify_one();
        } catch ([[maybe_unused]] const std::exception &e) {
            context->completion_promise.set_exception(std::current_exception());
        }

        return future;
    }

    std::future<void> ScriptScheduler::schedule_script_delayed(
        std::unique_ptr<ExecutionContext> context,
        const std::chrono::milliseconds delay) noexcept {
        context->scheduled_time = std::chrono::steady_clock::now() + delay;
        return schedule_script(std::move(context));
    }

    int ScriptScheduler::yield_script(
        lua_State *L,
        const std::chrono::milliseconds duration,
        std::function<bool()> resume_condition) noexcept {
        try {
            {
                YieldedCoroutine yielded{
                    .thread = L,
                    .yield_time = std::chrono::steady_clock::now(),
                    .yield_duration = duration,
                    .resume_condition = std::move(resume_condition)
                };
                std::lock_guard lock(m_yield_mutex);
                m_yielded_coroutines.push_back(std::move(yielded));
            }

            return lua_yield(L, 0);
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in yield_script: {}", e.what());
            return 0;
        }
    }

    bool ScriptScheduler::resume_script(lua_State *L, std::any resume_data) noexcept {
        try {
            std::lock_guard lock(m_yield_mutex);

            const auto it = std::ranges::find_if(m_yielded_coroutines,
                                                 [L](const YieldedCoroutine &yielded) {
                                                     return yielded.thread == L;
                                                 });

            if (it != m_yielded_coroutines.end()) {
                it->resume_data = std::move(resume_data);

                if (const int result = lua_resume(L, nullptr, 0); result == LUA_OK || result == LUA_YIELD) {
                    if (result == LUA_OK) {
                        m_yielded_coroutines.erase(it);
                    }
                    return true;
                }

                const char *error = lua_tostring(L, -1);
                LOG_ERROR("Error resuming script: {}", error ? error : "unknown error");
                m_yielded_coroutines.erase(it);
                return false;
            }

            return false;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in resume_script: {}", e.what());
            return false;
        }
    }

    bool ScriptScheduler::cancel_script(lua_State *L) noexcept {
        try {
            {
                std::lock_guard lock(m_queue_mutex);

                for (auto &queue: m_execution_queues) {
                    std::queue<std::unique_ptr<ExecutionContext> > new_queue;

                    while (!queue.empty()) {
                        auto context = std::move(queue.front());
                        queue.pop();

                        if (context->L == L) {
                            context->is_cancelled.store(true, std::memory_order_release);
                            context->completion_promise.set_exception(
                                std::make_exception_ptr(std::runtime_error("Script cancelled"))
                            );
                        } else {
                            new_queue.push(std::move(context));
                        }
                    }

                    queue = std::move(new_queue);
                }
            } {
                std::lock_guard lock(m_yield_mutex);

                const auto it = std::ranges::find_if(m_yielded_coroutines,
                                                     [L](const YieldedCoroutine &yielded) {
                                                         return yielded.thread == L;
                                                     });

                if (it != m_yielded_coroutines.end()) {
                    m_yielded_coroutines.erase(it);
                    return true;
                }
            }

            return false;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in cancel_script: {}", e.what());
            return false;
        }
    }

    void ScriptScheduler::cancel_all_scripts() noexcept {
        try {
            {
                std::lock_guard lock(m_queue_mutex);

                for (auto &queue: m_execution_queues) {
                    while (!queue.empty()) {
                        const auto context = std::move(queue.front());
                        queue.pop();

                        context->is_cancelled.store(true, std::memory_order_release);
                        context->completion_promise.set_exception(
                            std::make_exception_ptr(std::runtime_error("All scripts cancelled"))
                        );
                    }
                }
            } {
                std::lock_guard lock(m_yield_mutex);
                m_yielded_coroutines.clear();
            }
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in cancel_all_scripts: {}", e.what());
        }
    }

    bool ScriptScheduler::is_running() const noexcept {
        return m_is_running.load(std::memory_order_acquire);
    }

    std::size_t ScriptScheduler::get_total_queue_size() const noexcept {
        std::size_t total = 0;
        for (const auto &queue: m_execution_queues) {
            total += queue.size();
        }
        return total;
    }

    std::size_t ScriptScheduler::get_executing_script_count() const noexcept {
        return m_currently_executing.load(std::memory_order_relaxed);
    }

    std::size_t ScriptScheduler::get_yielded_script_count() noexcept {
        std::lock_guard lock(m_yield_mutex);
        return m_yielded_coroutines.size();
    }

    ScriptScheduler::Statistics ScriptScheduler::get_statistics() {
        Statistics stats;

        stats.total_executed = m_total_executed.load(std::memory_order_relaxed);
        stats.currently_executing = m_currently_executing.load(std::memory_order_relaxed);
        stats.total_execution_time = m_total_execution_time.load(std::memory_order_relaxed);

        if (stats.total_executed > 0) {
            stats.average_execution_time = stats.total_execution_time / stats.total_executed;
        } {
            std::lock_guard lock(m_queue_mutex);
            for (std::size_t i = 0; i < m_execution_queues.size(); ++i) {
                stats.queue_sizes[i] = m_execution_queues[i].size();
            }
            stats.queued_scripts = get_total_queue_size();
        } {
            std::lock_guard lock(m_yield_mutex);
            stats.yielded_scripts = m_yielded_coroutines.size();
        }

        return stats;
    }

    void ScriptScheduler::set_config(const SchedulerConfig &config) noexcept {
        m_config = config;
        LOG_INFO("Updated scheduler configuration");
    }

    const ScriptScheduler::SchedulerConfig &ScriptScheduler::get_config() const noexcept {
        return m_config;
    }

    bool ScriptScheduler::step() noexcept {
        if (!m_is_running.load(std::memory_order_acquire)) {
            return false;
        }

        try {
            process_yielded_coroutines();

            const auto context = get_next_script();
            if (!context) {
                return false;
            }

            ScriptContext::set_thread_identity(context->L, context->security_level, RBX::Security::FULL_CAPABILITIES);

            execute_script(context);
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in script_scheduler::step: {}", e.what());
            return false;
        }
    }

    void ScriptScheduler::process_yielded_coroutines() noexcept {
        try {
            std::lock_guard lock(m_yield_mutex);

            const auto now = std::chrono::steady_clock::now();

            for (auto it = m_yielded_coroutines.begin(); it != m_yielded_coroutines.end();) {
                bool should_resume = false;

                if (it->yield_duration.count() > 0) {
                    if (const auto elapsed = now - it->yield_time; elapsed >= it->yield_duration) {
                        should_resume = true;
                    }
                }

                if (!should_resume && it->resume_condition) {
                    try {
                        should_resume = it->resume_condition();
                    } catch (const std::exception &e) {
                        LOG_ERROR("Exception in resume condition: {}", e.what());
                        should_resume = true;
                    }
                }

                if (should_resume) {
                    if (const int result = lua_resume(it->thread, nullptr, 0); result == LUA_OK) {
                        it = m_yielded_coroutines.erase(it);
                    } else if (result == LUA_YIELD) {
                        it->yield_time = now;
                        ++it;
                    } else {
                        const char *error = lua_tostring(it->thread, -1);
                        LOG_ERROR("Error in yielded coroutine: {}", error ? error : "unknown error");
                        it = m_yielded_coroutines.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        } catch (const std::exception &e) {
            LOG_ERROR("Exception in process_yielded_coroutines: {}", e.what());
        }
    }

    std::unique_ptr<ScriptScheduler::ExecutionContext> ScriptScheduler::get_next_script() noexcept {
        std::lock_guard lock(m_queue_mutex);

        for (auto &queue: m_execution_queues) {
            if (!queue.empty()) {
                auto context = std::move(queue.front());
                queue.pop();

                if (const auto now = std::chrono::steady_clock::now(); context->scheduled_time > now) {
                    queue.push(std::move(context));
                    continue;
                }

                return context;
            }
        }

        return nullptr;
    }

    void ScriptScheduler::execute_script(const std::unique_ptr<ExecutionContext> &context) noexcept {
        if (!context || !context->L) {
            return;
        }

        try {
            if (context->is_cancelled.load(std::memory_order_acquire)) {
                context->completion_promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Script was cancelled"))
                );
                return;
            }

            m_currently_executing.fetch_add(1, std::memory_order_relaxed);

            const auto start_time = std::chrono::high_resolution_clock::now();

            g_pointers->m_roblox_pointers.task_defer(context->L);

            const auto end_time = std::chrono::high_resolution_clock::now();
            const auto execution_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

            m_total_execution_time.store(execution_time, std::memory_order_relaxed);
            m_total_executed.fetch_add(1, std::memory_order_relaxed);
        } catch ([[maybe_unused]] const std::exception &e) {
            context->completion_promise.set_exception(std::current_exception());
        }

        m_currently_executing.fetch_sub(1, std::memory_order_relaxed);
    }
}
