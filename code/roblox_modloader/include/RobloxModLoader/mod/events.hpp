#pragma once
#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <typeindex>

namespace rml::events {
    struct EventBase {
        virtual ~EventBase() = default;

        bool cancelled = false;
    };

    struct AuthenticationEvent final : EventBase {
        uint64_t *thisPtr;
        uint64_t docPanelProvider;
        uint64_t qImageProvider;

        AuthenticationEvent(uint64_t *_this, const uint64_t doc_panel, const uint64_t q_image)
            : thisPtr(_this), docPanelProvider(doc_panel), qImageProvider(q_image) {
        }
    };

    struct DataModelChangedEvent final : EventBase {
        uint64_t old_data_model;
        uint64_t new_data_model;
        int data_model_type;

        DataModelChangedEvent(uint64_t old_dm, uint64_t new_dm, int type)
            : old_data_model(old_dm), new_data_model(new_dm), data_model_type(type) {}
    };

    class EventManager {
    public:
        EventManager();
        ~EventManager();

        template<typename T>
        using EventHandler = std::function<void(T &)>;

        template<typename T>
        void register_handler(EventHandler<T> handler) {
            auto wrapped_handler = [handler](EventBase &e) {
                handler(static_cast<T &>(e));
            };
            std::unique_lock lock(m_mutex);
            m_handlers[std::type_index(typeid(T))].push_back(std::move(wrapped_handler));
        }

        template<typename T>
        void emit(T &event) {
            std::vector<HandlerFunc> snapshot;
            {
                std::shared_lock lock(m_mutex);
                const auto it = m_handlers.find(std::type_index(typeid(T)));
                if (it == m_handlers.end()) {
                    return;
                }
                snapshot = it->second;
            }

            for (const auto &handler: snapshot) {
                if (event.cancelled) {
                    break;
                }
                handler(event);
            }
        }

    private:
        using HandlerFunc = std::function<void(EventBase &)>;
        std::unordered_map<std::type_index, std::vector<HandlerFunc> > m_handlers;
        std::shared_mutex m_mutex;
    };

    [[nodiscard]] RML_EXPORT EventManager &event_manager();
}
