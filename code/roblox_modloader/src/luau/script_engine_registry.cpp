#include "RobloxModLoader/luau/script_engine_registry.hpp"

#include "RobloxModLoader/roblox/data_model.hpp"
#include "lstate.h"

RML_LOG_SCOPE("ScriptEngineRegistry");

namespace rml::luau
{
	ScriptEngineRegistry::~ScriptEngineRegistry()
	{
		shutdown();
	}

	std::shared_ptr<ScriptEngine> ScriptEngineRegistry::get_script_engine(const RBX::DataModelType data_model_type)
	{
		std::shared_lock lock(m_script_engines_mutex);

		if (const auto it = m_script_engines.find(data_model_type); it != m_script_engines.end())
		{
			return it->second;
		}

		return nullptr;
	}

	std::shared_ptr<ScriptEngine> ScriptEngineRegistry::get_script_engine(lua_State* thread_state)
	{
		std::shared_lock lock(m_script_engines_mutex);

		if (!thread_state)
		{
			return nullptr;
		}

		for (const auto& engine : m_script_engines | std::views::values)
		{
			if (engine && engine->get_context().get_thread_state()->global == thread_state->global)
			{
				return engine;
			}
		}

		return nullptr;
	}

	void ScriptEngineRegistry::cleanup_script_engine(const RBX::DataModelType data_model_type)
	{
		RML_INFO("Cleaning up ScriptEngine for DataModel type: {}", static_cast<int>(data_model_type));

		std::shared_ptr<ScriptEngine> engine_to_cleanup;
		{
			std::unique_lock lock(m_script_engines_mutex);
			if (const auto it = m_script_engines.find(data_model_type); it != m_script_engines.end())
			{
				engine_to_cleanup = it->second;
				m_script_engines.erase(it);
			}
		}

		if (!engine_to_cleanup)
			return;

		auto completed = std::make_shared<std::atomic<bool> >(false);
		std::jthread cleanup_thread([engine = std::move(engine_to_cleanup), data_model_type, completed]() {
			try
			{
				RML_INFO("Shutting down ScriptEngine for DataModel type: {}", static_cast<int>(data_model_type));
				engine->shutdown();
				RML_INFO("ScriptEngine shutdown completed for DataModel type: {}", static_cast<int>(data_model_type));
			}
			catch (const std::exception& e)
			{
				RML_ERROR("Error shutting down ScriptEngine for DataModel type {}: {}", static_cast<int>(data_model_type), e.what());
			}

			completed->store(true, std::memory_order_release);
		});

		std::lock_guard threads_lock(m_cleanup_threads_mutex);
		prune_finished_cleanup_threads();
		m_cleanup_threads.push_back(TrackedCleanup{std::move(cleanup_thread), std::move(completed)});
	}

	void ScriptEngineRegistry::cleanup_orphaned_script_engines(const DataModelLookup& data_model_lookup)
	{
		std::vector<RBX::DataModelType> orphaned_types;
		{
			std::shared_lock lock(m_script_engines_mutex);

			for (const auto& data_model_type : m_script_engines | std::views::keys)
			{
				if (!data_model_lookup(data_model_type))
				{
					orphaned_types.push_back(data_model_type);
				}
			}
		}

		for (const auto& orphaned_type : orphaned_types)
		{
			RML_WARN("Found orphaned ScriptEngine for DataModel type: {}, cleaning up", static_cast<int>(orphaned_type));
			cleanup_script_engine(orphaned_type);
		}
	}

	void ScriptEngineRegistry::maybe_cleanup_orphaned_script_engines(const DataModelLookup& data_model_lookup)
	{
		if (m_step_counter.fetch_add(1, std::memory_order_relaxed) % 500 == 0)
		{
			cleanup_orphaned_script_engines(data_model_lookup);
		}
	}

	void ScriptEngineRegistry::shutdown() noexcept
	{
		{
			std::unique_lock lock(m_script_engines_mutex);

			RML_INFO("Shutting down all ScriptEngines...");

			for (auto& engine : m_script_engines | std::views::values)
			{
				if (engine)
				{
					engine->shutdown();
				}
			}

			m_script_engines.clear();

			RML_INFO("All ScriptEngines shut down successfully.");
		}

		std::lock_guard threads_lock(m_cleanup_threads_mutex);
		m_cleanup_threads.clear();
	}

	void ScriptEngineRegistry::prune_finished_cleanup_threads()
	{
		std::erase_if(m_cleanup_threads, [](TrackedCleanup& tracked) {
			return tracked.completed->load(std::memory_order_acquire);
		});
	}
}
