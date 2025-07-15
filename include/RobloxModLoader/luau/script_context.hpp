#pragma once

namespace RBX::Security {
    enum Capabilities : std::uint64_t;
    enum class Permissions : std::uint32_t;
}

struct Closure;
struct Proto;

namespace rml::luau {
    class ScriptContext final {
    public:
        struct Context {
            lua_State *L{nullptr};
            lua_State *rL{nullptr};
        };

    private:
        Context m_context;

        mutable std::shared_mutex m_state_mutex;
        std::atomic<std::size_t> m_memory_usage{0};
        std::atomic<bool> m_is_initialized{false};

    public:
        explicit ScriptContext(Context context);

        ~ScriptContext() noexcept;

        ScriptContext(const ScriptContext &) = delete;

        ScriptContext &operator=(const ScriptContext &) = delete;

        ScriptContext(ScriptContext &&) = delete;

        ScriptContext &operator=(ScriptContext &&) = delete;

        [[nodiscard]] bool initialize() noexcept;

        void shutdown() noexcept;

        [[nodiscard]] lua_State *get_thread_state() const noexcept;

        [[nodiscard]] lua_State *get_main_state() const noexcept;

        Context get_context() const noexcept {
            return m_context;
        }

        static void set_thread_identity(const lua_State *L, RBX::Security::Permissions identity,
                                        std::uint64_t capabilities) noexcept;

        static void elevate_closure(const Closure *closure, std::uint64_t capabilities) noexcept;

        static void set_proto(Proto *proto, std::uint64_t *security) noexcept;

        [[nodiscard]] std::size_t get_memory_usage() const noexcept;

        [[nodiscard]] bool is_initialized() const noexcept;
    };
}
