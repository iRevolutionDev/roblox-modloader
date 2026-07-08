#pragma once
#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/luau/script_engine.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace RBX {
    enum class DataModelType;
    class DataModel;
}

namespace rml::luau {
    class ScriptEngineRegistry final {
    public:
        using DataModelLookup = std::function<const RBX::DataModel *(RBX::DataModelType)>;

        ScriptEngineRegistry() = default;

        ~ScriptEngineRegistry();

        ScriptEngineRegistry(const ScriptEngineRegistry &) = delete;

        ScriptEngineRegistry &operator=(const ScriptEngineRegistry &) = delete;

        ScriptEngineRegistry(ScriptEngineRegistry &&) noexcept = delete;

        ScriptEngineRegistry &operator=(ScriptEngineRegistry &&) noexcept = delete;

        std::shared_ptr<ScriptEngine> get_script_engine(RBX::DataModelType data_model_type);

        std::shared_ptr<ScriptEngine> get_script_engine(lua_State *thread_state);

        void cleanup_script_engine(RBX::DataModelType data_model_type);

        void cleanup_orphaned_script_engines(const DataModelLookup &data_model_lookup);

        void maybe_cleanup_orphaned_script_engines(const DataModelLookup &data_model_lookup);

        void shutdown() noexcept;

    private:
        struct TrackedCleanup {
            std::jthread thread;
            std::shared_ptr<std::atomic<bool> > completed;
        };

        void prune_finished_cleanup_threads();

        mutable std::shared_mutex m_script_engines_mutex;
        std::unordered_map<RBX::DataModelType, std::shared_ptr<ScriptEngine> > m_script_engines;

        std::mutex m_cleanup_threads_mutex;
        std::vector<TrackedCleanup> m_cleanup_threads;

        std::atomic<std::uint64_t> m_step_counter{0};
    };
}
