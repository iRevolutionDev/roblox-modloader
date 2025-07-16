#include "RobloxModLoader/luau/script_engine.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/luau/script_context.hpp"
#include "RobloxModLoader/luau/script_scheduler.hpp"

namespace rml::luau {
    ScriptEngine::ScriptEngine(ScriptContext::Context context)
        : m_context(std::make_unique<ScriptContext>(context))
          , m_scheduler(std::make_unique<ScriptScheduler>()) {
    }

    ScriptEngine::~ScriptEngine() noexcept {
        if (!m_is_destroyed.load(std::memory_order_acquire)) {
            shutdown();
        }
    }

    bool ScriptEngine::initialize() noexcept {
        try {
            std::unique_lock lock(m_state_mutex);

            if (m_is_running.load(std::memory_order_acquire)) {
                LOG_WARN("Execution engine is already running");
                return false;
            }

            if (!m_context->initialize()) {
                return false;
            }

            m_is_running.store(true, std::memory_order_release);

            LOG_INFO("Luau execution engine initialized successfully");
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception during execution engine initialization: {}", e.what());
            return false;
        }
    }

    void ScriptEngine::shutdown() noexcept {
        try {
            std::unique_lock lock(m_state_mutex);

            if (m_is_destroyed.load(std::memory_order_acquire)) {
                return;
            }

            LOG_INFO("Shutting down Luau execution engine...");

            m_is_running.store(false, std::memory_order_release);

            if (m_scheduler) {
                m_scheduler->shutdown();
            }

            if (m_context) {
                m_context->shutdown();
            }

            m_is_destroyed.store(true, std::memory_order_release);

            LOG_INFO("Luau execution engine shutdown complete");
        } catch (const std::exception &e) {
            LOG_ERROR("Exception during execution engine shutdown: {}", e.what());
        }
    }

    std::future<ScriptEngine::ExecutionResult> ScriptEngine::execute_script(
        const std::string_view source_code,
        const std::string_view chunk_name,
        const RBX::Security::Permissions security_level) const noexcept {
        return execute_internal(
            [source_code = std::string(source_code)](lua_State *L) -> int {
                constexpr auto compilation_opts = Luau::CompileOptions{
                    .optimizationLevel = 1,
                    .debugLevel = 2,
                };

                const auto bytecode = Luau::compile(source_code, compilation_opts);

                if (bytecode.empty()) {
                    lua_pushstring(L, "Failed to compile source code");
                    return -1;
                }

                return g_pointers->m_roblox_pointers.luau_load(L, "=rml_script", bytecode.data(), bytecode.size(), 0);
            },
            chunk_name,
            security_level
        );
    }

    std::future<ScriptEngine::ExecutionResult> ScriptEngine::execute_bytecode(
        std::span<const std::byte> bytecode,
        const std::string_view chunk_name,
        const RBX::Security::Permissions security_level) const noexcept {
        return execute_internal(
            [bytecode](lua_State *L) -> int {
                return g_pointers->m_roblox_pointers.luau_load(L, "=rml_bytecode",
                                                               reinterpret_cast<const char *>(bytecode.data()),
                                                               bytecode.size(), 0);
            },
            chunk_name,
            security_level
        );
    }

    std::expected<std::vector<std::byte>, std::string> ScriptEngine::compile_script(
        const std::string_view source_code) noexcept {
        try {
            constexpr auto compilation_opts = Luau::CompileOptions{};

            const auto bytecode = Luau::compile(std::string(source_code), compilation_opts);

            if (bytecode.empty()) {
                return std::unexpected("Failed to compile source code");
            }

            std::vector<std::byte> result;
            result.reserve(bytecode.size());

            std::ranges::transform(bytecode, std::back_inserter(result),
                                   [](char c) { return static_cast<std::byte>(c); });

            return result;
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Compilation error: {}", e.what()));
        }
    }

    const ScriptContext &ScriptEngine::get_context() const noexcept {
        return *m_context;
    }

    const ScriptScheduler &ScriptEngine::get_scheduler() const noexcept {
        return *m_scheduler;
    }

    bool ScriptEngine::is_running() const noexcept {
        return m_is_running.load(std::memory_order_acquire);
    }

    bool ScriptEngine::is_destroyed() const noexcept {
        return m_is_destroyed.load(std::memory_order_acquire);
    }

    std::size_t ScriptEngine::get_active_script_count() const noexcept {
        if (!m_scheduler) {
            return 0;
        }
        return m_scheduler->get_executing_script_count();
    }

    ScriptEngine::Statistics ScriptEngine::get_statistics() const noexcept {
        if (!m_scheduler) {
            return {};
        }

        const auto scheduler_stats = m_scheduler->get_statistics();

        return Statistics{
            .total_scripts_executed = scheduler_stats.total_executed,
            .successful_executions = scheduler_stats.total_executed, // TODO: Track failures separately
            .failed_executions = 0,
            .total_execution_time = scheduler_stats.total_execution_time,
            .average_execution_time = scheduler_stats.average_execution_time,
            .memory_usage_bytes = m_context ? m_context->get_memory_usage() : 0
        };
    }

    std::future<ScriptEngine::ExecutionResult> ScriptEngine::execute_internal(
        const std::function<int(lua_State *)> &loader,
        const std::string_view chunk_name,
        RBX::Security::Permissions security_level) const noexcept {
        auto promise = std::make_shared<std::promise<ExecutionResult> >();
        auto future = promise->get_future();

        try {
            if (!is_running()) {
                promise->set_value(ExecutionResult{
                    .success = false,
                    .error_message = "Execution engine is not running"
                });
                return future;
            }

            auto context = std::make_unique<ScriptScheduler::ExecutionContext>();
            context->L = lua_newthread(m_context->get_thread_state());
            context->chunk_name = std::string(chunk_name);
            context->security_level = security_level;
            context->priority = ScriptScheduler::Priority::Normal;
            context->scheduled_time = std::chrono::steady_clock::now();

            ScriptContext::set_thread_identity(context->L, context->security_level, RBX::Security::FULL_CAPABILITIES);

            const auto start_time = std::chrono::high_resolution_clock::now();

            if (const int load_result = loader(context->L); load_result != LUA_OK) {
                const std::string error_msg = lua_tostring(context->L, -1);

                promise->set_value(ExecutionResult{
                    .success = false,
                    .error_message = error_msg
                });
                return future;
            }

            auto completion_future = m_scheduler->schedule_script(std::move(context));

            std::thread([promise, completion_future = std::move(completion_future), start_time]() mutable {
                try {
                    completion_future.wait();
                    const auto end_time = std::chrono::high_resolution_clock::now();

                    promise->set_value(ExecutionResult{
                        .success = true,
                        .execution_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            end_time - start_time)
                    });
                } catch (const std::exception &e) {
                    promise->set_value(ExecutionResult{
                        .success = false,
                        .error_message = e.what()
                    });
                }
            }).detach();
        } catch (const std::exception &e) {
            promise->set_value(ExecutionResult{
                .success = false,
                .error_message = std::format("Internal execution error: {}", e.what())
            });
        }

        return future;
    }
}
