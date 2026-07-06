#pragma once
#include "RobloxModLoader/common.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <typeindex>

namespace events {
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
        void registerHandler(EventHandler<T> handler) {
            auto wrappedHandler = [handler](EventBase &e) {
                handler(static_cast<T &>(e));
            };
            std::unique_lock lock(mutex_);
            handlers[std::type_index(typeid(T))].push_back(std::move(wrappedHandler));
        }

        template<typename T>
        void emit(T &event) {
            std::vector<HandlerFunc> snapshot;
            {
                std::shared_lock lock(mutex_);
                const auto it = handlers.find(std::type_index(typeid(T)));
                if (it == handlers.end()) {
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
        std::unordered_map<std::type_index, std::vector<HandlerFunc> > handlers;
        std::shared_mutex mutex_;
    };

    inline EventManager *g_event_manager{};
}
