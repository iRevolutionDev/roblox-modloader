#include "RobloxModLoader/luau/luau_bridge.hpp"

#include "RobloxModLoader/luau/script_host.hpp"
#include "RobloxModLoader/luau/script_runtime.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"

RML_LOG_SCOPE("LuauBridge");

namespace rml::luau
{
	static std::string qualify(const std::string_view mod_name, const std::string_view function_name)
	{
		return std::format("{}.{}", mod_name, function_name);
	}

	EventSubscription::EventSubscription(Bridge* owner, std::string event, const std::size_t id) noexcept
		: m_owner(owner), m_event(std::move(event)), m_id(id)
	{
	}

	EventSubscription::~EventSubscription()
	{
		unsubscribe();
	}

	EventSubscription::EventSubscription(EventSubscription&& other) noexcept
		: m_owner(std::exchange(other.m_owner, nullptr)), m_event(std::move(other.m_event)),
		  m_id(std::exchange(other.m_id, 0))
	{
	}

	EventSubscription& EventSubscription::operator=(EventSubscription&& other) noexcept
	{
		if (this != &other)
		{
			unsubscribe();
			m_owner = std::exchange(other.m_owner, nullptr);
			m_event = std::move(other.m_event);
			m_id = std::exchange(other.m_id, 0);
		}
		return *this;
	}

	void EventSubscription::unsubscribe() noexcept
	{
		if (m_owner)
		{
			m_owner->remove_listener(m_event, m_id);
			m_owner = nullptr;
		}
	}

	Bridge::Bridge(ScriptRuntime& runtime) noexcept
		: m_runtime(&runtime)
	{
	}

	Bridge::~Bridge() = default;

	BridgeResult<void> Bridge::register_function(const std::string_view mod_name, const std::string_view function_name,
	                                             NativeFunction callback)
	{
		if (mod_name.empty() || function_name.empty())
		{
			return std::unexpected("a bridge function needs both a mod name and a function name");
		}

		if (!callback)
		{
			return std::unexpected("a bridge function needs a callable");
		}

		auto key = qualify(mod_name, function_name);

		std::unique_lock lock(m_mutex);
		if (m_functions.contains(key))
		{
			return std::unexpected(std::format("'{}' is already registered", key));
		}

		m_functions.emplace(std::move(key), std::move(callback));
		return {};
	}

	BridgeResult<void> Bridge::unregister_function(const std::string_view mod_name, const std::string_view function_name)
	{
		const auto key = qualify(mod_name, function_name);

		std::unique_lock lock(m_mutex);
		if (m_functions.erase(key) == 0)
		{
			return std::unexpected(std::format("'{}' is not registered", key));
		}

		return {};
	}

	std::optional<NativeFunction> Bridge::find_function(const std::string& qualified_name) const
	{
		std::shared_lock lock(m_mutex);
		const auto it = m_functions.find(qualified_name);
		return it == m_functions.end() ? std::nullopt : std::optional{it->second};
	}

	BridgeResult<void> Bridge::set_shared(const std::string_view key, BridgeValue value)
	{
		if (key.empty())
		{
			return std::unexpected("shared data needs a key");
		}

		std::unique_lock lock(m_mutex);
		m_shared.insert_or_assign(std::string{key}, std::move(value));
		return {};
	}

	BridgeResult<BridgeValue> Bridge::get_shared(const std::string_view key) const
	{
		std::shared_lock lock(m_mutex);
		const auto it = m_shared.find(std::string{key});
		if (it == m_shared.end())
		{
			return std::unexpected(std::format("no shared value under '{}'", key));
		}

		return it->second;
	}

	BridgeResult<EventSubscription> Bridge::listen(const std::string_view event_name, EventHandler handler)
	{
		if (event_name.empty() || !handler)
		{
			return std::unexpected("listening needs an event name and a handler");
		}

		const auto id = m_next_listener.fetch_add(1, std::memory_order_relaxed);

		{
			std::unique_lock lock(m_mutex);
			m_listeners[std::string{event_name}].emplace_back(id, std::move(handler));
		}

		return EventSubscription{this, std::string{event_name}, id};
	}

