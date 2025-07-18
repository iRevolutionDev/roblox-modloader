#pragma once

#include "RobloxModLoader/common.hpp"
#include "script_context.hpp"
#include "script_scheduler.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

namespace rml::luau {
    class ScriptEngine final {
    public:
        enum class ExecutionMode : std::uint8_t {
            Synchronous,
            Asynchronous,
            Deferred
        };

        struct Context {
            lua_State *L = nullptr;
        };

        struct ExecutionResult {
            bool success{false};
            std::string error_message;
            std::vector<std::any> return_values;
            std::chrono::nanoseconds execution_time{0};
        };

    private:
        std::unique_ptr<ScriptContext> m_context;
        std::unique_ptr<ScriptScheduler> m_scheduler;
        std::atomic<bool> m_is_running{false};
        std::atomic<bool> m_is_destroyed{false};

        mutable std::shared_mutex m_state_mutex;

    public:
        explicit ScriptEngine(ScriptContext::Context context);

        ~ScriptEngine() noexcept;

        [[nodiscard]] bool initialize() noexcept;

        void shutdown() noexcept;

        [[nodiscard]] std::future<ExecutionResult> execute_script(
            std::string_view source_code,
            std::string_view chunk_name = "=rml_script",
            RBX::Security::Permissions security_level = RBX::Security::Permissions::RobloxEngine
        ) const noexcept;

        [[nodiscard]] std::future<ExecutionResult> execute_script_with_context(
            std::string_view source_code,
            std::string_view chunk_name,
            const std::string &mod_name,
            const std::string &mod_version,
            const std::string &mod_description,
            const std::string &mod_author,
            const std::filesystem::path &mod_path,
            const std::vector<std::string> &mod_dependencies,
            RBX::Security::Permissions security_level = RBX::Security::Permissions::RobloxEngine
        ) const noexcept;

        [[nodiscard]] std::future<ExecutionResult> execute_bytecode(
            std::span<const std::byte> bytecode,
            std::string_view chunk_name = "=rml_bytecode",
            RBX::Security::Permissions security_level = RBX::Security::Permissions::RobloxEngine
        ) const noexcept;

        [[nodiscard]] static std::expected<std::vector<std::byte>, std::string> compile_script(
            std::string_view source_code
        ) noexcept;

        [[nodiscard]] const ScriptContext &get_context() const noexcept;

        [[nodiscard]] const ScriptScheduler &get_scheduler() const noexcept;

        [[nodiscard]] bool is_running() const noexcept;

        [[nodiscard]] bool is_destroyed() const noexcept;

        [[nodiscard]] std::size_t get_active_script_count() const noexcept;

        struct Statistics {
            std::size_t total_scripts_executed{0};
            std::size_t successful_executions{0};
            std::size_t failed_executions{0};
            std::chrono::nanoseconds total_execution_time{0};
            std::chrono::nanoseconds average_execution_time{0};
            std::size_t memory_usage_bytes{0};
        };

        [[nodiscard]] Statistics get_statistics() const noexcept;

    private:
        [[nodiscard]] std::future<ExecutionResult> execute_internal(
            const std::function<int(lua_State *)> &loader,
            std::string_view chunk_name,
            RBX::Security::Permissions security_level
        ) const noexcept;

        [[nodiscard]] std::future<ExecutionResult> execute_internal_with_context(
            const std::function<int(lua_State *)> &loader,
            std::string_view chunk_name,
            const std::string &mod_name,
            const std::string &mod_version,
            const std::string &mod_description,
            const std::string &mod_author,
            const std::filesystem::path &mod_path,
            const std::vector<std::string> &mod_dependencies,
            RBX::Security::Permissions security_level
        ) const noexcept;
    };
}