	BridgeResult<void> Bridge::emit(const std::string_view event_name, const BridgeArgs& args)
	{
		std::vector<EventHandler> snapshot;

		{
			std::shared_lock lock(m_mutex);
			if (const auto it = m_listeners.find(std::string{event_name}); it != m_listeners.end())
			{
				snapshot.reserve(it->second.size());
				for (const auto& listener : it->second)
				{
					snapshot.push_back(listener.handler);
				}
			}
		}

		for (const auto& handler : snapshot)
		{
			try
			{
				handler(args);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("A listener for '{}' threw: {}", event_name, e.what());
			}
		}

		dispatch_to_scripts(event_name, args);
		return {};
	}

	void Bridge::add_script_listener(const RBX::DataModelType context, const std::string_view event_name,
	                                 const RefId callback)
	{
		std::unique_lock lock(m_mutex);
		m_script_listeners[std::string{event_name}].emplace_back(context, callback);
	}

	void Bridge::drop_script_listeners(const RBX::DataModelType context) noexcept
	{
		std::unique_lock lock(m_mutex);

		for (auto& listeners : m_script_listeners | std::views::values)
		{
			std::erase_if(listeners, [context](const ScriptListener& listener) { return listener.context == context; });
		}

		std::erase_if(m_script_listeners, [](const auto& entry) { return entry.second.empty(); });
	}

	void Bridge::drop_script_callbacks(const RBX::DataModelType context, const std::span<const RefId> callbacks) noexcept
	{
		if (callbacks.empty())
		{
			return;
		}

		std::unique_lock lock(m_mutex);

		for (auto& listeners : m_script_listeners | std::views::values)
		{
			std::erase_if(listeners, [context, callbacks](const ScriptListener& listener) {
				return listener.context == context
				    && std::ranges::find(callbacks, listener.callback) != callbacks.end();
			});
		}

		std::erase_if(m_script_listeners, [](const auto& entry) { return entry.second.empty(); });
	}

	void Bridge::dispatch_to_scripts(const std::string_view event_name, const BridgeArgs& args)
	{
		std::vector<ScriptListener> snapshot;

		{
			std::shared_lock lock(m_mutex);
			const auto it = m_script_listeners.find(std::string{event_name});
			if (it == m_script_listeners.end())
			{
				return;
			}
			snapshot = it->second;
		}

		std::vector<Value> flattened;
		flattened.reserve(args.size());
		for (const auto& argument : args)
		{
			flattened.push_back(std::visit(
			    [](const auto& held) -> Value {
				    using Held = std::decay_t<decltype(held)>;
				    if constexpr (std::is_same_v<Held, std::shared_ptr<const BridgeTable>>)
				    {
					    return std::monostate{};
				    }
				    else
				    {
					    return held;
				    }
			    },
			    argument));
		}

		for (const auto& listener : snapshot)
		{
			auto* host = m_runtime->host(listener.context);
			if (!host)
			{
				continue;
			}

			host->dispatcher().post(CallRef{.target = listener.callback, .args = flattened});
		}
	}

	void Bridge::remove_listener(const std::string_view event_name, const std::size_t id) noexcept
	{
		std::unique_lock lock(m_mutex);

		const auto it = m_listeners.find(std::string{event_name});
		if (it == m_listeners.end())
		{
			return;
		}

		std::erase_if(it->second, [id](const Listener& listener) { return listener.id == id; });

		if (it->second.empty())
		{
			m_listeners.erase(it);
		}
	}

	std::vector<std::string> Bridge::registered_mods() const
	{
		std::shared_lock lock(m_mutex);

		std::vector<std::string> mods;
		for (const auto& key : m_functions | std::views::keys)
		{
			auto owner = key.substr(0, key.find('.'));
			if (std::ranges::find(mods, owner) == mods.end())
			{
				mods.push_back(std::move(owner));
			}
		}

		return mods;
	}

	std::vector<std::string> Bridge::registered_functions(const std::string_view mod_name) const
	{
		const auto prefix = std::format("{}.", mod_name);

		std::shared_lock lock(m_mutex);

		std::vector<std::string> names;
		for (const auto& key : m_functions | std::views::keys)
		{
			if (key.starts_with(prefix))
			{
				names.push_back(key.substr(prefix.size()));
			}
		}

		return names;
	}
}
